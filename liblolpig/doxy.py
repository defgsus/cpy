"""
Doxygen-XML parser
"""
import os, subprocess, tempfile
from xml.etree import ElementTree as ET

class ParseError(BaseException):
    pass

class XmlContext:
    def __init__(self):
        self.id = None
        self.context_id = None
        self.context = None
        self.location = ("", 0)

    def get_namespace_list(self):
        n = []
        if self.context:
            if hasattr(self.context, "c_name"):
                n.append(self.context.c_name)
            n = self.context.get_namespace_list() + n
        return n


class XmlNamespace(XmlContext):
    def __init__(self):
        super().__init__()
        self.c_name = None

    def __str__(self):
        return "XmlNamespace(%s)" % self.c_name

    def __repr__(self):
        return self.__str__()


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
        c.file = self.location[0]
        c.line = self.location[1]
        c.struct_size = self.size
        c.namespaces = self.get_namespace_list()
        return c



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
        pyname = self.py_name.split("@")
        f.c_name = self.c_name
        f.py_name = pyname[0]
        if len(pyname) > 1:
            f.is_property = pyname[1] == "get" or pyname[1] == "set"
            f.is_setter = pyname[1] == "set"
        f.py_doc = self.py_doc
        f.file = self.location[0]
        f.line = self.location[1]
        f.namespaces = self.get_namespace_list()
        f.c_return_type = self.return_type
        for arg in self.arguments:
            a = Argument()
            a.c_type, a.c_name = arg
            f.arguments.append(a)

        return f


