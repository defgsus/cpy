from .c_types import *

class Namespace:
    def __init__(self):
        self.c_name = ""

class Argument:
    def __init__(self):
        self.c_name = ""
        self.c_type = ""


class Function:
    def __init__(self):
        self.c_name = ""
        self.py_name = ""
        self.py_doc = ""
        self.line = 0
        self.end_line = 0
        self.c_return_type = ""
        self.arguments = []

    def c_arguments(self):
        """Returns the arguments list as list of c_type strings"""
        r = []
        for a in self.arguments:
            r.append(a.c_type)
        return r

    def c_argument_list(self):
        """Returns a c++ compatible string with the function arguments"""
        r = ""
        for a in self.arguments:
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
        n = self.py_name_single()
        # see if function's name requires special signature
        if self.is_class_method():
            for i in ALL_FUNCS:
                if n == i[0]:
                    return STRUCT_MEMBER_TO_TYPE[i[1]]
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
                return "METH_VARARGS"
            elif type == "ternaryfunc":
                return "METH_VARARGS | METH_KEYWORDS"
            return "METH_NOARGS"
        if type == "binaryfunc":
            return "METH_VARARGS | METH_KEYWORDS"
        if not self.arguments:
            return "METH_NOARGS"
        return "METH_VARARGS"

    def verify(self):
        type = self.get_function_type()
        if not type:
            return
        args = FUNCTIONS[type]
        if not self.c_return_type == args[0]:
            raise TypeError("Function %s has wrong return type %s, expected %s" % (
                self.c_name, self.c_return_type, args[0]
            ))
        if not len(self.arguments) == len(args[1]):
            raise TypeError("Function %s has wrong number of arguments, expected %d" % (
                self.c_name, len(args[1])
            ))
        for i, a in enumerate(args[1]):
            if not self.arguments[i].c_type == a:
                raise TypeError("Function %s has wrong argument #%d %s, expected %s" % (
                    self.c_name, i+1, self.arguments[i].c_type, a
                ))



class Class:
    def __init__(self):
        self.c_name = ""
        self.py_name = ""
        self.py_doc = ""
        self.line = 0
        self.struct_size = 0
        self.methods = []
        self.normal_methods = []

    def finalize(self):
        self._update_names()
        self._update_methods()
        for f in self.methods:
            f.verify()

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

    def _update_methods(self):
        self.normal_methods = []
        for f in self.methods:
            if f.is_normal_function():
                self.normal_methods.append(f)

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
        self.filename = ""
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
            print("  " + f.c_definition() + " " + str(f.get_function_type()))
        print("CLASSES:")
        for c in self.classes:
            print("  " + c.py_name)
            for f in c.methods:
                print("    " + f.c_definition() + " " + str(f.get_function_type()))

    def finalize(self):
        for f in self.functions:
            f.verify()
        for c in self.classes:
            c.finalize()