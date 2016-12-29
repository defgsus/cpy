#!/bin/python3
import lolpig


if __name__ == "__main__":

    p = lolpig.XmlParser()
    p.parse("./example/test.cpp")
    #p.dump()

    c = p.as_context()
    c.dump()

    r = lolpig.Renderer(c)
    r.write_to_file("./example/test_module.h", r.render_hpp())
    r.write_to_file("./example/test_module.cpp", r.render_cpp())



