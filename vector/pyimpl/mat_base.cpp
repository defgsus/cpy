#include "mat_base.h"
#include <vector>
#include <iomanip>

using namespace PyUtils;

namespace MO {
namespace PYTHON {

extern "C" {

LOLPIG_DEF( mat.__new__, )
PyObject* mat_new(struct _typeobject* type, PyObject* args, PyObject* kwargs)
{
    int r, c;
    if (!MatrixBase::getSizeFromArgs(args, kwargs, &r, &c))
        return NULL;

    MatrixBase* vec = PyObject_NEW(MatrixBase, type);
    vec->alloc(r * c);
    vec->num_rows = r;
    vec->num_cols = c;
    if (MatrixBase::parseSequence(args, vec->v, r*c) < 0)
    {
        Py_DECREF(vec);
        return NULL;
    }

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

LOLPIG_DEF( mat.string, Returns a multi-line string representation)
PyObject* mat_string(PyObject* self)
{
    MatrixBase* vec = reinterpret_cast<MatrixBase*>(self);
    return toPython(vec->matrixString(vec->v, vec->num_rows, vec->num_cols, "mat"));
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

        // mat * mat
        if (is_MatrixBase(right))
        {
            MatrixBase* mright = reinterpret_cast<MatrixBase*>(right);
            return reinterpret_cast<PyObject*>(mleft->matrixMultCopy(mright));
        }
        // mat * vec
        int len = VectorBase::parseSequence(right);
        if (len == 3)
        {
            double r[3];
            VectorBase::parseSequence(right, r, 3);

            const double* l = mleft->v;
            // mat4 * seq3
            if (mleft->num_cols==4 && mleft->num_rows==4)
            {
                return reinterpret_cast<PyObject*>(createVector(
                    l[0] * r[0] + l[4] * r[1] + l[8 ] * r[2] + l[12],
                    l[1] * r[0] + l[5] * r[1] + l[9 ] * r[2] + l[13],
                    l[2] * r[0] + l[6] * r[1] + l[10] * r[2] + l[14]));
            }
            // mat3 * vec3
            /*else if (mleft->num_cols==3 && mleft->num_rows==3)
            {
                return reinterpret_cast<PyObject*>(createVector(
                    l[0] * r[0] + l[3] * r[1] + l[6] * r[2] ,
                    l[1] * r[0] + l[4] * r[1] + l[7] * r[2] ,
                    l[2] * r[0] + l[5] * r[1] + l[8] * r[2] ));
            }*/
        }
        // mat * seq
        if (len == mleft->num_cols)
        {
            double r[len];
            VectorBase::parseSequence(right, r, len);
            return reinterpret_cast<PyObject*>(mleft->matrixMultCopy(r,len,1));
        }
        // mat * scalar
#ifdef CPP11
        return VectorBase::binary_op_copy(left, right,
                        [](double l, double r) { return l * r; });
#endif
    }
    else if (is_MatrixBase(right))
    {
        //MatrixBase* mright = reinterpret_cast<MatrixBase*>(left);

        // scalar * mat
#ifdef CPP11
        return VectorBase::binary_op_copy(right, left,
                        [](double l, double r) { return l * r; });
#endif
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

    MatrixBase* ret;
    if (cols > 1)
        ret = createMatrix(num_rows, cols);
    else
        ret = reinterpret_cast<MatrixBase*>(createVector(num_rows));

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

std::string MatrixBase::matrixString(const double *v, int rows, int cols,
                              const std::string& name)
{
    std::vector<int> widths;
    widths.push_back(name.size()+1);
    for (int c=0; c<cols; ++c)
    {
        int w = 0;
        for (int r=0; r<rows; ++r)
            w = std::max(w, (int)std::string(SStream() << v[r+rows*c]).size());
        widths.push_back(w);
    }

    std::stringstream s;
    for (int r=0; r<rows; ++r)
    {
        if (r == 0)
            s << name << "(";
        else
            s << std::setw(widths[0]) << "";
        for (int c=0; c<cols; ++c)
        {
            s << std::setw(widths[c+1]) << v[r+rows*c];
            if (r+rows*c+1 < rows*cols)
                s << ", ";
        }
        if (r+1 < rows)
            s << "\n";
        else
            s << ")";
    }

    return s.str();
}


bool MatrixBase::getSizeFromArgs(PyObject *args, PyObject *kwargs, int *rows, int *cols)
{
    int len;
    if (isEmpty(args))
        len = 1;
    else
    {
        len = MatrixBase::parseSequence(args);
        if (len==0)
            return false;
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

    *rows = r;
    *cols = c;
    return r > 0;
}


} // extern "C"

} // namespace PYTHON
} // namespace MO



