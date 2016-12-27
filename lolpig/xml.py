"""
GCC-XML parser
"""

class ParseError(BaseException):
    pass

class XmlType:
    id = None
    c_name = ""
    size = 0
    array_min = 0
    array_max = 0
    ref_id = None
    ref = None
    is_fundamental = True
    is_pointer = False
    is_const = False
    is_reference = False
    is_array = False

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


class XmlStruct:
    id = None
    c_name = None
    fields = []


class XmlField:
    id = None
    c_name = None
    line = 0
    type = None
    type_id = None


class XmlFunction:
    id = None
    c_name = ""
    py_name = ""
    py_doc = ""
    line = 0
    endline = 0

    def __str__(self):
        return "XmlFunction(%s, %s)" % (self.c_name, self.py_name)


class XmlParser:
    def __init__(self):
        self.filename = ""
        self.lines = []
        self.types = dict()
        self.structs = dict()
        self.functions = dict()
        self.fields = dict()

    def parse(self, filename):
        self._parse(filename)
        self._resolve_types()
        self.dump()

    def dump(self):
        print("types", self.types)
        print("functions", self.functions)

    def has_object(self, id):
        return id in self.types or id in self.structs or id in self.functions

    def get_object(self, id, default = None):
        if id in self.types:
            return self.types[id]
        if id in self.structs:
            return self.structs[id]
        if id in self.functions:
            return self.functions[id]
        return default

    def pos_str(self, line):
        return "%s:%d" % (self.filename, line)

    def _resolve_types(self):
        # resolve referenced types
        for key in self.types:
            t = self.types[key]
            if t.ref_id:
                t.is_fundamental = False
                if not self.has_object(t.ref_id):
                    raise ParseError("Unknown reference type id %s in %s" % (t.ref_id, t))
                t.ref = self.get_object(t.ref_id)
        # fill c_name field for all types
        for key in self.types:
            t = self.types[key]
            if not t.c_name:
                r = t.ref
                while not r.c_name:
                    r = r.ref
                t.c_name = r.c_name
        # resolve types for struct fields
        for key in self.fields:
            f = self.fields[key]
            if f.type_id:
                if not self.has_object(f.type_id):
                    raise ParseError("Unknown referenced field id %s in %s" % (f.type_id, f))
                f.type = self.get_object(f.type_id)

    def _parse(self, filename):
        import subprocess, os

        xmlname = filename + "_temp_.xml"
        subprocess.call(["gccxml", filename, "-fxml=%s" % xmlname])

        with open(filename) as f:
            self.lines = f.read().split("\n")

        import xml.etree.ElementTree as ET
        tree = ET.parse(xmlname)
        self._parse_xml(tree.getroot())
        # os.remove(xmlname)

        self.filename = filename

    def _parse_xml(self, root):

        for child in root:
            print(child.tag)
            if child.tag == "Function":
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
            elif child.tag == "Struct":
                self._parse_struct(child)
            elif child.tag == "Field":
                self._parse_field(child)

    def _parse_fundamental_type(self, node):
        t = XmlType()
        t.id = node.attrib.get("id")
        t.c_name = node.attrib.get("name")
        t.size = int(node.attrib.get("size", 0))
        self.types.setdefault(t.id, t)

    def _parse_pointer_type(self, node):
        t = XmlType()
        t.id = node.attrib.get("id")
        t.ref_id = node.attrib.get("type")
        t.size = int(node.attrib.get("size", 0))
        t.is_pointer = True
        self.types.setdefault(t.id, t)

    def _parse_reference_type(self, node):
        t = XmlType()
        t.id = node.attrib.get("id")
        t.ref_id = node.attrib.get("type")
        t.size = int(node.attrib.get("size", 0))
        t.is_reference = True
        self.types.setdefault(t.id, t)

    def _parse_array_type(self, node):
        t = XmlType()
        t.id = node.attrib.get("id")
        t.ref_id = node.attrib.get("type")
        t.size = int(node.attrib.get("size", 0))
        t.array_min = int(node.attrib.get("min", "0").replace("u", ""))
        t.array_max = int(node.attrib.get("max", "0").replace("u", ""))
        t.is_array = True
        self.types.setdefault(t.id, t)

    def _parse_cv_type(self, node):
        t = XmlType()
        t.id = node.attrib.get("id")
        t.ref_id = node.attrib.get("type")
        t.size = int(node.attrib.get("size", 0))
        t.is_const = True
        self.types.setdefault(t.id, t)

    def _parse_struct(self, node):
        s = XmlStruct()
        s.id = node.attrib.get("id")
        s.c_name = node.attrib.get("name")
        s.line = node.attrib.get("line")
        self.structs.setdefault(s.id, s)

    def _parse_function(self, node):
        if not "attributes" in node.attrib:
            return
        if "gccxml(cpy)" not in node.attrib["attributes"]:
            return

        func = XmlFunction()
        func.id = node.attrib.get("id")
        func.c_name = node.attrib.get("name")
        func.line = int(node.attrib.get("line", 0))
        func.endline = int(node.attrib.get("endline", func.line))
        func.py_name, func.py_doc = self._get_def(func.line, func.c_name)
        self.functions.setdefault(func.id, func)

    def _parse_field(self, node):
        field = XmlField()
        field.id = node.attrib.get("id")
        field.line = int(node.attrib.get("line", 0))
        field.type_id = node.attrib.get("type")
        self.fields.setdefault(field.id, field)

    def _get_def(self, line, name):
        """
        Returns python name and doc-string from the CPY_DEF macro.
        line expected to point at the beginning of the function/struct body
        :return: tuple
        """
        if line < 1 or line >= len(self.lines):
            raise ParseError("line number %d out of range" % line)
        endline = line-1
        while "CPY_DEF(" not in self.lines[line]:
            line -= 1
            if line <= 0:
                raise ParseError("CPY_DEF not found for %s" % name)
        txt = ""
        for i in range(line, endline):
            txt += self.lines[i] + "\n"
        import re
        match = None
        for i in re.finditer(r"CPY_DEF\([\s]*([A-Za-z0-9_\.]*)[\w]*,", txt):
            match = i
            break
        if not match or not match.groups():
            raise ParseError("Can not detect python name in CPY_DEF for %s in %s" % (name, self.pos_str(line)))
        py_name = match.groups()[0]
        py_doc = txt[match.span()[1]:].strip()
        if py_doc.endswith(")"):
            py_doc = py_doc[:-1]
        return py_name, py_doc.strip()
