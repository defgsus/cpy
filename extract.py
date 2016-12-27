import lolpig


if __name__ == "__main__":

    p = lolpig.XmlParser()
    p.parse("./test.cpp")
    #p.dump()

    c = p.as_context()
    #c.dump()

    r = lolpig.Renderer(c)
    print(r.render_cpp())



