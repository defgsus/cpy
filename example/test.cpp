#include <python3.4/Python.h>
#include <iostream>
#include <sstream>
#include <cmath>

#define PRINT(arg__) std::cout << arg__ << std::endl;

namespace MO {
namespace PYTHON {

extern "C" {

/** @addtogroup lolpig
    @{ */

/** @p Base
    Base class */
struct Base {
    PyObject_HEAD
    double v[4];
    int len;
};
/** @} */

size_t sizeof_Base() { return sizeof(Base); }

Base* new_Base();
bool is_Base(PyObject*);

/** @ingroup lolpig
    @p Derived
    Derived from Base
    */
struct Derived : public Base { };

size_t sizeof_Derived() { return sizeof(Derived); }

/** @ingroup lolpig
    @p _base_iter
    Base iterator */
struct BaseIter {
    PyObject_HEAD
    Base* b;
    int iter;
};
size_t sizeof_BaseIter() { return sizeof(BaseIter); }

BaseIter* new_BaseIter();
bool is_BaseIter(PyObject*);


/** @addtogroup lolpig
    @{ */

/** @p Base.__init__
    the docstring \n
    haste nich gesehn
*/
int base_init(PyObject* self, PyObject* args, PyObject* )
{
    Base* vec = reinterpret_cast<Base*>(self);
    if (!PyArg_ParseTuple(args, "ddd", &vec->v[0], &vec->v[1], &vec->v[2]))
        return -1;
    vec->len = 3;
	return 0;
}

/** @p Derived.__init__
    the docstring
    haste nich gesehn
*/
int derived_init(PyObject* self, PyObject* args, PyObject* )
{
    Derived* vec = reinterpret_cast<Derived*>(self);
    if (!PyArg_ParseTuple(args, "dddd", &vec->v[0], &vec->v[1], &vec->v[2], &vec->v[3]))
        return -1;
    vec->len = 4;
    return 0;
}

/** @p Base.copy
    Makes a copy
    */
PyObject* base_copy(PyObject* self)
{
    Base* vec = reinterpret_cast<Base*>(self);
    Base* nvec = new_Base();
    nvec->len = vec->len;
    for (int i=0; i<vec->len; ++i)
        nvec->v[i] = vec->v[i];
    return reinterpret_cast<PyObject*>(nvec);
}

/** @p Base.__repr__ */
PyObject* base_repr(PyObject* self)
{
    Base* vec = reinterpret_cast<Base*>(self);
    std::stringstream s;
    s << "Base(" << vec->v[0] << ", " << vec->v[1] << ", " << vec->v[2] << ")";
    return PyUnicode_FromString(s.str().c_str());
}

/** @p Derived.__repr__ */
PyObject* derived_repr(PyObject* self)
{
    Base* vec = reinterpret_cast<Base*>(self);
    std::stringstream s;
    s << "Derived(" << vec->v[0] << ", " << vec->v[1] << ", " << vec->v[2]
      << ", " << vec->v[3] << ")";
    return PyUnicode_FromString(s.str().c_str());
}


/** @p Base.__getitem__ */
PyObject* base_getitem(PyObject* self, Py_ssize_t idx)
{
    Base* vec = reinterpret_cast<Base*>(self);
    return PyFloat_FromDouble(vec->v[idx]);
}

/** @p Base.__setitem__ */
int base_setitem(PyObject* self, Py_ssize_t idx, PyObject* val)
{
    Base* vec = reinterpret_cast<Base*>(self);
    vec->v[idx] = PyFloat_AsDouble(val);
    return 0;
}

/** @p Base.__iter__ */
PyObject* vec3_iter(PyObject* self)
{
    Base* vec = reinterpret_cast<Base*>(self);
    BaseIter* iter = new_BaseIter();
    iter->iter = 0;
    iter->b = vec;
    return reinterpret_cast<PyObject*>(iter);
}

/** @p _base_iter.__iter__ */
PyObject* baseiter_iter(PyObject* self)
{
    BaseIter* iter = reinterpret_cast<BaseIter*>(self);
    Py_INCREF(iter);
    return reinterpret_cast<PyObject*>(iter);
}

/** @p _base_iter.__next__ */
PyObject* baseiter_next(PyObject* self)
{
    BaseIter* iter = reinterpret_cast<BaseIter*>(self);
    PyObject* ret = PyFloat_FromDouble(iter->b->v[iter->iter]);
    if (iter->iter++ >= iter->b->len)
    {
        PyErr_SetObject(PyExc_StopIteration, NULL);
        return NULL;
    }
    return ret;
}


/** @p Base.__contains__ */
int base_contains(PyObject* self, PyObject* val)
{
    if (!PyFloat_Check(val))
    {
        PyErr_SetString(PyExc_TypeError, "expected float");
        return -1;
    }
    double a = PyFloat_AsDouble(val);
    Base* vec = reinterpret_cast<Base*>(self);
    for (int i=0; i<vec->len; ++i)
        if (vec->v[i] == a)
            return 1;
    return 0;
}

/** @p Base.__len__ */
Py_ssize_t base_len(PyObject* self)
{
    Base* vec = reinterpret_cast<Base*>(self);
    return vec->len;
}

/** @p Base.stuff-get */
PyObject* base_stuff_get(PyObject* , void*)
{
    return PyUnicode_FromString("stuff");
}

/** @p Base.name-get */
PyObject* base_name_get(PyObject* , void*)
{
    return PyUnicode_FromString("base");
}

/** @p Derived.name-get */
PyObject* derived_name_get(PyObject* , void*)
{
    return PyUnicode_FromString("derived");
}


/** @} */

} // extern "C"
	
} // namespace PYTHON
} // namespace MO
