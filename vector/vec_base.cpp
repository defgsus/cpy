#include "vec_base.h"

namespace MO {
namespace PYTHON {

using namespace PyUtils;

extern "C" {


LOLPIG_DEF( vec.__new__, )
PyObject* vec_new(struct _typeobject* type, PyObject* args, PyObject* )
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
LOLPIG_DEF( vec.__init__, )
int vec_init(PyObject* self, PyObject* args, PyObject* )
{
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
    PRINT("INIT " << vec->len);
    if (!PyArg_ParseTuple(args, "ddd", &vec->v[0], &vec->v[1], &vec->v[2]))
        return -1;
    return 0;
}*/

LOLPIG_DEF( vec.__dealloc__, )
void vec_free(PyObject* self)
{
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
    //PRINT("DEALLOC " << vec->v);
    free(vec->v);
    vec->v = NULL;
    self->ob_type->tp_free(self);
}

LOLPIG_DEF( vec.__repr__, )
PyObject* vec_repr(PyObject* self)
{
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
    std::stringstream s;
    s << "vec(";
    for (int i=0; i<vec->len; ++i)
        s << vec->v[i] << ", ";
    s << ") " << self->ob_refcnt;
    return PyUnicode_FromString(s.str().c_str());
}



LOLPIG_DEF( vec.__getitem__, )
PyObject* vec_getitem(PyObject* self, Py_ssize_t idx)
{
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
    return toPython(vec->v[idx]);
}

LOLPIG_DEF( vec.__setitem__, )
int vec_setitem(PyObject* self, Py_ssize_t idx, PyObject* val)
{
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
    if (!fromPython(val, &vec->v[idx]))
        return -1;
    return 0;
}

LOLPIG_DEF( vec.__len__, )
Py_ssize_t vec_len(PyObject* self)
{
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
    return vec->len;
}

LOLPIG_DEF( vec.test, )
PyObject* vec_test(PyObject* self, PyObject* args)
{
    PRINT("SEQ " << PySequence_Check(args)
          << " LEN " << PySequence_Size(args) );
    Py_RETURN_NONE;
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
