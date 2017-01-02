#ifndef MAT_BASE_H
#define MAT_BASE_H

#include "vec_base.h"


namespace MO {
namespace PYTHON {

extern "C" {

    /** Intermediate matrix interface */
    LOLPIG_DEF(mat, The matrix base class)
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
    };

    MatrixBase* new_MatrixBase();
    bool is_MatrixBase(PyObject*);

    MatrixBase* createMatrix(int rows, int columns, double* data = NULL);

} // extern "C"

} // namespace PYTHON
} // namespace MO

#endif // MAT_BASE_H
