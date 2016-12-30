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

    struct VectorBase;

    LOLPIG_DEF(_vec_iter, Vector iterator)
    struct VectorIter {
        PyObject_HEAD
        VectorBase* vec;
        int iter;
    };

    VectorIter* new_VectorIter();
    bool is_VectorIter(PyObject*);


    LOLPIG_DEF(vec, The basic vector class)
    struct VectorBase {
        PyObject_HEAD
        double v[16];
        int len;

        std::string toString(const std::string& name="vec") const;
        static int parseSequence(PyObject* seq, double* v, int max_len);
    };

    VectorBase* new_VectorBase();
    VectorBase* copy_VectorBase(VectorBase*);
    bool is_VectorBase(PyObject*);


    LOLPIG_DEF(vec3, 3-dimensional vector class)
    struct Vector3 : public VectorBase { };

    Vector3* new_Vector3();
    Vector3* copy_Vector3(Vector3*);
    bool is_Vector3(PyObject*);


} // extern "C"
	
} // namespace PYTHON
} // namespace MO

#endif // VEC_BASE_H
