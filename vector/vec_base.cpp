#include "vec_base.h"

namespace MO {
namespace PYTHON {

using namespace PyUtils;

extern "C" {


LOLPIG_DEF( vec.__new__, )
PyObject* vec_new(struct _typeobject* type, PyObject* args, PyObject* )
{
    double v[16];
    int len = VectorBase::parseSequence(args, v, 16);
    if (len==0)
        return NULL;
    //PRINT(typeName(args) << " " << len);

    VectorBase* vec = PyObject_NEW(VectorBase, type);
    vec->alloc(len);
    for (int i=0; i<len; ++i)
        vec->v[i] = v[i];

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
    return toPython(vec->toRepr());
}

LOLPIG_DEF( vec.__str__, )
PyObject* vec_str(PyObject* self)
{
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
    return toPython(vec->toString());
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



// -------------- arithmetic ----------------------

LOLPIG_DEF( vec.__iadd__, )
PyObject* vec_iadd(PyObject* self, PyObject* arg)
{
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
#ifndef GCC_XML
    if (!vec->inplace_operator(arg, [](double& l, double r){ l += r; }))
        return NULL;
#endif
    Py_RETURN_SELF;
}

LOLPIG_DEF( vec.__add__, )
PyObject* vec_add(PyObject* left, PyObject* right)
{
#ifndef GCC_XML
    return VectorBase::copy_operator(left, right,
                                     [](double l, double r){ return l + r; });
#endif
}


LOLPIG_DEF( vec.__isub__, )
PyObject* vec_isub(PyObject* self, PyObject* arg)
{
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
#ifndef GCC_XML
    if (!vec->inplace_operator(arg, [](double& l, double r){ l -= r; }))
        return NULL;
#endif
    Py_RETURN_SELF;
}

LOLPIG_DEF( vec.__sub__, )
PyObject* vec_sub(PyObject* left, PyObject* right)
{
#ifndef GCC_XML
    return VectorBase::copy_operator(left, right,
                                     [](double l, double r){ return l - r; });
#endif
}


LOLPIG_DEF( vec.__imul__, )
PyObject* vec_imul(PyObject* self, PyObject* arg)
{
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
#ifndef GCC_XML
    if (!vec->inplace_operator(arg, [](double& l, double r){ l *= r; }))
        return NULL;
#endif
    Py_RETURN_SELF;
}

LOLPIG_DEF( vec.__mul__, )
PyObject* vec_mul(PyObject* left, PyObject* right)
{
#ifndef GCC_XML
    return VectorBase::copy_operator(left, right,
                                     [](double l, double r){ return l * r; });
#endif
}


LOLPIG_DEF( vec.__itruediv__, )
PyObject* vec_itruediv(PyObject* self, PyObject* arg)
{
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
#ifndef GCC_XML
    if (!vec->inplace_operator(arg, [](double& l, double r){ l /= r; }))
        return NULL;
#endif
    Py_RETURN_SELF;
}

LOLPIG_DEF( vec.__truediv__, )
PyObject* vec_truediv(PyObject* left, PyObject* right)
{
#ifndef GCC_XML
    return VectorBase::copy_operator(left, right,
                                     [](double l, double r){ return l / r; });
#endif
}


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

VectorBase* VectorBase::copy() const
{
    VectorBase* vec = PyObject_New(VectorBase, this->ob_base.ob_type);
    vec->alloc(this->len);
    for (int i=0; i<this->len; ++i)
        vec->v[i] = this->v[i];
    return vec;
}


std::string VectorBase::toString(const std::string& name) const
{
    std::stringstream s;
    s << name << "(";
    for (int i=0; i<this->len; ++i)
    {
        if (i>0)
             s << ", ";
        s << this->v[i];
    }
    s << ")";
    return s.str();
}

std::string VectorBase::toRepr(const std::string& name) const
{
    std::stringstream s;
    //s << (void*)this << ":";
    s << toString(name);
    return s.str();
}

std::string VectorIter::toString() const
{
    std::stringstream s;
    s << "VectorIter(" << (void*)this << ", ref=" << this->ob_base.ob_refcnt
      << ", pos="
      << this->iter << ", ";
    if (this->vec)
        s << this->vec->toString() << ", ref=" << this->vec->ob_base.ob_refcnt;
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
        if (fromPython(item, &v[*write]))
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


int VectorBase::parseSequence(PyObject* seq, double* v, int max_len)
{
    if (isNone(seq))
    // scalar value fills whole max_len
    if (fromPython(seq, v))
    {
        for (int i=1; i<max_len; ++i)
            v[i] = v[0];
        return 1;
    }
    if (!PySequence_Check(seq))
    {
        setPythonError(PyExc_TypeError, SStream() << "expected scalar or sequence, got "
                       << typeName(seq));
        return 0;
    }
    // scalar value again
    if (PySequence_Length(seq) == 1)
    {
        PyObject* item = PySequence_GetItem(seq, 0);
        if (fromPython(item, v))
        {
            for (int i=1; i<max_len; ++i)
                v[i] = v[0];
            return 1;
        }
    }
    // parse any float/sequence combination
    int write = 0;
    if (!parseSequencePart(seq, &write, max_len, v))
        return 0;

    for (int i=write; i<max_len; ++i)
        v[i] = 0.;

    return write;
}


bool VectorBase::inplace_operator(PyObject* arg,
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

PyObject* VectorBase::copy_operator(PyObject* left, PyObject* right,
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


} // extern "C"
	
} // namespace PYTHON
} // namespace MO
