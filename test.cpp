//#include <python3.4/Python.h>
#include "test.h"

#define CPY_DEF( name_, doc_) __attribute((gccxml("cpy")))
struct PyObject;

namespace MO {
namespace PYTHON {

extern "C" {

CPY_DEF(vec3, The vector class)
struct Vector3
{
    double v[3];
};


CPY_DEF( vec3.__init__,
the docstring
haste nich gesehn
>>> vec3(1,2,3)
vec3(1,2,3)
)
PyObject* vec3_init(PyObject* self, PyObject* arg)
{
	return self;
}

CPY_DEF(add_func, Adds two numbers)
PyObject* add_func(PyObject* args) {
	return args;
}

} // extern "C"
	
}
}
