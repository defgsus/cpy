
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
lolpig.py -i files -o file [-m modulename] [-n namespaces]
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



if __name__ == "__main__":
    para = ["/pythonprog/lolpig.py", "-i", "cpp/rein1.cpp", "cpp/rein2.cpp",
            "-o", "gen/raus",
            "-n", "OUT", "SPACE",
            "-m", "das_mordul"]

    a = Arguments()
    if not a.parse(para):
        print(a.error_txt)
    if not a.verify():
        print(a.error_txt)

    a.dump()