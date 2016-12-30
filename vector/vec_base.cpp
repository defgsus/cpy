#include "vec_base.h"

namespace MO {
namespace PYTHON {

using namespace PyUtils;

extern "C" {


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



VectorBase* copy_VectorBase(VectorBase* src)
{
    VectorBase* dst = new_VectorBase();
    dst->len = src->len;
    for (int i=0; i<src->len; ++i)
        dst->v[i] = src->v[i];
    return dst;
}


/*
LOLPIG_DEF( vec.__new__, )
PyObject* vec_new(struct _typeobject* type, PyObject* args, PyObject* )
{
    double v[16];
    int len = VectorBase::parseSequence(args, v, 16);
    if (len==0)
        return NULL;
    //PRINT(typeName(args) << " " << len);

    VectorBase* vec;
    switch (len)
    {
        //case 3: vec = reinterpret_cast<VectorBase*>(new_Vector3());
        default: vec = PyObject_NEW(VectorBase, type);
    }

    vec->len = len;
    for (int i=0; i<len; ++i)
        vec->v[i] = v[i];

    return reinterpret_cast<PyObject*>(vec);
}
*/


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


/*
LOLPIG_DEF( vec.__dealloc__, )
void vec_free(PyObject* self)
{
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
    //PRINT("DEALLOC " << vec->v);
    free(vec->v);
    vec->v = NULL;
    self->ob_type->tp_free(self);
}
*/

LOLPIG_DEF( vec.copy, )
PyObject* vec_copy(PyObject* self)
{
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
    return reinterpret_cast<PyObject*>(copy_VectorBase(vec));
}


LOLPIG_DEF( vec.__repr__, )
PyObject* vec_repr(PyObject* self)
{
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
    std::stringstream s;
    s << vec->toString() << "@" << (void*)vec;
    return toPython(s.str());
}

LOLPIG_DEF( vec.__str__, )
PyObject* vec_str(PyObject* self)
{
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
    return toPython(vec->toString());
}



LOLPIG_DEF( vec.test, )
PyObject* vec_test(PyObject* , PyObject* obj)
{
    PRINT("SEQ " << PySequence_Check(obj)
          << " LEN " << PySequence_Size(obj) );
    Py_RETURN_NONE;
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
    PRINT("NEWITER " << iter->toString());
    return reinterpret_cast<PyObject*>(iter);
}

LOLPIG_DEF( _vec_iter.__iter__, )
PyObject* veciter_iter(PyObject* self)
{
    //Py_RETURN_SELF;
    VectorIter* iter = reinterpret_cast<VectorIter*>(self);
    PRINT("ITER " << iter->toString());
    Py_RETURN_OBJECT(iter);
}

LOLPIG_DEF( _vec_iter.__next__, )
PyObject* veciter_next(PyObject* self)
{
    VectorIter* iter = reinterpret_cast<VectorIter*>(self);
    PRINT("NEXT " << iter->toString());
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
    PRINT("DEALLOC " << iter->toString());
    Py_CLEAR(iter->vec);
    self->ob_type->tp_free(self);
}




} // extern "C"
	
} // namespace PYTHON
} // namespace MO
