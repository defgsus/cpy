"""
Collection of formatting helper functions
and the final Renderer to generate the output
"""
from .c_types import *

INDENT = "    "

def to_c_string(text):
    """
    Make 'text' agreeable as C/C++ string literal
    :return: str
    """
    text = text.replace("\\", "\\\\")
    text = text.replace("\n", "\\n")
    text = text.replace("\r", "")
    text = text.replace('"', '\\"')
    return text


def is_whitespace(c):
    return c == ' ' or c == '\n' or c == '\t' or c == '\r'


def strip_newlines(code):
    """
    Removes \n from beginning and end of string.
    All Whitespace is removed as well, except for whitespace directly
    preceding the text
    :return: str
    """
    start = 0
    for i, c in enumerate(code):
        if c == '\n':
            start = i+1
        if not (c == ' ' or c == '\t' or c == '\n'):
            break
    i = len(code)-1
    while i > 0 and is_whitespace(code[i]):
        i -= 1
    return code[start:i+1]


#def indent_code(code, indent):
#    import re
#    return indent + re.sub(r"\n[ |\t]*", "\n"+indent, code.strip())


def change_text_indent(code, len):
    """
    Changes the indentation of a block of text.
    All leading whitespace on each line is stripped up to the
    maximum common length of ws for each line and then 'len' spaces are inserted.
    Also merges multiple new-lines into one
    :return: str
    """
    lines = code.replace("\t", INDENT).split("\n")
    min_space = -1
    for line in lines:
        for i, k in enumerate(line):
            if not (k == " " or k == "\n"):
                if min_space < 0:
                    min_space = i
                else:
                    min_space = min(min_space, i)
                break
    pre = " " * len
    code = ""
    was_nl = False
    for line in lines:
        li = line[min_space:]
        if li:
            code += pre + li + "\n"
            was_nl = False
        else:
            if not was_nl:
                code += "\n"
            was_nl = True
    if code.endswith("\n"):
        code = code[:-1]
    return code


def apply_string_dict(code_, dic):
    """
    Replaces %(key)s tags in the given code_ with values from the dictionary dic.
    The indentation of code_ and multi-line dic values will be preserved,
    e.g., given a dic value for "bar":

    for i in bar:
        baz

    then

    foo:
        %(bar)s

    will be converted to:

    foo:
        for i in bar:
            baz

    The original indentation of dic values will be stripped using change_text_indent()
    :return: str
    """
    code = str(code_)
    for key in dic:
        skey = "%(" + key + ")s"
        pos = code.find(skey)
        while pos >= 0:
            linestart = code.rfind("\n", 0, pos)
            if linestart < 0:
                linestart = pos
                indent = 0
            else:
                linestart += 1
                for i in range(linestart, pos):
                    if not is_whitespace(code[i]):
                        linestart = pos
                        break
                indent = pos - linestart
            #print(linestart, pos, indent)
            text = change_text_indent(dic[key], indent)
            code = code[:linestart] + text + code[pos + len(skey):]
            pos = code.find(skey)
    return code



def split_doc_cpp(text):
    """
    Splits the text into a doc part and a dict of CPP annotations
    :param text:
    :return: tuple (str, dict)
    """
    if not "_CPP_" in text:
        return (text, {})
    doc_end = 0
    idxs = []
    import re
    for i in re.finditer(r"_CPP_(\([A-Za-z]*\))?:", text):
        if not doc_end:
            doc_end = i.start()
        if i.groups():
            key = i.groups()[0]
            if key:
                key = key.replace("(", "").replace(")", "").upper()
            idxs.append((key, i.start(), i.end()))

    dic = {}
    for i, x in enumerate(idxs):
        end = len(text)
        if i+1 < len(idxs):
            end = idxs[i+1][1]
        dic.setdefault(x[0], strip_newlines(text[x[2]:end]))

    return (text[:doc_end].strip(), dic)



def render_func_def(name, type):
    """
    Render a function definition with all function arguments
    :param name: str, name of the function
    :param type: str, name of the function type, e.g. "unaryfunc", see c_types.py
    :return: str
    """
    if not type in FUNCTIONS:
        raise ValueError("Function type for %s not in c_types.FUNCTIONS" % type)
    args = FUNCTIONS[type]
    code = "static %s %s(" % (args[0], name)
    for i, a in enumerate(args[1]):
        code += "%s arg%d" % (a, i)
        if i + 1 < len(args[1]):
            code += ", "
    return code + ")"


