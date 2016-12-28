

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
        self.c_return_type = ""
        self.py_name = ""
        self.py_doc = ""
        self.line = 0
        self.end_line = 0
        self.arguments = []

    def c_argument_list(self):
        r = ""
        for a in self.arguments:
            if r:
                r += ", "
            r += "%s %s" % (a.c_type, a.c_name if a.c_name else "")
        return r

    def c_definition(self):
        return "%s %s(%s)" % (self.c_return_type, self.c_name, self.c_argument_list())


class Context:
    def __init__(self):
        self.filename = ""
        self.module_name = "module"
        self.header_name = "test_module.h"
        self.module_doc = "The module documentation"
        self.struct_name = "module_struct"
        self.method_struct_name = "module_method_struct"
        self.functions = []


    def dump(self):
        print("FUNCTIONS:")
        for f in self.functions:
            print("  " + f.c_definition())
