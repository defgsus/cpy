#include "vec_base.h"

using namespace PyUtils;

namespace MO {
namespace PYTHON {

extern "C" {

Vector3* copy_Vector3(Vector3* src)
{
    Vector3* dst = new_Vector3();
    dst->len = src->len;
    for (int i=0; i<src->len; ++i)
        dst->v[i] = src->v[i];
    return dst;
}


LOLPIG_DEF( vec3.__init__, )
int vec3_init(PyObject* self, PyObject* args, PyObject* )
{
    Vector3* vec = reinterpret_cast<Vector3*>(self);
    if (VectorBase::parseSequence(args, vec->v, 3) == 0)
        return -1;
    vec->len = 3;
    return 0;
}


LOLPIG_DEF( vec3.copy, )
PyObject* vec3_copy(PyObject* self)
{
    Vector3* vec = reinterpret_cast<Vector3*>(self);
    return reinterpret_cast<PyObject*>(copy_Vector3(vec));
}

LOLPIG_DEF( vec3.__repr__, )
PyObject* vec3_repr(PyObject* self)
{
    Vector3* vec = reinterpret_cast<Vector3*>(self);
    std::stringstream s;
    s << (void*)vec << ":" << vec->toString("vec3");
    return toPython(s.str());
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

