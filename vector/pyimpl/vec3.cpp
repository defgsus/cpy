#include "vec_base.h"
#include "vector_math.h"

using namespace PyUtils;

namespace MO {
namespace PYTHON {

extern "C" {

/** @ingroup lolpig
    @p vec3.__new__
*/
PyObject* vec3_new(_typeobject* type, PyObject* args, PyObject* )
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

#if 0
/** @ingroup lolpig
    @p vec3.__init__
*/
int vec3_init(PyObject* self, PyObject* args, PyObject* )
{
    Vector3* vec = pyobject_cast<Vector3*>(self);
    if (VectorBase::parseSequence(args, vec->v, 3) == 0)
        return -1;
    vec->len = 3;
    return 0;
}
#endif

/** @ingroup lolpig
    @p vec3.__repr__
*/
PyObject* vec3_repr(PyObject* self)
{
    Vector3* vec = pyobject_cast<Vector3*>(self);
    return toPython(vec->toRepr("vec3"));
}

/** @ingroup lolpig
    @p vec3.__str__
*/
PyObject* vec3_str(PyObject* self)
{
    Vector3* vec = pyobject_cast<Vector3*>(self);
    return toPython(vec->toString("vec3"));
}


// -------------- inplace funcs ----------------------------

/** @ingroup lolpig
    @p vec3.cross
    cross(seq3) -> self \n
    Calculates the cross-product of this vector and seq3, INPLACE \n
    The cross product is always perpendicular to the plane \n
    on which the two vectors lie. \n
*/
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

/** @ingroup lolpig
    @p vec3.crossed
    crossed(seq3) -> vec3 \n
    Returns the cross-product of this vector and seq3 \n
    The cross product is always perpendicular to the plane \n
    on which the two vectors lie. \n
*/
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

/** @ingroup lolpig
    @p vec3.rotate_x
    rotate_x(degree) -> self \n
    Rotates the vector, INPLACE \n
    >>> vec3(1,2,3).rotate_x(90).rounded() \n
    vec3(1, -3, 2) \n
*/
PyObject* vec3_rotate_x(PyObject* self, PyObject *obj)
{
    double degree;
    if (!expectFromPython(obj, &degree))
       return NULL;
    VectorBase* vec = pyobject_cast<VectorBase*>(self);
    VEC::vec3_rotate_x_deg_inplace(vec->v, degree);
    Py_RETURN_SELF;
}

/** @ingroup lolpig
    @p vec3.rotate_y
    rotate_y(degree) -> self \n
    Rotates the vector, INPLACE \n
    >>> vec3((1,2,3)).rotate_y(90).round() \n
    vec3(3, 2, -1)
*/
PyObject* vec3_rotate_y(PyObject* self, PyObject *obj)
{
    double degree;
    if (!expectFromPython(obj, &degree))
       return NULL;
    VectorBase* vec = pyobject_cast<VectorBase*>(self);
    VEC::vec3_rotate_y_deg_inplace(vec->v, degree);
    Py_RETURN_SELF;
}

/** @ingroup lolpig
    @p vec3.rotate_z
    rotate_z(degree) -> self \n
    Rotates the vector, INPLACE \n
    >>> vec3((1,2,3)).rotate_z(90).round() \n
    vec3(-2, 1, 3) \n
*/
PyObject* vec3_rotate_z(PyObject* self, PyObject *obj)
{
    double degree;
    if (!expectFromPython(obj, &degree))
       return NULL;
    VectorBase* vec = pyobject_cast<VectorBase*>(self);
    VEC::vec3_rotate_z_deg_inplace(vec->v, degree);
    Py_RETURN_SELF;
}

/** @ingroup lolpig
    @p vec3.rotate_axis
    rotate_axis(vec3, degree) -> self \n
    Rotates the vector around an axis, INPLACE \n
    Axis must be normalized! \n
    >>> vec3(1,2,3).rotate_axis((1,0,0), 90) == vec3(1,2,3).rotate_x(90) \n
    True
*/
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

/** @ingroup lolpig
    @p vec3.rotated_x
    rotated_x(degree) -> vec3 \n
    Returns a rotated vector \n
    >>> vec3(1,2,3).rotated_x(90).rounded() \n
    vec3(1, -3, 2) \n
*/
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

/** @ingroup lolpig
    @p vec3.rotated_y
    rotated_y(degree) -> vec3 \n
    Returns a rotated vector \n
    >>> vec3((1,2,3)).rotated_y(90).rounded() \n
    vec3(3, 2, -1) \n
*/
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

/** @ingroup lolpig
    @p vec3.rotated_z
    rotated_z(degree) -> vec3 \n
    Returns a rotated vector \n
    >>> vec3((1,2,3)).rotated_z(90).rounded() \n
    vec3(-2, 1, 3) \n
*/
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

/** @ingroup lolpig
    @p vec3.rotated_axis
    rotated_axis(vec3, degree) -> vec3 \n
    Returns a vector rotated around an axis. \n
    Axis must be normalized! \n
    >>> vec3(1,2,3).rotated_axis((1,0,0), 90) == vec3(1,2,3).rotated_x(90) \n
    True
*/
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

