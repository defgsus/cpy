#include "vec_base.h"
#include "vector_math.h"

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
    return pyobject_cast<PyObject*>(vec);
}

/*
LOLPIG_DEF( vec3.__init__, )
int vec3_init(PyObject* self, PyObject* args, PyObject* )
{
    Vector3* vec = pyobject_cast<Vector3*>(self);
    if (VectorBase::parseSequence(args, vec->v, 3) == 0)
        return -1;
    vec->len = 3;
    return 0;
}*/


LOLPIG_DEF( vec3.__repr__, )
PyObject* vec3_repr(PyObject* self)
{
    Vector3* vec = pyobject_cast<Vector3*>(self);
    return toPython(vec->toRepr("vec3"));
}

LOLPIG_DEF( vec3.__str__, )
PyObject* vec3_str(PyObject* self)
{
    Vector3* vec = pyobject_cast<Vector3*>(self);
    return toPython(vec->toString("vec3"));
}


// -------------- inplace funcs ----------------------------

LOLPIG_DEF( vec3.cross, (
        cross(seq3) -> self
        Calculates the cross-product of this vector and seq3, INPLACE
        The cross product is always perpendicular to the plane
        on which the two vectors lie.
        ))
PyObject* vec3_cross(PyObject* self, PyObject* obj)
{
    double v[3];
    if (!VectorBase::parseSequenceExactly(obj, v, 3))
        return NULL;
    VectorBase* vec = pyobject_cast<VectorBase*>(self);
    VEC::vec3_cross_inplace(vec->v, v);
    Py_RETURN_SELF;
}


// ---------------- copying funcs --------------------------

LOLPIG_DEF( vec3.crossed, (
        crossed(seq3) -> vec3
        Returns the cross-product of this vector and seq3
        The cross product is always perpendicular to the plane
        on which the two vectors lie.
        ))
PyObject* vec3_crossed(PyObject* self, PyObject* obj)
{
    double v[3];
    if (!VectorBase::parseSequenceExactly(obj, v, 3))
        return NULL;
    VectorBase* vec = pyobject_cast<VectorBase*>(self);
    Vector3* ret = pyobject_cast<Vector3*>(createVector(3));
    VEC::vec3_cross(ret->v, vec->v, v);
    return pyobject_cast<PyObject*>(ret);
}




// -------------- transformation copy ----------------------

LOLPIG_DEF(vec3.rotate_x, (
        rotate_x(degree) -> self
        Rotates the vector, INPLACE
        >>> vec3(1,2,3).rotate_x(90).rounded()
        vec3(1, -3, 2)
        ))
PyObject* vec3_rotate_x(PyObject* self, PyObject *obj)
{
    double degree;
    if (!expectFromPython(obj, &degree))
       return NULL;
    VectorBase* vec = pyobject_cast<VectorBase*>(self);
    VEC::vec3_rotate_x_deg_inplace(vec->v, degree);
    Py_RETURN_SELF;
}

LOLPIG_DEF(vec3.rotate_y, (
        rotate_y(degree) -> self
        Rotates the vector, INPLACE
        >>> vec3((1,2,3)).rotate_y(90).round()
        vec3(3, 2, -1)
        ))
PyObject* vec3_rotate_y(PyObject* self, PyObject *obj)
{
    double degree;
    if (!expectFromPython(obj, &degree))
       return NULL;
    VectorBase* vec = pyobject_cast<VectorBase*>(self);
    VEC::vec3_rotate_y_deg_inplace(vec->v, degree);
    Py_RETURN_SELF;
}

LOLPIG_DEF(vec3.rotate_z, (
        rotate_z(degree) -> self
        Rotates the vector, INPLACE
        >>> vec3((1,2,3)).rotate_z(90).round()
        vec3(-2, 1, 3)
        ))
PyObject* vec3_rotate_z(PyObject* self, PyObject *obj)
{
    double degree;
    if (!expectFromPython(obj, &degree))
       return NULL;
    VectorBase* vec = pyobject_cast<VectorBase*>(self);
    VEC::vec3_rotate_z_deg_inplace(vec->v, degree);
    Py_RETURN_SELF;
}

