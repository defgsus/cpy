import os, inspect
from importlib.machinery import SourceFileLoader
from .context import Context, Function, Class
from .gccxml import ParseError

def _get_name(obj):
    try:
        return obj.__name__
    except AttributeError:
        pass
    if isinstance(obj, property):
        try:
            return obj.fget.__name__
        except AttributeError:
            raise ParseError("Can not retrieve name of property %s" % str(obj))
    raise ParseError("Unhandled object %s" % str(obj))


class _Exporter:
    """
    Class responsible to traverse a module and it's members
    and to generate an ExportContext instance from it
    """
    def __init__(self):
        self.scope_stack = []
        self.context = None

    def log(self, str):
        if 1:
            print(" " * len(self.scope_stack) + self.full_scope_name() + ": " + str)

    def scope_name(self):
        return self.scope_stack[-1] if self.scope_stack else ""

    def full_scope_name(self, name = None):
        r = ""
        for i in self.scope_stack:
            r += i + "."
        if not name:
            r = r[:-1]
        else:
            r += name
        return r

    def push_scope(self, name):
        self.scope_stack.append(name)

    def pop_scope(self):
        self.scope_stack.pop()

    def inspect_module(self, module):
        mod_name = module.__name__
        self.context = Context()
        self.log("inspect module '%s'" % mod_name)
        self.push_scope(mod_name)
        self._inspect_names(module, dir(module))
        self.pop_scope()
        self._resolve_classes()
        self.context.finalize()

    def _inspect_names(self, parent, names):
        for name in names:
            try:
                #print(self.full_scope_name(name))
                o = eval("parent.%s" % name)
            except BaseException as e:
                self.log("EXCEPTION %s\nskipping %s" % (e, self.full_scope_name(name)))
                continue
            self._inspect_entity(o)

    def _inspect_entity(self, o):
        if inspect.ismodule(o):
            self.inspect_module(o)
        elif inspect.isfunction(o):
            self._inspect_function(o)
        elif inspect.isclass(o):
            self._inspect_class(o)
        elif inspect.isgetsetdescriptor(o):
            print("GETSET %s" % o)

    def _inspect_function(self, funcobj, class_obj=None, pycls=None):
        func = funcobj
        isprob = False
        if isinstance(funcobj, property):
            func = funcobj.fget
            isprob = True
        name = _get_name(func)
        self.log("inspecting function '%s'" % name)
        self.push_scope(name)
        if not isprob:
            o = Function.from_python(func, pycls)
            self._add_function(o, class_obj)
            self.pop_scope()
            return
        o = Function.from_python(func, pycls, True, False)
        o.c_name += "__getter"
        self._add_function(o, class_obj)
        if funcobj.fset:
            o = Function.from_python(funcobj.fset, pycls, True, True)
            o.c_name += "__setter"
            self._add_function(o, class_obj)
        self.pop_scope()

    def _add_function(self, o, class_obj):
        if not class_obj:
            self.context.functions.append(o)
        else:
            class_obj.methods.append(o)

    def _inspect_class(self, cls):
        name = _get_name(cls)
        self.log("inspecting class '%s'" % name)
        self.push_scope(name)

        class_obj = Class.from_python(cls)
        for n, mem in inspect.getmembers(cls):
            foo = None
            isprop = False
            if inspect.isfunction(mem):
                foo = mem
            elif isinstance(mem, property):
                foo = mem.fset if mem.fset else mem.fget
            if foo:
                qual = foo.__qualname__.split(".")
                # skip methods which are defined in base classes
                if len(qual) == 2 and not qual[0] == class_obj.py_name:
                    continue
                self._inspect_function(mem, class_obj, cls)

        self.context.classes.append(class_obj)
        self.pop_scope()

    def _resolve_classes(self):
        for c in self.context.classes:
            c.bases = []
            for b in c.py_class.__bases__:
                bname = _get_name(b)
                if bname == "object":
                    continue
                for i in self.context.classes:
                    if bname == i.py_name:
                        c.bases.append(i)


def scan_module(module):
    """
    Scans the module and returns a liblolpig.Context class
    :param module: a loaded module
    """
    if not inspect.ismodule(module):
        raise TypeError("Expected module, got %s" % type(module))

    c = _Exporter()
    c.inspect_module(module)
    return c.context

def _import_module_file(module_file):
    module_name = os.path.basename(module_file).split(".")[0]
    print("Importing module '%s'" % module_name)
    # Python 3.4 way
    module = SourceFileLoader(module_name, module_file).load_module()
    return module

def scan_module_files(files):
    mods = []
    for f in files:
        mods.append(_import_module_file(f))

    if not mods:
        raise RuntimeError("No modules to scan")

    context = scan_module(mods[0])
    for i in range(1,len(mods)):
        context.merge(scan_module(mods[i]))

    return context

