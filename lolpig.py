#!/bin/python3
import lolpig


if __name__ == "__main__":

    ctx = lolpig.Context()

    for fn in ["./vector/vec_base.cpp", "./vector/vec3.cpp"]:
        p = lolpig.XmlParser()
        p.parse(fn)
        #p.dump()
        ctx.merge(p.as_context())
    ctx.finalize()

    ctx.module_name = "vec"
    ctx.header_name = "vec_module.h"
    ctx.dump()

    r = lolpig.Renderer(ctx)
    r.namespaces = ["MOP"]
    r.write_to_file("./vector/vec_module.h", r.render_hpp())
    r.write_to_file("./vector/vec_module.cpp", r.render_cpp())



