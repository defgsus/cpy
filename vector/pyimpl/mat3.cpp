#include "mat_base.h"
#include "vector_math.h"

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
        return pyobject_cast<PyObject*>(vec);
    }
    double val;
    if (fromPython(removeArgumentTuple(args), &val))
    {
        vec->setIdentity(val);
        return pyobject_cast<PyObject*>(vec);
    }

    int len = VectorBase::parseSequence(args, vec->v, vec->len);
    if (len<0)
    {
        Py_DECREF(vec);
        return NULL;
    }
    return pyobject_cast<PyObject*>(vec);
}

/*
LOLPIG_DEF( mat3.__init__, )
int mat3_init(PyObject* self, PyObject* args, PyObject* )
{
    Matrix33* vec = pyobject_cast<Matrix33*>(self);
    if (VectorBase::parseSequence(args, vec->v, 3) == 0)
        return -1;
    vec->len = 3;
    return 0;
}*/


LOLPIG_DEF( mat3.__repr__, )
PyObject* mat3_repr(PyObject* self)
{
    Matrix33* vec = pyobject_cast<Matrix33*>(self);
    return toPython(vec->toRepr("mat3"));
}

LOLPIG_DEF( mat3.__str__, )
PyObject* mat3_str(PyObject* self)
{
    Matrix33* vec = pyobject_cast<Matrix33*>(self);
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
    Matrix33* mat = pyobject_cast<Matrix33*>(self);
    VEC::mat3_set_rotate_x_deg(mat->v, degree);
    Py_RETURN_SELF;
}


// -------------- transformation mult ----------------------

LOLPIG_DEF(mat3.rotate_x, (
        rotate_x(degree) -> self
        Adds a rotation to the current matrix, INPLACE
        >>> mat3().rotate_x(90).round()
        mat3(1,0,0, 0,0,1, 0,-1,0)
        ))
PyObject* mat3_rotate_x(PyObject* self, PyObject *obj)
{
    double degree;
    if (!expectFromPython(obj, &degree))
       return NULL;
    Matrix33* mat = pyobject_cast<Matrix33*>(self);
    double v[9];
    VEC::mat3_set_rotate_x_deg(v, degree);
    VEC::matnn_multiply_inplace(mat->v, v, 3);
    Py_RETURN_SELF;
}




// -------------- transformation mult copy ----------------------

LOLPIG_DEF(mat3.rotated_x, (
        rotated_x(degree) -> mat3
        Returns a rotated matrix
        >>> mat3().rotated_x(90).rounded()
        mat3(1,0,0, 0,0,1, 0,-1,0)
        ))
PyObject* mat3_rotated_x(PyObject* self, PyObject *obj)
{
    double degree;
    if (!expectFromPython(obj, &degree))
       return NULL;
    Matrix33* mat = pyobject_cast<Matrix33*>(self);
    Matrix33* ret = pyobject_cast<Matrix33*>(mat->copy());
    double v[9];
    VEC::mat3_set_rotate_x_deg(v, degree);
    VEC::matnn_multiply(ret->v, mat->v, v, 3);
    return pyobject_cast<PyObject*>(ret);
}


} // extern "C"

} // namespace PYTHON
} // namespace MO

