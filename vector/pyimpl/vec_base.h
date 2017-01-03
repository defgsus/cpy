#ifndef VEC_BASE_H
#define VEC_BASE_H

#include <python3.4/Python.h>
#include <iostream>
#include <sstream>
#include <cmath>
#ifdef CPP11
#   include <functional>
#endif

#include "../py_utils.h"

#if 0
#   define PYVEC_DEBUG(arg__) { std::cout << arg__ << std::endl; }
#else
#   define PYVEC_DEBUG(unused__) { }
#endif

#define PRINT(arg__) std::cout << arg__ << std::endl;

namespace MO {
namespace PYTHON {

extern "C" {
    struct VectorBase;
}


/** Create the appropriate vector instance.
    Values from @p init will be copied to vector, if not NULL. */
VectorBase* createVector(int len, const double* init = NULL, int stride=1);
VectorBase* createVector(double x, double y, double z);


extern "C" {

    /** Base vector class and variable length vector */
    LOLPIG_DEF(vec, The basic vector class)
    struct VectorBase {
        PyObject_HEAD
        double* v;
        int len;

        void alloc(int len);
        void dealloc();
        void set(double val);
        /** Returns a copy of the class (or subclass).
            The vector will have the same length
            (and same rows/columns for matrices)
            and v will alloced but not initialized. */
        VectorBase* copyClass() const;
        /** Returns a deep copy of the class (or subclass) */
        VectorBase* copy() const;
        /** Text repr of vector as in constructor call. */
        std::string toString(const std::string& class_name, int group=0) const;
        std::string toRepr(const std::string& class_name, int group=0) const;

#ifdef CPP11
        /** Returns a deep copy of the class (or subclass)
            with op applied to each element. */
        VectorBase* unary_op_copy(std::function<double(double)> op) const;
        /** Applies op to all elements */
        void unary_op_inplace(std::function<double(double)> op) const;
#endif
        /** Apply right operand to left self */
        bool binary_op_inplace(PyObject* right, void(*op)(double& l, double r));
        /** Apply binary operator.
            Handles any type combination of left and right,
            like vec*scalar, scalar*vec, vec*vec, etc.. */
        static PyObject* binary_op_copy(PyObject* left, PyObject* right,
                                       double(*op)(double l, double r));

        /** Scans any sequence or sub-sequence into @p v.
         * Eg. It understands: 1 or (1,) or (1,2),3, ((1,2),(3,4)), etc..
         * @param seq Python scalar or sequence
         * @param v pointer to @p max_len bytes, or NULL for just-parse
         * @param max_len maximum number of elements reserved in @p v.
         *        If zero, max_len defaults to maximum possible!
         * @param default_len If not zero, a minimum length v should have.
         *        Scalars fill this whole range, sequences will be zero-padded to fit.
         * @return The actual number of floats read from python argument,
         *         or -1 on error.
         */
        static int parseSequence(PyObject* seq, double* v=NULL,
                                 int max_len=0, int default_len=0);

        double lengthSquared() const;
    };

    VectorBase* new_VectorBase();
    bool is_VectorBase(PyObject*);



    LOLPIG_DEF(vec3, 3-dimensional vector class)
    struct Vector3 : public VectorBase { };

    Vector3* new_Vector3();
    bool is_Vector3(PyObject*);



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

} // extern "C"
	
} // namespace PYTHON
} // namespace MO

#endif // VEC_BASE_H
