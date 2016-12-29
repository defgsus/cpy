#include "vec_base.h"

namespace MO {
namespace PYTHON {

extern "C" {


LOLPIG_DEF( vec_base.__new__, )
PyObject* vec_base_new(struct _typeobject* type, PyObject* args, PyObject* )
{
    VectorBase* vec = PyObject_NEW(VectorBase, type);
    vec->len = 3;
    //PRINT("NEW " << vec->len);
    vec->v = (double*)malloc(vec->len * sizeof(double));

    if (!PyArg_ParseTuple(args, "ddd", &vec->v[0], &vec->v[1], &vec->v[2]))
    {
        Py_DECREF(vec);
        return NULL;
    }
    return reinterpret_cast<PyObject*>(vec);
}

/*
LOLPIG_DEF( vec_base.__init__, )
int vec_base_init(PyObject* self, PyObject* args, PyObject* )
{
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
    PRINT("INIT " << vec->len);
    if (!PyArg_ParseTuple(args, "ddd", &vec->v[0], &vec->v[1], &vec->v[2]))
        return -1;
    return 0;
}*/

LOLPIG_DEF( vec_base.__dealloc__, )
void vec_base_free(PyObject* self)
{
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
    //PRINT("DEALLOC " << vec->v);
    free(vec->v);
    vec->v = NULL;
    self->ob_type->tp_free(self);
}

LOLPIG_DEF( vec_base.__repr__, )
PyObject* vec_base_repr(PyObject* self)
{
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
    std::stringstream s;
    s << "vec_base(";
    for (int i=0; i<vec->len; ++i)
        s << vec->v[i] << ", ";
    s << ") " << self->ob_refcnt;
    return PyUnicode_FromString(s.str().c_str());
}



LOLPIG_DEF( vec_base.__getitem__, )
PyObject* vec_base_getitem(PyObject* self, Py_ssize_t idx)
{
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
    return PyFloat_FromDouble(vec->v[idx]);
}

LOLPIG_DEF( vec_base.__setitem__, )
int vec_base_setitem(PyObject* self, Py_ssize_t idx, PyObject* val)
{
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
    vec->v[idx] = PyFloat_AsDouble(val);
    return 0;
}

LOLPIG_DEF( vec_base.__len__, )
Py_ssize_t vec_base_len(PyObject* self)
{
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
    return vec->len;
}

/*
LOLPIG_DEF( vec.__iter__, )
PyObject* vec3_iter(PyObject* self)
{
    Vector3* vec = reinterpret_cast<Vector3*>(self);
    Vector3Iter* iter = new_Vector3Iter();
    iter->iter = 0;
    iter->vec = vec;
    return reinterpret_cast<PyObject*>(iter);
}

LOLPIG_DEF( _vec3_iter.__iter__, )
PyObject* vec3iter_iter(PyObject* self)
{
    Vector3Iter* iter = reinterpret_cast<Vector3Iter*>(self);
    Py_INCREF(iter);
    return reinterpret_cast<PyObject*>(iter);
}

LOLPIG_DEF( _vec3_iter.__next__, )
PyObject* vec3iter_next(PyObject* self)
{
    Vector3Iter* iter = reinterpret_cast<Vector3Iter*>(self);
    PyObject* ret = PyFloat_FromDouble(iter->vec->v[iter->iter]);
    if (iter->iter++ >= iter->vec->len)
    {
        PyErr_SetObject(PyExc_StopIteration, NULL);
        return NULL;
    }
    return ret;
}


LOLPIG_DEF( vec3.__contains__, )
int vec3_contains(PyObject* self, PyObject* val)
{
    if (!PyFloat_Check(val))
    {
        PyErr_SetString(PyExc_TypeError, "expected float");
        return -1;
    }
    double a = PyFloat_AsDouble(val);
    Vector3* vec = reinterpret_cast<Vector3*>(self);
    for (int i=0; i<3; ++i)
        if (vec->v[i] == a)
            return 1;
    return 0;
}
*/


} // extern "C"
	
} // namespace PYTHON
} // namespace MO
