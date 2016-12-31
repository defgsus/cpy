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
    };

    MatrixBase* new_MatrixBase();
    bool is_MatrixBase(PyObject*);

    MatrixBase* createMatrix(int columns, int rows, double* data = NULL);

} // extern "C"

} // namespace PYTHON
} // namespace MO

#endif // MAT_BASE_H
