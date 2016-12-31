
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

    def dump(self):
        print("""
input:      %s
module:     %s
output:     %s %s (#include "%s")
namespaces: %s
        """ % (str(self.input_filenames),
               self.module_name,
               self.output_hpp, self.output_cpp, self.header_inc,
               self.namespaces
               ))

    def help(self):
        print("""
Usage: lolpig.py -i files -o file [-m modulename] [-n namespaces]
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
        expect = ""
        for a in argv[1:]:
            if a.startswith("-") and not a == "-":
                expect = ""

            if expect:
                if expect == "-i":
                    self.input_filenames.append(a)
                elif expect == "-n":
                    self.namespaces.append(a)
                elif expect == "-o":
                    self.set_output_name(a)
                    expect = ""
                elif expect == "-m":
                    self.module_name = a
                    expect = ""
            else:
                if a in ["-i", "-n", "-o", "-m"]:
                    expect = a
                else:
                    self.error("Unknown command '%s'" % a)
                    return False
        return self.verify()

    def set_output_name(self, n):
        self.output_cpp = n + ".cpp"
        self.output_hpp = n + ".h"
        import os
        self.header_inc = os.path.basename(self.output_hpp)


def process_commands(argv=None):
    if not argv:
        import sys
        argv = sys.argv

    a = Arguments()
    if not a.parse(argv) or not a.verify():
        a.help()
        print(a.error_txt)
        exit(1)

    a.dump()

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


if __name__ == "__main__":
    process_commands()