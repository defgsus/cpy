#ifndef MAT_BASE_H
#define MAT_BASE_H

#include "vec_base.h"


namespace MO {
namespace PYTHON {

extern "C" {

    /** @ingroup lolpig
        @p mat
        Intermediate matrix interface */
    struct MatrixBase : VectorBase
    {
        int num_rows,
            num_cols;

        void setIdentity(double val=1.);
        MatrixBase* matrixMultCopy(const double* right, int rows, int cols) const;
        MatrixBase* matrixMultCopy(MatrixBase* right) const
            { return matrixMultCopy(right->v, right->num_rows, right->num_cols); }
        std::string dimensionString() const { return dimensionString(num_rows, num_cols); }
        static std::string dimensionString(int r, int c);
        static bool getSizeFromArgs(PyObject* args, PyObject* kwargs, int* rows, int*cols);
        static std::string matrixString(const double* v, int rows, int cols,
                                        const std::string& name="mat");
    };

    MatrixBase* new_MatrixBase();
    bool is_MatrixBase(PyObject*);
    size_t sizeof_MatrixBase();

    MatrixBase* createMatrix(int rows, int columns, double* data = NULL);


    /** @ingroup lolpig
        @p mat3
        A classic 3x3 rotation matrix
    */
    struct Matrix33 : MatrixBase { };

    Matrix33* new_Matrix33();
    bool is_Matrix33(PyObject*);
    size_t sizeof_Matrix33();


} // extern "C"

} // namespace PYTHON
} // namespace MO

#endif // MAT_BASE_H
