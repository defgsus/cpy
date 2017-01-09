
class Arguments:
    def __init__(self):
        self.ok = False
        self.error_txt = ""
        self.module_name = "module"
        self.output_cpp = ""
        self.output_hpp = ""
        self.header_inc = ""
        self.input_filenames = []
        self.namespaces = []
        self.is_export = False
        self.is_doxygen = False

    def dump(self):
        print("""
mode:       %s
input:      %s
module:     %s
output:     %s%s %s
namespaces: %s
        """ % ("python -> cpp" if self.is_export else "cpp -> cpp",
               str(self.input_filenames),
               self.module_name,
               self.output_hpp + " " if not self.is_export else "",
               self.output_cpp,
               '(#include "%s")' % self.header_inc if not self.is_export else "",
               self.namespaces
               ))

    def help(self):
        print("""
Usage: lolpig.py [-export] -i files -o file [-m modulename] [-n namespaces]
""")

    def verify(self):
        self.ok = True
        self.error_txt = ""
        if not self.output_cpp or not self.output_hpp:
            self.error("No output file specified (-o)")
        if not self.input_filenames:
            self.error("No input files specified (-i)")
        return self.ok

    def error(self, txt):
        if self.error_txt:
            self.error_txt += "\n"
        self.error_txt += txt
        self.ok = False

    def parse(self, argv=None):
        if not argv:
            import sys
            argv = sys.argv
        param = [("-i", -1), ("-n", -1), ("-o", 1), ("-m", 1), ("-export", 0), ("-doxygen", 0)]
        expect = ""
        expect_len = 0
        arg_cnt = 0
        for a in argv[1:]:

            # break unlimited params on next command switch
            if expect_len < 0 and a.startswith("-") and not a == "-":
                expect = ""

            if expect:
                if expect == "-export":
                    self.is_export = True
                elif expect == "-doxygen":
                    self.is_doxygen = True
                elif expect == "-i":
                    self.input_filenames.append(a)
                elif expect == "-n":
                    self.namespaces.append(a)
                elif expect == "-o":
                    self.set_output_name(a)
                elif expect == "-m":
                    self.module_name = a
                else:
                    raise NotImplementedError("unimplemented switch %s" % expect)

                arg_cnt += 1
                if expect_len >= 0 and arg_cnt >= expect_len:
                    expect = ""
                if not expect_len == 0:
                    continue

            if not expect:
                for cmd, le in param:
                    if a == cmd:
                        expect = cmd
                        expect_len = le
                        arg_cnt = 0
                        break
                else:
                    self.error("Unknown command '%s'" % a)
                    return False
        return self.verify()

    def set_output_name(self, n):
        self.output_cpp = n + ".cpp"
        self.output_hpp = n + ".h"
        import os
        self.header_inc = os.path.basename(self.output_hpp)


def _get_gcc_xml(filenames):
    from liblolpig import XmlParser, Context

    ctx = Context()

    for fn in filenames:
        print("parsing %s ..." % fn)
        p = XmlParser()
        p.parse(fn)
        #p.dump()
        c = p.as_context()
        #c.dump()
        ctx.merge(c)
    ctx.finalize()
    return ctx

def _get_doxygen(filenames):
    from liblolpig import DoxygenParser
    p = DoxygenParser()
    p.parse_files(filenames)

    return p.as_context()


def _render_module(a):
    """Renders the module code from scanning cpp files"""
    from liblolpig import Renderer

    if a.is_doxygen:
        ctx = _get_doxygen(a.input_filenames)
    else:
        ctx = _get_gcc_xml(a.input_filenames)

    ctx.module_name = a.module_name
    ctx.header_name = a.header_inc
    ctx.dump()

    r = Renderer(ctx)
    r.namespaces = a.namespaces
    r.write_to_file(a.output_hpp, r.render_hpp())
    r.write_to_file(a.output_cpp, r.render_cpp())


def _render_export(a):
    """Renders cpp stub from python code"""
    from liblolpig.export import scan_module_files
    from liblolpig import Renderer
    ctx = scan_module_files(a.input_filenames)

    r = Renderer(ctx)
    r.namespaces = a.namespaces
    r.write_to_file(a.output_cpp, r.render_export())


def process_commands(argv=None):
    if not argv:
        import sys
        argv = sys.argv

    #-export -i example/export/stuff.py -o example/export/pydef -n MO PYTHON
    #s = "-i vector/pyimpl/vec_base.cpp vector/pyimpl/mat_base.cpp vector/pyimpl/vec3.cpp -o vector/vec_module -n MOP -m vec"
    #s = "-doxygen -i test_doxygen/code/vec_base.h -o test_doxygen/gen/vec_module -n MOP -m vec"
    s = "-doxygen -i example/test.cpp -o example/test_module -m module"
    argv = ["lolpig.py", ] + s.split()


    a = Arguments()
    if not a.parse(argv) or not a.verify():
        a.help()
        print(a.error_txt)
        exit(1)

    a.dump()

    if a.is_export:
        _render_export(a)
    else:
        _render_module(a)


if __name__ == "__main__":
    process_commands()