def render_function(name, type, cpp, for_class=None):
    """
    Render a function declaration
    :param name: str, name of the function
    :param type: str, name of the function type, e.g. "unaryfunc", see c_types.py
    :param cpp: str, the function body
    :param for_class: Class, if provided, a cast from 'arg0' to 'self' for the given
            class will be rendered before the user code
    :return: str
    """
    if not type in FUNCTIONS:
        raise ValueError("Function type for %s not in c_types.FUNCTIONS" % type)
    get_self = ""
    unused = ""
    for i in range(len(FUNCTIONS[type][1])):
        unused += "CPPY_UNUSED(arg%d); " % i
    if unused:
        unused = INDENT + unused + "\n"

    if for_class:
        get_self = INDENT + "%(struct)s* self = reinterpret_cast<%(struct)s*>(arg0);\n" % {
                                                    "struct": for_class.class_struct_name }
    code = "%s\n{\n%s%s%s\n}\n" % (
        render_func_def(name, type),
        unused,
        get_self,
        change_text_indent(strip_newlines(cpp), 4)
    )
    return code


def render_struct(structtypename, struct_table, name, dictionary, first_line=""):
    """
    Renders a struct with the contents from 'dictionary'
    :param structtypename: str, name of the struct type, e.g. "PyNumberMethods"
    :param struct_table: list, something like, e.g. c_types.PyNumberMethods
    :param name: str, name of the struct variable
    :param dictionary: dict, key-value for the struct members, e.g. { "nb_add": "my_add_method" }
    :param first_line: optional first line in struct entry, e.g. "PyVarObject_HEAD_INIT(NULL, 0)"
    :return: str
    """
    name_width = 1
    type_width = 1
    for i in struct_table:
        name_width = max(name_width, len(i[0]))
        type_width = max(type_width, len(i[1]))

    code = "static %(type)s %(name)s =\n{\n" % {
        "type": structtypename, "name": name
    }
    if first_line:
        code += INDENT + first_line + "\n"
    for i in struct_table:
        cast = "static"
        # return type of cppy's 'new' function is the class struct, not PyObject
        if i[0] == "tp_new":
            cast = "reinterpret"
        code += "%(indent)s%(name)s %(type)s(%(value)s)" % {
            "indent": INDENT,
            "name" : ("/* %s */" % i[0]).ljust(name_width + 6),
            "type" : ("%s_cast<%s>" % (cast, i[1])).ljust(type_width + 13),
            "value" : str(dictionary.get(i[0], "NULL"))
        }
        if not i == struct_table[-1]:
            code += ","
        code += "\n"
    code += "}; /* %s */\n" % name
    return code




