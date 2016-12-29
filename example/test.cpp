#include <python3.4/Python.h>
#include <iostream>
#include <sstream>
#include <cmath>

#define LOLPIG_DEF(name, doc)

#define PRINT(arg__) std::cout << arg__ << std::endl;

namespace MO {
namespace PYTHON {

extern "C" {

LOLPIG_DEF(vec3, The vector class)
struct Vector3 {
    PyObject_HEAD
    const static int len = 3;
    double v[3];
};

Vector3* new_Vector3();
bool is_Vector3(PyObject*);

LOLPIG_DEF(_vec3_iter, vector iterator)
struct Vector3Iter {
    PyObject_HEAD
    Vector3* vec;
    int iter;
};

Vector3Iter* new_Vector3Iter();
bool is_Vector3Iter(PyObject*);


LOLPIG_DEF( vec3.__init__,
    the docstring
    haste nich gesehn
    >>> vec3(1,2,3)
    vec3(1,2,3)
)
int vec3_init(PyObject* self, PyObject* args, PyObject* )
{
    Vector3* vec = reinterpret_cast<Vector3*>(self);
    if (!PyArg_ParseTuple(args, "ddd", &vec->v[0], &vec->v[1], &vec->v[2]))
        return -1;
	return 0;
}

LOLPIG_DEF( vec3.copy, Makes a copy)
PyObject* vec3_copy(PyObject* self)
{
    Vector3* vec = reinterpret_cast<Vector3*>(self);
    Vector3* nvec = new_Vector3();
    for (int i=0; i<3; ++i)
        nvec->v[i] = vec->v[i];
    return reinterpret_cast<PyObject*>(nvec);
}

LOLPIG_DEF( vec3.__repr__, )
PyObject* vec3_repr(PyObject* self)
{
    Vector3* vec = reinterpret_cast<Vector3*>(self);
    std::stringstream s;
    s << "vec3(" << vec->v[0] << ", " << vec->v[1] << ", " << vec->v[2] << ")";
    return PyUnicode_FromString(s.str().c_str());
}

LOLPIG_DEF( vec3.__call__, )
PyObject* vec3_call(PyObject* self, PyObject*, PyObject*)
{
    Vector3* vec = reinterpret_cast<Vector3*>(self);
    PRINT("Called! " << vec->v[0] << ", " << vec->v[1] << ", " << vec->v[2]);
    Py_RETURN_NONE;
}

LOLPIG_DEF( vec3.__round__, )
PyObject* vec3_round(PyObject* self)
{
    Vector3* vec = reinterpret_cast<Vector3*>(self);
    Vector3* nvec = new_Vector3();
    for (int i=0; i<3; ++i)
        nvec->v[i] = std::floor(vec->v[i]);
    return reinterpret_cast<PyObject*>(nvec);
}

LOLPIG_DEF( vec3.__iconcat__, )
PyObject* vec3_iconcat(PyObject* self, PyObject* other)
{
    Vector3* vec = reinterpret_cast<Vector3*>(self);
    PRINT("iconcat");
    Py_INCREF(vec);
    return (PyObject*)vec;
}

LOLPIG_DEF( vec3.__concat__, )
PyObject* vec3_concat(PyObject* self, PyObject* other)
{
    Vector3* vec = reinterpret_cast<Vector3*>(self);
    PRINT("concat");
    Py_INCREF(vec);
    return (PyObject*)vec;
}

LOLPIG_DEF( vec3.__mul__, )
PyObject* vec3_mul(PyObject* self, PyObject* arg)
{
    Vector3* vec = reinterpret_cast<Vector3*>(self);
    if (!PyFloat_Check(arg))
    {
        PyErr_SetString(PyExc_TypeError, "expected float");
        return NULL;
    }
    double a = PyFloat_AsDouble(arg);
    Vector3* nvec = new_Vector3();
    for (int i=0; i<3; ++i)
        nvec->v[i] = vec->v[i] * a;
    return reinterpret_cast<PyObject*>(nvec);
}


LOLPIG_DEF( vec3.__getitem__, )
PyObject* vec3_getitem(PyObject* self, Py_ssize_t idx)
{
    Vector3* vec = reinterpret_cast<Vector3*>(self);
    return PyFloat_FromDouble(vec->v[idx]);
}

LOLPIG_DEF( vec3.__setitem__, )
int vec3_setitem(PyObject* self, Py_ssize_t idx, PyObject* val)
{
    Vector3* vec = reinterpret_cast<Vector3*>(self);
    vec->v[idx] = PyFloat_AsDouble(val);
    return 0;
}

LOLPIG_DEF( vec3.__iter__, )
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

LOLPIG_DEF( vec3.__len__, )
Py_ssize_t vec3_len(PyObject* )
{
    return 3;
}

PyObject* helper_func(PyObject* arg) { return arg; }

LOLPIG_DEF(func23, Returns 23.)
PyObject* func23() { return PyFloat_FromDouble(23.); }

LOLPIG_DEF(add,( Some comment, with comma ))
PyObject* add_func(PyObject* args)
{
    double a, b;
    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;
    return PyFloat_FromDouble(a + b);
}

} // extern "C"
	
} // namespace PYTHON
} // namespace MO
