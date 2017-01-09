#include <vector>
#include <limits>

#include "vec_base.h"
#include "mat_base.h"
#include "vector_math.h"

namespace MO {
namespace PYTHON {

using namespace PyUtils;


extern "C" {

size_t sizeof_VectorBase() { return sizeof(VectorBase); }
size_t sizeof_Vector3() { return sizeof(Vector3); }
size_t sizeof_VectorIter() { return sizeof(VectorIter); }

/** @ingroup lolpig
    @p vec.__new__
*/
PyObject* vec_new(_typeobject* type, PyObject* args, PyObject* )
{
    int len = VectorBase::parseSequence(args);
    if (len<0)
        return NULL;

    VectorBase* vec = PyObject_NEW(VectorBase, type);
    vec->alloc(len);
    if (VectorBase::parseSequence(args, vec->v, len, 1) < 0)
    {
        Py_DECREF(vec);
        return NULL;
    }

    return pyobject_cast<PyObject*>(vec);
}

#if 0
/** @ingroup lolpig
    @p vec.__init__
*/
int vec_init(PyObject* self, PyObject* args, PyObject* )
{
    VectorBase* vec = pyobject_cast<VectorBase*>(self);
    vec->len = VectorBase::parseSequence(args, vec->v, 16);
    if (vec->len == 0)
        return -1;
    PRINT("NEW " << vec->toString());
    return 0;
}
#endif

/** @ingroup lolpig
    @p vec.__dealloc__
*/
void vec_free(PyObject* self)
{
    VectorBase* vec = pyobject_cast<VectorBase*>(self);
    vec->dealloc();
    self->ob_type->tp_free(self);
}

/** @ingroup lolpig
    @p vec.copy
*/
PyObject* vec_copy(PyObject* self)
{
    VectorBase* vec = pyobject_cast<VectorBase*>(self);
    return pyobject_cast<PyObject*>(vec->copy());
}


/** @ingroup lolpig
    @p vec.__repr__
*/
PyObject* vec_repr(PyObject* self)
{
    VectorBase* vec = pyobject_cast<VectorBase*>(self);
    return toPython(vec->toRepr("vec"));
}

/** @ingroup lolpig
    @p vec.__str__
*/
PyObject* vec_str(PyObject* self)
{
    VectorBase* vec = pyobject_cast<VectorBase*>(self);
    return toPython(vec->toString("vec"));
}


#if 0
/** @ingroup lolpig
    @p vec.test@get
*/
PyObject* vec_test__getter(PyObject* self, void*)
{
    //VectorBase* vec = pyobject_cast<VectorBase*>(self);
    return toPython(23.);
}

/** @ingroup lolpig
    @p vec.test@set
*/
int vec_test__setter(PyObject* self, PyObject* arg, void*)
{
    //VectorBase* vec = pyobject_cast<VectorBase*>(self);
    return 0;
}
#endif

// ------------------------ sequence methods ---------------------------

/** @ingroup lolpig
    @p vec.__contains__
*/
int vec_contains(PyObject* self, PyObject* obj)
{
    VectorBase* vec = pyobject_cast<VectorBase*>(self);

    // scalar in vec
    double s;
    if (fromPython(obj, &s))
    {
        if (!vec->len)
            return 0;
        for (int i=0; i<vec->len; ++i)
            if (vec->v[i] == s)
                return 1;
        return 0;
    }
    if (PyErr_Occurred())
        return -1;
    // seq in vec
    double v[vec->len+1];
    int len = VectorBase::parseSequence(obj, v, vec->len+1);
    if (len < 0)
    {
        setPythonError(PyExc_TypeError, SStream() << "Expected scalar or sequence, got "
                       << typeName(obj));
        return -1;
    }
    if (!vec->len || len > vec->len)
        return 0;
    for (int i=0; i<=vec->len-len; ++i)
    {
        bool c = true;
        for (int j=0; j<len; ++j)
            if (vec->v[i+j] != v[j])
                { c = false; break; }
        if (c)
            return 1;
    }
    return 0;
}

/** @ingroup lolpig
    @p vec.__getitem__
*/
PyObject* vec_getitem(PyObject* self, Py_ssize_t idx)
{
    VectorBase* vec = pyobject_cast<VectorBase*>(self);
    if (!checkIndex(idx, vec->len))
        return NULL;
    return toPython(vec->v[idx]);
}

/** @ingroup lolpig
    @p vec.__setitem__
*/
int vec_setitem(PyObject* self, Py_ssize_t idx, PyObject* val)
{
    VectorBase* vec = pyobject_cast<VectorBase*>(self);
    if (!checkIndex(idx, vec->len))
        return -1;
    if (!expectFromPython(val, &vec->v[idx]))
        return -1;
    return 0;
}

/** @ingroup lolpig
    @p vec.__len__
*/
Py_ssize_t vec_len(PyObject* self)
{
    VectorBase* vec = pyobject_cast<VectorBase*>(self);
    return vec->len;
}


/** @ingroup lolpig
    @p vec.__iter__
*/
PyObject* vec_iter(PyObject* self)
{
    VectorBase* vec = pyobject_cast<VectorBase*>(self);
    VectorIter* iter = new_VectorIter();
    iter->iter = 0;
    iter->vec = vec;
    Py_INCREF(vec);
    //PRINT("NEWITER " << iter->toString());
    return pyobject_cast<PyObject*>(iter);
}

/** @ingroup lolpig
    @p _vec_iter.__iter__
*/
PyObject* veciter_iter(PyObject* self)
{
    //Py_RETURN_SELF;
    VectorIter* iter = pyobject_cast<VectorIter*>(self);
    //PRINT("ITER " << iter->toString());
    Py_RETURN_OBJECT(iter);
}

/** @ingroup lolpig
    @p _vec_iter.__next__
*/
PyObject* veciter_next(PyObject* self)
{
    VectorIter* iter = pyobject_cast<VectorIter*>(self);
    //PRINT("NEXT " << iter->toString());
    if (!iter->vec)
        return NULL;
    if (iter->iter >= iter->vec->len)
    {
        Py_CLEAR(iter->vec);
        //PyErr_SetObject(PyExc_StopIteration, NULL);
        return NULL;
    }
    return toPython(iter->vec->v[iter->iter++]);
}

/** @ingroup lolpig
    @p _vec_iter.__dealloc__
*/
void veciter_dealloc(PyObject* self)
{
    VectorIter* iter = pyobject_cast<VectorIter*>(self);
    //PRINT("DEALLOC " << iter->toString());
    Py_CLEAR(iter->vec);
    self->ob_type->tp_free(self);
}

// ------------ attributes ----------------

/** @ingroup lolpig
    @p vec.__getattro__
    Swizzle access
*/
PyObject* vec_getattro(PyObject* self, PyObject* name)
{
    std::string s;
    if (fromPython(name, &s))
    {
        VectorBase* vec = pyobject_cast<VectorBase*>(self);
        std::vector<double> ret;
        for (auto c : s)
        {
            int idx = -1;
            if (c == 'x' || c == 'r' || c == 's')
                idx = 0;
            else if (c == 'y' || c == 'g' || c == 't')
                idx = 1;
            else if (c == 'z' || c == 'b' || c == 'p')
                idx = 2;
            else if (c == 'w' || c == 'a' || c == 'q')
                idx = 3;

            if (idx < 0 || idx >= vec->len)
                return PyObject_GenericGetAttr(self, name);
            ret.push_back(vec->v[idx]);
        }
        if (ret.size() == 1)
            toPython(ret[0]);
        return pyobject_cast<PyObject*>(createVector(ret.size(), ret.data()));
    }
    if (PyErr_Occurred())
        return NULL;
    return PyObject_GenericGetAttr(self, name);
}

/** @ingroup lolpig
    @p vec.__setattro__
    Swizzle write access
*/
int vec_setattro(PyObject* self, PyObject* name, PyObject* args)
{
    std::string s;
    if (fromPython(name, &s))
    {
        VectorBase* vec = pyobject_cast<VectorBase*>(self);
        std::vector<int> idxs;
        for (auto c : s)
        {
            int idx = -1;
            if (c == 'x' || c == 'r' || c == 's')
                idx = 0;
            else if (c == 'y' || c == 'g' || c == 't')
                idx = 1;
            else if (c == 'z' || c == 'b' || c == 'p')
                idx = 2;
            else if (c == 'w' || c == 'a' || c == 'q')
                idx = 3;

            if (idx < 0 || idx >= vec->len)
                return PyObject_GenericSetAttr(self, name, args);
            for (auto i : idxs)
                if (idx == i)
                    return PyObject_GenericSetAttr(self, name, args);
            idxs.push_back(idx);
        }
        if (args == NULL)
        {
            for (auto idx : idxs)
                vec->v[idx] = 0.;
            return 0;
        }
        double v[idxs.size()+1];
        int len = VectorBase::parseSequence(args, v, idxs.size()+1);
        if (len < 0)
            return -1;
        if ((size_t)len != idxs.size())
        {
            setPythonError(PyExc_TypeError, SStream()
                           << "Expected sequence of length " << idxs.size() << ", got"
                           << len);
            return -1;
        }
        double* ptr=v;
        for (auto idx : idxs)
            vec->v[idx] = *ptr++;
        return 0;
    }
    if (PyErr_Occurred())
        return -1;
    return PyObject_GenericSetAttr(self, name, args);
}


// ------------ splitting -----------------

/** @ingroup lolpig
    @p vec.split
    split(int) -> [vec,] \n
    Returns a list of vectors split at even intervals. \n
    >>> vec(1,2,3,4).split(2) \n
    [vec(1, 2), vec(3, 4)] \n
*/
PyObject* vec_split(PyObject* self, PyObject* obj)
{
    VectorBase* vec = pyobject_cast<VectorBase*>(self);
    long n;
    if (!expectFromPython(obj, &n))
        return NULL;
    if (n < 1)
        n = 1;
    int len = (vec->len+n-1) / n;
    PyObject* ret = PyList_New(len);
    int read = 0;
    for (int i=0; i<len; ++i)
    {
        int r = std::min(n, (long)(vec->len-read));
        if (r > 0)
        {
            VectorBase* v = createVector(r, &vec->v[read]);
            PyList_SetItem(ret, i, pyobject_cast<PyObject*>(v));
            read += r;
        }
    }
    return ret;
}



// --------------- compare --------------------------

/** @ingroup lolpig
    @p vec.__eq__
*/
PyObject* vec_cmp(PyObject* self, PyObject* arg, int op)
{
    VectorBase* vec = pyobject_cast<VectorBase*>(self);
    bool cmp = true;
    double scalar;
    if (fromPython(arg, &scalar))
    {
        switch (op)
        {
            case Py_LT: for (int i=0; i<vec->len; ++i)
                            if (vec->v[i] >= scalar) { cmp = false; break; } break;
            case Py_LE: for (int i=0; i<vec->len; ++i)
                            if (vec->v[i] > scalar) { cmp = false; break; } break;
            case Py_EQ: for (int i=0; i<vec->len; ++i)
                            if (vec->v[i] != scalar) { cmp = false; break; } break;
            case Py_NE: for (int i=0; i<vec->len; ++i)
                            if (vec->v[i] == scalar) { cmp = false; break; } break;
            case Py_GT: for (int i=0; i<vec->len; ++i)
                            if (vec->v[i] <= scalar) { cmp = false; break; } break;
            case Py_GE: for (int i=0; i<vec->len; ++i)
                            if (vec->v[i] < scalar) { cmp = false; break; } break;
        }
        return toPython(cmp);
    }
    if (PyErr_Occurred())
        return NULL;

    double v[vec->len+1];
    int len = VectorBase::parseSequence(arg, v, vec->len+1);
    if (len < 0)
        return NULL;
    if (len == vec->len)
    {
        switch (op)
        {
            case Py_LT: for (int i=0; i<vec->len; ++i)
                            if (vec->v[i] >= v[i]) { cmp = false; break; } break;
            case Py_LE: for (int i=0; i<vec->len; ++i)
                            if (vec->v[i] > v[i]) { cmp = false; break; } break;
            case Py_EQ: for (int i=0; i<vec->len; ++i)
                            if (vec->v[i] != v[i]) { cmp = false; break; } break;
            case Py_NE: for (int i=0; i<vec->len; ++i)
                            if (vec->v[i] == v[i]) { cmp = false; break; } break;
            case Py_GT: for (int i=0; i<vec->len; ++i)
                            if (vec->v[i] <= v[i]) { cmp = false; break; } break;
            case Py_GE: for (int i=0; i<vec->len; ++i)
                            if (vec->v[i] < v[i]) { cmp = false; break; } break;
        }
        return toPython(cmp);
    }

    Py_RETURN_FALSE;
    /*
    setPythonError(PyExc_TypeError, SStream()
                    << "Expected scalar or float sequence of length "
                    << vec->len << " for comparison, got " << len);
    return NULL;*/
}



// -------------- number stuff ----------------------

/** @ingroup lolpig
    @p vec.__abs__
*/
PyObject* vec_abs(PyObject* self)
{
    VectorBase* vec = pyobject_cast<VectorBase*>(self);
    return pyobject_cast<PyObject*>(vec->unary_op_copy([](double x)
        { return std::abs(x); }));
}

/** @ingroup lolpig
    @p vec.__neg__
*/
PyObject* vec_neg(PyObject* self)
{
    VectorBase* vec = pyobject_cast<VectorBase*>(self);
    return pyobject_cast<PyObject*>(vec->unary_op_copy([](double x)
        { return -x; }));
}

/** @ingroup lolpig
    @p vec.__pos__
*/
PyObject* vec_pos(PyObject* self)
{
    Py_RETURN_SELF;
}

/** @ingroup lolpig
    @p vec.__round__
*/
PyObject* vec_round__(PyObject* self, PyObject* args)
{
    long digits = 0;
    if (PyTuple_Check(args) && PyTuple_Size(args) == 1)
    {
        if (!expectFromPython(PyTuple_GetItem(args, 0), &digits))
            return NULL;
    }
    VectorBase* vec = pyobject_cast<VectorBase*>(self);
    return pyobject_cast<PyObject*>(vec->unary_op_copy([=](double x)
        { return pythonRound(x, digits); }));
}

/** @ingroup lolpig
    @p vec.__ipow__
    __ipow__(float|seq) -> self \n
    Applies the pow() function to all elements, INPLACE \n
    >>> v = vec(1,2,3) \n
    vec(1,2,3) \n
    >>> v **= 2 \n
    vec(1,4,9) \n
    >>> v **= (3,2,1) \n
    vec(1,16,9) \n
*/
PyObject* vec_ipow__(PyObject* self, PyObject* arg, PyObject* /*mod*/)
{
    //PRINT(typeName(arg,true) << " " << typeName(mod, true));
    VectorBase* vec = pyobject_cast<VectorBase*>(self);
    bool err = false;
    if (!vec->binary_op_inplace(arg, [&err](double& l, double r)
    {
        if (!pythonPower(&l, l, r))
            err = true;
    }))
        return NULL;
    if (err)
        return NULL;
    Py_RETURN_SELF;
}

/** @ingroup lolpig
    @p vec.__pow__
    pow(vec, float|seq) -> vec \n
    Applies the pow() function to all elements \n
    >>> pow(vec(1,2,3), 2) \n
    vec(1,4,9) \n
    >>> pow(vec(1,2,3), (1,2,3)) \n
    vec(1,4,27) \n
*/
PyObject* vec_pow__(PyObject* left, PyObject* right, PyObject* /*mod*/)
{
    bool err = false;
    PyObject* ret = VectorBase::binary_op_copy(left, right, [&err](double l, double r)
    {
        if (!pythonPower(&l, l, r))
            err = true;
        return l;
    });
    if (err)
        return NULL;
    return ret;
}



// -------------- arithmetic ----------------------

/** @ingroup lolpig
    @p vec.__iadd__
*/
PyObject* vec_iadd(PyObject* self, PyObject* arg)
{
    VectorBase* vec = pyobject_cast<VectorBase*>(self);
    if (!vec->binary_op_inplace(arg, [](double& l, double r){ l += r; }))
        return NULL;
    Py_RETURN_SELF;
}

/** @ingroup lolpig
    @p vec.__add__
*/
PyObject* vec_add(PyObject* left, PyObject* right)
{
    return VectorBase::binary_op_copy(left, right,
                                     [](double l, double r){ return l + r; });
}


/** @ingroup lolpig
    @p vec.__isub__
*/
PyObject* vec_isub(PyObject* self, PyObject* arg)
{
    VectorBase* vec = pyobject_cast<VectorBase*>(self);
    if (!vec->binary_op_inplace(arg, [](double& l, double r){ l -= r; }))
        return NULL;
    Py_RETURN_SELF;
}

/** @ingroup lolpig
    @p vec.__sub__
*/
PyObject* vec_sub(PyObject* left, PyObject* right)
{
    return VectorBase::binary_op_copy(left, right,
                                     [](double l, double r){ return l - r; });
}


/** @ingroup lolpig
    @p vec.__imul__
*/
PyObject* vec_imul(PyObject* self, PyObject* arg)
{
    VectorBase* vec = pyobject_cast<VectorBase*>(self);
    if (!vec->binary_op_inplace(arg, [](double& l, double r){ l *= r; }))
        return NULL;
    Py_RETURN_SELF;
}

/** @ingroup lolpig
    @p vec.__mul__
*/
PyObject* vec_mul(PyObject* left, PyObject* right)
{
    return VectorBase::binary_op_copy(left, right,
                                     [](double l, double r){ return l * r; });
}


/** @ingroup lolpig
    @p vec.__itruediv__
*/
PyObject* vec_itruediv(PyObject* self, PyObject* arg)
{
    VectorBase* vec = pyobject_cast<VectorBase*>(self);
    bool zero = false;
    if (!vec->binary_op_inplace(arg, [&zero](double& l, double r)
    {
        if (r == 0.) { zero = true; return; }
        l /= r;
    }))
        return NULL;
    if (zero)
    {
        setPythonError(PyExc_ZeroDivisionError, "vector division by zero");
        return NULL;
    }
    Py_RETURN_SELF;
}

/** @ingroup lolpig
    @p vec.__truediv__
*/
PyObject* vec_truediv(PyObject* left, PyObject* right)
{
    bool zero = false;
    PyObject* ret = VectorBase::binary_op_copy(left, right, [&zero](double l, double r)
    {
        if (r == 0.) { zero = true; return 0.; }
        return l / r;
    });
    if (zero)
    {
        setPythonError(PyExc_ZeroDivisionError, "vector division by zero");
        return NULL;
    }
    return ret;
}




/** @ingroup lolpig
    @p vec.__imod__
*/
PyObject* vec_imod(PyObject* self, PyObject* arg)
{
    VectorBase* vec = pyobject_cast<VectorBase*>(self);
    bool zero = false;
    if (!vec->binary_op_inplace(arg, [&zero](double& l, double r)
    {
        if (r == 0.) { zero = true; return; }
        l = pythonModulo(l, r);
    }))
        return NULL;
    if (zero)
    {
        setPythonError(PyExc_ZeroDivisionError, "vector modulo with zero value");
        return NULL;
    }
    Py_RETURN_SELF;
}

/** @ingroup lolpig
    @p vec.__mod__
*/
PyObject* vec_mod(PyObject* left, PyObject* right)
{
    bool zero = false;
    PyObject* ret = VectorBase::binary_op_copy(left, right, [&zero](double l, double r)
    {
        if (r == 0.) { zero = true; return 0.; }
        return pythonModulo(l, r);
    });
    if (zero)
    {
        setPythonError(PyExc_ZeroDivisionError, "vector modulo with zero value");
        return NULL;
    }
    return ret;
}


// ----------------- inplace methods -------------------------

/** @ingroup lolpig
    @p vec.round
    round() -> self \n
    round(num_digits) -> self \n
    Applies the round() function to all elements, INPLACE \n
*/
PyObject* vec_round(PyObject* self, PyObject* args)
{
    long digits = 0;
    if (PyTuple_Check(args) && PyTuple_Size(args) == 1)
    {
        if (!expectFromPython(PyTuple_GetItem(args, 0), &digits))
            return NULL;
    }
    VectorBase* vec = pyobject_cast<VectorBase*>(self);
    vec->unary_op_inplace([=](double x){ return pythonRound(x, digits); });
    Py_RETURN_SELF;
}


/** @ingroup lolpig
    @p vec.floor
    floor() -> self \n
    Applies the floor() function to all elements, INPLACE
*/
PyObject* vec_floor(PyObject* self)
{
    VectorBase* vec = pyobject_cast<VectorBase*>(self);
    vec->unary_op_inplace([](double x){ return std::floor(x); });
    Py_RETURN_SELF;
}


/** @ingroup lolpig
    @p vec.normalize
    normalize() -> self \n
    Normalizes the vector, INPLACE \n
    This essentially makes the vector length 1 \n
    Does nothing when vector is length 0 \n
*/
PyObject* vec_normalize(PyObject* self)
{
    VectorBase* vec = pyobject_cast<VectorBase*>(self);
    double l = vec->lengthSquared();
    if (l)
    {
        l = 1. / std::sqrt(l);
        vec->unary_op_inplace([=](double x){ return x*l; });
    }
    Py_RETURN_SELF;
}



// ----------------- getter -------------------------------

/** @ingroup lolpig
    @p vec.length
    length() -> float \n
    Returns euclidean length of vector.
*/
PyObject* vec_length(PyObject* self)
{
    VectorBase* vec = pyobject_cast<VectorBase*>(self);
    return toPython(std::sqrt(vec->lengthSquared()));
}

/** @ingroup lolpig
    @p vec.length_squared
    length_squared() -> float \n
    Returns square of euclidean length of vector. \n
    Faster than length()
*/
PyObject* vec_length_squared(PyObject* self)
{
    VectorBase* vec = pyobject_cast<VectorBase*>(self);
    return toPython(vec->lengthSquared());
}


/** @ingroup lolpig
    @p vec.distance
    distance(seq) -> float \n
    Returns euclidean distance between this and other vector. \n
    Length must be equal.
*/
PyObject* vec_distance(PyObject* self, PyObject* args)
{
    VectorBase* vec = pyobject_cast<VectorBase*>(self);
    double v[vec->len];
    if (VectorBase::parseSequence(args, v, vec->len) < 0)
        return NULL;
    double l = 0.;
    for (int i=0; i<vec->len; ++i)
        l += std::pow(v[i] - vec->v[i], 2.);
    return toPython(std::sqrt(l));
}

/** @ingroup lolpig
    @p vec.distance_squared
    distance_squared(seq) -> float \n
    Returns square of euclidean distance between this and other vector. \n
    Length must be equal.
*/
PyObject* vec_distance_squared(PyObject* self, PyObject* args)
{
    VectorBase* vec = pyobject_cast<VectorBase*>(self);
    double v[vec->len];
    if (VectorBase::parseSequence(args, v, vec->len) < 0)
        return NULL;
    double l = 0.;
    for (int i=0; i<vec->len; ++i)
        l += std::pow(v[i] - vec->v[i], 2.);
    return toPython(l);
}


/** @ingroup lolpig
    @p vec.dot
    dot(seq) -> float \n
    Returns dot product of this vector and other sequence. \n
    Number of elements must be equal.
*/
PyObject* vec_dot(PyObject* self, PyObject* args)
{
    VectorBase* vec = pyobject_cast<VectorBase*>(self);
    double v[vec->len+1];
    int len = VectorBase::parseSequence(args, v, vec->len+1);
    if (len < 0)
        return NULL;
    if (len != vec->len)
    {
        setPythonError(PyExc_TypeError, SStream() << "expected sequence of length "
                        << vec->len << ", got " << len);
        return NULL;
    }
    double l = 0.;
    for (int i=0; i<vec->len; ++i)
        l += v[i] * vec->v[i];
    return toPython(l);
}



// -------------- copy functions --------------------

/** @ingroup lolpig
    @p vec.rounded
*/
PyObject* vec_rounded(PyObject* self, PyObject* args)
{
    return vec_round__(self, args);
}

/** @ingroup lolpig
    @p vec.floored
*/
PyObject* vec_floored(PyObject* self)
{
    VectorBase* vec = pyobject_cast<VectorBase*>(self);
    return pyobject_cast<PyObject*>(
        vec->unary_op_copy([](double x) { return std::floor(x); }));
}

/** @ingroup lolpig
    @p vec.normalized
*/
PyObject* vec_normalized(PyObject* self)
{
    VectorBase* vec = pyobject_cast<VectorBase*>(self);
    double l = vec->lengthSquared();
    if (!l)
        Py_RETURN_SELF;
    l = 1./std::sqrt(l);
    return pyobject_cast<PyObject*>(
        vec->unary_op_copy([=](double x) { return x * l; }));
}




// -------------------- C constructors -----------------------

} // extern "C"

VectorBase* createVector(int len, const double* v, int stride)
{
    VectorBase* vec;
    switch (len)
    {
        default: vec = new_VectorBase(); break;
        case 3: vec = pyobject_cast<VectorBase*>(new_Vector3()); break;
    }
    vec->alloc(len);
    if (v)
        for (int i=0; i<len; ++i)
            vec->v[i] = v[i*stride];
    return vec;
}

VectorBase* createVector(double x, double y, double z)
{
    VectorBase* vec = createVector(3);
    vec->v[0] = x;
    vec->v[1] = y;
    vec->v[2] = z;
    return vec;
}



extern "C" {

// ---------------------- helper -----------------------------



void VectorBase::alloc(int len)
{
    PYVEC_DEBUG("ALLOC " << len);
    this->v = (double*)malloc(len * sizeof(double));
    this->len = len;
}

void VectorBase::dealloc()
{
    PYVEC_DEBUG("DEALLOC");
    if (v)
        free(v);
    v = NULL;
}

void VectorBase::set(double val)
{
    for (int i=0; i<len; ++i)
        v[i] = val;
}

VectorBase* VectorBase::copyClass() const
{
    VectorBase* vec = PyObject_New(VectorBase, this->ob_base.ob_type);
    vec->alloc(this->len);
    if (is_MatrixBase(const_cast<PyObject*>(reinterpret_cast<const PyObject*>(this))))
    {
        pyobject_cast<MatrixBase*>(vec)->num_rows = reinterpret_cast<const MatrixBase*>(this)->num_rows;
        pyobject_cast<MatrixBase*>(vec)->num_cols = reinterpret_cast<const MatrixBase*>(this)->num_cols;
    }
    return vec;
}

VectorBase* VectorBase::copy() const
{
    VectorBase* vec = this->copyClass();
    for (int i=0; i<this->len; ++i)
        vec->v[i] = this->v[i];
    return vec;
}


VectorBase* VectorBase::unary_op_copy(std::function<double(double)> op) const
{
    VectorBase* vec = this->copyClass();
    for (int i=0; i<this->len; ++i)
        vec->v[i] = op(this->v[i]);
    return vec;
}
void VectorBase::unary_op_inplace(std::function<double(double)> op) const
{
    for (int i=0; i<this->len; ++i)
        this->v[i] = op(this->v[i]);
}

bool VectorBase::binary_op_inplace(
        PyObject* arg, std::function<void(double&, double)> op)
{
    double f;
    if (fromPython(arg, &f))
    {
        for (int i=0; i<this->len; ++i)
            op(this->v[i], f);
        return true;
    }
    if (PyErr_Occurred())
        return false;
    if (is_VectorBase(arg))
    {
        VectorBase* varg = pyobject_cast<VectorBase*>(arg);
        if (this->len != varg->len)
        {
            setPythonError(PyExc_TypeError,
                           SStream() << "expected vector of length " << this->len
                           << ", got " << varg->len);
            return false;
        }
        for (int i=0; i<this->len; ++i)
            op(this->v[i], varg->v[i]);
        return true;
    }
    if (PySequence_Check(arg))
    {
        if (this->len != PySequence_Length(arg))
        {
            setPythonError(PyExc_TypeError,
                           SStream() << "expected vector of length " << this->len
                           << ", got " << PySequence_Length(arg));
            return false;
        }
        for (int i=0; i<this->len; ++i)
        {
            double f;
            if (!expectFromPython(PySequence_GetItem(arg, i), &f))
                return false;
            op(this->v[i], f);
        }
        return true;
    }
    setPythonError(PyExc_TypeError, SStream() <<
                   "expected float scalar or sequence, got " << typeName(arg));
    return false;
}

PyObject* VectorBase::binary_op_copy(PyObject* left, PyObject* right,
                                     std::function<double(double l, double r)> op)
{
    // scalar * vec
    double val;
    if (fromPython(left, &val))
    {
        if (!is_VectorBase(right))
        {
            setPythonError(PyExc_TypeError,
                           SStream() << "unexpected right operand " << typeName(right));
            return NULL;
        }
        VectorBase* vright = pyobject_cast<VectorBase*>(right);
        VectorBase* ret = PyObject_NEW(VectorBase, vright->ob_base.ob_type);
        ret->alloc(vright->len);
        for (int i=0; i<vright->len; ++i)
            ret->v[i] = op(val, vright->v[i]);
        return pyobject_cast<PyObject*>(ret);
    }
    if (PyErr_Occurred())
        return NULL;

    // list * vec
    if (!is_VectorBase(left))
    {
        if (!is_VectorBase(right))
        {
            setPythonError(PyExc_TypeError,
                           SStream() << "unexpected right operand " << typeName(right));
            return NULL;
        }
        if (!PySequence_Check(left))
        {
            setPythonError(PyExc_TypeError,
                           SStream() << "unexpected left operand " << typeName(left));
            return NULL;
        }
        VectorBase* vright = pyobject_cast<VectorBase*>(right);
        if (vright->len != PySequence_Length(left))
        {
            setPythonError(PyExc_TypeError,
                           SStream() << "expected vector of length " << vright->len
                           << ", got " << PySequence_Length(left));
            return NULL;
        }
        VectorBase* ret = PyObject_NEW(VectorBase, vright->ob_base.ob_type);
        ret->alloc(vright->len);
        for (int i=0; i<vright->len; ++i)
        {
            double f;
            if (!expectFromPython(PySequence_GetItem(left, i), &f))
            {
                Py_DECREF(ret);
                return NULL;
            }
            ret->v[i] = op(f, vright->v[i]);
        }
        return pyobject_cast<PyObject*>(ret);
    }
    VectorBase* vleft = pyobject_cast<VectorBase*>(left);

    VectorBase* ret = PyObject_NEW(VectorBase, left->ob_type);
    ret->alloc(vleft->len);

    // vec * scalar
    if (fromPython(right, &val))
    {
        for (int i=0; i<vleft->len; ++i)
            ret->v[i] = op(vleft->v[i], val);
        return pyobject_cast<PyObject*>(ret);
    }
    if (PyErr_Occurred())
        return NULL;

    // vec * vec
    if (is_VectorBase(right))
    {
        VectorBase* vright = pyobject_cast<VectorBase*>(right);
        if (vleft->len != vright->len)
        {
            setPythonError(PyExc_TypeError,
                           SStream() << "expected vector of length " << vleft->len
                           << ", got " << vright->len);
            Py_DECREF(ret);
            return NULL;
        }
        for (int i=0; i<vleft->len; ++i)
            ret->v[i] = op(vleft->v[i], vright->v[i]);
        return pyobject_cast<PyObject*>(ret);
    }
    // vec * seq
    if (PySequence_Check(right))
    {
        if (vleft->len != PySequence_Length(right))
        {
            setPythonError(PyExc_TypeError,
                           SStream() << "expected vector of length " << vleft->len
                           << ", got " << PySequence_Length(right));
            Py_DECREF(ret);
            return NULL;
        }
        for (int i=0; i<vleft->len; ++i)
        {
            double f;
            if (!expectFromPython(PySequence_GetItem(right, i), &f))
            {
                Py_DECREF(ret);
                return NULL;
            }
            ret->v[i] = op(vleft->v[i], f);
        }
        return pyobject_cast<PyObject*>(ret);
    }
    setPythonError(PyExc_TypeError, SStream() <<
                   "expected float scalar or sequence as right argument, got "
                   << typeName(right));
    Py_DECREF(ret);
    return NULL;
}



std::string VectorBase::toString(const std::string& name, int group) const
{
    // auto square-matrix formatting
    if (!group && this->len >= 9)
        if (double a = std::sqrt(this->len))
            if (a == std::floor(a))
                group = a;

    std::stringstream s;
    s << name << "(";
    for (int i=0; i<this->len; ++i)
    {
        if (i>0)
        {
             s << ",";
             if (!group || i%group == 0)
                 s << " ";
        }
        s << this->v[i];
    }
    s << ")";
    return s.str();
}

std::string VectorBase::toRepr(const std::string& name, int groups) const
{
    std::stringstream s;
    //s << (void*)this << ":";
    s << toString(name, groups);
    return s.str();
}

std::string VectorIter::toString() const
{
    std::stringstream s;
    s << "VectorIter(" << (void*)this << ", ref=" << this->ob_base.ob_refcnt
      << ", pos="
      << this->iter << ", ";
    if (this->vec)
        s << this->vec->toString("vec") << ", ref=" << this->vec->ob_base.ob_refcnt;
    else
        s << "NULL";
    s << ")";
    return s.str();
}


namespace {

    bool isValidSequenceType(PyObject* seq)
    {
        return PySequence_Check(seq) && !(
                    PyUnicode_Check(seq)
                 || PyByteArray_Check(seq)
                    );
    }

    bool parseSequencePart(PyObject* seq, int* write, int max_len, double* v)
    {
        int seq_len = PySequence_Length(seq);
        int seq_pos = 0;
        while (*write < max_len && seq_pos < seq_len)
        {
            PyObject* item = PySequence_GetItem(seq, seq_pos);
            if (fromPython(item, v ? &v[*write] : NULL))
            {
                ++(*write);
                ++seq_pos;
                continue;
            }
            if (PyErr_Occurred())
                return false;
            if (isValidSequenceType(item))
            {
                if (!parseSequencePart(item, write, max_len, v))
                    return false;
                ++seq_pos;
            }
            else
            {
                setPythonError(PyExc_TypeError, SStream()
                               << "expected float in sequence, got "
                               << typeName(item));
                return false;
            }
        }
        return true;
    }

}

int VectorBase::parseSequence(PyObject* seq, double* v, int max_len, int def_len)
{
    if (def_len==0 && max_len)
        def_len = max_len;
    if (max_len==0)
        max_len = std::numeric_limits<int>::max();
    // scalar value fills whole def_len
    if (fromPython(seq, v))
    {
        if (v)
            for (int i=1; i<def_len; ++i)
                v[i] = v[0];
        return 1;
    }
    if (PyErr_Occurred())
        return -1;
    // otherwise parse any sequence type
    if (!isValidSequenceType(seq))
    {
        setPythonError(PyExc_TypeError, SStream() << "expected scalar or sequence, got "
                       << typeName(seq));
        return -1;
    }
    // scalar value in tuple/seq?
    if (PySequence_Length(seq) == 1)
    {
        PyObject* item = PySequence_GetItem(seq, 0);
        if (fromPython(item, v))
        {
            if (v)
                for (int i=1; i<def_len; ++i)
                    v[i] = v[0];
            return 1;
        }
        if (PyErr_Occurred())
            return -1;
    }
    // parse any float/sequence combination
    int write = 0;
    if (!parseSequencePart(seq, &write, max_len, v))
        return -1;

    // zero rest
    if (v)
        for (int i=write; i<def_len; ++i)
            v[i] = 0.;

    return write;
}

bool VectorBase::parseSequenceExactly(PyObject* seq, double* v, int elen)
{
    if (!v)
    {
        int len = parseSequence(seq, NULL, elen+1);
        if (len < 0)
            return false;
        if (len != elen)
        {
            setPythonError(PyExc_TypeError, SStream()
                           << "Expected sequence of length " << elen << ", got "
                           << len);
            return false;
        }
        return true;
    }
    double tmp[elen+1];
    int len = parseSequence(seq, tmp, elen+1);
    if (len < 0)
        return false;
    if (len != elen)
    {
        setPythonError(PyExc_TypeError, SStream()
                       << "Expected sequence of length " << elen << ", got "
                       << len);
        return false;
    }
    VEC::vec_copy(v, tmp, elen);
    return true;
}


double VectorBase::lengthSquared() const
{
    double l = 0.;
    for (int i=0; i<len; ++i)
        l += v[i] * v[i];
    return l;
}

} // extern "C"
	
} // namespace PYTHON
} // namespace MO
