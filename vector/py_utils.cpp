#include <cmath>

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

bool isEmpty(PyObject* o)
{
    if (!o || isNone(o))
        return true;
    if (PySequence_Check(o))
        return PySequence_Size(o) < 1;
    if (PyMapping_Check(o))
        return PyMapping_Size(o) < 1;
    return false;
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
        const char* v = PyUnicode_AsUTF8(obj);
        if (PyErr_Occurred())
            return false;
        if (s)
            *s = v;
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
        const double v = PyFloat_AsDouble(obj);
        if (PyErr_Occurred())
            return false;
        if (val)
            *val = v;
        return true;
    }
    if (PyLong_Check(obj))
    {
        const double v = PyLong_AsLong(obj);
        if (PyErr_Occurred())
            return false;
        if (val)
            *val = v;
        return true;
    }
    return false;
}


bool expectFromPython(PyObject* obj, std::string* s)
{
    if (fromPython(obj, s))
        return true;
    if (!PyErr_Occurred())
        setPythonError(PyExc_TypeError,
                   SStream() << "Expected string, got " << typeName(obj));
    return false;
}

bool expectFromPython(PyObject* obj, long* s)
{
    if (fromPython(obj, s))
        return true;
    if (!PyErr_Occurred())
        setPythonError(PyExc_TypeError,
                   SStream() << "Expected int, got " << typeName(obj));
    return false;
}

bool expectFromPython(PyObject* obj, double* val)
{
    if (fromPython(obj, val))
        return true;
    if (!PyErr_Occurred())
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
    if (index < 0)
    {
        setPythonError(PyExc_IndexError,
                  SStream() << "Index out of range, " << index << " < 0");
        return false;
    }
    if (index >= len)
    {
        setPythonError(PyExc_IndexError,
                  SStream() << "Index out of range, " << index << " >= " << len);
        return false;
    }
    return true;
}



double pythonRound(double v, int n)
{
    if (n > 0)
    {
        n = std::pow(10, n);
        return std::round(v*n) / n;
    }
    else if (n < 0)
    {
        n = std::pow(10, -n);
        return std::round(v/n) * n;
    }
    else
    {
        double f = std::floor(v);
        return v - f != .5
                    ? std::round(v)
                    : v < 0. ? f + 1. : f;
    }
}

double pythonModulo(double x, double m)
{
#if 0
    return x - std::floor(x/m) * m;
#else
    if (double mod = std::fmod(x, m))
    {
        if ((m < 0) != (mod < 0))
            mod += m;
        return mod;
    }
    else
        return std::copysign(0.0, m);
#endif
}

