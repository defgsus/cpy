#include "mat_base.h"
#include "vector_math.h"
#include <vector>
#include <iomanip>

using namespace PyUtils;

namespace MO {
namespace PYTHON {

extern "C" {

size_t sizeof_MatrixBase() { return sizeof(MatrixBase); }
size_t sizeof_Matrix33() { return sizeof(Matrix33); }

/** @ingroup lolpig
    @p mat.__new__
*/
PyObject* mat_new(_typeobject* type, PyObject* args, PyObject* kwargs)
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

    return pyobject_cast<PyObject*>(vec);
}


/** @ingroup lolpig
    @p mat.__repr__
*/
PyObject* mat_repr(PyObject* self)
{
    MatrixBase* vec = pyobject_cast<MatrixBase*>(self);
    return toPython(vec->toRepr("mat", vec->num_rows));
}

/** @ingroup lolpig
    @p mat.__str__
*/
PyObject* mat_str(PyObject* self)
{
    MatrixBase* vec = pyobject_cast<MatrixBase*>(self);
    return toPython(vec->toString("mat", vec->num_rows));
}

/** @ingroup lolpig
    @p mat.string
    Returns a multi-line string representation
*/
PyObject* mat_string(PyObject* self)
{
    MatrixBase* vec = pyobject_cast<MatrixBase*>(self);
    return toPython(vec->matrixString(vec->v, vec->num_rows, vec->num_cols, "mat"));
}


/** @ingroup lolpig
    @p mat.num_rows
    num_rows() -> int \n
    Returns number of rows \n
*/
PyObject* mat_num_rows(PyObject* self)
{
    MatrixBase* vec = pyobject_cast<MatrixBase*>(self);
    return toPython((long)vec->num_rows);
}

/** @ingroup lolpig
    @p mat.num_columns
    num_columns() -> int \n
    Returns number of columns \n
*/
PyObject* mat_num_columns(PyObject* self)
{
    MatrixBase* vec = pyobject_cast<MatrixBase*>(self);
    return toPython((long)vec->num_cols);
}


/** @ingroup lolpig
    @p mat.trace
    trace() -> float \n
    Returns the sum of the diagonal \n
*/
PyObject* mat_trace(PyObject* self)
{
    MatrixBase* vec = pyobject_cast<MatrixBase*>(self);
    return toPython(VEC::mat_trace(vec->v, vec->num_rows, vec->num_cols));
}

/** @ingroup lolpig
    @p mat.column
    column(int) -> vec \n
    Returns the specified column as vector \n
*/
PyObject* mat_column(PyObject* self, PyObject* obj)
{
    MatrixBase* vec = pyobject_cast<MatrixBase*>(self);
    long idx;
    if (!expectFromPython(obj, &idx))
        return NULL;
    if (!checkIndex(idx, vec->num_cols))
        return NULL;
    return pyobject_cast<PyObject*>(
                createVector(vec->num_rows, &vec->v[idx*vec->num_rows]) );
}

/** @ingroup lolpig
    @p mat.row
    row(int) -> vec \n
    Returns the specified row as vector \n
*/
PyObject* mat_row(PyObject* self, PyObject* obj)
{
    MatrixBase* vec = pyobject_cast<MatrixBase*>(self);
    long idx;
    if (!expectFromPython(obj, &idx))
        return NULL;
    if (!checkIndex(idx, vec->num_rows))
        return NULL;
    return pyobject_cast<PyObject*>(
                createVector(vec->num_rows, &vec->v[idx], vec->num_rows) );
}


/** @ingroup lolpig
    @p mat.columns
    columns() -> [vec,] \n
    Returns the columns as list of vectors \n
*/
PyObject* mat_columns(PyObject* self)
{
    MatrixBase* vec = pyobject_cast<MatrixBase*>(self);

    PyObject* ret = PyList_New(vec->num_cols);
    for (int i=0; i<vec->num_cols; ++i)
    {
        int idx = i * vec->num_rows;
        VectorBase* v = createVector(vec->num_rows, &vec->v[idx]);
        PyList_SetItem(ret, i, pyobject_cast<PyObject*>(v));
    }
    return ret;
}

/** @ingroup lolpig
    @p mat.rows
    rows() -> [vec,] \n
    Returns the rows as list of vectors \n
*/
PyObject* mat_rows(PyObject* self)
{
    MatrixBase* vec = pyobject_cast<MatrixBase*>(self);

    PyObject* ret = PyList_New(vec->num_rows);
    for (int i=0; i<vec->num_rows; ++i)
    {
        VectorBase* ve = createVector(vec->num_cols, &vec->v[i], vec->num_rows);
        PyList_SetItem(ret, i, pyobject_cast<PyObject*>(ve));
    }
    return ret;
}



/** @ingroup lolpig
    @p mat.set_identity
    set_identity() -> self \n
    set_identity(float) -> self \n
    Sets the identity matrix, with optional value other than 1.0. \n
*/
PyObject* mat_set_identity(PyObject* self, PyObject* args)
{
    MatrixBase* vec = pyobject_cast<MatrixBase*>(self);
    double i=1.;
    if (!isEmpty(args))
    {
        if (!expectFromPython(removeArgumentTuple(args), &i))
            return NULL;
    }
    vec->setIdentity(i);
    Py_RETURN_SELF;
}

/** @ingroup lolpig
    @p mat.transpose
    transpose() -> self \n
    Transposes the matrix, INPLACE \n
*/
PyObject* mat_transpose(PyObject* self)
{
    MatrixBase* vec = pyobject_cast<MatrixBase*>(self);
    VEC::mat_transpose_inplace(vec->v, vec->num_rows, vec->num_cols);
    std::swap(vec->num_rows, vec->num_cols);
    Py_RETURN_SELF;
}


/** @ingroup lolpig
    @p mat.__mul__
*/
PyObject* mat_mul__(PyObject* left, PyObject* right)
{
    if (is_MatrixBase(left))
    {
        MatrixBase* mleft = pyobject_cast<MatrixBase*>(left);

        // mat * mat
        if (is_MatrixBase(right))
        {
            MatrixBase* mright = pyobject_cast<MatrixBase*>(right);
            return pyobject_cast<PyObject*>(mleft->matrixMultCopy(mright));
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
                return pyobject_cast<PyObject*>(createVector(
                    l[0] * r[0] + l[4] * r[1] + l[8 ] * r[2] + l[12],
                    l[1] * r[0] + l[5] * r[1] + l[9 ] * r[2] + l[13],
                    l[2] * r[0] + l[6] * r[1] + l[10] * r[2] + l[14]));
            }
            // mat3 * vec3
            /*else if (mleft->num_cols==3 && mleft->num_rows==3)
            {
                return pyobject_cast<PyObject*>(createVector(
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
            return pyobject_cast<PyObject*>(mleft->matrixMultCopy(r,len,1));
        }
        // mat * scalar
        return VectorBase::binary_op_copy(left, right,
                        [](double l, double r) { return l * r; });
    }
    else if (is_MatrixBase(right))
    {
        //MatrixBase* mright = pyobject_cast<MatrixBase*>(left);

        // scalar * mat
        return VectorBase::binary_op_copy(right, left,
                        [](double l, double r) { return l * r; });
    }

    setPythonError(PyExc_TypeError, SStream() << "Invalid operands for multiplication, "
                    << typeName(left) << " and " << typeName(right));
    return NULL;
}



// -------------- value copying ----------------

/** @ingroup lolpig
    @p mat.transposed
    transposed() -> mat \n
    Returns the transpose of the matrix \n
*/
PyObject* mat_transposed(PyObject* self)
{
    MatrixBase* vec = pyobject_cast<MatrixBase*>(self);
    MatrixBase* ret = createMatrix(vec->num_cols, vec->num_rows);
    VEC::mat_transpose(ret->v, vec->v, vec->num_rows, vec->num_cols);
    return pyobject_cast<PyObject*>(ret);
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
        VEC::vec_copy(vec->v, data, vec->len);

    return vec;
}

void MatrixBase::setIdentity(double val)
{
    VEC::mat_set_identity(this->v, this->num_rows, this->num_cols, val);
}

std::string MatrixBase::dimensionString(int r, int c)
{
    return SStream() << r << "x" << c;
}

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
        ret = pyobject_cast<MatrixBase*>(createVector(num_rows));

    VEC::mat_multiply(ret->v, this->v, this->num_rows, this->num_cols, v, cols);
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