class DoxygenParser:
    def __init__(self):
        self.filenames = []
        self.group_names = ["python", "lolpig"]

        self.types = dict()
        self.structs = dict()
        self.functions = dict()
        self.fields = dict()
        self.namespaces = dict()
        self.files = dict()
        self.classes = dict()

        self.error_stack = []
        self.py_tag_open = "%LOLPIG{{{%"
        self.py_tag_close = "%LOLPIG}}}%"

    def push_stack(self, str):
        self.error_stack.append(str)

    def pop_stack(self):
        if self.error_stack:
            self.error_stack = self.error_stack[:-1]

    def error(self, str):
        trace = ""
        for i, e in enumerate(self.error_stack):
            trace += " " * i + e + ":\n"
        raise ParseError("\n" + trace + str)

    def parse_files(self, filenames):
        self._parse(filenames)
        #self._resolve_types()
        #self._resolve_context(self.namespaces.values())
        #self._resolve_context(self.functions.values())
        #self._resolve_context(self.types.values())
        #self._resolve_context(self.structs.values())
        #self._resolve_context(self.fields.values())
        #self._find_lolpig_def(self.functions.values())
        #self._find_lolpig_def(self.structs.values())
        self.dump()

    def dump(self):
        print("namespaces", self.namespaces)
        print("types", self.types)
        print("structs", self.structs)
        print("functions", self.functions)

    def as_context(self):
        from .context import Context
        c = Context()
        c.filenames = [self.filenames]
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

    def _parse(self, filenames):
        """Generate doxygen-xml and parse it"""

        doxygen_config = """
        INPUT                  = %(filenames)s
        INPUT_ENCODING         = UTF-8
        RECURSIVE              = NO
        QUIET                  = YES
        WARNINGS               = NO
        WARN_IF_UNDOCUMENTED   = NO

        GENERATE_HTML          = NO
        GENERATE_LATEX         = NO
        GENERATE_XML           = YES
        XML_OUTPUT             = xml
        XML_PROGRAMLISTING     = NO

        DOXYFILE_ENCODING      = UTF-8
        PROJECT_NAME           = "My Project"
        # relative or absolute
        OUTPUT_DIRECTORY       = %(output_dir)s
        CREATE_SUBDIRS         = NO
        OUTPUT_LANGUAGE        = English
        BRIEF_MEMBER_DESC      = NO
        REPEAT_BRIEF           = YES
        EXTRACT_PRIVATE        = YES
        EXTRACT_LOCAL_METHODS  = YES
        EXTRACT_ANON_NSPACES   = YES
        EXTRACT_STATIC         = YES
        EXTRACT_PACKAGE        = YES
        HIDE_UNDOC_MEMBERS     = YES
        HIDE_UNDOC_CLASSES     = YES
        HIDE_IN_BODY_DOCS      = NO
        CASE_SENSE_NAMES       = NO
        HIDE_SCOPE_NAMES       = NO
        SHOW_INCLUDE_FILES     = YES
        SORT_MEMBER_DOCS       = NO
        SORT_BRIEF_DOCS        = NO
        SORT_MEMBERS_CTORS_1ST = NO
        SORT_GROUP_NAMES       = NO
        SORT_BY_SCOPE_NAME     = NO
        STRICT_PROTO_MATCHING  = NO
        GENERATE_TODOLIST      = NO
        GENERATE_TESTLIST      = NO
        GENERATE_BUGLIST       = NO
        GENERATE_DEPRECATEDLIST= NO

        ENABLE_PREPROCESSING   = YES
        MACRO_EXPANSION        = YES
        # If the SKIP_FUNCTION_MACROS tag is set to YES then doxygen's preprocessor will
        # remove all refrences to function-like macros that are alone on a line, have an
        # all uppercase name, and do not end with a semicolon. Such function macros are
        # typically used for boiler-plate code, and will confuse the parser if not
        # removed.
        # The default value is: YES.
        # This tag requires that the tag ENABLE_PREPROCESSING is set to YES.
        SKIP_FUNCTION_MACROS   = YES
        """

        with tempfile.TemporaryDirectory() as xml_dir:
            self.xml_dir = os.path.join(xml_dir, "xml")
            # create config file
            doxygen_config %= {
               "filenames": " ".join(filenames),
                "output_dir": xml_dir
            }
            conf_filename = os.path.join(xml_dir, "config_file.doxygen")
            self.push_stack("creating doxygen config")
            try:
                with open(conf_filename, "w", encoding="utf-8") as f:
                    f.write(doxygen_config)
            except IOError as e:
                self.error(str(e))
            self.pop_stack()
            # create doxygen output
            self.push_stack("doxygen call")
            try:
                subprocess.call(args=["doxygen", conf_filename])
            except subprocess.CalledProcessError as e:
                self.error(str(e))
            self.pop_stack()

            # parse doxygen output
            self.push_stack("reading doxygen output")
            try:
                for root, dirs, files in os.walk(xml_dir):
                    for f in files:
                        if f.startswith("group__") and f.endswith(".xml"):
                            group_name = f[7:f.index(".xml")]
                            if group_name in self.group_names:
                                self._parse_doxy_xml(os.path.join(root, f))
            except IOError as e:
                self.error(str(e))
            self.pop_stack()

            self.filenames = filenames

    def _parse_doxy_xml(self, filename):
        self.push_stack("scanning xml %s" % filename)
        root = ET.parse(filename).getroot()
        comp = root.find("compounddef")
        if not comp:
            self.error("No compounddef found in xml")
        kind = comp.attrib.get("kind", "")
        if kind == "struct":
            self._parse_struct(comp)
            return
        for child in comp:
            #print(child.tag)
            if child.tag == "innerclass":
                self._parse_innerclass(child)
            elif child.tag == "sectiondef":
                for mem in child:
                    if mem.tag == "memberdef":
                        self.push_stack("sectiondef.memberdef %s" % mem.get("id", "unknown"))
                        self._parse_section_member(mem)
                        self.pop_stack()
        self.pop_stack()

    def _parse_section_member(self, root):
        if not "kind" in root.attrib:
            self.error("Expected 'kind' attribute in section member %s" % root)
        kind = root.attrib["kind"]
        if kind == "function":
            self._parse_function(root)

    def _get_text(self, node):
        txt = node.text
        if txt is None:
            txt = ""
        for c in node:
            txt += self._get_text(c)
        if node.tail:
            txt += node.tail.strip()
        return txt

    def _get_sub_doc(self, node):
        py_doc = ""
        if node.text is not None:
            py_doc = node.text
        for c in node:
            doc = self._get_sub_doc(c)
            py_doc += doc
        if node.tag == "computeroutput":
            py_doc = self.py_tag_open + py_doc + self.py_tag_close
        if node.tail:
            py_doc += node.tail.strip()
        return py_doc

    def _get_doc(self, node):
        dnode = node.find("detaileddescription")
        if dnode is None:
            self.error("No <detaileddescription> in %s" % node)
        py_doc = self._get_sub_doc(dnode).strip()
        if not py_doc.startswith(self.py_tag_open):
            self.error("No python name defined in documentation for %s" % dnode)
        py_name = py_doc[len(self.py_tag_open):py_doc.find(self.py_tag_close)]
        py_doc = py_doc[py_doc.find(self.py_tag_close)+len(self.py_tag_close):-1]

        dnode = node.find("briefdescription")
        if dnode is not None:
            doc = self._get_sub_doc(dnode).strip()
            if doc:
                py_doc = doc + "\n" + py_doc

        return py_name, py_doc

    def _get_type(self, node):
        tnode = node.find("type")
        if tnode is None:
            self.error("Expected 'type' in %s" % node)
        return self._get_text(tnode).strip().replace(" ", "")

    def _get_location(self, node):
        lnode = node.find("location")
        if lnode is None:
            self.error("Location not found in %s" % node)
        return lnode.get("file", ""), lnode.get("line", 0)

    def _parse_function(self, node):
        o = XmlFunction()
        o.c_name = node.find("name").text
        self.push_stack("parsing function %s" % o.c_name)
        o.id = node.attrib["id"]
        o.return_type = self._get_type(node)
        o.py_name, o.py_doc = self._get_doc(node)
        o.location = self._get_location(node)
        self.push_stack("parsing arguments")
        for i in node.iterfind("param"):
            o.arguments.append((self._get_type(i), i.find("declname").text))
        self.pop_stack()
        self.pop_stack()
        self.functions.setdefault(o.id, o)

    def _parse_innerclass(self, node):
        self.push_stack("parse <innerclass>")
        refid = node.attrib.get("refid", None)
        if not refid:
            self.error("refid not found")
        self._parse_doxy_xml(os.path.join(self.xml_dir, "%s.xml" % refid))

    def _parse_struct(self, node):
        o = XmlStruct()
        o.id = node.attrib.get("id")
        o.c_name = node.find("compoundname").text
        self.push_stack("parsing struct %s" % o.c_name)
        o.py_name, o.py_doc = self._get_doc(node)
        o.location = self._get_location(node)
        self.pop_stack()
        self.structs.setdefault(o.id, o)


    """
    def _parse_context(self, node, ctx):
        ctx.id = node.attrib.get("id", None)
        ctx.line = int(node.attrib.get("line", 0))
        ctx.end_line = int(node.attrib.get("endline", ctx.line))
        ctx.context_id = node.attrib.get("context", None)
        ctx.file_id = node.attrib.get("file", None)
        ctx.mangled = node.attrib.get("mangled", None)

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
    """

    def _get_def(self, file, line, name):
        """
        Returns python name and doc-string from the LOLPIG_DEF macro.
        line expected to point at the beginning of the function/struct body
        :return: tuple
        """
        if file.do_scan:
            file.do_scan = False
            if file.name.startswith("<"):# or file.name.startswith("/")):
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
        for i in re.finditer(r"LOLPIG_DEF\([\s]*([A-Za-z0-9_\.@]*)[\w]*,", txt):
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
        while py_doc.endswith(")"):
            py_doc = py_doc[:-1].strip()
        while py_doc.startswith("("):
            py_doc = py_doc[1:].strip()
        from .renderer import change_text_indent
        py_doc = change_text_indent(py_doc, 0)
        return py_name, py_doc
