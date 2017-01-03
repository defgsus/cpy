"""
Collection of lists and dicts representing types in the python C-API
"""

"""
CPython's function pointers as dict:
typename: (return_type, (args,))
"""
FUNCTIONS = {
    "unaryfunc":            ("PyObject*",   ("PyObject*",)),
    "binaryfunc":           ("PyObject*",   ("PyObject*", "PyObject*")),
    "ternaryfunc":          ("PyObject*",   ("PyObject*", "PyObject*", "PyObject*")),
    "inquiry":              ("int",         ("PyObject*",)),
    "lenfunc":              ("Py_ssize_t",  ("PyObject*",)),
    "ssizeargfunc":         ("PyObject*",   ("PyObject*", "Py_ssize_t")),
    "ssizessizeargfunc":    ("PyObject*",   ("PyObject*", "Py_ssize_t", "Py_ssize_t")),
    "ssizeobjargproc":      ("int",         ("PyObject*", "Py_ssize_t", "PyObject*")),
    "ssizessizeobjargproc": ("int",         ("PyObject*", "Py_ssize_t", "Py_ssize_t", "PyObject*")),
    "objobjargproc":        ("int",         ("PyObject*", "PyObject*", "PyObject*")),
    "freefunc":             ("void",        ("void*",)),
    "destructor":           ("void",        ("PyObject*",)),
    "printfunc":            ("int",         ("PyObject*", "FILE*", "int")),
    "getattrfunc":          ("PyObject*",   ("PyObject*", "char*")),
    "getattrofunc":         ("PyObject*",   ("PyObject*", "PyObject*")),
    "setattrfunc":          ("int",         ("PyObject*", "char*", "PyObject*")),
    "setattrofunc":         ("int",         ("PyObject*", "PyObject*", "PyObject*")),
    "reprfunc":             ("PyObject*",   ("PyObject*",)),
    "hashfunc":             ("Py_hash_t",   ("PyObject*",)),
    "richcmpfunc":          ("PyObject*",   ("PyObject*", "PyObject*", "int")),
    "getiterfunc":          ("PyObject*",   ("PyObject*",)),
    "iternextfunc":         ("PyObject*",   ("PyObject*",)),
    "descrgetfunc":         ("PyObject*",   ("PyObject*", "PyObject*", "PyObject*")),
    "descrsetfunc":         ("int",         ("PyObject*", "PyObject*", "PyObject*")),
    "initproc":             ("int",         ("PyObject*", "PyObject*", "PyObject*")),
    "newfunc":              ("PyObject*",   ("_typeobject*", "PyObject*", "PyObject*")),
    "allocfunc":            ("PyObject*",   ("_typeobject*", "Py_ssize_t")),

    "getter":               ("PyObject*",   ("PyObject*", "void*")),
    "setter":               ("int",         ("PyObject*", "PyObject*", "void*")),
    "objobjproc":           ("int",         ("PyObject*", "PyObject*")),
    "visitproc":            ("int",         ("PyObject*", "void*")),
    "traverseproc":         ("int",         ("PyObject*", "visitproc", "void*")),
}


"""
All members of PyTypeObject (member_name, type)
"""
PyTypeObject = [
    ("tp_name",             "const char*"),
    ("tp_basicsize",        "Py_ssize_t"),
    ("tp_itemsize",         "Py_ssize_t"),
    ("tp_dealloc",          "destructor"),
    ("tp_print",            "printfunc"),
    ("tp_getattr",          "getattrfunc"),
    ("tp_setattr",          "setattrfunc"),
    ("tp_reserved",         "void*"),
    ("tp_repr",             "reprfunc"),
    ("tp_as_number",        "PyNumberMethods*"),
    ("tp_as_sequence",      "PySequenceMethods*"),
    ("tp_as_mapping",       "PyMappingMethods*"),
    ("tp_hash",             "hashfunc"),
    ("tp_call",             "ternaryfunc"),
    ("tp_str",              "reprfunc"),
    ("tp_getattro",         "getattrofunc"),
    ("tp_setattro",         "setattrofunc"),
    ("tp_as_buffer",        "PyBufferProcs*"),
    ("tp_flags",            "unsigned long"),
    ("tp_doc",              "const char*"),
    ("tp_traverse",         "traverseproc"),
    ("tp_clear",            "inquiry"),
    ("tp_richcompare",      "richcmpfunc"),
    ("tp_weaklistoffset",   "Py_ssize_t"),
    ("tp_iter",             "getiterfunc"),
    ("tp_iternext",         "iternextfunc"),
    ("tp_methods",          "struct PyMethodDef*"),
    ("tp_members",          "struct PyMemberDef*"),
    ("tp_getset",           "struct PyGetSetDef*"),
    ("tp_base",             "struct _typeobject*"),
    ("tp_dict",             "PyObject*"),
    ("tp_descr_get",        "descrgetfunc"),
    ("tp_descr_set",        "descrsetfunc"),
    ("tp_dictoffset",       "Py_ssize_t"),
    ("tp_init",             "initproc"),
    ("tp_alloc",            "allocfunc"),
    ("tp_new",              "newfunc"),
    ("tp_free",             "freefunc"),
    ("tp_is_gc",            "inquiry"),
    ("tp_bases",            "PyObject*"),
    ("tp_mro",              "PyObject*"),
    ("tp_cache",            "PyObject*"),
    ("tp_subclasses",       "PyObject*"),
    ("tp_weaklist",         "PyObject*"),
    ("tp_del",              "destructor"),
    ("tp_version_tag",      "unsigned int"),
    ("tp_finalize",         "destructor"),
]

