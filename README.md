#### LOw-Level Python Interface Generator

![lolpig_logo](lolpig_logo.png)

Work in progress.. but getting there :)

After the attempt to [convert annotated python to c](https://github.com/defgsus/cppy) 
which basically works but requires writing C code in a python file 
(without the help of an IDE) i now try writing the annotaed C code instead
and generate the module code automatically. For example:

```C++
// def.cpp

LOLPIG_DEF(add,
    Adds two numbers, returns float.
    Multi-line doc-strings, yeah!
)
PyObject* func_add(PyObject* args)
{
    double a, b;
    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;
    return PyFloat_FromDouble(a+b);
}
```

Calling:

```bash
lolpig.py -i def.cpp -o module -m test_mod 
```

will create a `module.h` and `module.cpp` file containing all the necessary 
c-api tango. In C, you can then, say:

```c++
#include "test_mod.h"
...
initialize_module_test_mod();
Py_Main(argc, argv);
```

and in Python:

```python
from test_mod import *
add(1, 2)
```

Again, this is supposed to be a low-level helper. 
No automatic type conversions and such. You write the pure Python/C-api functions, 
and *lolpig* generates all the boiler-plate code for the python module. 

Defining a class and a more complete example looks like this:

```C++
// module_impl.cpp

#include <python3.4/Python.h>

#define LOLPIG_DEF(name, doc)

extern "C" {

LOLPIG_DEF(vec3, A vector class)
struct Vector3 {
    PyObject_HEAD
    double v[3];
};

// these functions are automatically defined in the generated module
Vector3* new_Vector3();
bool is_Vector3(PyObject*);

LOLPIG_DEF(vec3.__init__, Initialize the vector)
int vec3_init(PyObject* self, PyObject* args, PyObject* kwargs)
{
    // do something meaningful..
	return 0;
}

LOLPIG_DEF( vec3.copy, Makes a copy of the vector)
PyObject* vec3_copy(PyObject* self)
{
    Vector3* vec = reinterpret_cast<Vector3*>(self);
    Vector3* nvec = new_Vector3();
    for (int i=0; i<3; ++i)
        nvec->v[i] = vec->v[i];
    return reinterpret_cast<PyObject*>(nvec);
}

LOLPIG_DEF( vec3.__getitem__, Returns a component)
PyObject* vec3_getitem(PyObject* self, Py_ssize_t idx)
{
    Vector3* vec = reinterpret_cast<Vector3*>(self);
    return PyFloat_FromDouble(vec->v[idx]);
}

LOLPIG_DEF( vec3.__setitem__, Sets a component)
int vec3_setitem(PyObject* self, Py_ssize_t idx, PyObject* val)
{
    Vector3* vec = reinterpret_cast<Vector3*>(self);
    vec->v[idx] = PyFloat_AsDouble(val);
    return 0;
}

} // extern "C"
```

Every function and struct prefixed with the `LOLPIG_DEF` macro will be 
considered part of the module. 

Running *lolpig* on the above .cpp file creates a .h and .cpp file defining
the c-api python module. All three files (your own and the two created ones)
need to be compiled and that's just it :)

*lolpig* verifies function signatures of known functions, e.g. **__getitem__**.

*lolpig* currently uses **gcc-xml** to parse the C/C++ files. That means that
C++11 features are not supported - unfortunately. Fixes are:
- Hide all C++11 stuff via a **GCC_XML** macro. This macro is defined when
*lolpig* analyzes your code
- Switch to CastXML or directly to LLVM for c++11.. (not keen about this yet)
- or look into Doxygen if it parses enough of the C/C++1x to replace gcc-xml  

