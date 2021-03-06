import inspect
from .c_types import *
from .gccxml import ParseError

class Namespaced:
    def __init__(self):
        self.namespaces = []
        self.id = None

    def has_namespace(self):
        return len(self.namespaces) > 1 or (len(self.namespaces) == 1 and not self.namespaces == ["::"])

    def get_namespace_prefix(self):
        n = []
        for i in self.namespaces:
            if not i == "::":
                n.append(i)
        return "::".join(n) + "::" if n else "::"

    def key(self):
        """unique C identifier"""
        if self.id:
            return self.id
        k = self.get_namespace_prefix()
        if hasattr(self, "c_name"):
            k += self.c_name
        return k


class Argument:
    def __init__(self, c_type="", c_name=""):
        self.c_name = c_name
        self.c_type = c_type


class Function(Namespaced):
    def __init__(self):
        super(Function, self).__init__()
        self.c_name = ""
        self.py_name = ""
        self.py_doc = ""
        self.line = 0
        self.end_line = 0
        self.c_return_type = ""
        self.arguments = []
        self.namespaces = []
        self.is_property = False
        self.is_setter = False
        self.py_args = None

    def __hash__(self):
        return hash(self.c_name)

    @classmethod
    def from_python(cls, pyfunc, pyclass=None, is_prop=False, is_set=False):
        f = Function()
        f.py_name = pyfunc.__name__
        if pyclass:
            f.py_name = pyclass.__name__ + "." + f.py_name
        f.py_doc = inspect.cleandoc(pyfunc.__doc__) if pyfunc.__doc__ else ""
        f.c_name = f.py_name.replace(".", "_")
        f.c_name = f.c_name.replace("___", "__")
        f.py_args = inspect.getfullargspec(pyfunc)
        f.is_property = is_prop
        f.is_setter = is_set
        f.c_return_type, f.arguments = f.get_args_from_py()
        return f

    @property
    def full_c_name(self):
        return self.get_namespace_prefix() + self.c_name

    def c_arguments(self):
        """Returns the arguments list as list of c_type strings"""
        r = []
        for a in self.arguments:
            r.append(a.c_type)
        return r

    def c_argument_list(self, args=None):
        """Returns a c++ compatible string with the function arguments"""
        if not args:
            args = self.arguments
        r = ""
        for a in args:
            if r:
                r += ", "
            r += "%s %s" % (a.c_type, a.c_name if a.c_name else "")
        return r

    def c_definition(self):
        """Returns the whole c++ function definition"""
        return "%s %s(%s)" % (self.c_return_type, self.c_name, self.c_argument_list())

    def py_name_single(self):
        """Returns the python name of the function, without class"""
        return self.py_name.split(".")[-1]

    def is_class_method(self):
        return "." in self.py_name

    def is_normal_function(self):
        if not self.is_class_method():
            return True
        return not (self.is_type_function() or self.is_number_function() or self.is_sequence_function())

    def is_special_function(self, func_list):
        if self.is_property:
            return True
        n = self.py_name_single()
        for f in func_list:
            if n == f[0]:
                return True
        return False

    def is_type_function(self):
        return self.is_special_function(TYPE_FUNCS)

    def is_number_function(self):
        return self.is_special_function(NUMBER_FUNCS)

    def is_sequence_function(self):
        return self.is_special_function(SEQUENCE_FUNCS)

    def get_function_type(self):
        """Returns the type of function, e.g 'unary', 'lenfunc', etc.."""
        if self.is_property:
            return "setter" if self.is_setter else "getter"
        n = self.py_name_single()
        # see if function's name requires special signature
        if self.is_class_method():
            for i in ALL_FUNCS:
                if n == i[0]:
                    return STRUCT_MEMBER_TO_TYPE[i[1]]
            for i in SPECIAL_FUNCS:
                if n == i[0]:
                    return i[1]
        # determine type from signature
        selfargs = tuple(self.c_arguments())
        if self.c_return_type == "PyObject*":
            # make sure that we get "unaryfunc", "binaryfunc" and "ternaryfunc"
            # for the common signatures instead of a random (by hash) entry
            # from the FUNCTIONS dict
            if selfargs == ("PyObject*",):
                return "unaryfunc"
            if selfargs == ("PyObject*", "PyObject*"):
                return "binaryfunc"
            if selfargs == ("PyObject*", "PyObject*", "PyObject*"):
                return "ternaryfunc"
        for key in FUNCTIONS.keys():
            args = FUNCTIONS[key]
            if args[0] == self.c_return_type:
                if selfargs == args[1]:
                    return key
        # dunno
        return None

    def get_c_method_type(self):
        """Returns one of the METH_xxx enums used in PyMethodDef"""
        type = self.get_function_type()
        if self.is_class_method():
            if type == "unaryfunc":
                return "METH_NOARGS"
            elif type == "binaryfunc":
                return "METH_O" if self.arguments[1].c_name.upper() == "OBJ" else "METH_VARARGS"
            elif type == "ternaryfunc":
                return "METH_VARARGS | METH_KEYWORDS"
            return "METH_NOARGS"
        if type == "unaryfunc":
            return "METH_O" if self.arguments[0].c_name.upper() == "OBJ" else "METH_VARARGS"
        if type == "binaryfunc":
            return "METH_VARARGS | METH_KEYWORDS"
        if not self.arguments:
            return "METH_NOARGS"
        return "METH_VARARGS"

    def get_args_from_py(self):
        """Return tuple of (c_return_type, [Argument,]) from inspecting py_name and/or py_args"""
        sig = None
        if self.is_property:
            sig = FUNCTIONS.get("setter") if self.is_setter else FUNCTIONS.get("getter")
        if not self.py_name:
            raise ParseError("No py_name defined for %s" % self)
        if not sig:
            sig = FUNCTIONS.get(FUNCNAME_TO_TYPE.get(self.py_name_single()))
        if sig:
           ret = (sig[0], [Argument(sig[1][x], sig[2][x]) for x in range(len(sig[1]))])
        else:
            num_args = 10
            has_varkw = False
            has_vararg = False
            if self.py_args:
                #print(self.py_args)
                num_args = len(self.py_args.args)
                has_varkw = bool(self.py_args.varkw)
                has_vararg = bool(self.py_args.varargs)
                if self.py_args.defaults:
                    has_varkw = True
                if has_varkw:
                    num_args += 1
                if has_vararg:
                    num_args += 1
            ret = ("PyObject*", [])
            if self.is_class_method():
                ret[1].append(Argument("PyObject*", "self"))
                num_args -= 1
            if num_args:
                aname = "obj"
                if num_args > 1 or has_vararg or has_varkw:
                    aname = "args"
                ret[1].append(Argument("PyObject*", aname))
                if has_varkw:
                    ret[1].append(Argument("PyObject*", "kwargs"))
        return ret

    def verify(self):
        type = self.get_function_type()
        if not type:
            return
        args = FUNCTIONS[type]
        if not self.c_return_type == args[0]:
            raise TypeError("Function %s has wrong return type %s, expected %s\nshould be like: %s" % (
                self.c_name, self.c_return_type, args[0], self.render_ideal()
            ))
        if not len(self.arguments) == len(args[1]):
            raise TypeError("Function %s has wrong number of arguments %d, expected %d\nshould be like: %s" % (
                self.c_name, len(self.arguments), len(args[1]), self.render_ideal()
            ))
        for i, a in enumerate(args[1]):
            if not self.arguments[i].c_type == a:
                raise TypeError("Function %s has wrong argument #%d %s, expected %s\nshould be like: %s" % (
                    self.c_name, i+1, self.arguments[i].c_type, a, self.render_ideal()
                ))

    def render_ideal(self):
        type = self.get_function_type()
        if not type:
            raise NotImplementedError("XXX")
        fargs = FUNCTIONS[type]
        args = []
        assert(len(fargs[1]) == len(fargs[2]))
        for i, a in enumerate(fargs[1]):
            arg = Argument()
            arg.c_type = a
            arg.c_name = self.arguments[i].c_name if i < len(self.arguments) and self.arguments[i].c_name else fargs[2][i]
            args.append(arg)
        return "%s %s(%s)" % (fargs[0], self.c_name, self.c_argument_list(args))

    def c_return_statement(self, error=False):
        if self.is_normal_function() or self.c_return_type == "PyObject*":
            if error:
                return "return NULL"
            if self.is_class_method():
                return "Py_RETURN_SELF"
            return "Py_RETURN_NONE"
        if self.c_return_type == "int" or self.c_return_type == "Py_ssize_t":
            return "return -1" if error else "return 0"
        return "return"

    def py_arguments(self):
        """arguments as string as in a python function def"""
        if not self.py_args:
            return ""
        ret = ""
        defoffs = len(self.py_args.args)
        if self.py_args.defaults:
            defoffs -= len(self.py_args.defaults)
        for i, a in enumerate(self.py_args.args):
            ret += ", " + a
            if i >= defoffs:
                ret += "=" + str(self.py_args.defaults[i-defoffs])
        if self.py_args.varargs:
            ret += ", *" + self.py_args.varargs
        if self.py_args.varkw:
            ret += ", **" + self.py_args.varkw
        return ret[2:]



