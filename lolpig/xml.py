"""
GCC-XML parser
"""

class ParseError(BaseException):
    pass

class XmlFile:
    def __init__(self):
        self.id = None
        self.name = None
        self.content = None
        self.lines = None
        self.do_scan = True

class XmlContext:
    def __init__(self):
        self.id = None
        self.context_id = None
        self.context = None
        self.file_id = None
        self.line = 0
        self.end_line = 0

    def get_namespace_list(self):
        n = []
        if self.context:
            if hasattr(self.context, "c_name"):
                n.append(self.context.c_name)
            n += self.context.get_namespace_list()
        return n


class XmlNamespace(XmlContext):
    def __init__(self):
        super().__init__()
        self.c_name = None

    def __str__(self):
        return "XmlNamespace(%s)" % self.c_name

    def __repr__(self):
        return self.__str__()


class XmlClass(XmlContext):
    def __init__(self):
        super().__init__()
        self.c_name = None


class XmlType(XmlContext):
    def __init__(self):
        super().__init__()
        self.c_name = None
        self.size = 0
        self.array_min = 0
        self.array_max = 0
        self.ref_id = None
        self.ref = None
        self.is_fundamental = True
        self.is_pointer = False
        self.is_const = False
        self.is_reference = False
        self.is_array = False
        self.is_function = False
        self.is_union = False
        self.is_typedef = False
        self.is_enum = False

    def __str__(self):
        return "XmlType(%s)" % self.c_string()

    def __repr__(self):
        return self.__str__()

    def c_string(self):
        s = ""
        if self.is_const:
            s += "const "
        s += self.c_name if self.c_name else self.id
        if self.is_reference:
            s += "&"
        if self.is_pointer:
            s += "*"
        return s


class XmlStruct(XmlContext):
    def __init__(self):
        super().__init__()
        self.c_name = None
        self.py_name = None
        self.py_doc = None
        self.size = 0
        self.fields = []
        self.bases_id = []
        self.bases = []

    def c_string(self):
        return self.c_name

    def as_class(self):
        from .context import Class
        c = Class()
        c.py_name = self.py_name
        c.py_doc = self.py_doc
        c.c_name = self.c_name
        c.line = self.line
        c.struct_size = self.size
        c.namespaces = self.get_namespace_list()
        return c


class XmlField(XmlContext):
    def __init__(self):
        super().__init__()
        self.c_name = None
        self.type = None
        self.type_id = None


class XmlArgument:
    def __init__(self):
        self.c_name = None
        self.type_id = None
        self.type = None

    def __str__(self):
        return "Arg(%s %s)" % (self.c_name, self.type)

    def __repr__(self):
        return self.__str__()

    def as_argument(self):
        from .context import Argument
        a = Argument()
        a.c_name = self.c_name
        a.c_type = self.type.c_string()
        return a


class XmlFunction(XmlContext):
    def __init__(self):
        super().__init__()
        self.c_name = None
        self.py_name = None
        self.py_doc = None
        self.return_type_id = None
        self.return_type = None
        self.arguments = []

    def __str__(self):
        return "XmlFunction(%s, %s, %s)" % (self.c_name, self.py_name, str(self.arguments))

    def __repr__(self):
        return self.__str__()

    def is_class_function(self):
        return "." in self.py_name

    def as_function(self):
        from .context import Function, Argument
        f = Function()
        f.c_name = self.c_name
        f.py_name = self.py_name
        f.py_doc = self.py_doc
        f.line = self.line
        f.end_line = self.end_line
        f.namespaces = self.get_namespace_list()
        f.c_return_type = self.return_type.c_string()
        for arg in self.arguments:
            f.arguments.append(arg.as_argument())
        return f