PyNumberMethods = [
    ("nb_add",                  "binaryfunc"),
    ("nb_subtract",             "binaryfunc"),
    ("nb_multiply",             "binaryfunc"),
    ("nb_remainder",            "binaryfunc"),
    ("nb_divmod",               "binaryfunc"),
    ("nb_power",                "ternaryfunc"),
    ("nb_negative",             "unaryfunc"),
    ("nb_positive",             "unaryfunc"),
    ("nb_absolute",             "unaryfunc"),
    ("nb_bool",                 "inquiry"),
    ("nb_invert",               "unaryfunc"),
    ("nb_lshift",               "binaryfunc"),
    ("nb_rshift",               "binaryfunc"),
    ("nb_and",                  "binaryfunc"),
    ("nb_xor",                  "binaryfunc"),
    ("nb_or",                   "binaryfunc"),
    ("nb_int",                  "unaryfunc"),
    ("nb_reserved",             "void*"),
    ("nb_float",                "unaryfunc"),
    ("nb_inplace_add",          "binaryfunc"),
    ("nb_inplace_subtract",     "binaryfunc"),
    ("nb_inplace_multiply",     "binaryfunc"),
    ("nb_inplace_remainder",    "binaryfunc"),
    ("nb_inplace_power",        "ternaryfunc"),
    ("nb_inplace_lshift",       "binaryfunc"),
    ("nb_inplace_rshift",       "binaryfunc"),
    ("nb_inplace_and",          "binaryfunc"),
    ("nb_inplace_xor",          "binaryfunc"),
    ("nb_inplace_or",           "binaryfunc"),
    ("nb_floor_divide",         "binaryfunc"),
    ("nb_true_divide",          "binaryfunc"),
    ("nb_inplace_floor_divide", "binaryfunc"),
    ("nb_inplace_true_divide",  "binaryfunc"),
    ("nb_index",                "unaryfunc"),
]

PySequenceMethods = [
    ("sq_length",           "lenfunc"),
    ("sq_concat",           "binaryfunc"),
    ("sq_repeat",           "ssizeargfunc"),
    ("sq_item",             "ssizeargfunc"),
    ("was_sq_slice",        "void*"),
    ("sq_ass_item",         "ssizeobjargproc"),
    ("was_sq_ass_slice",    "void*"),
    ("sq_contains",         "objobjproc"),
    ("sq_inplace_concat",   "binaryfunc"),
    ("sq_inplace_repeat",   "ssizeargfunc"),
]

PyMappingMethods = [
    ("mp_length",           "lenfunc"),
    ("mp_subscript",        "binaryfunc"),
    ("mp_ass_subscript",    "objobjargproc"),
]

PyBufferProcs = [
    ("bf_getbuffer",        "getbufferproc"),
    ("bf_releasebuffer",    "releasebufferproc"),
]

PyModuleDef = [
    ("m_name",      "const char*"),
    ("m_doc",       "const char*"),
    ("m_size",      "Py_ssize_t"),
    ("m_methods",   "PyMethodDef*"),
    ("m_reload",    "inquiry"),
    ("m_traverse",  "traverseproc"),
    ("m_clear",     "inquiry"),
    ("m_free",      "freefunc"),
]

