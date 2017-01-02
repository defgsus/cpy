#include "mat_base.h"
#include <vector>

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

LOLPIG_DEF( mat.column, (
    column(int) -> vec
    Returns the specified column as vector
    ))
PyObject* mat_column(PyObject* self, PyObject* obj)
{
    MatrixBase* vec = reinterpret_cast<MatrixBase*>(self);
    long idx;
    if (!fromPython(obj, &idx))
        return NULL;
    if (!checkIndex(idx, vec->num_cols))
        return NULL;
    return reinterpret_cast<PyObject*>(
                createVector(vec->num_rows, &vec->v[idx*vec->num_rows]) );
}

LOLPIG_DEF( mat.row, (
    row(int) -> vec
    Returns the specified row as vector
    ))
PyObject* mat_row(PyObject* self, PyObject* obj)
{
    MatrixBase* vec = reinterpret_cast<MatrixBase*>(self);
    long idx;
    if (!fromPython(obj, &idx))
        return NULL;
    if (!checkIndex(idx, vec->num_rows))
        return NULL;
    return reinterpret_cast<PyObject*>(
                createVector(vec->num_rows, &vec->v[idx], vec->num_rows) );
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
PyObject* mat_rows(PyObject* self)
{
    MatrixBase* vec = reinterpret_cast<MatrixBase*>(self);

    //double v[vec->num_cols];
    PyObject* ret = PyList_New(vec->num_rows);
    for (int i=0; i<vec->num_rows; ++i)
    {
        //for (int j=0; j<vec->num_cols; ++j)
        //    v[j] = vec->v[j*vec->num_rows+i];
        //VectorBase* ve = createVector(vec->num_cols, v);
        VectorBase* ve = createVector(vec->num_cols, &vec->v[i], vec->num_rows);
        PyList_SetItem(ret, i, reinterpret_cast<PyObject*>(ve));
    }
    return ret;
}



LOLPIG_DEF( mat.set_identity, (
    set_identity() -> self
    set_identity(float) -> self
    Sets the identity matrix, with optional value other than 1.0.
    ))
PyObject* mat_set_identity(PyObject* self, PyObject* args)
{
    MatrixBase* vec = reinterpret_cast<MatrixBase*>(self);
    args = removeArgumentTuple(args);
    double i=1.;
    fromPython(args, &i);
    vec->setIdentity(i);
    Py_RETURN_SELF;
}

LOLPIG_DEF( mat.transpose, (
    transpose() -> self
    Transposes the matrix, INPLACE
    ))
PyObject* mat_transpose(PyObject* self)
{
    MatrixBase* vec = reinterpret_cast<MatrixBase*>(self);

    std::vector<double> tmp(vec->len);
    for (int i=0; i<vec->len; ++i)
        tmp[i] = vec->v[i];

    for (int c=0; c<vec->num_cols; ++c)
    for (int r=0; r<vec->num_rows; ++r)
        vec->v[r*vec->num_cols+c] = tmp[c*vec->num_rows+r];
    std::swap(vec->num_rows, vec->num_cols);

    Py_RETURN_SELF;
}





LOLPIG_DEF( mat.__mul__, )
PyObject* mat_mul__(PyObject* left, PyObject* right)
{
    if (is_MatrixBase(left))
    {
        MatrixBase* mleft = reinterpret_cast<MatrixBase*>(left);

        if (is_MatrixBase(right))
        {
            MatrixBase* mright = reinterpret_cast<MatrixBase*>(right);
            return reinterpret_cast<PyObject*>(mleft->matrixMultCopy(mright));
        }
        else
            return left->ob_type->tp_base->tp_as_number->nb_multiply(left, right);
    }
    else if (is_MatrixBase(right))
    {
        MatrixBase* mright = reinterpret_cast<MatrixBase*>(left);

        if (is_MatrixBase(left))
        {
            MatrixBase* mleft = reinterpret_cast<MatrixBase*>(right);
            return reinterpret_cast<PyObject*>(mleft->matrixMultCopy(mright));
        }
        else
            return right->ob_type->tp_base->tp_as_number->nb_multiply(right, left);
    }
    setPythonError(PyExc_TypeError, SStream() << "Invalid operands for multiplication, "
                    << typeName(left) << " and " << typeName(right));
    return NULL;
}



// -------------- value copying ----------------

LOLPIG_DEF( mat.transposed, (
    transposed() -> mat
    Returns the transpose of the matrix
    ))
PyObject* mat_transposed(PyObject* self)
{
    MatrixBase* vec = reinterpret_cast<MatrixBase*>(self);
    MatrixBase* ret = createMatrix(vec->num_cols, vec->num_rows);

    for (int c=0; c<vec->num_cols; ++c)
    for (int r=0; r<vec->num_rows; ++r)
        ret->v[r*vec->num_cols+c] = vec->v[c*vec->num_rows+r];

    return reinterpret_cast<PyObject*>(ret);
}



// ---------------------- helper ------------------

MatrixBase* createMatrix(int rows, int columns, double *data)
{
    MatrixBase* vec = NULL;
    if (columns == rows) switch (rows)
    {
        default: break;
    }

    if (!vec)
        vec = new_MatrixBase();

    vec->alloc(columns * rows);
    vec->num_cols = columns;
    vec->num_rows = rows;

    if (data)
        for (int i=0; i<vec->len; ++i)
            vec->v[i] = *data++;

    return vec;
}

void MatrixBase::setIdentity(double val)
{
    set(0);
    int sm = std::min(num_rows, num_cols);
    for (int i=0; i<sm; ++i)
    {
        int idx = i*(num_rows+1);
        if (idx < len)
            v[idx] = val;
    }
}

std::string MatrixBase::dimensionString(int r, int c)
{
    return SStream() << r << "x" << c;
}

/* r1xc1 * c1xc2 = r1*c2
 *
    1 3 5     a d     1a+3b+5c 1d+3e+5f
    2 4 6  x  b e  =  2a+4b+6c 2d+5e+6f
              c f
*/
MatrixBase* MatrixBase::matrixMultCopy(const double* v, int rows, int cols) const
{
    if (num_cols != rows)
    {
        setPythonError(PyExc_TypeError, SStream()
            << "Can not matrix-multiply " << dimensionString()
            << " with " << dimensionString(rows, cols)
            << ", " << num_cols << " != " << rows
        );
        return NULL;
    }

    MatrixBase* ret = createMatrix(num_rows, cols);

    for (int row=0; row<num_rows; ++row)
    for (int col=0; col<cols; ++col)
    {
        double s = 0.;
        for (int i=0; i<num_cols; ++i)
            s += this->v[i * num_rows + row] * v[col * rows + i];
        ret->v[col * num_rows + row] = s;
    }
    return ret;
}

} // extern "C"

} // namespace PYTHON
} // namespace MO



