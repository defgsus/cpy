#include "py_utils.h"

#ifndef PYUTILS_PRINT
#   include <iostream>
#   define PYUTILS_PRINT(arg__) \
        std::cout << arg__ << std::endl;
#endif

namespace PyUtils {


bool isNone(PyObject* o)
{
    return PyObject_TypeCheck(o, Py_TYPE(Py_None));
}


PyObject* toPython(const std::string& s)
{
    return PyUnicode_FromString(s.data());
}

PyObject* toPython(const char* s)
{
    return PyUnicode_FromString(s);
}

PyObject* toPython(long x)
{
    return PyLong_FromLong(x);
}

PyObject* toPython(double x)
{
    return PyFloat_FromDouble(x);
}

PyObject* toPython(bool b)
{
    if (b)
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}

PyObject* toPython(long unsigned x) { return toPython((long)x); }
PyObject* toPython(int x) { return toPython((long)x); }


bool fromPython(PyObject* obj, std::string* s)
{
    if (PyUnicode_Check(obj))
    {
        if (s)
            *s = PyUnicode_AsUTF8(obj);
        return true;
    }
    return false;
}

bool fromPython(PyObject* obj, long* s)
{
    if (PyLong_Check(obj))
    {
        if (s)
            *s = PyLong_AsLong(obj);
        return true;
    }
    return false;
}

bool fromPython(PyObject* obj, double* val)
{
    if (PyFloat_Check(obj))
    {
        if (val)
            *val = PyFloat_AsDouble(obj);
        return true;
    }
    if (PyLong_Check(obj))
    {
        if (val)
            *val = PyLong_AsLong(obj);
        return true;
    }
    return false;
}

bool expectFromPython(PyObject* obj, std::string* s)
{
    if (fromPython(obj, s))
        return true;
    setPythonError(PyExc_TypeError,
                   SStream() << "Expected string, got " << typeName(obj));
    return false;
}

bool expectFromPython(PyObject* obj, long* s)
{
    if (fromPython(obj, s))
        return true;
    setPythonError(PyExc_TypeError,
                   SStream() << "Expected int, got " << typeName(obj));
    return false;
}

bool expectFromPython(PyObject* obj, double* val)
{
    if (fromPython(obj, val))
        return true;
    setPythonError(PyExc_TypeError,
                   SStream() << "Expected double, got " << typeName(obj));

    return false;
}

template <class T>
bool fromPythonSequenceT(PyObject* seq, T* vec, size_t len)
{
    if (!PySequence_Check(seq))
        return false;
    if (PySequence_Size(seq) != len)
        return false;
    for (Py_ssize_t i=0; i<len; ++i)
    {
        if (!fromPython(PySequence_GetItem(seq, i), &vec[i]))
            return false;
    }
    return true;
}

bool fromPythonSequence(PyObject *seq, std::string *vec, size_t len)
{
    return fromPythonSequenceT(seq, vec, len);
}

bool fromPythonSequence(PyObject *seq, long *vec, size_t len)
{
    return fromPythonSequenceT(seq, vec, len);
}

bool fromPythonSequence(PyObject *seq, double *vec, size_t len)
{
    return fromPythonSequenceT(seq, vec, len);
}


template <class T>
bool expectFromPythonSequenceT(PyObject* seq, T* vec, size_t len, const std::string& type)
{
    if (!PySequence_Check(seq))
    {
        setPythonError(PyExc_TypeError,
                       SStream() << "Expected sequence of "
                       << type << ", got " << typeName(seq));
        return false;
    }
    if (PySequence_Size(seq) != len)
    {
        setPythonError(PyExc_ValueError,
                       SStream() << "Expected sequence of length "
                        << len << ", got " << PySequence_Size(seq));
        return false;
    }
    for (Py_ssize_t i=0; i<len; ++i)
    {
        if (!expectFromPython(PySequence_GetItem(seq, i), &vec[i]))
            return false;
    }
    return true;
}

bool expectFromPythonSequence(PyObject *seq, std::string *vec, size_t len)
{
    return expectFromPythonSequenceT(seq, vec, len, "string");
}

bool expectFromPythonSequence(PyObject *seq, long *vec, size_t len)
{
    return expectFromPythonSequenceT(seq, vec, len, "int");
}

bool expectFromPythonSequence(PyObject *seq, double *vec, size_t len)
{
    return expectFromPythonSequenceT(seq, vec, len, "float");
}


bool checkIndex(Py_ssize_t index, Py_ssize_t len)
{
    if (index >= len)
    {
        setPythonError(PyExc_IndexError,
                  SStream() << "Index out of range, " << index << " >= " << len);
        return false;
    }
    return true;
}





void setPythonError(PyObject* exc, const std::string& txt)
{
    PyErr_SetObject(exc, toPython(txt));
}

std::string typeName(const PyObject *arg)
{
    if (!arg)
        return "NULL";
    auto s = std::string(arg->ob_type->tp_name);
    return s;
}

bool iterateSequence(PyObject* seq, std::function<bool(PyObject*item)> foo)
{
    if (!PySequence_Check(seq))
    {
        setPythonError(PyExc_TypeError,
                       SStream() << "expected sequence, got " << typeName(seq));
        return false;
    }
    Py_ssize_t size = PySequence_Size(seq);
    for (Py_ssize_t i = 0; i < size; ++i)
    {
        auto item = PySequence_GetItem(seq, i);
        if (!item)
        {
            setPythonError(PyExc_TypeError,
                       SStream() << "NULL object in sequence[" << i << "]");
            return false;
        }
        if (!foo(item))
            return false;
    }
    return true;
}


PyObject* removeArgumentTuple(PyObject* arg)
{
    if (PyTuple_Check(arg) && PyTuple_Size(arg) == 1)
        return PyTuple_GetItem(arg, 0);
    return arg;
}


void dumpObject(PyObject* arg, bool introspect)
{
    #define PYUTILS__PRINTPY(what__) \
        if (what__) \
        { \
            if (introspect) \
            { \
                auto s = PyObject_CallMethod(what__, "__str__", ""); \
                PYUTILS_PRINT(#what__ ": " << PyUnicode_AsUTF8(s)); \
            } \
            else PYUTILS_PRINT(#what__ ": " << what__); \
        } else PYUTILS_PRINT(#what__ ": NULL");

    #define PYUTILS__PRINT(what__) \
        PYUTILS_PRINT(#what__ ": " << what__);

    PYUTILS__PRINTPY(arg);
    if (arg)
    {
        PYUTILS__PRINT(arg->ob_refcnt);
        PYUTILS__PRINT(arg->ob_type);
        if (arg->ob_type)
        {
            PYUTILS__PRINT(arg->ob_type->tp_name);
            PYUTILS__PRINT(arg->ob_type->tp_basicsize);
            PYUTILS__PRINT(arg->ob_type->tp_itemsize);

            PYUTILS__PRINT(arg->ob_type->tp_dealloc);
            PYUTILS__PRINT(arg->ob_type->tp_print);
            PYUTILS__PRINT(arg->ob_type->tp_getattr);
            PYUTILS__PRINT(arg->ob_type->tp_setattr);
            PYUTILS__PRINT(arg->ob_type->tp_reserved);
            PYUTILS__PRINT(arg->ob_type->tp_repr);
            PYUTILS__PRINT(arg->ob_type->tp_as_number);
            PYUTILS__PRINT(arg->ob_type->tp_as_sequence);
            PYUTILS__PRINT(arg->ob_type->tp_as_mapping);
            PYUTILS__PRINT(arg->ob_type->tp_hash);
            PYUTILS__PRINT(arg->ob_type->tp_call);
            PYUTILS__PRINT(arg->ob_type->tp_str);
            PYUTILS__PRINT(arg->ob_type->tp_getattro);
            PYUTILS__PRINT(arg->ob_type->tp_setattro);
            PYUTILS__PRINT(arg->ob_type->tp_as_buffer);
            PYUTILS__PRINT(arg->ob_type->tp_flags);
            //PYUTILS__PRINT(arg->ob_type->tp_doc);
            PYUTILS__PRINT(arg->ob_type->tp_traverse);
            PYUTILS__PRINT(arg->ob_type->tp_clear);
            PYUTILS__PRINT(arg->ob_type->tp_richcompare);
            PYUTILS__PRINT(arg->ob_type->tp_weaklistoffset);
            PYUTILS__PRINT(arg->ob_type->tp_iter);
            PYUTILS__PRINT(arg->ob_type->tp_iternext);
            PYUTILS__PRINT(arg->ob_type->tp_methods);
            PYUTILS__PRINT(arg->ob_type->tp_members);
            PYUTILS__PRINT(arg->ob_type->tp_getset);
            PYUTILS__PRINT(arg->ob_type->tp_base);
            PYUTILS__PRINTPY(arg->ob_type->tp_dict);
            PYUTILS__PRINT(arg->ob_type->tp_descr_get);
            PYUTILS__PRINT(arg->ob_type->tp_descr_set);
            PYUTILS__PRINT(arg->ob_type->tp_dictoffset);
            PYUTILS__PRINT(arg->ob_type->tp_init);
            PYUTILS__PRINT(arg->ob_type->tp_alloc);
            PYUTILS__PRINT(arg->ob_type->tp_new);
            PYUTILS__PRINT(arg->ob_type->tp_free);
            PYUTILS__PRINT(arg->ob_type->tp_is_gc);

            PYUTILS__PRINTPY(arg->ob_type->tp_bases);
            PYUTILS__PRINTPY(arg->ob_type->tp_mro);
            PYUTILS__PRINTPY(arg->ob_type->tp_cache);
            PYUTILS__PRINTPY(arg->ob_type->tp_subclasses);
            PYUTILS__PRINTPY(arg->ob_type->tp_weaklist);
            PYUTILS__PRINT(arg->ob_type->tp_del);
            PYUTILS__PRINT(arg->ob_type->tp_version_tag);
            PYUTILS__PRINT(arg->ob_type->tp_finalize);

    #ifdef COUNT_ALLOCS
            PYUTILS__PRINT(arg->ob_type->tp_allocs);
            PYUTILS__PRINT(arg->ob_type->tp_frees);
            PYUTILS__PRINT(arg->ob_type->tp_maxalloc);
            PYUTILS__PRINT(arg->ob_type->tp_prev);
            PYUTILS__PRINT(arg->ob_type->tp_next);
    #endif
        }
    }
#undef PYUTILS__PRINT
#undef PYUTILS__PRINTPY
}

} // namespace PyUtils