LOLPIG_DEF(vec3.rotate_axis, (
        rotate_axis(vec3, degree) -> self
        Rotates the vector around an axis, INPLACE
        Axis must be normalized!
        >>> vec3(1,2,3).rotate_axis((1,0,0), 90) == vec3(1,2,3).rotate_x(90)
        True
        ))
PyObject* vec3_rotate_axis(PyObject* self, PyObject *args)
{
    if (!PyTuple_Check(args) || PyTuple_Size(args) != 2)
    {
        setPythonError(PyExc_TypeError, SStream()
                       << "Expected vec3 and float, got " << typeName(args, true));
        return NULL;
    }
    double axis[3];
    if (!VectorBase::parseSequenceExactly(PyTuple_GetItem(args, 0), axis, 3))
        return NULL;
    double degree;
    if (!expectFromPython(PyTuple_GetItem(args, 1), &degree))
       return NULL;
    VectorBase* vec = pyobject_cast<VectorBase*>(self);
    VEC::vec3_rotate_axis_deg_inplace(vec->v, axis, degree);
    Py_RETURN_SELF;
}




// -------------- transformation copy ----------------------

LOLPIG_DEF(vec3.rotated_x, (
        rotated_x(degree) -> vec3
        Returns a rotated vector
        >>> vec3(1,2,3).rotated_x(90).rounded()
        vec3(1, -3, 2)
        ))
PyObject* vec3_rotated_x(PyObject* self, PyObject *obj)
{
    double degree;
    if (!expectFromPython(obj, &degree))
       return NULL;
    VectorBase* vec = pyobject_cast<VectorBase*>(self);
    VectorBase* ret = vec->copyClass();
    VEC::vec3_rotate_x_deg(ret->v, vec->v, degree);
    return pyobject_cast<PyObject*>(ret);
}

LOLPIG_DEF(vec3.rotated_y, (
        rotated_y(degree) -> vec3
        Returns a rotated vector
        >>> vec3((1,2,3)).rotated_y(90).rounded()
        vec3(3, 2, -1)
        ))
PyObject* vec3_rotated_y(PyObject* self, PyObject *obj)
{
    double degree;
    if (!expectFromPython(obj, &degree))
       return NULL;
    VectorBase* vec = pyobject_cast<VectorBase*>(self);
    VectorBase* ret = vec->copyClass();
    VEC::vec3_rotate_y_deg(ret->v, vec->v, degree);
    return pyobject_cast<PyObject*>(ret);
}

LOLPIG_DEF(vec3.rotated_z, (
        rotated_z(degree) -> vec3
        Returns a rotated vector
        >>> vec3((1,2,3)).rotated_z(90).rounded()
        vec3(-2, 1, 3)
        ))
PyObject* vec3_rotated_z(PyObject* self, PyObject *obj)
{
    double degree;
    if (!expectFromPython(obj, &degree))
       return NULL;
    VectorBase* vec = pyobject_cast<VectorBase*>(self);
    VectorBase* ret = vec->copyClass();
    VEC::vec3_rotate_z_deg(ret->v, vec->v, degree);
    return pyobject_cast<PyObject*>(ret);
}

LOLPIG_DEF(vec3.rotated_axis, (
        rotated_axis(vec3, degree) -> vec3
        Returns a vector rotated around an axis.
        Axis must be normalized!
        >>> vec3(1,2,3).rotated_axis((1,0,0), 90) == vec3(1,2,3).rotated_x(90)
        True
        ))
PyObject* vec3_rotated_axis(PyObject* self, PyObject *args)
{
    if (!PyTuple_Check(args) || PyTuple_Size(args) != 2)
    {
        setPythonError(PyExc_TypeError, SStream()
                       << "Expected vec3 and float, got " << typeName(args, true));
        return NULL;
    }
    double axis[3];
    if (!VectorBase::parseSequenceExactly(PyTuple_GetItem(args, 0), axis, 3))
        return NULL;
    double degree;
    if (!expectFromPython(PyTuple_GetItem(args, 1), &degree))
       return NULL;
    VectorBase* vec = pyobject_cast<VectorBase*>(self);
    VectorBase* ret = vec->copyClass();
    VEC::vec3_rotate_axis_deg(ret->v, vec->v, axis, degree);
    return pyobject_cast<PyObject*>(ret);
}


} // extern "C"
	
} // namespace PYTHON
} // namespace MO