class Class(Namespaced):
    def __init__(self):
        super(Class, self).__init__()
        self.c_name = ""
        self.py_name = ""
        self.py_doc = ""
        self.line = 0
        self.struct_size = 0
        self.methods = []
        self.normal_methods = [] # generated
        self.bases = []

    def __hash__(self):
        return hash(self.c_name)

    @classmethod
    def from_python(cls, pyclass):
        c = Class()
        c.py_name = pyclass.__name__
        c.py_doc = inspect.cleandoc(pyclass.__doc__) if pyclass.__doc__ else ""
        c.c_name = str(c.py_name)
        c.py_class = pyclass
        return c

    def finalize(self):
        self._update_names()
        self._update_methods()
        for f in self.methods:
            f.verify()

    def merge(self, other):
        for i in other.methods:
            if not self.has_method(i.c_name):
                self.methods.append(i)
            if not self.bases:
                self.bases = i.bases

    def properties(self):
        """Returns all Functions which are properties"""
        p = []
        for f in self.methods:
            if f.is_property:
                tup = None
                for i in p:
                    if (i[0] and i[0].py_name == f.py_name) or (i[1] and i[1].py_name == f.py_name):
                        tup = i
                        break
                if not tup:
                    tup = [None, None]
                    p.append(tup)
                if f.is_setter:
                    tup[1] = f
                else:
                    tup[0] = f
        return p

    def _update_names(self):
        self.class_struct_name = self.c_name
        self.type_struct_name = "%s_type_struct" % self.c_name
        self.method_struct_name = "%s_method_struct" % self.c_name
        self.number_struct_name = "%s_as_number_struct" % self.c_name
        self.mapping_struct_name = "%s_as_mapping_struct" % self.c_name
        self.sequence_struct_name = "%s_as_sequence_struct" % self.c_name
        self.getset_struct_name = "%s_getset_struct" % self.c_name
        self.class_new_func_name = "create_%s" % self.c_name
        self.class_copy_func_name = "copy_%s" % self.c_name
        self.class_dealloc_func_name = "destroy_%s" % self.c_name
        self.init_func_name = "initialize_class_%s" % self.py_name
        self.doc_string_name = "%s_doc_string" % self.c_name
        self.user_new_func = "new_%s" % self.c_name
        self.user_is_func = "is_%s" % self.c_name
        self.user_type_func = "type_%s" % self.c_name
        self.sizeof_func = "sizeof_%s" % self.c_name

    def _update_methods(self):
        self.normal_methods = []
        for f in self.methods:
            if f.is_normal_function():
                self.normal_methods.append(f)

    def has_base(self, cls):
        if cls in self.bases:
            return True
        for c in self.bases:
            if c.has_base(cls):
                return True
        return False

    def has_method(self, py_name):
        for i in self.methods:
            if i.py_name_single() == py_name:
                return True
        return False

    def get_method(self, py_name):
        for i in self.methods:
            if i.py_name_single() == py_name:
                return i
        return None

    def has_special_method(self, func_list):
        for f in func_list:
            if self.has_method(f[0]):
                return True
        return False

    def has_number_method(self):
        return self.has_special_method(NUMBER_FUNCS)

    def has_sequence_method(self):
        return self.has_special_method(SEQUENCE_FUNCS)



