

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

    def c_argument_list(self):
        r = ""
        for a in self.arguments:
            if r:
                r += ", "
            r += "%s %s" % (a.c_type, a.c_name if a.c_name else "")
        return r

    def c_definition(self):
        return "%s %s(%s)" % (self.c_return_type, self.c_name, self.c_argument_list())


class Class:
    def __init__(self):
        self.c_name = ""
        self.py_name = ""
        self.py_doc = ""
        self.line = 0
        self.struct_size = 0
        self.methods = []

    def update_names(self):
        self.class_struct_name = self.c_name
        self.type_struct_name = "%s_type_struct" % self.c_name
        self.method_struct_name = "%s_method_struct" % self.c_name
        self.number_struct_name = "%s_number_struct" % self.c_name
        self.mapping_struct_name = "%s_mapping_struct" % self.c_name
        self.sequence_struct_name = "%s_sequence_struct" % self.c_name
        self.getset_struct_name = "%s_getset_struct" % self.c_name
        self.class_new_func_name = "create_%s" % self.c_name
        self.class_copy_func_name = "copy_%s" % self.c_name
        self.class_dealloc_func_name = "destroy_%s" % self.c_name
        self.init_func_name = "initialize_class_%s" % self.py_name
        self.doc_string_name = "%s_doc_string" % self.c_name

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
            print("  " + f.c_definition())
        print("CLASSES:")
        for c in self.classes:
            print("  " + c.py_name)
            for f in c.methods:
                print("    " + f.c_definition())
