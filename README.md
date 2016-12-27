#### LOw-Level Python Interface Generator

Work in progress..

After the attempt to [convert annotated python to c](https://github.com/defgsus/cppy) which 
basically works but requires writing C code in a python file (without the help of an IDE)
i'll try write the C code instead, annotate it with the python info and generate the 
module code automatically. For example:

```C++

CPY_DEF(name_of_python_func,
Adds two numbers, returns float.
Multi-line doc-strings :)
)
PyObject* func_add(PyObject* a, PyObject* b)
{
    double a, b;
    if (!PyArg_ParseTuple(arg1, "dd", &a, &b))
        return NULL;
    return PyFloat_FromDouble(a+b);
}
```

Again, this is supposed to be a low-level helper. 
No automatic type conversions and such. It's just for generating the boiler-plate code
to create the python module