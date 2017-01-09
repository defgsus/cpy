#### LOw-Level Python Interface Generator

![lolpig_logo](lolpig_logo.png)

Work in progress.. but getting there :)

After the attempt to [convert annotated python to c](https://github.com/defgsus/cppy) 
which basically works but requires writing C code in a python file 
(without the help of an IDE) i now try writing the annotaed C code instead
and generate the module code automatically. For example:

```C++
// def.cpp

/** @addtogroup lolpig
    @{ */

/** @p add
    Adds two numbers, returns float. \n
    Multi-line doc-strings need a trailing \n
    in doxygen!
*/
PyObject* func_add(PyObject* args)
{
    double a, b;
    if (!PyArg_ParseTuple(args, "dd", &a, &b))
        return NULL;
    return PyFloat_FromDouble(a+b);
}

/** @} */
```

Calling:

```bash
lolpig.py -i def.cpp -o module -m test_mod 
```

will create a `module.h` and `module.cpp` file containing all the necessary 
c-api tango. In C, you can then, say:

```c++
#include "module.h"
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

extern "C" {

/** @addtogroup lolpig 
    @{ */

/** @p vec3
    A vector class */
struct Vector3 {
    PyObject_HEAD
    double v[3];
};

/** @} */

// This must be implemented so that the C-API module knows about the size
// of your struct. 
size_t sizeof_Vector3() { return sizeof(Vector3); }

// These helper functions are automatically defined in the generated module
Vector3* new_Vector3();
bool is_Vector3(PyObject*);

/** @ingroup lolpig
    @p vec3.__init__
    Initialize the vector */
int vec3_init(PyObject* self, PyObject* args, PyObject* kwargs)
{
    // do something meaningful..
	return 0;
}

/** @ingroup lolpig
    @p vec3.copy
    Makes a copy of the vector */
PyObject* vec3_copy(PyObject* self)
{
    Vector3* vec = reinterpret_cast<Vector3*>(self);
    Vector3* nvec = new_Vector3();
    for (int i=0; i<3; ++i)
        nvec->v[i] = vec->v[i];
    return reinterpret_cast<PyObject*>(nvec);
}

/** @ingroup lolpig
    @p vec3.__getitem__
    Returns a component */
PyObject* vec3_getitem(PyObject* self, Py_ssize_t idx)
{
    Vector3* vec = reinterpret_cast<Vector3*>(self);
    return PyFloat_FromDouble(vec->v[idx]);
}

/** @ingroup lolpig
    @p vec3.__setitem__
    Sets a component */
int vec3_setitem(PyObject* self, Py_ssize_t idx, PyObject* val)
{
    Vector3* vec = reinterpret_cast<Vector3*>(self);
    vec->v[idx] = PyFloat_AsDouble(val);
    return 0;
}

} // extern "C"
```

Every function and struct put into the `lolpig` group will be 
considered part of the module. 

Running *lolpig* on the above .cpp file creates a .h and .cpp file defining
the c-api python module. All three files (your own and the two created ones)
need to be compiled and that's just it :)

*lolpig* verifies function signatures of known functions, e.g. **__getitem__**. 
There are some peculiarities in the Python/C API. On deviations, *lolpig* will 
stop compiling and print the expected function signature instead.

*lolpig* uses **doxygen** to parse the C/C++ code. You have to create
a doxygen group named `lolpig` at some point in the code. See the documentation for
[defgroup](http://www.stack.nl/~dimitri/doxygen/manual/commands.html#cmddefgroup),
[addtogroup](http://www.stack.nl/~dimitri/doxygen/manual/commands.html#cmdaddtogroup)
or 
[ingroup](http://www.stack.nl/~dimitri/doxygen/manual/commands.html#cmdingroup).
If a function or struct is in the `lolpig` group, it's comment **must** start with
`@p python_name` or `\p python_name`. This name will define the name in the python
module, as you probably have guessed. Some examples:

```c++
/** @addtogroup lolpig
    @{ */

/** @p global_func */
PyObject* global_func() { Py_RETURN_NONE; }

/** @p myclass.method */
PyObject* myclass_method(PyObject* self) { Py_RETURN_NONE; }

/** @p myclass.prop-get */
PyObject* myclass_prop_getter(PyObject* self, void*) { Py_RETURN_NONE; }

/** @p myclass.prop-set */
int myclass_prop_setter(PyObject* self, PyObject* obj, void*) { return 0; }

/** @} */    
```
