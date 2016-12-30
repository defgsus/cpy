#ifndef VEC_BASE_H
#define VEC_BASE_H

#include <python3.4/Python.h>
#include <iostream>
#include <sstream>
#include <cmath>

#include "py_utils.h"

#define PYVEC_DEBUG(arg__) { std::cout << arg__ << std::endl; }


namespace MO {
namespace PYTHON {

extern "C" {

    struct VectorBase;

    /** iterator for vec classes */
    LOLPIG_DEF(_vec_iter, Vector iterator)
    struct VectorIter {
        PyObject_HEAD
        VectorBase* vec;
        int iter;

        std::string toString() const;
    };

    VectorIter* new_VectorIter();
    bool is_VectorIter(PyObject*);

    /** Base vector class and variable length vector */
    LOLPIG_DEF(vec, The basic vector class)
    struct VectorBase {
        PyObject_HEAD
        double* v;
        int len;

        void alloc(int len);
        void dealloc();
        /** Returns a deep copy of the class (or subclass) */
        VectorBase* copy() const;
        std::string toString(const std::string& name="vec") const;
        std::string toRepr(const std::string& name="vec") const;
        bool inplace_operator(PyObject* right, void(*op)(double& l, double r));
        static PyObject* copy_operator(PyObject* left, PyObject* right,
                                       double(*op)(double l, double r));

        static int parseSequence(PyObject* seq, double* v=NULL,
                                 int max_len=0, int default_len=0);
    };

    VectorBase* new_VectorBase();
    bool is_VectorBase(PyObject*);

    LOLPIG_DEF(vec3, 3-dimensional vector class)
    struct Vector3 : public VectorBase { };

    Vector3* new_Vector3();
    bool is_Vector3(PyObject*);


} // extern "C"
	
} // namespace PYTHON
} // namespace MO

#endif // VEC_BASE_H
