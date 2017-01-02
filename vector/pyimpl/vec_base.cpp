#include "vec_base.h"
#include "mat_base.h"

#ifdef CPP11
#   include <limits>
#endif

namespace MO {
namespace PYTHON {

using namespace PyUtils;

extern "C" {


LOLPIG_DEF( vec.__new__, )
PyObject* vec_new(struct _typeobject* type, PyObject* args, PyObject* )
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

    return reinterpret_cast<PyObject*>(vec);
}

/*
LOLPIG_DEF( vec.__init__, )
int vec_init(PyObject* self, PyObject* args, PyObject* )
{
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
    vec->len = VectorBase::parseSequence(args, vec->v, 16);
    if (vec->len == 0)
        return -1;
    PRINT("NEW " << vec->toString());
    return 0;
}
*/

LOLPIG_DEF( vec.__dealloc__, )
void vec_free(PyObject* self)
{
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
    vec->dealloc();
    self->ob_type->tp_free(self);
}

LOLPIG_DEF( vec.copy, )
PyObject* vec_copy(PyObject* self)
{
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
    return reinterpret_cast<PyObject*>(vec->copy());
}


LOLPIG_DEF( vec.__repr__, )
PyObject* vec_repr(PyObject* self)
{
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
    return toPython(vec->toRepr("vec"));
}

LOLPIG_DEF( vec.__str__, )
PyObject* vec_str(PyObject* self)
{
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
    return toPython(vec->toString("vec"));
}





// ------------------------ sequence methods ---------------------------

LOLPIG_DEF( vec.__contains__, )
int vec_contains(PyObject* self, PyObject* obj)
{
    double v;
    if (!fromPython(obj, &v))
        return 0;

    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
    for (int i=0; i<vec->len; ++i)
        if (vec->v[i] == v)
            return 1;
    return 0;
}

LOLPIG_DEF( vec.__getitem__, )
PyObject* vec_getitem(PyObject* self, Py_ssize_t idx)
{
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
    return toPython(vec->v[idx]);
}

LOLPIG_DEF( vec.__setitem__, )
int vec_setitem(PyObject* self, Py_ssize_t idx, PyObject* val)
{
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
    if (!fromPython(val, &vec->v[idx]))
        return -1;
    return 0;
}

LOLPIG_DEF( vec.__len__, )
Py_ssize_t vec_len(PyObject* self)
{
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
    return vec->len;
}


LOLPIG_DEF( vec.__iter__, )
PyObject* vec_iter(PyObject* self)
{
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
    VectorIter* iter = new_VectorIter();
    iter->iter = 0;
    iter->vec = vec;
    Py_INCREF(vec);
    //PRINT("NEWITER " << iter->toString());
    return reinterpret_cast<PyObject*>(iter);
}

LOLPIG_DEF( _vec_iter.__iter__, )
PyObject* veciter_iter(PyObject* self)
{
    //Py_RETURN_SELF;
    VectorIter* iter = reinterpret_cast<VectorIter*>(self);
    //PRINT("ITER " << iter->toString());
    Py_RETURN_OBJECT(iter);
}

LOLPIG_DEF( _vec_iter.__next__, )
PyObject* veciter_next(PyObject* self)
{
    VectorIter* iter = reinterpret_cast<VectorIter*>(self);
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

LOLPIG_DEF( _vec_iter.__dealloc__, )
void veciter_dealloc(PyObject* self)
{
    VectorIter* iter = reinterpret_cast<VectorIter*>(self);
    //PRINT("DEALLOC " << iter->toString());
    Py_CLEAR(iter->vec);
    self->ob_type->tp_free(self);
}


// ------------ splitting -----------------

LOLPIG_DEF( vec.split, (
    split(int) -> [vec,]
    Returns a list of vectors split at even intervals.
    >>> vec(1,2,3,4).split(2)
    [vec(1, 2), vec(3, 4)]
    ))
PyObject* vec_split(PyObject* self, PyObject* obj)
{
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
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
            PyList_SetItem(ret, i, reinterpret_cast<PyObject*>(v));
            read += r;
        }
    }
    return ret;
}



// -------------- number stuff ----------------------

LOLPIG_DEF( vec.__abs__, )
PyObject* vec_abs(PyObject* self)
{
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
#ifdef CPP11
    return reinterpret_cast<PyObject*>(vec->unary_op_copy([](double x)
        { return std::abs(x); }));
#endif
}

LOLPIG_DEF( vec.__neg__, )
PyObject* vec_neg(PyObject* self)
{
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
#ifdef CPP11
    return reinterpret_cast<PyObject*>(vec->unary_op_copy([](double x)
        { return -x; }));
#endif
}

LOLPIG_DEF( vec.__pos__, )
PyObject* vec_pos(PyObject* self)
{
    Py_RETURN_SELF;
}

LOLPIG_DEF( vec.__round__, )
PyObject* vec_round__(PyObject* self, PyObject* args)
{
    long digits = 0;
    if (PyTuple_Check(args) && PyTuple_Size(args) == 1)
        fromPython(PyTuple_GetItem(args, 0), &digits);
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
#ifdef CPP11
    return reinterpret_cast<PyObject*>(vec->unary_op_copy([=](double x)
        { return pythonRound(x, digits); }));
#endif
}

LOLPIG_DEF( test, )
PyObject* test_foo(PyObject*, PyObject* args)
{
    PYVEC_DEBUG(typeName(args, true));
    Py_RETURN_NONE;
}

LOLPIG_DEF( vec.__pow__, (
    pow(seq[, seq]) -> vec
    Applies the pow() function to all elements
    >>> pow(vec(1,2,3), 2)
    vec(1,4,9)
    >>> pow(vec(1,2,3), (1,2,3))
    vec(1,4,27)
    ))
PyObject* vec_pow__(PyObject* self, PyObject* args, PyObject* /*kwargs*/)
{
    PYVEC_DEBUG(typeName(args, true));
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
    // exp only
    double e[vec->len];
    if (VectorBase::parseSequence(args, e, vec->len) < 0)
        return NULL;
#ifdef CPP11
    double* p=e;
    return reinterpret_cast<PyObject*>(
                vec->unary_op_copy([&](double x) { return std::pow(x, *p++); }));
#endif
    Py_RETURN_SELF;
}


// -------------- arithmetic ----------------------

LOLPIG_DEF( vec.__iadd__, )
PyObject* vec_iadd(PyObject* self, PyObject* arg)
{
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
#ifndef GCC_XML
    if (!vec->binary_op_inplace(arg, [](double& l, double r){ l += r; }))
        return NULL;
#endif
    Py_RETURN_SELF;
}

LOLPIG_DEF( vec.__add__, )
PyObject* vec_add(PyObject* left, PyObject* right)
{
#ifndef GCC_XML
    return VectorBase::binary_op_copy(left, right,
                                     [](double l, double r){ return l + r; });
#endif
}


LOLPIG_DEF( vec.__isub__, )
PyObject* vec_isub(PyObject* self, PyObject* arg)
{
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
#ifndef GCC_XML
    if (!vec->binary_op_inplace(arg, [](double& l, double r){ l -= r; }))
        return NULL;
#endif
    Py_RETURN_SELF;
}

LOLPIG_DEF( vec.__sub__, )
PyObject* vec_sub(PyObject* left, PyObject* right)
{
#ifndef GCC_XML
    return VectorBase::binary_op_copy(left, right,
                                     [](double l, double r){ return l - r; });
#endif
}


LOLPIG_DEF( vec.__imul__, )
PyObject* vec_imul(PyObject* self, PyObject* arg)
{
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
#ifndef GCC_XML
    if (!vec->binary_op_inplace(arg, [](double& l, double r){ l *= r; }))
        return NULL;
#endif
    Py_RETURN_SELF;
}

LOLPIG_DEF( vec.__mul__, )
PyObject* vec_mul(PyObject* left, PyObject* right)
{
#ifndef GCC_XML
    return VectorBase::binary_op_copy(left, right,
                                     [](double l, double r){ return l * r; });
#endif
}


LOLPIG_DEF( vec.__itruediv__, )
PyObject* vec_itruediv(PyObject* self, PyObject* arg)
{
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
#ifndef GCC_XML
    if (!vec->binary_op_inplace(arg, [](double& l, double r){ l /= r; }))
        return NULL;
#endif
    Py_RETURN_SELF;
}

LOLPIG_DEF( vec.__truediv__, )
PyObject* vec_truediv(PyObject* left, PyObject* right)
{
#ifndef GCC_XML
    return VectorBase::binary_op_copy(left, right,
                                     [](double l, double r){ return l / r; });
#endif
}



// ----------------- inplace methods -------------------------

LOLPIG_DEF( vec.round, (
    round() -> self
    round(num_digits) -> self
    Applies the round() function to all elements, INPLACE
    ))
PyObject* vec_round(PyObject* self, PyObject* args)
{
    long digits = 0;
    if (PyTuple_Check(args) && PyTuple_Size(args) == 1)
        fromPython(PyTuple_GetItem(args, 0), &digits);
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
#ifdef CPP11
    vec->unary_op_inplace([=](double x){ return pythonRound(x, digits); });
#endif
    Py_RETURN_SELF;
}


LOLPIG_DEF( vec.floor, (
    floor() -> self
    Applies the floor() function to all elements, INPLACE
    ))
PyObject* vec_floor(PyObject* self)
{
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
#ifdef CPP11
    vec->unary_op_inplace([](double x){ return std::floor(x); });
#endif
    Py_RETURN_SELF;
}


LOLPIG_DEF( vec.normalize, (
    normalize() -> self
    Normalizes the vector, INPLACE
    This essentially makes the vector length 1
    Does nothing when vector is length 0
    ))
PyObject* vec_normalize(PyObject* self)
{
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
    double l = vec->lengthSquared();
    if (l)
    {
        l = 1. / std::sqrt(l);
#ifdef CPP11
        vec->unary_op_inplace([=](double x){ return x*l; });
#endif
    }
    Py_RETURN_SELF;
}



// ----------------- getter -------------------------------

LOLPIG_DEF(vec.length, (
    length() -> float
    Returns euclidean length of vector.
    ))
PyObject* vec_length(PyObject* self)
{
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
    return toPython(std::sqrt(vec->lengthSquared()));
}

LOLPIG_DEF(vec.length_squared, (
    length_squared() -> float
    Returns square of euclidean length of vector.
    Faster than length()
    ))
PyObject* vec_length_squared(PyObject* self)
{
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
    return toPython(vec->lengthSquared());
}


LOLPIG_DEF(vec.distance, (
    distance(seq) -> float
    Returns euclidean distance between this and other vector.
    Length must be equal.
    ))
PyObject* vec_distance(PyObject* self, PyObject* args)
{
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
    double v[vec->len];
    if (VectorBase::parseSequence(args, v, vec->len) < 0)
        return NULL;
    double l = 0.;
    for (int i=0; i<vec->len; ++i)
        l += std::pow(v[i] - vec->v[i], 2.);
    return toPython(std::sqrt(l));
}

LOLPIG_DEF(vec.distance_squared, (
    distance_squared(seq) -> float
    Returns square of euclidean distance between this and other vector.
    Length must be equal.
    ))
PyObject* vec_distance_squared(PyObject* self, PyObject* args)
{
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
    double v[vec->len];
    if (VectorBase::parseSequence(args, v, vec->len) < 0)
        return NULL;
    double l = 0.;
    for (int i=0; i<vec->len; ++i)
        l += std::pow(v[i] - vec->v[i], 2.);
    return toPython(l);
}


LOLPIG_DEF(vec.dot, (
    dot(seq) -> float
    Returns dot product of this vector and other sequence.
    Length must be equal.
    ))
PyObject* vec_dot(PyObject* self, PyObject* args)
{
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
    double v[vec->len];
    if (VectorBase::parseSequence(args, v, vec->len) < 0)
        return NULL;
    double l = 0.;
    for (int i=0; i<vec->len; ++i)
        l += v[i] * vec->v[i];
    return toPython(l);
}




// -------------- copy functions --------------------

LOLPIG_DEF( vec.rounded, )
PyObject* vec_rounded(PyObject* self, PyObject* args)
{
    return vec_round__(self, args);
}

LOLPIG_DEF( vec.floored, )
PyObject* vec_floored(PyObject* self)
{
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
#ifdef CPP11
    return reinterpret_cast<PyObject*>(
        vec->unary_op_copy([](double x) { return std::floor(x); }));
#endif
}

LOLPIG_DEF( vec.normalized, )
PyObject* vec_normalized(PyObject* self)
{
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
    double l = vec->lengthSquared();
    if (!l)
        Py_RETURN_SELF;
    l = 1./std::sqrt(l);
#ifdef CPP11
    return reinterpret_cast<PyObject*>(
        vec->unary_op_copy([=](double x) { return x * l; }));
#endif
}




// -------------------- C constructors -----------------------

} // extern "C"

