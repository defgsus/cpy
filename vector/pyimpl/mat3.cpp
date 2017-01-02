#include "mat_base.h"

using namespace PyUtils;

namespace MO {
namespace PYTHON {

extern "C" {


LOLPIG_DEF( mat3.__new__, )
PyObject* mat3_new(struct _typeobject* type, PyObject* args, PyObject* )
{
    Matrix33* vec = PyObject_NEW(Matrix33, type);
    vec->alloc(3*3);
    vec->num_rows = vec->num_cols = 3;

    if (isEmpty(args))
    {
        vec->setIdentity(1.);
        return reinterpret_cast<PyObject*>(vec);
    }
    double val;
    if (fromPython(removeArgumentTuple(args), &val))
    {
        vec->setIdentity(val);
        return reinterpret_cast<PyObject*>(vec);
    }

    int len = VectorBase::parseSequence(args, vec->v, vec->len);
    if (len<0)
    {
        Py_DECREF(vec);
        return NULL;
    }
    return reinterpret_cast<PyObject*>(vec);
}

/*
LOLPIG_DEF( mat3.__init__, )
int mat3_init(PyObject* self, PyObject* args, PyObject* )
{
    Matrix33* vec = reinterpret_cast<Matrix33*>(self);
    if (VectorBase::parseSequence(args, vec->v, 3) == 0)
        return -1;
    vec->len = 3;
    return 0;
}*/


LOLPIG_DEF( mat3.__repr__, )
PyObject* mat3_repr(PyObject* self)
{
    Matrix33* vec = reinterpret_cast<Matrix33*>(self);
    return toPython(vec->toRepr("mat3"));
}

LOLPIG_DEF( mat3.__str__, )
PyObject* mat3_str(PyObject* self)
{
    Matrix33* vec = reinterpret_cast<Matrix33*>(self);
    return toPython(vec->toString("mat3"));
}



// ----------------- transformation setter ----------------

LOLPIG_DEF( mat3.set_rotate_x, (
    set_rotate_x(degree) -> self
    Initializes the matrix with a rotation transform, INPLACE
    >>> mat3().set_rotate_x(90).round()
    mat3(1,0,0, 0,0,1, 0,-1,0)
    ))
PyObject* mat3_set_rotate_x(PyObject* self, PyObject *obj)
{
    double degree;
    if (!expectFromPython(obj, &degree))
        return NULL;
    Matrix33* mat = reinterpret_cast<Matrix33*>(self);
    degree *= DEG_TO_TWO_PI;
    double  sa = std::sin(degree), ca = std::cos(degree);
    mat->setIdentity();
    mat->v[4] = ca;
    mat->v[5] = sa;
    mat->v[7] = -sa;
    mat->v[8] = ca;

    Py_RETURN_SELF;
}

} // extern "C"

} // namespace PYTHON
} // namespace MO

