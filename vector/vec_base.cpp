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
        return max_len;
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
            return max_len;
        }
    }
    // parse any float/sequence combination
    int write = 0;
    if (!parseSequencePart(seq, &write, max_len, v))
        return 0;

    for (int i=write; i<max_len; ++i)
        v[i] = 0.;

    return max_len;
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
    VectorBase* vec = PyObject_NEW(VectorBase, type);
    vec->len = VectorBase::parseSequence(args, vec->v, 4);
    if (vec->len)
    {
        Py_DECREF(vec);
        return NULL;
    }
    return reinterpret_cast<PyObject*>(vec);
}
*/

/*
LOLPIG_DEF( vec.__init__, )
int vec_init(PyObject* self, PyObject* args, PyObject* )
{
    VectorBase* vec = reinterpret_cast<VectorBase*>(self);
    vec->len = VectorBase::parseSequence(args, vec->v, 4);
    if (!vec->len)
        return -1;
    return 0;
}
*/

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

LOLPIG_DEF( vec.test, )
PyObject* vec_test(PyObject* , PyObject* obj)
{
    PRINT("SEQ " << PySequence_Check(obj)
          << " LEN " << PySequence_Size(obj) );
    Py_RETURN_NONE;
}

/*
LOLPIG_DEF( vec.__iter__, )
PyObject* vec3_iter(PyObject* self)
{
    Vector3* vec = reinterpret_cast<Vector3*>(self);
    Vector3Iter* iter = new_Vector3Iter();
    iter->iter = 0;
    iter->vec = vec;
    return reinterpret_cast<PyObject*>(iter);
}

LOLPIG_DEF( _vec3_iter.__iter__, )
PyObject* vec3iter_iter(PyObject* self)
{
    Vector3Iter* iter = reinterpret_cast<Vector3Iter*>(self);
    Py_INCREF(iter);
    return reinterpret_cast<PyObject*>(iter);
}

LOLPIG_DEF( _vec3_iter.__next__, )
PyObject* vec3iter_next(PyObject* self)
{
    Vector3Iter* iter = reinterpret_cast<Vector3Iter*>(self);
    PyObject* ret = PyFloat_FromDouble(iter->vec->v[iter->iter]);
    if (iter->iter++ >= iter->vec->len)
    {
        PyErr_SetObject(PyExc_StopIteration, NULL);
        return NULL;
    }
    return ret;
}


LOLPIG_DEF( vec3.__contains__, )
int vec3_contains(PyObject* self, PyObject* val)
{
    if (!PyFloat_Check(val))
    {
        PyErr_SetString(PyExc_TypeError, "expected float");
        return -1;
    }
    double a = PyFloat_AsDouble(val);
    Vector3* vec = reinterpret_cast<Vector3*>(self);
    for (int i=0; i<3; ++i)
        if (vec->v[i] == a)
            return 1;
    return 0;
}
*/


} // extern "C"
	
} // namespace PYTHON
} // namespace MO
