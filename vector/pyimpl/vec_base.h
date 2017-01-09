#ifndef VEC_BASE_H
#define VEC_BASE_H

#include <python3.4/Python.h>
#include <iostream>
#include <sstream>
#include <cmath>
#include <functional>
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
    struct MatrixBase;
    struct Vector3;
    struct Matrix33;
}


/** Create the appropriate vector instance.
    Values from @p init will be copied to vector, if not NULL. */
VectorBase* createVector(int len, const double* init = NULL, int stride=1);
VectorBase* createVector(double x, double y, double z);


extern "C" {

    /** @addtogroup lolpig
        @{ */

    /** @p vec
        Base vector class and variable length vector
    */
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

        /** Returns a deep copy of the class (or subclass)
            with op applied to each element. */
        VectorBase* unary_op_copy(std::function<double(double)> op) const;
        /** Applies op to all elements */
        void unary_op_inplace(std::function<double(double)> op) const;
        /** Apply right operand to left self */
        bool binary_op_inplace(PyObject* right,
                               std::function<void(double& l, double r)> op);
        /** Apply binary operator.
            Handles any type combination of left and right,
            like vec*scalar, scalar*vec, vec*vec, etc.. */
        static PyObject* binary_op_copy(PyObject* left, PyObject* right,
                                       std::function<double(double l, double r)> op);

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
        /** Same as parseSequence, but length is expected to be exactly @p len. */
        static bool parseSequenceExactly(PyObject* seq, double* v, int len);

        double lengthSquared() const;
    };

    /** @} */

    VectorBase* new_VectorBase();
    bool is_VectorBase(PyObject*);


    /** @ingroup lolpig
        @p vec3
        3-dimensional vector class */
    struct Vector3 : public VectorBase { };

    Vector3* new_Vector3();
    bool is_Vector3(PyObject*);


    /** @ingroup lolpig
        @p _vec_iter
        Iterator for vec classes */
    struct VectorIter {
        PyObject_HEAD
        VectorBase* vec;
        int iter;

        std::string toString() const;

        operator PyObject* () { return reinterpret_cast<PyObject*>(this); }
    };

    VectorIter* new_VectorIter();
    bool is_VectorIter(PyObject*);

} // extern "C"
	
#if 1

template <class T, class F>
inline T pyobject_cast(F o) { return reinterpret_cast<T>(o); }

#else
    // XXX Maybe we can provide at least some type-safety for pyobject_cast ??

    template <class T>
    struct pyobject_cast { };

    template <>
    struct pyobject_cast<PyObject*>
    {
        PyObject* o;
        operator PyObject* () const { return o; }
        pyobject_cast(VectorBase* o) : o(reinterpret_cast<PyObject*>(o)) { }
        pyobject_cast(VectorIter* o) : o(reinterpret_cast<PyObject*>(o)) { }
        pyobject_cast(Vector3* o) : o(reinterpret_cast<PyObject*>(o)) { }
        pyobject_cast(MatrixBase* o) : o(reinterpret_cast<PyObject*>(o)) { }
        pyobject_cast(Matrix33* o) : o(reinterpret_cast<PyObject*>(o)) { }
    };

    #define PYVEC_DEFINE_CAST(class__) \
    template <> \
    struct pyobject_cast<class__*> \
    { \
        class__* o; \
        operator class__* () const { return o; } \
        pyobject_cast(PyObject* o) : o(reinterpret_cast<class__*>(o)) { } \
    };

    PYVEC_DEFINE_CAST(VectorBase)
    PYVEC_DEFINE_CAST(VectorIter)
    PYVEC_DEFINE_CAST(Vector3)
    PYVEC_DEFINE_CAST(MatrixBase)
    PYVEC_DEFINE_CAST(Matrix33)
#endif


} // namespace PYTHON
} // namespace MO

#endif // VEC_BASE_H
