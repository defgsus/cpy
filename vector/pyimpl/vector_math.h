#ifndef VECTOR_MATH_H
#define VECTOR_MATH_H

#include <cmath>

namespace VEC {

/** the infamous PI */
#ifndef PI
#define PI (3.1415926535897932384626433832795)
#endif

/** 2.0 * PI */
#ifndef TWO_PI
#define TWO_PI (6.283185307179586476925286766559)
#endif

/** PI / 2.0 */
#ifndef HALF_PI
#define HALF_PI (1.5707963267948966192313216916398)
#endif

/** 2.0/3.0 * PI */
#ifndef TWOTHIRD_PI
#define TWOTHIRD_PI (2.0943951023931954923084289221863)
#endif

/** degree to 2*pi multiplier (TWO_PI/360.0) */
#ifndef DEG_TO_TWO_PI
#define DEG_TO_TWO_PI (PI/180.0)
#endif

/** 2*pi to degree multiplier (360.0/TWO_PI) */
#ifndef TWO_PI_TO_DEG
#define TWO_PI_TO_DEG (180.0/PI)
#endif

template <typename F>
void vec_copy(F* dst, const F* src, int len)
{
    for (int i=0; i<len; ++i)
        *dst++ = *src++;
}


// -------------- rotation copy ------------------

template <typename F>
void vec3_rotate_x_deg(F* dst, const F* src, F degree)
{
    degree *= DEG_TO_TWO_PI;
    F sa = std::sin(degree), ca = std::cos(degree);
    dst[0] = src[0];
    dst[1] = src[1] * ca - src[2] * sa;
    dst[2] = src[1] * sa + src[2] * ca;
}

template <typename F>
void vec3_rotate_y_deg(F* dst, const F* src, F degree)
{
    degree *= DEG_TO_TWO_PI;
    F sa = std::sin(degree), ca = std::cos(degree);
    dst[0] = src[0] * ca + src[2] * sa;
    dst[1] = src[1];
    dst[2] = -src[0] * sa + src[2] * ca;
}

template <typename F>
void vec3_rotate_z_deg(F* dst, const F* src, F degree)
{
    degree *= DEG_TO_TWO_PI;
    F sa = std::sin(degree), ca = std::cos(degree);
    dst[0] = src[0] * ca - src[1] * sa;
    dst[1] = src[0] * sa + src[1] * ca;
    dst[2] = src[2];
}

template <typename F>
void vec3_rotate_axis_deg(F* dst, const F* src, const F* axis, F degree)
{
    degree *= DEG_TO_TWO_PI;
    F sa = std::sin(degree), ca = std::cos(degree),
      m = axis[0] * axis[0] + axis[1] * axis[1] + axis[2] * axis[2],
      ms = std::sqrt(m);

    dst[0] = (axis[0] * (axis[0] * src[0] + axis[1] * src[1] + axis[2] * src[2])
                + ca * (src[0] * (axis[1] * axis[1] + axis[2] * axis[2]) + axis[0] * (-axis[1] * src[1] - axis[2] * src[2]))
                + sa * ms * (-axis[2] * src[1] + axis[1] * src[2])) / m;
    dst[1] = (axis[1] * (axis[0] * src[0] + axis[1] * src[1] + axis[2] * src[2])
                + ca * (src[1] * (axis[0] * axis[0] + axis[2] * axis[2]) + axis[1] * (-axis[0] * src[0] - axis[2] * src[2]))
                + sa * ms * (axis[2] * src[0] - axis[0] * src[2])) / m;
    dst[2] = (axis[2] * (axis[0] * src[0] + axis[1] * src[1] + axis[2] * src[2])
                + ca * (src[2] * (axis[0] * axis[0] + axis[1] * axis[1]) + axis[2] * (-axis[0] * src[0] - axis[1] * src[1]))
                + sa * ms * (-axis[1] * src[0] + axis[0] * src[1])) / m;
}


// -------------------- rotation inplace -----------------------

template <typename F>
void vec3_rotate_x_deg_inplace(F* vec, F degree)
{
    degree *= DEG_TO_TWO_PI;
    F sa = std::sin(degree), ca = std::cos(degree);
    F y =    vec[1] * ca - vec[2] * sa;
    vec[2] = vec[1] * sa + vec[2] * ca;
    vec[1] = y;
}

template <typename F>
void vec3_rotate_y_deg_inplace(F* vec, F degree)
{
    degree *= DEG_TO_TWO_PI;
    F sa = std::sin(degree), ca = std::cos(degree);
    F x =     vec[0] * ca + vec[2] * sa;
    vec[2] = -vec[0] * sa + vec[2] * ca;
    vec[0] = x;
}

template <typename F>
void vec3_rotate_z_deg_inplace(F* vec, F degree)
{
    degree *= DEG_TO_TWO_PI;
    F sa = std::sin(degree), ca = std::cos(degree);
    F x =    vec[0] * ca - vec[1] * sa;
    vec[1] = vec[0] * sa + vec[1] * ca;
    vec[0] = x;
}

template <typename F>
void vec3_rotate_axis_deg_inplace(F* vec, const F* axis, F degree)
{
    degree *= DEG_TO_TWO_PI;
    F sa = std::sin(degree), ca = std::cos(degree),
      m = axis[0] * axis[0] + axis[1] * axis[1] + axis[2] * axis[2],
      ms = std::sqrt(m);

    F x    = (axis[0] * (axis[0] * vec[0] + axis[1] * vec[1] + axis[2] * vec[2])
                + ca * (vec[0] * (axis[1] * axis[1] + axis[2] * axis[2]) + axis[0] * (-axis[1] * vec[1] - axis[2] * vec[2]))
                + sa * ms * (-axis[2] * vec[1] + axis[1] * vec[2])) / m;
    F y    = (axis[1] * (axis[0] * vec[0] + axis[1] * vec[1] + axis[2] * vec[2])
                + ca * (vec[1] * (axis[0] * axis[0] + axis[2] * axis[2]) + axis[1] * (-axis[0] * vec[0] - axis[2] * vec[2]))
                + sa * ms * (axis[2] * vec[0] - axis[0] * vec[2])) / m;
    vec[2] = (axis[2] * (axis[0] * vec[0] + axis[1] * vec[1] + axis[2] * vec[2])
                + ca * (vec[2] * (axis[0] * axis[0] + axis[1] * axis[1]) + axis[2] * (-axis[0] * vec[0] - axis[1] * vec[1]))
                + sa * ms * (-axis[1] * vec[0] + axis[0] * vec[1])) / m;
    vec[1] = y;
    vec[0] = x;
}



template <typename F>
void mat_set_identity(F* v, int rows, int cols, F val = F(1))
{
    for (int c=0; c<cols; ++c)
        for (int r=0; r<rows; ++r)
            *v++ = r == c ? val : F(0);
}

/** Multiply rows*cols matrix l with cols*cols2 matrix r into rows*cols2 matrix in dst.

    r1xc1 * c1xc2 = r1*c2

    1 3 5     a d     1a+3b+5c 1d+3e+5f
    2 4 6  x  b e  =  2a+4b+6c 2d+5e+6f
              c f
*/
template <typename F>
void mat_multiply(F* dst, const F* l, int rows, int cols, const F* r, int cols2)
{
    for (int ci=0; ci<cols2; ++ci)
    for (int ri=0; ri<rows; ++ri)
    {
        double s = 0.;
        for (int i=0; i<cols; ++i)
            s += l[i * rows + ri] * r[ci * cols + i];
        *dst++ = s;
    }
}



/** Multiplication of two equal-sized square matrices.
    dst = l * r
    @note dst must not overlap with l or r
*/
template <typename F>
void matnn_multiply(F* dst, const F* l, const F* r, int rows)
{
    for (int ci=0; ci<rows; ++ci)
    for (int ri=0; ri<rows; ++ri)
    {
        F s = 0.;
        for (int i=0; i<rows; ++i)
            s += l[i*rows + ri] * r[ci*rows + i];
        *dst++ = s;
    }
}

/** Inplace multiplication of two equal-sized square matrices.
    l *= r
*/
template <typename F>
void matnn_multiply_inplace(F* l, const F* r, int rows)
{
    F tmp[rows*rows];
    matnn_multiply(tmp, l, r, rows);
    vec_copy(l, tmp, rows*rows);
}

/** Takes rows*cols matrix src and transposes it into cols*rows matrix dst */
template <typename F>
void mat_transpose(F* dst, const F* src, int rows, int cols)
{
    for (int r=0; r<rows; ++r)
    for (int c=0; c<cols; ++c)
        *dst++ = src[c*rows+r];
}

/** Takes rows*cols matrix m and replaces it with cols*rows matrix */
template <typename F>
void mat_transpose_inplace(F* m, int rows, int cols)
{
    F tmp[rows*cols];
    vec_copy(tmp, m, rows*cols);
    mat_transpose(m, tmp, rows, cols);
}


template <typename F>
void mat3_set_rotate_x_deg(F* v, F degree)
{
    degree *= F(DEG_TO_TWO_PI);
    F sa = std::sin(degree), ca = std::cos(degree);
    v[0] = F(1);
    v[1] = F(0);
    v[2] = F(0);
    v[3] = F(0);
    v[4] = ca;
    v[5] = sa;
    v[6] = F(0);
    v[7] = -sa;
    v[8] = ca;
}

template <typename F>
void mat3_set_rotate_y_deg(F* v, F degree)
{
    degree *= F(DEG_TO_TWO_PI);
    F sa = std::sin(degree), ca = std::cos(degree);
    v[0] = ca;
    v[1] = F(0);
    v[2] = -sa;
    v[3] = F(0);
    v[4] = F(1);
    v[5] = F(0);
    v[6] = sa;
    v[7] = F(0);
    v[8] = ca;
}

template <typename F>
void mat3_set_rotate_z_deg(F* v, F degree)
{
    degree *= F(DEG_TO_TWO_PI);
    F sa = std::sin(degree), ca = std::cos(degree);
    v[0] = ca;
    v[1] = sa;
    v[2] = F(0);
    v[3] = -sa;
    v[4] = ca;
    v[5] = F(0);
    v[6] = F(0);
    v[7] = F(0);
    v[8] = F(1);
}


template <typename F>
F mat_trace(F* v, int rows, int cols)
{
    int len = rows*cols; rows += 1;
    double s = 0.;
    for (int i=0; i<len; i += rows)
        s += v[i];
    return s;
}


} // namespace VEC


#endif // VECTOR_MATH_H
