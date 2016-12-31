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
    };

    MatrixBase* new_MatrixBase();
    bool is_MatrixBase(PyObject*);

} // extern "C"

} // namespace PYTHON
} // namespace MO

#endif // MAT_BASE_H