class Context:
    def __init__(self):
        self.filenames = []
        self.module_name = "module"
        self.header_name = "test_module.h"
        self.module_doc = "The module documentation"
        self.struct_name = "module_struct"
        self.method_struct_name = "module_method_struct"
        self.functions = []
        self.classes = []

    def dump(self):
        print("FUNCTIONS:")
        for f in self.functions:
            print("  " + f.get_namespace_prefix() + f.c_definition() + " " + str(f.get_function_type())
                  + " (" + f.key() + ") [" + f.py_doc + "]")
        print("CLASSES:")
        for c in self.classes:
            print("  " + c.get_namespace_prefix() + c.py_name + " -> ".join([""]+[x.py_name for x in c.bases])
                  + " (" + c.key() + ")")
            for f in c.methods:
                print("    " + f.py_name + " " + str(f.get_function_type()) + " (" + f.key() + ")")

    def finalize(self):
        for f in self.functions:
            f.verify()
        for c in self.classes:
            c.finalize()
            c.methods.sort(key=lambda f: f.c_name)
        self.functions.sort(key=lambda f: f.c_name)
        self.classes.sort(key=lambda c: c.c_name)
        self._sort_classes_by_bases()

    def merge(self, other):
        dic = dict()
        for ns in self.functions + self.classes:
            dic.setdefault(ns.key(), ns)
        for i in other.functions:
            if not i.key() in dic:
                self.functions.append(i)
        for i in other.classes:
            if not i.key() in dic:
                self.classes.append(i)
            else:
                dic[i.key()].merge(i)


    def _sort_classes_by_bases(self):
        srt = []
        for c in self.classes:
            i = 0
            while i < len(srt):
                if srt[i].has_base(c):
                    break
                i += 1
            srt.insert(i, c)
        self.classes = srt

    def get_object_by_id(self, id):
        for i in self.functions + self.classes:
            if i.id == id:
                return i
        return None

    def get_object_by_c_name(self, c_name):
        for i in self.functions + self.classes:
            if i.c_name == c_name:
                return i
        return None
