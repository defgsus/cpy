#include <python3.4/Python.h>

#define LOLPIG_DEF(name, doc)

namespace MO {
namespace PYTHON {

extern "C" {

LOLPIG_DEF(vec3, The vector class)
struct Vector3 {
    PyObject_HEAD
    double v[3];
};

Vector3* new_Vector3();
bool is_Vector3(PyObject*);


LOLPIG_DEF(vec4, The vector class)
struct Vector4
{
    PyObject_HEAD
    double v[4];
};

LOLPIG_DEF( vec3.__init__,
    the docstring
    haste nich gesehn
    >>> vec3(1,2,3)
    vec3(1,2,3)
)
int vec3_init(PyObject* self, PyObject* args, PyObject* kwargs)
{
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