bool pythonPower(double* res, double iv, double iw)
{
    // adopted from
    // https://github.com/python/cpython/blob/master/Objects/floatobject.c float_pow()

    /* determine whether x is an odd integer or not;  assumes that
       x is not an infinity or nan. */
    #define DOUBLE_IS_ODD_INTEGER(x) (fmod(fabs(x), 2.0) == 1.0)
    #define PYPOW_RETURN(x) { *res = (x); return true; }

    int negate_result = 0;

    /* Sort out special cases here instead of relying on pow() */
    if (iw == 0) {              /* v**0 is 1, even 0**0 */
        PYPOW_RETURN(1.0);
    }
    if (Py_IS_NAN(iv)) {        /* nan**w = nan, unless w == 0 */
        PYPOW_RETURN(iv);
    }
    if (Py_IS_NAN(iw)) {        /* v**nan = nan, unless v == 1; 1**nan = 1 */
        PYPOW_RETURN(iv == 1.0 ? 1.0 : iw);
    }
    if (Py_IS_INFINITY(iw)) {
        /* v**inf is: 0.0 if abs(v) < 1; 1.0 if abs(v) == 1; inf if
         *     abs(v) > 1 (including case where v infinite)
         *
         * v**-inf is: inf if abs(v) < 1; 1.0 if abs(v) == 1; 0.0 if
         *     abs(v) > 1 (including case where v infinite)
         */
        iv = std::fabs(iv);
        if (iv == 1.0)
            PYPOW_RETURN(1.0)
        else if ((iw > 0.0) == (iv > 1.0))
            PYPOW_RETURN(std::fabs(iw)) /* return inf */
        else
            PYPOW_RETURN(0.0);
    }
    if (Py_IS_INFINITY(iv)) {
        /* (+-inf)**w is: inf for w positive, 0 for w negative; in
         *     both cases, we need to add the appropriate sign if w is
         *     an odd integer.
         */
        int iw_is_odd = DOUBLE_IS_ODD_INTEGER(iw);
        if (iw > 0.0)
            PYPOW_RETURN(iw_is_odd ? iv : fabs(iv))
        else
            PYPOW_RETURN(iw_is_odd ? std::copysign(0.0, iv) : 0.0);
    }
    if (iv == 0.0) {  /* 0**w is: 0 for w positive, 1 for w zero
                         (already dealt with above), and an error
                         if w is negative. */
        if (iw < 0.0) {
            PyErr_SetString(PyExc_ZeroDivisionError,
                            "0.0 cannot be raised to a "
                            "negative power");
            return NULL;
        }
        int iw_is_odd = DOUBLE_IS_ODD_INTEGER(iw);
        /* use correct sign if iw is odd */
        PYPOW_RETURN(iw_is_odd ? iv : 0.0);
    }

    if (iv < 0.0) {
        /* Whether this is an error is a mess, and bumps into libm
         * bugs so we have to figure it out ourselves.
         */
        if (iw != floor(iw)) {
            /* Negative numbers raised to fractional powers
             * become complex.
             */
            //return PyComplex_Type.tp_as_number->nb_power(v, w, z);
            setPythonError(PyExc_ArithmeticError,
                           "Negative numbers raised to fractional powers "
                           "become complex, which is not supported here");
            return false;
        }
        /* iw is an exact integer, albeit perhaps a very large
         * one.  Replace iv by its absolute value and remember
         * to negate the pow result if iw is odd.
         */
        iv = -iv;
        negate_result = DOUBLE_IS_ODD_INTEGER(iw);
    }

    if (iv == 1.0) { /* 1**w is 1, even 1**inf and 1**nan */
        /* (-1) ** large_integer also ends up here.  Here's an
         * extract from the comments for the previous
         * implementation explaining why this special case is
         * necessary:
         *
         * -1 raised to an exact integer should never be exceptional.
         * Alas, some libms (chiefly glibc as of early 2003) return
         * NaN and set EDOM on pow(-1, large_int) if the int doesn't
         * happen to be representable in a *C* integer.  That's a
         * bug.
         */
        PYPOW_RETURN(negate_result ? -1.0 : 1.0);
    }

    /* Now iv and iw are finite, iw is nonzero, and iv is
     * positive and not equal to 1.0.  We finally allow
     * the platform pow to step in and do the rest.
     */
    errno = 0;
    double ix;
    PyFPE_START_PROTECT("pow", return false)
    ix = std::pow(iv, iw);
    PyFPE_END_PROTECT(ix)
    Py_ADJUST_ERANGE1(ix);
    if (negate_result)
        ix = -ix;

    if (errno != 0) {
        /* We don't expect any errno value other than ERANGE, but
         * the range of libm bugs appears unbounded.
         */
        PyErr_SetFromErrno(errno == ERANGE ? PyExc_OverflowError :
                             PyExc_ValueError);
        return false;
    }
    PYPOW_RETURN(ix);

    #undef PYPOW_RETURN
    #undef DOUBLE_IS_ODD_INTEGER
}


void setPythonError(PyObject* exc)
{
    PyErr_SetFromErrno(exc);
}

void setPythonError(PyObject* exc, const std::string& txt)
{
    PyErr_SetObject(exc, toPython(txt));
}

std::string typeName(PyObject *arg, bool expand)
{
    if (!arg)
        return "NULL";
    if (expand)
    {
        std::string s;
        if (PyList_Check(arg))
        {
            s = "[";
            for (Py_ssize_t i=0; i<PyList_Size(arg); ++i)
                s += typeName(PyList_GetItem(arg, i), true) + ",";
            s += "]";
            return s;
        }
        else if (PyTuple_Check(arg))
        {
            s = "(";
            for (Py_ssize_t i=0; i<PyTuple_Size(arg); ++i)
                s += typeName(PyTuple_GetItem(arg, i), true) + ",";
            s += ")";
            return s;
        }
    }
    return std::string(arg->ob_type->tp_name);
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