class Renderer:
    """
    Main renderer for a module
    """
    def __init__(self, context):
        if not context:
            raise ValueError("No context given to Renderer")
        self.context = context
        self.namespaces = []

    @property
    def classes(self):
        return self.context.classes

    @property
    def functions(self):
        return self.context.functions

    @classmethod
    def write_to_file(cls, filename, code):
        import codecs
        with codecs.open(filename, "w", "utf-8") as file:
            file.write(code)

    def render_hpp(self):
        code = """
        /* generated by lolpig on %(date)s */

        #include <python3.4/Python.h>

        %(namespace_open)s

        /* Call this before Py_Initialize() */
        bool initialize_module_%(name)s();

        extern "C" {
            %(struct_defs)s
            %(func_defs)s
        } // extern "C"

        %(namespace_close)s
        """
        code = change_text_indent(code, 0)

        import datetime
        code = apply_string_dict(code, {
            "name": self.context.module_name,
            "date": str(datetime.datetime.now()),
            "struct_defs": self._render_struct_defs(),
            "func_defs": self._render_function_defs(),
            "namespace_open": self._render_namespace_open(),
            "namespace_close": self._render_namespace_close(),
        })
        return code


    def render_cpp(self):
        code = """
        /* generated by lolpig on %(date)s */

        #include <python3.4/Python.h>
        #include <python3.4/structmember.h>
        #include "%(header_name)s"

        #ifndef CPPY_ERROR
        #   include <iostream>
        #   define CPPY_ERROR(arg__) { std::cerr << arg__ << std::endl; }
        #endif

        /* compatibility checks */
        %(static_asserts)s

        %(namespace_open)s

        /* the python c-api tango */

        extern "C" {

            %(module_def)s
        } // extern "C"

        %(module_init)s

        %(namespace_close)s
        """
        code = change_text_indent(code, 0)

        import datetime
        code = apply_string_dict(code, {
            "date": str(datetime.datetime.now()),
            "module_name": self.context.module_name,
            "header_name": self.context.header_name,
            "static_asserts" : self._render_static_asserts(),
            "namespace_open": self._render_namespace_open(),
            "namespace_close": self._render_namespace_close(),
            "module_def": self._render_module_def(),
            "module_init": self._render_module_init(),
        })
        return code

    def _render_static_asserts(self):
        code = "#include <type_traits>\n"
        for functype in FUNCTIONS:
            params = FUNCTIONS[functype]
            parstr = params[1][0]
            for j in range(1, len(params[1])):
                parstr += ", %s" % params[1][j]
            typedef = "%(ret)s(*)(%(params)s)" % { "ret": params[0], "params": parstr }
            code += 'static_assert(std::is_same<%s,\n    %s>::value, "lolpig/python api mismatch");\n' % (functype, typedef)
        return code

    def _render_function_defs(self):
        code = ""
        for i in self.functions:
            code += "%s;\n" % i.c_definition()
        for c in self.classes:
            for i in c.methods:
                code += "%s;\n" % i.c_definition()
        return code

    def _render_struct_defs(self):
        code = ""
        for i in self.classes:
            code += "struct %s;\n" % i.class_struct_name
        return code

    def _render_namespace_open(self):
        code = ""
        for i in self.namespaces:
            code += "namespace %s {\n" % i
        return code

    def _render_namespace_close(self):
        code = ""
        for i in reversed(self.namespaces):
            code += "} // namespace %s\n" % i
        return code

    def _render_module_init(self):
        code = """
        namespace {

            PyMODINIT_FUNC create_module_%(name)s_func()
            {
                auto module = PyModule_Create(&%(module_def)s);
                if (!module)
                    return nullptr;

                %(init_calls)s

                return module;
            }

        } // namespace

        bool initialize_module_%(name)s()
        {
            PyImport_AppendInittab("%(name)s", create_module_%(name)s_func);
            return true;
        }
        """
        code = change_text_indent(code, 0)

        init_calls = ""
        if self.classes:
            init_calls += "// add classes to module\n"
            for i in self.classes:
                init_calls += "%s(module);\n" % i.init_func_name

        code = apply_string_dict(code, {
            "name": self.context.module_name,
            "module_def": self.context.struct_name,
            "init_calls": init_calls,
        })

        return code

    def _render_module_def(self):
        """Renders classes, methods and global functions"""
        dic = { "name": self.context.module_name,
                "m_name": '"%s"' % self.context.module_name,
                "struct_name": self.context.struct_name,
                "m_doc": "%s_doc" % self.context.struct_name,
                "m_methods" : "nullptr",
                "m_size": "-1",
                "doc": to_c_string(self.context.module_doc) }
        code = ""
        if self.classes:
            code += "/* ---- classes ---- */\n\n"
            for c in self.classes:
                code += self._render_class_def(c)
        if self.functions:
            code += "/* ---- global functions ---- */\n%s\n" \
                    % self._render_method_struct(self.context.method_struct_name, self.functions)
            dic.update({ "m_methods": self.context.method_struct_name})

        code += """/* module definition for '%(name)s' */\nstatic const char* %(m_doc)s = "%(doc)s";\n""" % dic
        code += render_struct("PyModuleDef", PyModuleDef, dic["struct_name"], dic,
                              first_line="PyModuleDef_HEAD_INIT,")
        return code

    def _render_method_struct(self, struct_name, functions):
        code = "static PyMethodDef %s[] =\n{\n" % struct_name
        for i in functions:
            code += INDENT + self._render_method_struct_entry(i) + "\n"
        code += "\n" + INDENT + "{ NULL, NULL, 0, NULL }\n};\n"
        return code

    def _render_method_struct_entry(self, func):
        return '{ "%s", reinterpret_cast<PyCFunction>(%s), %s, "%s" },' % (
            func.py_name_single(),
            func.c_name,
            func.get_c_method_type(),
            to_c_string(func.py_doc)
        )

    def _render_class_def(self, cls):
        """Renders full class definition"""
        cls.finalize()
        code = "/* ---- class %s ---- */\n\n" % cls.py_name

        code += 'static const char* %s = "%s";\n\n' % (cls.doc_string_name, to_c_string(cls.py_doc))

        # general methods
        if cls.normal_methods:
            code += "\n\n/* ---- %s methods ---- */\n" % cls.py_name
            code += self._render_method_struct(cls.method_struct_name, cls.normal_methods)

        # special methods
        if cls.has_number_method():
            code += "\n\n/* ---- %s number methods ---- */\n" % cls.py_name
            code += self._render_class_number_struct(cls)
        if cls.has_sequence_method():
            code += "\n\n/* ---- %s sequence methods ---- */\n" % cls.py_name
            code += self._render_class_sequence_struct(cls)

        # init/dealloc
        code += "\n" + self._render_class_init_funcs(cls)

        # c-api type struct
        code += "\n" + self._render_class_type_struct(cls)

        # user-helper
        code += "\n" + self._render_class_user_funcs(cls)

        # class->module init func
        code += "\n" + self._render_class_init_func(cls)

        return code + "\n"

    def _render_class_number_struct(self, cls):
        dic = {}
        for i in NUMBER_FUNCS:
            if cls.has_method(i[0]):
                val = cls.get_method(i[0]).c_name
                dic.update({i[1]: val})
        return render_struct("PyNumberMethods", PyNumberMethods, cls.number_struct_name, dic)

    def _render_class_sequence_struct(self, cls):
        dic = {}
        for i in SEQUENCE_FUNCS:
            if cls.has_method(i[0]):
                val = cls.get_method(i[0]).c_name
                dic.update({i[1]: val})
        return render_struct("PySequenceMethods", PySequenceMethods, cls.sequence_struct_name, dic)

    def _render_class_type_struct(self, cls):
        dic = {}
        for i in PyTypeObject:
            dic[i[0]] = "NULL"
        dic.update({
            "tp_name": '"%s.%s"' % (self.context.module_name, cls.py_name),
            "tp_basicsize": str(cls.struct_size),
            "tp_dealloc": cls.class_dealloc_func_name,
            "tp_getattro": "PyObject_GenericGetAttr",
            "tp_setattro": "PyObject_GenericSetAttr",
            "tp_flags": "Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE",
            "tp_doc": cls.doc_string_name,
            "tp_new": cls.class_new_func_name
        })
        if cls.normal_methods:
            dic.update({"tp_methods": cls.method_struct_name})
        #if self.bases:
        #    dic.update({ "tp_base": "&" + self.bases[0].type_struct_name })
        for i in TYPE_FUNCS:
            if cls.has_method(i[0]):
                dic.update({i[1]: cls.get_method(i[0]).c_name})
        if cls.has_sequence_method():
            dic.update({"tp_as_sequence": "&" + cls.sequence_struct_name})
        if cls.has_number_method():
            dic.update({"tp_as_number": "&" + cls.number_struct_name})
        #if self.properties:
        #    dic.update({"tp_getset": self.getset_struct_name})

        return "/* https://docs.python.org/3/c-api/typeobj.html */\n" + \
                render_struct("PyTypeObject", PyTypeObject,
                             cls.type_struct_name, dic,
                             first_line="PyVarObject_HEAD_INIT(NULL, 0)")

    def _render_class_init_func(self, cls):
        code = """
        bool %(func_name)s(PyObject* module)
        {
            if (0 != PyType_Ready(&%(struct_name)s))
            {
                CPPY_ERROR("Failed to readify class %(name)s for Python module");
                return false;
            }

            PyObject* object = reinterpret_cast<PyObject*>(&%(struct_name)s);
            Py_INCREF(object);
            if (0 != PyModule_AddObject(module, "%(name)s", object))
            {
                Py_DECREF(object);
                CPPY_ERROR("Failed to add class %(name)s to Python module");
                return false;
            }
            return true;
        }
        """ % {
            "name": cls.py_name,
            "func_name": cls.init_func_name,
            "struct_name": cls.type_struct_name
        }
        return change_text_indent(code, 0)

    def _render_class_init_funcs(self, cls):
        code = """
        /* Creates new instance of %(name)s class. */
        PyObject* %(new_func)s(struct _typeobject * type, PyObject *, PyObject *)
        {
            return PyObject_New(PyObject, type);
        }

        /* Deletes a %(name)s instance */
        void %(dealloc_func)s(PyObject* self)
        {
            self->ob_type->tp_free(self);
        }
        """
        code %= {
            "name": cls.py_name,
            "struct_name": cls.class_struct_name,
            "type_struct": cls.type_struct_name,
            "new_func": cls.class_new_func_name,
            "dealloc_func": cls.class_dealloc_func_name,
        }
        return change_text_indent(code, 0)

    def _render_class_user_funcs(self, cls):
        code = """
        /* user helper functions */
        %(struct)s* %(new_func)s() { return PyObject_NEW(%(struct)s, &%(type_struct)s); }
        bool %(is_func)s(PyObject* obj) { return PyObject_TypeCheck(obj, &%(type_struct)s); }
        """ % {
            "struct": cls.class_struct_name,
            "type_struct": cls.type_struct_name,
            "new_func": cls.user_new_func,
            "is_func": cls.user_is_func,
        }
        return change_text_indent(code, 0)
