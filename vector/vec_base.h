#ifndef VEC_BASE_H
#define VEC_BASE_H

#include <python3.4/Python.h>
#include <iostream>
#include <sstream>
#include <cmath>

#include "py_utils.h"

#define PRINT(arg__) std::cout << arg__ << std::endl;

    LOLPIG_DEF(some, )
    struct Some {
        PyObject_HEAD
    };

namespace MO {
namespace PYTHON {

extern "C" {

    LOLPIG_DEF(vec, The basic vector class)
    struct VectorBase {
        PyObject_HEAD
        double* v;
        int len;
    };

    VectorBase* new_VectorBase();
    bool is_VectorBase(PyObject*);


    LOLPIG_DEF(_vec_iter, Vector iterator)
    struct VectorIter {
        PyObject_HEAD
        VectorBase* vec;
        int iter;
    };

    VectorIter* new_VectorIter();
    bool is_VectorIter(PyObject*);


} // extern "C"
	
} // namespace PYTHON
} // namespace MO

#endif // VEC_BASE_H
