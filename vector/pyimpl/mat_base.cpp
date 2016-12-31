#include "mat_base.h"

using namespace PyUtils;

namespace MO {
namespace PYTHON {

extern "C" {

LOLPIG_DEF( mat.__new__, )
PyObject* mat_new(struct _typeobject* type, PyObject* args, PyObject* kwargs)
{
    int len;
    if (isEmpty(args))
        len = 1;
    else
    {
        len = MatrixBase::parseSequence(args);
        if (len==0)
            return NULL;
    }

    long r = -1, c = -1;

    if (kwargs && PyArg_ValidateKeywordArguments(kwargs))
    {
        if (PyObject* cols = PyDict_GetItemString(kwargs, "columns"))
        {
            if (!expectFromPython(cols, &c))
                return NULL;
            r = c;
        }
        if (PyObject* rows = PyDict_GetItemString(kwargs, "rows"))
        {
            if (!expectFromPython(rows, &r))
                return NULL;
            if (c <= 0)
                c = r;
        }
        //PYUTILS_PRINT("KWARGS " << typeName(cols));
    }

    if (r < 0)
        r = std::ceil(std::sqrt(len));
    if (c < 0)
        c = r;

    len = r * c;
    //PYUTILS_PRINT(r << ", " << c << ", " << len);
    MatrixBase* vec = PyObject_NEW(MatrixBase, type);
    vec->alloc(len);
    vec->num_rows = r;
    vec->num_cols = c;
    MatrixBase::parseSequence(args, vec->v, len);

    return reinterpret_cast<PyObject*>(vec);
}


LOLPIG_DEF( mat.__repr__, )
PyObject* mat_repr(PyObject* self)
{
    MatrixBase* vec = reinterpret_cast<MatrixBase*>(self);
    return toPython(vec->toRepr("mat", vec->num_rows));
}

LOLPIG_DEF( mat.__str__, )
PyObject* mat_str(PyObject* self)
{
    MatrixBase* vec = reinterpret_cast<MatrixBase*>(self);
    return toPython(vec->toString("mat", vec->num_rows));
}



LOLPIG_DEF( mat.num_rows, (
    num_rows() -> int
    Returns number of rows
    ))
PyObject* mat_num_rows(PyObject* self)
{
    MatrixBase* vec = reinterpret_cast<MatrixBase*>(self);
    return toPython((long)vec->num_rows);
}

LOLPIG_DEF( mat.num_columns, (
    num_columns() -> int
    Returns number of columns
    ))
PyObject* mat_num_columns(PyObject* self)
{
    MatrixBase* vec = reinterpret_cast<MatrixBase*>(self);
    return toPython((long)vec->num_cols);
}


LOLPIG_DEF( mat.trace, (
    trace() -> float
    Returns the sum of the diagonal
    ))
PyObject* mat_trace(PyObject* self)
{
    MatrixBase* vec = reinterpret_cast<MatrixBase*>(self);
    double s = 0.;
    for (int i=0; i<vec->num_rows; ++i)
    {
        int idx = i * (1+vec->num_rows);
        if (idx < vec->len)
            s += vec->v[idx];
    }
    return toPython(s);
}


LOLPIG_DEF( mat.columns, (
    columns() -> [vec,]
    Returns the columns as list of vectors
    ))
PyObject* mat_columns(PyObject* self)
{
    MatrixBase* vec = reinterpret_cast<MatrixBase*>(self);

    PyObject* ret = PyList_New(vec->num_cols);
    for (int i=0; i<vec->num_cols; ++i)
    {
        int idx = i * vec->num_rows;
        VectorBase* v = createVector(vec->num_rows, &vec->v[idx]);
        PyList_SetItem(ret, i, reinterpret_cast<PyObject*>(v));
    }
    return ret;
}

LOLPIG_DEF( mat.rows, (
    rows() -> [vec,]
    Returns the rows as list of vectors
    ))
PyObject* mat_row(PyObject* self)
{
    MatrixBase* vec = reinterpret_cast<MatrixBase*>(self);

    double v[vec->num_cols];
    PyObject* ret = PyList_New(vec->num_rows);
    for (int i=0; i<vec->num_rows; ++i)
    {
        for (int j=0; j<vec->num_cols; ++j)
            v[j] = vec->v[j*vec->num_rows+i];
        VectorBase* ve = createVector(vec->num_cols, v);
        PyList_SetItem(ret, i, reinterpret_cast<PyObject*>(ve));
    }
    return ret;
}


} // extern "C"

} // namespace PYTHON
} // namespace MO



