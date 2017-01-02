#include "vec_base.h"

using namespace PyUtils;

namespace MO {
namespace PYTHON {

extern "C" {

LOLPIG_DEF( vec3.__new__, )
PyObject* vec3_new(struct _typeobject* type, PyObject* args, PyObject* )
{
    Vector3* vec = PyObject_NEW(Vector3, type);
    vec->alloc(3);
    int len = VectorBase::parseSequence(args, vec->v, vec->len);
    if (len<0)
    {
        Py_DECREF(vec);
        return NULL;
    }
    return reinterpret_cast<PyObject*>(vec);
}

/*
LOLPIG_DEF( vec3.__init__, )
int vec3_init(PyObject* self, PyObject* args, PyObject* )
{
    Vector3* vec = reinterpret_cast<Vector3*>(self);
    if (VectorBase::parseSequence(args, vec->v, 3) == 0)
        return -1;
    vec->len = 3;
    return 0;
}*/


LOLPIG_DEF( vec3.__repr__, )
PyObject* vec3_repr(PyObject* self)
{
    Vector3* vec = reinterpret_cast<Vector3*>(self);
    return toPython(vec->toRepr("vec3"));
}

LOLPIG_DEF( vec3.__str__, )
PyObject* vec3_str(PyObject* self)
{
    Vector3* vec = reinterpret_cast<Vector3*>(self);
    return toPython(vec->toString("vec3"));
}



} // extern "C"
	
} // namespace PYTHON
} // namespace MO