VectorBase* createVector(int len, const double* v, int stride)
{
    VectorBase* vec;
    switch (len)
    {
        default: vec = new_VectorBase(); break;
        case 3: vec = reinterpret_cast<VectorBase*>(new_Vector3()); break;
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
        reinterpret_cast<MatrixBase*>(vec)->num_rows = reinterpret_cast<const MatrixBase*>(this)->num_rows;
        reinterpret_cast<MatrixBase*>(vec)->num_cols = reinterpret_cast<const MatrixBase*>(this)->num_cols;
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


#ifdef CPP11
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

#endif

std::string VectorBase::toString(const std::string& name, int group) const
{
    if (!group && this->len > 5)
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
        }
        else if (PySequence_Check(item))
        {
            if (!parseSequencePart(item, write, max_len, v))
                return false;
            ++seq_pos;
        }
        else
        {
            setPythonError(PyExc_TypeError, SStream() << "expected float in sequence, got "
                           << typeName(item));
            return false;
        }
    }
    return true;
}


int VectorBase::parseSequence(PyObject* seq, double* v, int max_len, int def_len)
{
    if (def_len==0 && max_len)
        def_len = max_len;
    if (max_len==0)
    {
#ifdef CPP11
        max_len = std::numeric_limits<int>::max();
#endif
    }
    // scalar value fills whole def_len
    if (fromPython(seq, v))
    {
        if (v)
            for (int i=1; i<def_len; ++i)
                v[i] = v[0];
        return 1;
    }
    if (!PySequence_Check(seq))
    {
        setPythonError(PyExc_TypeError, SStream() << "expected scalar or sequence, got "
                       << typeName(seq));
        return -1;
    }
    // scalar value again
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


bool VectorBase::binary_op_inplace(PyObject* arg,
                                  void(*op)(double& l, double r))
{
    double f;
    if (fromPython(arg, &f))
    {
        for (int i=0; i<this->len; ++i)
            op(this->v[i], f);
        return true;
    }
    if (is_VectorBase(arg))
    {
        VectorBase* varg = reinterpret_cast<VectorBase*>(arg);
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
                                    double(*op)(double l, double r))
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
        VectorBase* vright = reinterpret_cast<VectorBase*>(right);
        VectorBase* ret = PyObject_NEW(VectorBase, vright->ob_base.ob_type);
        ret->alloc(vright->len);
        for (int i=0; i<vright->len; ++i)
            ret->v[i] = op(val, vright->v[i]);
        return reinterpret_cast<PyObject*>(ret);
    }
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
        VectorBase* vright = reinterpret_cast<VectorBase*>(right);
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
        return reinterpret_cast<PyObject*>(ret);
    }
    VectorBase* vleft = reinterpret_cast<VectorBase*>(left);

    VectorBase* ret = PyObject_NEW(VectorBase, left->ob_type);
    ret->alloc(vleft->len);

    // vec * scalar
    if (fromPython(right, &val))
    {
        for (int i=0; i<vleft->len; ++i)
            ret->v[i] = op(vleft->v[i], val);
        return reinterpret_cast<PyObject*>(ret);
    }
    // vec * vec
    if (is_VectorBase(right))
    {
        VectorBase* vright = reinterpret_cast<VectorBase*>(right);
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
        return reinterpret_cast<PyObject*>(ret);
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
        return reinterpret_cast<PyObject*>(ret);
    }
    setPythonError(PyExc_TypeError, SStream() <<
                   "expected float scalar or sequence as right argument, got "
                   << typeName(right));
    Py_DECREF(ret);
    return NULL;
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
