#include <python3.4/Python.h>

#define LOLPIG_DEF(name, doc)
//__attribute((gccxml("cpy")))
//struct PyObject;

//namespace MO {
//namespace PYTHON {

extern "C" {

LOLPIG_DEF(vec3, The vector class)
struct Vector3 {
    PyObject_HEAD
    double v[3];
};

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
PyObject* vec3_init(PyObject* self, PyObject* arg)
{
	return self;
}

LOLPIG_DEF( vec3.copy, Makes a copy)
PyObject* vec3_copy(PyObject* self)
{
    return PyFloat_FromDouble(5.);
}


LOLPIG_DEF(add_func, Adds two numbers)
PyObject* add_func(PyObject* args) {
	//return args;
	return PyFloat_FromDouble(23.);
}

LOLPIG_DEF(add_func2,( Some strange, comment ))
PyObject* add_func2(PyObject* args) {
	//return args;
	return PyFloat_FromDouble(23.);
}

PyObject* helper_func(PyObject* arg) { return arg; }

} // extern "C"
	
//}
//}
