#!/bin/python3
import lolpig


if __name__ == "__main__":

    p = lolpig.XmlParser()
    p.parse(["./vector/vec_base.cpp",])
    #p.dump()

    c = p.as_context()
    c.module_name = "vec"
    c.header_name = "vec_module.h"
    c.dump()

    r = lolpig.Renderer(c)
    r.namespaces = ["MOP"]
    r.write_to_file("./vector/vec_module.h", r.render_hpp())
    r.write_to_file("./vector/vec_module.cpp", r.render_cpp())



