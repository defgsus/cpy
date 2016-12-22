"""
dexml models for parsing gcc-xml's output
"""
import xml



"""
class XmlFundamentalType(dexml.Model):
    class meta:
        tagname = "FundamentalType"
    id = dexml.fields.String()
    name = dexml.fields.String()
    size = dexml.fields.Integer(required=False)


class XmlArgument(dexml.Model):
    class meta:
        tagname = "Argument"
    name = dexml.fields.String(required=False)
    type = dexml.fields.String()

class XmlFunction(dexml.Model):
    class meta:
        tagname = "Function"
    id = dexml.fields.String()
    name = dexml.fields.String()
    returns = dexml.fields.String()
    context = dexml.fields.String()
    file = dexml.fields.String()
    line = dexml.fields.String()
    endline = dexml.fields.String(required=False)
    arguments = dexml.fields.List(XmlArgument)
    attributes = dexml.fields.String(required=False)


class XmlStruct(dexml.Model):
    class meta:
        tagname = "Struct"
    id = dexml.fields.String()
    name = dexml.fields.String()
    context = dexml.fields.String()
    line = dexml.fields.String()
    members = dexml.fields.String(required=False)



class XmlFile(dexml.Model):
    class meta:
        tagname = "File"
    id = dexml.fields.String()
    name = dexml.fields.String()

class Xml(dexml.Model):
    class meta:
        tagname = "GCC_XML"
        ignore_unknown_elements = True
    cvs_revision = dexml.fields.String()
    structs = dexml.fields.List(XmlStruct)
    fundamental_types = dexml.fields.Dict(XmlFundamentalType, key="id")
    functions = dexml.fields.Dict(XmlFunction, key="id")
    files = dexml.fields.List(XmlFile)
"""