SEQUENCE_FUNCS = [
    ("__len__",         "sq_length"),
    ("__concat__",      "sq_concat"),
    ("__repeat__",      "sq_repeat"),
    ("__getitem__",     "sq_item"),
    ("__???__",         "was_sq_slice"),
    ("__setitem__",     "sq_ass_item"),
    ("__???__",         "was_sq_ass_slice"),
    ("__contains__",    "sq_contains"),
    ("__iconcat__",     "sq_inplace_concat"),
    ("__irepeat__",     "sq_inplace_repeat"),
]

NUMBER_FUNCS = [
    ("__add__",         "nb_add"),
    ("__sub__",         "nb_subtract"),
    ("__mul__",         "nb_multiply"),
    ("__mod__",         "nb_remainder"),
    ("__divmod__",      "nb_divmod"),
    ("__pow__",         "nb_power"),
    ("__neg__",         "nb_negative"),
    ("__pos__",         "nb_positive"),
    ("__abs__",         "nb_absolute"),
    ("__bool__",        "nb_bool"),
    ("__invert__",      "nb_invert"),
    ("__lshift__",      "nb_lshift"),
    ("__rshift__",      "nb_rshift"),
    ("__and__",         "nb_and"),
    ("__xor__",         "nb_xor"),
    ("__or__",          "nb_or"),
    ("__int__",         "nb_int"),
    # ("__???__",         "nb_reserved"),
    ("__float__",       "nb_float"),
    ("__iadd__",        "nb_inplace_add"),
    ("__isub__",        "nb_inplace_subtract"),
    ("__imul__",        "nb_inplace_multiply"),
    ("__imod__",        "nb_inplace_remainder"),
    ("__ipow__",        "nb_inplace_power"),
    ("__ilshift__",     "nb_inplace_lshift"),
    ("__irshift__",     "nb_inplace_rshift"),
    ("__iand__",        "nb_inplace_and"),
    ("__ixor__",        "nb_inplace_xor"),
    ("__ior__",         "nb_inplace_or"),
    ("__floordiv__",    "nb_floor_divide"),
    ("__truediv__",     "nb_true_divide"),
    ("__ifloordiv__",   "nb_inplace_floor_divide"),
    ("__itruediv__",    "nb_inplace_true_divide"),
    ("__index__",       "nb_index"),
]

# https://docs.python.org/3/reference/datamodel.html#basic-customization
# https://docs.python.org/3/c-api/typeobj.html#type-objects
TYPE_FUNCS = [
    ("__new__",         "tp_new"),
    ("__init__",        "tp_init"),
    ("__del__",         "tp_del"),
    ("__str__",         "tp_str"),
    ("__unicode__",     "tp_str"),
    ("__repr__",        "tp_repr"),
    ("__lt__",          "tp_richcompare"),
    ("__le__",          "tp_richcompare"),
    ("__eq__",          "tp_richcompare"),
    ("__ne__",          "tp_richcompare"),
    ("__gt__",          "tp_richcompare"),
    ("__ge__",          "tp_richcompare"),
    ("__hash__",        "tp_hash"),
    ("__getattr__",     "tp_getattr"),
    ("__setattr__",     "tp_setattr"),
    ("__getattro__",    "tp_getattro"),
    ("__setattro__",    "tp_setattro"),
    ("__call__",        "tp_call"),
    ("__iter__",        "tp_iter"),
    ("__next__",        "tp_iternext"),
    # these are unofficial
    ("__dealloc__",     "tp_dealloc"),
    ("__finalize__",    "tp_finalize"),
]

ALL_FUNCS = TYPE_FUNCS + SEQUENCE_FUNCS + NUMBER_FUNCS


SPECIAL_FUNCS = [
    ("__floor__",       "unaryfunc"),
    ("__ceil__",        "unaryfunc"),
    ("__complex__",     "unaryfunc"),
]


FUNCNAME_TO_STRUCT_MEMBER = dict()
for i in ALL_FUNCS:
    FUNCNAME_TO_STRUCT_MEMBER.setdefault(i[0], i[1])

STRUCT_MEMBER_TO_TYPE = dict()
for i in PyModuleDef + PyBufferProcs + PyMappingMethods + PySequenceMethods + PyNumberMethods + PyTypeObject:
    STRUCT_MEMBER_TO_TYPE.setdefault(i[0], i[1])

FUNCNAME_TO_TYPE = dict()
for i in FUNCNAME_TO_STRUCT_MEMBER:
    mem = FUNCNAME_TO_STRUCT_MEMBER.get(i)
    if mem in STRUCT_MEMBER_TO_TYPE:
        FUNCNAME_TO_TYPE.setdefault(i, STRUCT_MEMBER_TO_TYPE[mem])