
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

    def dump(self):
        print("""
mode:       %s
input:      %s
module:     %s
output:     %s %s (#include "%s")
namespaces: %s
        """ % ("python -> cpp" if self.is_export else "cpp -> cpp",
               str(self.input_filenames),
               self.module_name,
               self.output_hpp, self.output_cpp, self.header_inc,
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
        param = [("-i", -1), ("-n", -1), ("-o", 1), ("-m", 1), ("-export", 0)]
        expect = ""
        expect_len = 0
        arg_cnt = 0
        for a in argv[1:]:

            if expect_len < 0 and a.startswith("-") and not a == "-":
                expect = ""

            if expect:
                if expect == "-export":
                    self.is_export = True
                elif expect == "-i":
                    self.input_filenames.append(a)
                elif expect == "-n":
                    self.namespaces.append(a)
                elif expect == "-o":
                    self.set_output_name(a)
                elif expect == "-m":
                    self.module_name = a

                arg_cnt += 1
                if expect_len >= 0 and arg_cnt > expect_len:
                    expect = ""

            if not expect:
                for cmd, le in param:
                    if a == cmd:
                        expect = cmd
                        expect_len = le
                        arg_cnt = 0
                        print(expect)
                        break;
                else:
                    self.error("Unknown command '%s'" % a)
                    return False
        return self.verify()

    def set_output_name(self, n):
        self.output_cpp = n + ".cpp"
        self.output_hpp = n + ".h"
        import os
        self.header_inc = os.path.basename(self.output_hpp)


def _render_module(a):
    """Renders the module code from scanning cpp files"""
    from liblolpig import XmlParser, Context, Renderer

    ctx = Context()

    for fn in a.input_filenames:
        print("parsing %s ..." % fn)
        p = XmlParser()
        p.parse(fn)
        #p.dump()
        c = p.as_context()
        #c.dump()
        ctx.merge(c)
    ctx.finalize()

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

    #argv = ["lolpig.py", "-export", "-i", "example/export/stuff.py", "-o", "example/export/pydef"]
    #-export -i example/export/stuff.py -o example/export/pydef -n MO PYTHON
    #-i vector/pyimpl/vec_base.cpp vector/pyimpl/mat_base.cpp vector/pyimpl/vec3.cpp -o vector/vec_module -n MOP -m vec

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