class XmlParser:
    def __init__(self):
        self.filenames = []
        self.types = dict()
        self.structs = dict()
        self.functions = dict()
        self.fields = dict()
        self.namespaces = dict()
        self.files = dict()
        self.classes = dict()

    def parse(self, filenames):
        for f in filenames:
            self._parse(f)
        self._resolve_types()
        self._resolve_context(self.namespaces.values())
        self._resolve_context(self.functions.values())
        self._resolve_context(self.types.values())
        self._resolve_context(self.structs.values())
        self._resolve_context(self.fields.values())
        self._find_lolpig_def(self.functions.values())
        self._find_lolpig_def(self.structs.values())

    def dump(self):
        print("namespaces", self.namespaces)
        print("types", self.types)
        print("functions", self.functions)

    def as_context(self):
        from .context import Context
        c = Context()
        c.filenames = self.filenames
        for func in self.functions.values():
            if func.py_name and not func.is_class_function():
                c.functions.append(func.as_function())
        for struct in self.structs.values():
            if struct.py_name:
                cls = struct.as_class()
                for func in self.functions.values():
                    if func.py_name and func.py_name.split(".")[0] == cls.py_name:
                        cls.methods.append(func.as_function())
                c.classes.append(cls)
        # resolve bases
        for xmlstruct in self.structs.values():
            struct = c.get_object_by_c_name(xmlstruct.c_name)
            if struct:
                for b in xmlstruct.bases:
                    base = c.get_object_by_c_name(b.c_name)
                    if base:
                        struct.bases.append(base)
        c.finalize()
        return c

    def has_object(self, id):
        return id in self.types \
            or id in self.structs \
            or id in self.functions \
            or id in self.namespaces \
            or id in self.classes

    def get_object(self, id, default = None):
        if id in self.types:
            return self.types[id]
        if id in self.structs:
            return self.structs[id]
        if id in self.functions:
            return self.functions[id]
        if id in self.namespaces:
            return self.namespaces[id]
        if id in self.classes:
            return self.classes[id]
        return default

    def pos_str(self, line):
        return "%s:%d" % (self.filename, line)

    def _resolve_types(self):
        # resolve referenced types
        for t in self.types.values():
            if t.ref_id:
                t.is_fundamental = False
                if not self.has_object(t.ref_id):
                    raise ParseError("Unknown reference type id %s in %s" % (t.ref_id, t))
                t.ref = self.get_object(t.ref_id)

        # fill c_name field for all types
        for t in self.types.values():
            if t.ref_id and not t.c_name:
                r = t.ref
                if not r:
                    continue
                while not r.c_name and hasattr(r, "ref"):
                    if not r.ref:
                        break
                    r = r.ref
                t.c_name = r.c_name

        # resolve types for function arguments and return-type
        for func in self.functions.values():
            if not self.has_object(func.return_type_id):
                raise ParseError("Unknown reference type id %s for "
                                 "function return type of %s" % (func.return_type_id, func))
            func.return_type = self.get_object(func.return_type_id)
            for arg in func.arguments:
                if not self.has_object(arg.type_id):
                    raise ParseError("Unknown reference type id %s for function %s argument" % (arg.ref_id, func))
                arg.type = self.get_object(arg.type_id)

        # resolve types for struct fields
        for f in self.fields.values():
            if not self.has_object(f.type_id):
                raise ParseError("Unknown referenced field id %s in %s" % (f.type_id, f))
            f.type = self.get_object(f.type_id)

        # resolve struct bases
        for s in self.structs.values():
            if s.bases_id:
                s.bases = []
                for b in s.bases_id:
                    if not self.has_object(b):
                        raise ParseError("Unknown struct base reference %s in struct %s" % (b, s))
                    s.bases.append(self.get_object(b))


    def _resolve_context(self, iter):
        for i in iter:
            if i.file_id:
                if not i.file_id in self.files:
                    raise ParseError("Unknown file id %s in object %s" % (i.file_id, i))
                i.file = self.files[i.file_id]
            if i.context_id:
                if not self.has_object(i.context_id):
                    raise ParseError("Unknown context id %s in object %s" % (i.context_id, i))
                i.context = self.get_object(i.context_id)

    def _find_lolpig_def(self, iter):
        for obj in iter:
            if obj.file:# and func.file.name == self.filename:
                try:
                    obj.py_name, obj.py_doc = self._get_def(obj.file, obj.line, obj.c_name)
                except ParseError:
                    continue

    def _parse(self, filename):
        """Generate gcc-xml and parse it"""
        import subprocess, os

        xmlname = filename + "_temp_.xml"
        subprocess.call(["gccxml", filename,
                         "--gccxml-cxxflags", "-DGCC_XML",
                         #"--gccxml-cxxflags", "-std=c++11",
                         #"--gccxml-executable", "g++",
                         "-fxml=%s" % xmlname])

        import xml.etree.ElementTree as ET
        tree = ET.parse(xmlname)
        self._parse_xml(tree.getroot())
        os.remove(xmlname)

        self.filenames.append(filename)

    def _parse_xml(self, root):

        for child in root:
            #print(child.tag)
            if child.tag == "Namespace":
                self._parse_namespace(child)
            elif child.tag == "File":
                self._parse_file(child)
            elif child.tag == "Function":
                self._parse_function(child)
            elif child.tag == "FundamentalType":
                self._parse_fundamental_type(child)
            elif child.tag == "PointerType":
                self._parse_pointer_type(child)
            elif child.tag == "ReferenceType":
                self._parse_reference_type(child)
            elif child.tag == "ArrayType":
                self._parse_array_type(child)
            elif child.tag == "CvQualifiedType":
                self._parse_cv_type(child)
            elif child.tag == "FunctionType":
                self._parse_function_type(child)
            elif child.tag == "Union":
                self._parse_union_type(child)
            elif child.tag == "Typedef":
                self._parse_typedef(child)
            elif child.tag == "Enumeration":
                self._parse_enum(child)
            elif child.tag == "Struct":
                self._parse_struct(child)
            elif child.tag == "Field":
                self._parse_field(child)
            elif child.tag == "Class":
                self._parse_class(child)

    def _parse_context(self, node, ctx):
        ctx.id = node.attrib.get("id", None)
        ctx.line = int(node.attrib.get("line", 0))
        ctx.end_line = int(node.attrib.get("endline", ctx.line))
        ctx.context_id = node.attrib.get("context", None)
        ctx.file_id = node.attrib.get("file", None)

    def _parse_file(self, node):
        t = XmlFile()
        t.id = node.attrib.get("id")
        t.name = node.attrib.get("name")
        self.files.setdefault(t.id, t)

    def _parse_namespace(self, node):
        t = XmlNamespace()
        self._parse_context(node, t)
        t.c_name = node.attrib.get("name")
        self.namespaces.setdefault(t.id, t)

    def _parse_any_type(self, node):
        t = XmlType()
        self._parse_context(node, t)
        t.c_name = node.attrib.get("name")
        t.size = int(node.attrib.get("size", 0))
        t.ref_id = node.attrib.get("type", None)
        self.types.setdefault(t.id, t)
        return t

    def _parse_fundamental_type(self, node):
        self._parse_any_type(node)

    def _parse_pointer_type(self, node):
        t = self._parse_any_type(node)
        t.is_pointer = True

    def _parse_reference_type(self, node):
        t = self._parse_any_type(node)
        t.is_reference = True

    def _parse_array_type(self, node):
        t = self._parse_any_type(node)
        s = node.attrib.get("min", "0").replace("u", "")
        t.array_min = int(s) if s else 0
        s = node.attrib.get("max", "0").replace("u", "")
        t.array_max = int(s) if s else 0
        t.is_array = True

    def _parse_cv_type(self, node):
        t = self._parse_any_type(node)
        t.is_const = True

    def _parse_function_type(self, node):
        t = self._parse_any_type(node)
        t.is_function = True

    def _parse_union_type(self, node):
        t = self._parse_any_type(node)
        t.is_union = True

    def _parse_typedef(self, node):
        t = self._parse_any_type(node)
        t.is_typedef = True

    def _parse_enum(self, node):
        t = self._parse_any_type(node)
        t.is_enum = True

    def _parse_struct(self, node):
        s = XmlStruct()
        self._parse_context(node, s)
        s.c_name = node.attrib.get("name")
        s.size = int(node.attrib.get("size", 0))
        s.bases_id = node.attrib.get("bases", "").split()
        self.structs.setdefault(s.id, s)

    def _parse_function(self, node):
        func = XmlFunction()
        self._parse_context(node, func)
        func.c_name = node.attrib.get("name")
        func.return_type_id = node.attrib.get("returns")
        for child in node:
            if child.tag == "Argument":
                self._parse_argument(func, child)
        self.functions.setdefault(func.id, func)

    def _parse_argument(self, func, node):
        a = XmlArgument()
        a.c_name = node.attrib.get("name")
        a.type_id = node.attrib.get("type")
        func.arguments.append(a)

    def _parse_field(self, node):
        field = XmlField()
        self._parse_context(node, field)
        field.type_id = node.attrib.get("type")
        self.fields.setdefault(field.id, field)

    def _parse_class(self, node):
        c = XmlClass()
        self._parse_context(node, c)
        c.c_name = node.attrib.get("name")
        self.classes.setdefault(c.id, c)

    def _get_def(self, file, line, name):
        """
        Returns python name and doc-string from the LOLPIG_DEF macro.
        line expected to point at the beginning of the function/struct body
        :return: tuple
        """
        if file.do_scan:
            file.do_scan = False
            if file.name.startswith("<") or file.name.startswith("/"):
                return None, None
            with open(file.name) as f:
                file.content = f.read()
            if not "LOLPIG_DEF" in file.content:
                file.content = None
            else:
                file.lines = file.content.split("\n")

        if not file.content:
            return None, None
        if line < 1 or line >= len(file.lines):
            raise ParseError("line number %d out of range" % line)

        endline = line-1
        if file.lines[endline].strip() == "{":
            endline -= 1

        # find next DEF before given line
        while "LOLPIG_DEF(" not in file.lines[line]:
            line -= 1
            if line <= 0:
                return None, None
        txt = ""
        for i in range(line, endline):
            txt += file.lines[i] + "\n"

        # scan first part of DEF for proper syntax
        import re
        match = None
        for i in re.finditer(r"LOLPIG_DEF\([\s]*([A-Za-z0-9_\.]*)[\w]*,", txt):
            match = i
            break
        if not match or not match.groups():
            return None, None

        # see if this DEF actually belongs to the object
        # by making sure that only whitespace follows after end of macro
        docpos = match.span()[1]
        endpos = len(txt)-1
        num_brack = 1
        for i in range(docpos, len(txt)):
            if txt[i] == "(":
                num_brack += 1
            elif txt[i] == ")":
                num_brack -= 1
            if num_brack == 0:
                endpos = i
                break
        from .renderer import is_whitespace
        for i in range(endpos, len(txt)):
            if not (is_whitespace(txt[i]) or txt[i] == ")"):
                return None, None

        py_name = match.groups()[0]
        py_doc = txt[match.span()[1]:].strip()
        if py_doc.endswith(")"):
            py_doc = py_doc[:-1]
        return py_name, py_doc.strip()
