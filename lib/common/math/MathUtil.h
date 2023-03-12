// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Math.h"
#include "Vec2.h"

#include <type_traits>
#include <limits.h>

namespace scene_rdl2 {
namespace math {

/// Given A and b in the linear system Ax = b, solve for x, i.e. find (A^-1 * b)
/// A is an array of 4 floats laid out row by row, x and b are both 2
/// component columns of an arbitrary type. Returns false if a solution could
/// not be found (matrix was non-invertible).
template<typename T>
inline bool
solve2x2LinearSystem(float const *A, T *x, T const *b)
{
    const auto det_a = differenceOfProducts(A[0], A[3], A[1], A[2]);
    if (!std::isnormal(det_a)) {
        return false;
    }

    const auto inv_det_a = 1.0f / det_a;
    x[0] = differenceOfProducts(b[0], A[3], b[1], A[1]) * inv_det_a;
    x[1] = differenceOfProducts(b[1], A[0], b[0], A[2]) * inv_det_a;

    return true;
}

///
/// Compute the first order partial derivatives for some arbitrary quantity
/// over the surface of a triangle with respect to the triangle's uv coordinates.
///
/// For a value x over the surface of a triangle, we want to compute the
/// quantities dxdu and dxdv such that they satisfy the exression:
///
///     x' = x + (d_u * dxdu) + (d_v * dxdv)
///
/// Given the inputs, we can create 3 equations based on the above:
///
///  1. x0 = x + (u0-u0) * dxdu + (v0-v0) * dxdv
///  2. x1 = x + (u1-u0) * dxdu + (v1-v0) * dxdv
///  3. x2 = x + (u2-u0) * dxdu + (v2-v0) * dxdv
///
/// Eqn 1. just says x = x0. Eqns 2 & 3 form a simple linear system
/// of 2 equations and 2 unknowns. By rearranging these eqns, we get:
///
///     x1 - x0 = (u1-u0) * dxdu + (v1-v0) * dxdv
///     x2 - x0 = (u2-u0) * dxdu + (v2-v0) * dxdv
///
/// which, when put into matrix form (Ax=b), equals:
///
///     | u1-u0  v1-v0 |   | dxdu |   | x1 - x0 |
///     | u2-u0  v2-v0 | X | dxdv | = | x2 - x0 |
///
/// Now all that remains is to solve for dxdu and dxdv (x=(A^-1)*b):
///
///     | dxdu |   | u1-u0  v1-v0 |-1   | x1 - x0 |
///     | dxdv | = | u2-u0  v2-v0 |   X | x2 - x0 |
///
template<typename T>
inline bool
computeTrianglePartialDerivatives(T const &x0, T const &x1, T const &x2,
                                  Vec2f const &uv0, Vec2f const &uv1, Vec2f const &uv2,
                                  T *results)
{
    float A[4] =
    {
        uv1.x - uv0.x, uv1.y - uv0.y,
        uv2.x - uv0.x, uv2.y - uv0.y,
    };

    T b[2] =
    {
        x1 - x0,
        x2 - x0,
    };

    return solve2x2LinearSystem(A, results, b) && isFinite(results[0]) && isFinite(results[1]);
}

namespace compile_time {
namespace detail {
constexpr int isqrt(int n, int b)
{
    return (n < 0) ? b - 1 : isqrt((n-b) - (b+1), b+1);
}
} // namespace detail

constexpr int isqrt(int n)
{
    return detail::isqrt(n, 0);
}

namespace primeDetail
{
    constexpr bool isPrimeDivTest(int n, int i)
    {
        return n % i == 0 || n % (i + 2) == 0;
    }

    constexpr bool isPrimeHelper(int n, int i)
    {
        return i*i > n || (!isPrimeDivTest(n, i) && isPrimeHelper(n, i + 6));
    }
}

// This code is an ugly implementation of the following algorithm, fitting the
// constraints of C++11's constexpr.
// function is_prime(n)
//    if n <= 1
//        return false
//    else if n <= 3
//        return true
//    else if n mod 2 = 0 or n mod 3 = 0
//        return false
//    let i = 5
//    while i * i <= n
//        if n mod i = 0 or n mod (i + 2) = 0
//            return false
//        i = i + 6
//    return true
//
// When last checked, this function handled compile-time values a little over
// 9,300,000 in GCC and 35,500,000 in ICC. Anything much larger blew up the
// compile-time stack space (which can be changed with the compiler flag
// -fconstexpr-depth).
constexpr bool isPrime(int n)
{
    return n > 1 && (n == 2 || n == 3 || (n % 2 != 0 && n % 3 != 0 && primeDetail::isPrimeHelper(n, 5)));
}

template <typename IntType>
constexpr IntType log2i(IntType v)
{
    static_assert(std::is_integral<IntType>::value, "Requires an integer type.");
    return (v <= 1) ? 0 : 1 + log2i(v/2);
}
} // namespace compile_time

template <typename IntType>
IntType log2i(IntType v)
{
    static_assert(std::is_integral<IntType>::value, "Requires an integer type.");
    IntType r = 0;
    while (v >>= 1) {
        ++r;
    }

    return r;
}


//----------------------------------------------------------------------------

inline float
bias(float value, float bias)
{
    if (bias == 0.5f || value <= 0.0f || value >= 1.0f)
        return value;

    if (bias <= 0.0f)
        return 0.0f;

    static const float sFactor = 1.0f / math::log(0.5f);
    return math::pow(value, math::log(bias) * sFactor);
}


inline float
gain(float value, float gain)
{
    if (gain == 0.5f) {
        return value;
    } else {
        return value < 0.5f ?
            bias(2.0f * value, 1.0f - gain) * 0.5f :
            1.0f - bias(2.0f - 2.0f * value, 1.0f - gain) * 0.5f;
    }
}

inline float
degreesToRadians(float degrees)
{
    return degrees * sPi / 180.0f;
}


inline float
radiansToDegrees(float radians)
{
    return radians * 180.0f / sPi;
}

//----------------------------------------------------------------------------

finline int32_t
float2Int(float f)
{
    return static_cast<int32_t>(f * static_cast<float>(INT_MAX));
}


finline float
int2Float(int32_t u)
{
    return float(u) / float(INT_MAX);
}


finline uint32_t
float2Uint(float f)
{
    return static_cast<uint32_t>(f * static_cast<float>(UINT_MAX));
}


finline float
uint2Float(uint32_t u)
{
    return float(u) / float(UINT_MAX);
}


// clamp to [0, 1]
finline float
saturate(float val)
{
    return (val > 1.0f) ? 1.0f : ((val < 0.0f) ? 0.0f : val);
}


//
// Compute a scale and offset such that when an input in the range (start, end)
// has this scale and offset applied, we get a result within (0, 1). That is:
//
//  f(x) = x * scale + offset
//  f(start) = 0
//  f(end) = 1
//
// Derivation:
//
//  s * scale + offset = 0
//  e * scale + offset = 1
//  (s * scale) - (e * scale) = -1
//  (s - e) * scale = -1
//  scale = -1 / (s - e)
//  scale = 1 / (e - s)
//
//  offset = -(s * scale)
//  offset = -s * scale
//
template <typename T>
finline void
getScaleOffset(T start, T end, T *scale, T *offset)
{
    *scale = T(1) / (end - start);
    *offset = -start * (*scale);
}

//
// Compute a scale and offset such that when an input in the range (start, end)
// has this scale and offset applied, we get a result within
// (startMapTo, endMapTo). That is:
//
//  f(x) = x * scale + offset
//  f(start) = startMapTo
//  f(end) = endMapTo
//
template <typename T>
finline void
getScaleOffset(T start, T end, T startMapTo, T endMapTo, T *scale, T *offset)
{
    *scale = (endMapTo - startMapTo) / (end - start);
    *offset = startMapTo - (start * (*scale));
}

//----------------------------------------------------------------------------


//
// AOS <--> SOA utils:
//
 
__forceinline void
transposeAOSToSOA_16x16(const  uint32_t *__restrict srcRows[16],  uint32_t *__restrict dst)
{
#if (VLEN == 16u)

#ifndef __INTEL_COMPILER
#error transposeAOSToSOA_16x16() : Can not compile other than ICC so far
#else
    /* naive C code just for reference.
    const unsigned inputW = 16;
    const unsigned inputH = 16;

    for (unsigned i = 0; i < inputH; ++i) {
        for (unsigned j = 0; j < inputW; ++j) {
            dst[(j << 4) + i] = srcRows[i][j];
        }
    }
    */
    __m512i index1, index2;  
    int uIndex1[16] = {0, 1, 2, 3, 16, 17, 18, 19, 4, 5, 6, 7, 20, 21, 22, 23};
    memcpy(&index1, uIndex1, 64);    
    int uIndex2[16] = {8, 9, 10, 11, 24, 25, 26, 27, 12, 13, 14, 15, 28, 29, 30, 31};
    memcpy(&index2, uIndex2, 64);
    
    __m512 a0  = _mm512_load_ps((const float *)srcRows[0]);
    __m512 a1  = _mm512_load_ps((const float *)srcRows[1]);
    __m512 a2  = _mm512_load_ps((const float *)srcRows[2]);
    __m512 a3  = _mm512_load_ps((const float *)srcRows[3]);
    __m512 a4  = _mm512_load_ps((const float *)srcRows[4]);
    __m512 a5  = _mm512_load_ps((const float *)srcRows[5]);
    __m512 a6  = _mm512_load_ps((const float *)srcRows[6]);
    __m512 a7  = _mm512_load_ps((const float *)srcRows[7]);
    __m512 a8  = _mm512_load_ps((const float *)srcRows[8]);
    __m512 a9  = _mm512_load_ps((const float *)srcRows[9]);
    __m512 a10 = _mm512_load_ps((const float *)srcRows[10]);
    __m512 a11 = _mm512_load_ps((const float *)srcRows[11]);
    __m512 a12 = _mm512_load_ps((const float *)srcRows[12]);
    __m512 a13 = _mm512_load_ps((const float *)srcRows[13]);
    __m512 a14 = _mm512_load_ps((const float *)srcRows[14]);
    __m512 a15 = _mm512_load_ps((const float *)srcRows[15]);  
    
    
    __m512 b0  = _mm512_unpacklo_ps(a0, a1);
    __m512 b1  = _mm512_unpackhi_ps(a0, a1);
    __m512 b2  = _mm512_unpacklo_ps(a2, a3);
    __m512 b3  = _mm512_unpackhi_ps(a2, a3);
    __m512 b4  = _mm512_unpacklo_ps(a4, a5);
    __m512 b5  = _mm512_unpackhi_ps(a4, a5);
    __m512 b6  = _mm512_unpacklo_ps(a6, a7);
    __m512 b7  = _mm512_unpackhi_ps(a6, a7);  
    __m512 b8  = _mm512_unpacklo_ps(a8, a9);
    __m512 b9  = _mm512_unpackhi_ps(a8, a9);
    __m512 b10 = _mm512_unpacklo_ps(a10, a11);
    __m512 b11 = _mm512_unpackhi_ps(a10, a11);
    __m512 b12 = _mm512_unpacklo_ps(a12, a13);
    __m512 b13 = _mm512_unpackhi_ps(a12, a13);
    __m512 b14 = _mm512_unpacklo_ps(a14, a15);
    __m512 b15 = _mm512_unpackhi_ps(a14, a15);      
    
    a0  = _mm512_shuffle_ps(b0,  b2,_MM_SHUFFLE(1, 0, 1, 0));
    a1  = _mm512_shuffle_ps(b0,  b2,_MM_SHUFFLE(3, 2, 3, 2));
    a2  = _mm512_shuffle_ps(b1,  b3,_MM_SHUFFLE(1, 0, 1, 0));
    a3  = _mm512_shuffle_ps(b1,  b3,_MM_SHUFFLE(3, 2, 3, 2));
    a4  = _mm512_shuffle_ps(b4,  b6,_MM_SHUFFLE(1, 0, 1, 0));
    a5  = _mm512_shuffle_ps(b4,  b6,_MM_SHUFFLE(3, 2, 3, 2));
    a6  = _mm512_shuffle_ps(b5,  b7,_MM_SHUFFLE(1, 0, 1, 0));
    a7  = _mm512_shuffle_ps(b5,  b7,_MM_SHUFFLE(3, 2, 3, 2));    
    a8  = _mm512_shuffle_ps(b8,  b10,_MM_SHUFFLE(1, 0, 1, 0));
    a9  = _mm512_shuffle_ps(b8,  b10,_MM_SHUFFLE(3, 2, 3, 2));
    a10 = _mm512_shuffle_ps(b9,  b11,_MM_SHUFFLE(1, 0, 1, 0));
    a11 = _mm512_shuffle_ps(b9,  b11,_MM_SHUFFLE(3, 2, 3, 2));
    a12 = _mm512_shuffle_ps(b12, b14,_MM_SHUFFLE(1, 0, 1, 0));
    a13 = _mm512_shuffle_ps(b12, b14,_MM_SHUFFLE(3, 2, 3, 2));
    a14 = _mm512_shuffle_ps(b13, b15,_MM_SHUFFLE(1, 0, 1, 0));
    a15 = _mm512_shuffle_ps(b13, b15,_MM_SHUFFLE(3, 2, 3, 2));     
       
        
    b0  =  _mm512_permutex2var_ps(a0,  index1, a4);
    b1  =  _mm512_permutex2var_ps(a0,  index2, a4);    
    b2  =  _mm512_permutex2var_ps(a1,  index1, a5);
    b3  =  _mm512_permutex2var_ps(a1,  index2, a5);
    b4  =  _mm512_permutex2var_ps(a2,  index1, a6);
    b5  =  _mm512_permutex2var_ps(a2,  index2, a6);
    b6  =  _mm512_permutex2var_ps(a3,  index1, a7);
    b7  =  _mm512_permutex2var_ps(a3,  index2, a7);
    b8  =  _mm512_permutex2var_ps(a8,  index1, a12);
    b9  =  _mm512_permutex2var_ps(a8,  index2, a12);
    b10 =  _mm512_permutex2var_ps(a9,  index1, a13);
    b11 =  _mm512_permutex2var_ps(a9,  index2, a13);
    b12 =  _mm512_permutex2var_ps(a10, index1, a14);
    b13 =  _mm512_permutex2var_ps(a10, index2, a14);
    b14 =  _mm512_permutex2var_ps(a11, index1, a15);
    b15 =  _mm512_permutex2var_ps(a11, index2, a15);      
    
    a0  =  _mm512_shuffle_f32x4(b0, b8,  0x44);
    a4  =  _mm512_shuffle_f32x4(b0, b8,  0xEE);
    a8  =  _mm512_shuffle_f32x4(b1, b9,  0x44);
    a12 =  _mm512_shuffle_f32x4(b1, b9,  0xEE);    
    a1  =  _mm512_shuffle_f32x4(b2, b10, 0x44);
    a5  =  _mm512_shuffle_f32x4(b2, b10, 0xEE);    
    a9  =  _mm512_shuffle_f32x4(b3, b11, 0x44);
    a13 =  _mm512_shuffle_f32x4(b3, b11, 0xEE);  
    a2  =  _mm512_shuffle_f32x4(b4, b12, 0x44);
    a6  =  _mm512_shuffle_f32x4(b4, b12, 0xEE);
    a10 =  _mm512_shuffle_f32x4(b5, b13, 0x44);
    a14 =  _mm512_shuffle_f32x4(b5, b13, 0xEE);    
    a3 =   _mm512_shuffle_f32x4(b6, b14, 0x44);
    a7  =  _mm512_shuffle_f32x4(b6, b14, 0xEE);    
    a11 =  _mm512_shuffle_f32x4(b7, b15, 0x44);
    a15 =  _mm512_shuffle_f32x4(b7, b15, 0xEE);     
    
    _mm512_store_ps((float *)(dst +  0), a0);
    _mm512_store_ps((float *)(dst + 16), a1);
    _mm512_store_ps((float *)(dst + 32), a2);
    _mm512_store_ps((float *)(dst + 48), a3);
    _mm512_store_ps((float *)(dst + 64), a4);
    _mm512_store_ps((float *)(dst + 80), a5);
    _mm512_store_ps((float *)(dst + 96), a6);
    _mm512_store_ps((float *)(dst +112), a7);  
    _mm512_store_ps((float *)(dst +128), a8);
    _mm512_store_ps((float *)(dst +144), a9);
    _mm512_store_ps((float *)(dst +160), a10);
    _mm512_store_ps((float *)(dst +176), a11);
    _mm512_store_ps((float *)(dst +192), a12);
    _mm512_store_ps((float *)(dst +208), a13);
    _mm512_store_ps((float *)(dst +224), a14);
    _mm512_store_ps((float *)(dst +240), a15);
#endif

#endif // VLEN == 16u
}

__forceinline void
transposeSOAToAOS_16x16(const uint32_t * __restrict src, uint32_t * __restrict dstRows[16])
{
#if (VLEN == 16u)

#ifndef __INTEL_COMPILER
#error transposeSOAToAOS_16x16() : Can not compile other than ICC so far
#else
    /* naive C code just for reference.
    const unsigned inputW = 16;
    const unsigned inputH = 16;

    for (unsigned i = 0; i < inputH; ++i) {
        for (unsigned j = 0; j < inputW; ++j) {
            dstRows[j][i] = src[(i << 4) + j];
        }
    }
    */
   __m512i index1 = _mm512_set_epi32(23, 22, 21, 20, 7, 6, 5, 4, 19, 18, 17, 16, 3, 2, 1, 0);
   __m512i index2 = _mm512_set_epi32(31, 30, 29, 28, 15, 14, 13, 12, 27, 26, 25, 24, 11, 10, 9, 8);
    
    __m512 a0  = _mm512_load_ps((const float *)(src +  0));
    __m512 a1  = _mm512_load_ps((const float *)(src + 16));
    __m512 a2  = _mm512_load_ps((const float *)(src + 32));
    __m512 a3  = _mm512_load_ps((const float *)(src + 48));
    __m512 a4  = _mm512_load_ps((const float *)(src + 64));
    __m512 a5  = _mm512_load_ps((const float *)(src + 80));
    __m512 a6  = _mm512_load_ps((const float *)(src + 96));
    __m512 a7  = _mm512_load_ps((const float *)(src +112));
    __m512 a8  = _mm512_load_ps((const float *)(src +128));
    __m512 a9  = _mm512_load_ps((const float *)(src +144));
    __m512 a10 = _mm512_load_ps((const float *)(src +160));
    __m512 a11 = _mm512_load_ps((const float *)(src +176));
    __m512 a12 = _mm512_load_ps((const float *)(src +192));
    __m512 a13 = _mm512_load_ps((const float *)(src +208));
    __m512 a14 = _mm512_load_ps((const float *)(src +224));
    __m512 a15 = _mm512_load_ps((const float *)(src +240));  
    
    
    __m512 b0  = _mm512_unpacklo_ps(a0, a1);
    __m512 b1  = _mm512_unpackhi_ps(a0, a1);
    __m512 b2  = _mm512_unpacklo_ps(a2, a3);
    __m512 b3  = _mm512_unpackhi_ps(a2, a3);
    __m512 b4  = _mm512_unpacklo_ps(a4, a5);
    __m512 b5  = _mm512_unpackhi_ps(a4, a5);
    __m512 b6  = _mm512_unpacklo_ps(a6, a7);
    __m512 b7  = _mm512_unpackhi_ps(a6, a7);  
    __m512 b8  = _mm512_unpacklo_ps(a8, a9);
    __m512 b9  = _mm512_unpackhi_ps(a8, a9);
    __m512 b10 = _mm512_unpacklo_ps(a10, a11);
    __m512 b11 = _mm512_unpackhi_ps(a10, a11);
    __m512 b12 = _mm512_unpacklo_ps(a12, a13);
    __m512 b13 = _mm512_unpackhi_ps(a12, a13);
    __m512 b14 = _mm512_unpacklo_ps(a14, a15);
    __m512 b15 = _mm512_unpackhi_ps(a14, a15);      
    
    a0  = _mm512_shuffle_ps(b0,  b2,_MM_SHUFFLE(1, 0, 1, 0));
    a1  = _mm512_shuffle_ps(b0,  b2,_MM_SHUFFLE(3, 2, 3, 2));
    a2  = _mm512_shuffle_ps(b1,  b3,_MM_SHUFFLE(1, 0, 1, 0));
    a3  = _mm512_shuffle_ps(b1,  b3,_MM_SHUFFLE(3, 2, 3, 2));
    a4  = _mm512_shuffle_ps(b4,  b6,_MM_SHUFFLE(1, 0, 1, 0));
    a5  = _mm512_shuffle_ps(b4,  b6,_MM_SHUFFLE(3, 2, 3, 2));
    a6  = _mm512_shuffle_ps(b5,  b7,_MM_SHUFFLE(1, 0, 1, 0));
    a7  = _mm512_shuffle_ps(b5,  b7,_MM_SHUFFLE(3, 2, 3, 2));    
    a8  = _mm512_shuffle_ps(b8,  b10,_MM_SHUFFLE(1, 0, 1, 0));
    a9  = _mm512_shuffle_ps(b8,  b10,_MM_SHUFFLE(3, 2, 3, 2));
    a10 = _mm512_shuffle_ps(b9,  b11,_MM_SHUFFLE(1, 0, 1, 0));
    a11 = _mm512_shuffle_ps(b9,  b11,_MM_SHUFFLE(3, 2, 3, 2));
    a12 = _mm512_shuffle_ps(b12, b14,_MM_SHUFFLE(1, 0, 1, 0));
    a13 = _mm512_shuffle_ps(b12, b14,_MM_SHUFFLE(3, 2, 3, 2));
    a14 = _mm512_shuffle_ps(b13, b15,_MM_SHUFFLE(1, 0, 1, 0));
    a15 = _mm512_shuffle_ps(b13, b15,_MM_SHUFFLE(3, 2, 3, 2));     
       
        
    b0  =  _mm512_permutex2var_ps(a0,  index1, a4);
    b1  =  _mm512_permutex2var_ps(a0,  index2, a4);    
    b2  =  _mm512_permutex2var_ps(a1,  index1, a5);
    b3  =  _mm512_permutex2var_ps(a1,  index2, a5);
    b4  =  _mm512_permutex2var_ps(a2,  index1, a6);
    b5  =  _mm512_permutex2var_ps(a2,  index2, a6);
    b6  =  _mm512_permutex2var_ps(a3,  index1, a7);
    b7  =  _mm512_permutex2var_ps(a3,  index2, a7);
    b8  =  _mm512_permutex2var_ps(a8,  index1, a12);
    b9  =  _mm512_permutex2var_ps(a8,  index2, a12);
    b10 =  _mm512_permutex2var_ps(a9,  index1, a13);
    b11 =  _mm512_permutex2var_ps(a9,  index2, a13);
    b12 =  _mm512_permutex2var_ps(a10, index1, a14);
    b13 =  _mm512_permutex2var_ps(a10, index2, a14);
    b14 =  _mm512_permutex2var_ps(a11, index1, a15);
    b15 =  _mm512_permutex2var_ps(a11, index2, a15);      
    
    a0  =  _mm512_shuffle_f32x4(b0, b8,  0x44);
    a4  =  _mm512_shuffle_f32x4(b0, b8,  0xEE);
    a8  =  _mm512_shuffle_f32x4(b1, b9,  0x44);
    a12 =  _mm512_shuffle_f32x4(b1, b9,  0xEE);    
    a1  =  _mm512_shuffle_f32x4(b2, b10, 0x44);
    a5  =  _mm512_shuffle_f32x4(b2, b10, 0xEE);    
    a9  =  _mm512_shuffle_f32x4(b3, b11, 0x44);
    a13 =  _mm512_shuffle_f32x4(b3, b11, 0xEE);  
    a2  =  _mm512_shuffle_f32x4(b4, b12, 0x44);
    a6  =  _mm512_shuffle_f32x4(b4, b12, 0xEE);
    a10 =  _mm512_shuffle_f32x4(b5, b13, 0x44);
    a14 =  _mm512_shuffle_f32x4(b5, b13, 0xEE);    
    a3 =   _mm512_shuffle_f32x4(b6, b14, 0x44);
    a7  =  _mm512_shuffle_f32x4(b6, b14, 0xEE);    
    a11 =  _mm512_shuffle_f32x4(b7, b15, 0x44);
    a15 =  _mm512_shuffle_f32x4(b7, b15, 0xEE);     
    
    _mm512_store_ps((float *)dstRows[0],  a0);
    _mm512_store_ps((float *)dstRows[1],  a1);
    _mm512_store_ps((float *)dstRows[2],  a2);
    _mm512_store_ps((float *)dstRows[3],  a3);
    _mm512_store_ps((float *)dstRows[4],  a4);
    _mm512_store_ps((float *)dstRows[5],  a5);
    _mm512_store_ps((float *)dstRows[6],  a6);
    _mm512_store_ps((float *)dstRows[7],  a7);  
    _mm512_store_ps((float *)dstRows[8],  a8);
    _mm512_store_ps((float *)dstRows[9],  a9);
    _mm512_store_ps((float *)dstRows[10], a10);
    _mm512_store_ps((float *)dstRows[11], a11);
    _mm512_store_ps((float *)dstRows[12], a12);
    _mm512_store_ps((float *)dstRows[13], a13);
    _mm512_store_ps((float *)dstRows[14], a14);
    _mm512_store_ps((float *)dstRows[15], a15);    
#endif

#endif // VLEN == 16u
}

//
// - Performs a full 8x8 transposition using AXV intrinsics, gathering from
//   randomly scattered rows (to facilitate the abililty to sort data).
// - Source and destination memory addresses must be 32-byte aligned.
// - The destination buffer is contiguous.
// - Element sizes are assumed to be 32-bits.
// - No prefetching is done in this function. It is up to the caller to perform
//   the necessary prefetching.
// - This function is marked __forceinline since there are zero optimization
//   barriers internally. This may not be obvious to the compiler if it's only
//   looking at function size.
//
__forceinline void
transposeAOSToSOA_8x8(const uint32_t *__restrict srcRows[8], uint32_t *__restrict dst)
{
#if 0

    const unsigned inputW = 8;
    const unsigned inputH = 8;

    for (unsigned i = 0; i < inputH; ++i) {
        for (unsigned j = 0; j < inputW; ++j) {
            dst[(j << 3) + i] = srcRows[i][j];
        }
    }

#else

    // http://stackoverflow.com/questions/25622745/transpose-an-8x8-float-using-avx-avx2

    __m256 a0 = _mm256_load_ps((const float *)srcRows[0]);
    __m256 a1 = _mm256_load_ps((const float *)srcRows[1]);
    __m256 a2 = _mm256_load_ps((const float *)srcRows[2]);
    __m256 a3 = _mm256_load_ps((const float *)srcRows[3]);
    __m256 a4 = _mm256_load_ps((const float *)srcRows[4]);
    __m256 a5 = _mm256_load_ps((const float *)srcRows[5]);
    __m256 a6 = _mm256_load_ps((const float *)srcRows[6]);
    __m256 a7 = _mm256_load_ps((const float *)srcRows[7]);

    __m256 b0 = _mm256_unpacklo_ps(a0, a1);
    __m256 b1 = _mm256_unpackhi_ps(a0, a1);
    __m256 b2 = _mm256_unpacklo_ps(a2, a3);
    __m256 b3 = _mm256_unpackhi_ps(a2, a3);
    __m256 b4 = _mm256_unpacklo_ps(a4, a5);
    __m256 b5 = _mm256_unpackhi_ps(a4, a5);
    __m256 b6 = _mm256_unpacklo_ps(a6, a7);
    __m256 b7 = _mm256_unpackhi_ps(a6, a7);

    a0 = _mm256_shuffle_ps(b0, b2,_MM_SHUFFLE(1, 0, 1, 0));
    a1 = _mm256_shuffle_ps(b0, b2,_MM_SHUFFLE(3, 2, 3, 2));
    a2 = _mm256_shuffle_ps(b1, b3,_MM_SHUFFLE(1, 0, 1, 0));
    a3 = _mm256_shuffle_ps(b1, b3,_MM_SHUFFLE(3, 2, 3, 2));
    a4 = _mm256_shuffle_ps(b4, b6,_MM_SHUFFLE(1, 0, 1, 0));
    a5 = _mm256_shuffle_ps(b4, b6,_MM_SHUFFLE(3, 2, 3, 2));
    a6 = _mm256_shuffle_ps(b5, b7,_MM_SHUFFLE(1, 0, 1, 0));
    a7 = _mm256_shuffle_ps(b5, b7,_MM_SHUFFLE(3, 2, 3, 2));

    b0 = _mm256_permute2f128_ps(a0, a4, 0x20);
    b1 = _mm256_permute2f128_ps(a1, a5, 0x20);
    b2 = _mm256_permute2f128_ps(a2, a6, 0x20);
    b3 = _mm256_permute2f128_ps(a3, a7, 0x20);
    b4 = _mm256_permute2f128_ps(a0, a4, 0x31);
    b5 = _mm256_permute2f128_ps(a1, a5, 0x31);
    b6 = _mm256_permute2f128_ps(a2, a6, 0x31);
    b7 = _mm256_permute2f128_ps(a3, a7, 0x31);

    _mm256_store_ps((float *)(dst +  0), b0);
    _mm256_store_ps((float *)(dst +  8), b1);
    _mm256_store_ps((float *)(dst + 16), b2);
    _mm256_store_ps((float *)(dst + 24), b3);
    _mm256_store_ps((float *)(dst + 32), b4);
    _mm256_store_ps((float *)(dst + 40), b5);
    _mm256_store_ps((float *)(dst + 48), b6);
    _mm256_store_ps((float *)(dst + 56), b7);

#endif
}

//
// - Performs a full 8x8 transposition using AXV intrinsics, writing to
//   randomly scattered rows.
// - Element sizes are assumed to be 32-bits.
// - Source and destination memory addresses must be 32-byte aligned.
// - The source buffer is contiguous.
// - No prefetching is done in this function. It is up to the caller to perform
//   the necessary prefetching.
//
__forceinline void
transposeSOAToAOS_8x8(const uint32_t *__restrict src, uint32_t *__restrict dstRows[8])
{
#if 0

    const unsigned inputW = 8;
    const unsigned inputH = 8;

    for (unsigned i = 0; i < inputH; ++i) {
        for (unsigned j = 0; j < inputW; ++j) {
            dstRows[j][i] = src[(i << 3) + j];
        }
    }

#else

    __m256 a0 = _mm256_load_ps((const float *)(src +  0));
    __m256 a1 = _mm256_load_ps((const float *)(src +  8));
    __m256 a2 = _mm256_load_ps((const float *)(src + 16));
    __m256 a3 = _mm256_load_ps((const float *)(src + 24));
    __m256 a4 = _mm256_load_ps((const float *)(src + 32));
    __m256 a5 = _mm256_load_ps((const float *)(src + 40));
    __m256 a6 = _mm256_load_ps((const float *)(src + 48));
    __m256 a7 = _mm256_load_ps((const float *)(src + 56));

    __m256 b0 = _mm256_unpacklo_ps(a0, a1);
    __m256 b1 = _mm256_unpackhi_ps(a0, a1);
    __m256 b2 = _mm256_unpacklo_ps(a2, a3);
    __m256 b3 = _mm256_unpackhi_ps(a2, a3);
    __m256 b4 = _mm256_unpacklo_ps(a4, a5);
    __m256 b5 = _mm256_unpackhi_ps(a4, a5);
    __m256 b6 = _mm256_unpacklo_ps(a6, a7);
    __m256 b7 = _mm256_unpackhi_ps(a6, a7);

    a0 = _mm256_shuffle_ps(b0, b2,_MM_SHUFFLE(1, 0, 1, 0));
    a1 = _mm256_shuffle_ps(b0, b2,_MM_SHUFFLE(3, 2, 3, 2));
    a2 = _mm256_shuffle_ps(b1, b3,_MM_SHUFFLE(1, 0, 1, 0));
    a3 = _mm256_shuffle_ps(b1, b3,_MM_SHUFFLE(3, 2, 3, 2));
    a4 = _mm256_shuffle_ps(b4, b6,_MM_SHUFFLE(1, 0, 1, 0));
    a5 = _mm256_shuffle_ps(b4, b6,_MM_SHUFFLE(3, 2, 3, 2));
    a6 = _mm256_shuffle_ps(b5, b7,_MM_SHUFFLE(1, 0, 1, 0));
    a7 = _mm256_shuffle_ps(b5, b7,_MM_SHUFFLE(3, 2, 3, 2));

    b0 = _mm256_permute2f128_ps(a0, a4, 0x20);
    b1 = _mm256_permute2f128_ps(a1, a5, 0x20);
    b2 = _mm256_permute2f128_ps(a2, a6, 0x20);
    b3 = _mm256_permute2f128_ps(a3, a7, 0x20);
    b4 = _mm256_permute2f128_ps(a0, a4, 0x31);
    b5 = _mm256_permute2f128_ps(a1, a5, 0x31);
    b6 = _mm256_permute2f128_ps(a2, a6, 0x31);
    b7 = _mm256_permute2f128_ps(a3, a7, 0x31);

    _mm256_store_ps((float *)(dstRows[0]), b0);
    _mm256_store_ps((float *)(dstRows[1]), b1);
    _mm256_store_ps((float *)(dstRows[2]), b2);
    _mm256_store_ps((float *)(dstRows[3]), b3);
    _mm256_store_ps((float *)(dstRows[4]), b4);
    _mm256_store_ps((float *)(dstRows[5]), b5);
    _mm256_store_ps((float *)(dstRows[6]), b6);
    _mm256_store_ps((float *)(dstRows[7]), b7);

#endif
}

// Similar functionality as the above AVX transpose functions, but optimized
// to transpose 4x4 matrices for SSE architectures.
__forceinline void
transposeAOSToSOA_4x4(const uint32_t *__restrict srcRows[4], uint32_t *__restrict dst)
{
    __m128 a0 = _mm_load_ps((const float *)srcRows[0]); 
    __m128 a1 = _mm_load_ps((const float *)srcRows[1]); 
    __m128 a2 = _mm_load_ps((const float *)srcRows[2]); 
    __m128 a3 = _mm_load_ps((const float *)srcRows[3]); 

#if 0
    __m128 b0 = _mm_shuffle_ps(a0, a1, _MM_SHUFFLE(1, 0, 1, 0));
    __m128 b1 = _mm_shuffle_ps(a2, a3, _MM_SHUFFLE(1, 0, 1, 0));
    __m128 b2 = _mm_shuffle_ps(a0, a1, _MM_SHUFFLE(3, 2, 3, 2));
    __m128 b3 = _mm_shuffle_ps(a2, a3, _MM_SHUFFLE(3, 2, 3, 2));

    a0 = _mm_shuffle_ps(b0, b1, _MM_SHUFFLE(2, 0, 2, 0));
    a1 = _mm_shuffle_ps(b0, b1, _MM_SHUFFLE(3, 1, 3, 1));
    a2 = _mm_shuffle_ps(b2, b3, _MM_SHUFFLE(2, 0, 2, 0));
    a3 = _mm_shuffle_ps(b2, b3, _MM_SHUFFLE(3, 1, 3, 1));
#else
    _MM_TRANSPOSE4_PS(a0, a1, a2, a3);
#endif

     _mm_store_ps((float *)(dst +  0), a0); 
     _mm_store_ps((float *)(dst +  4), a1); 
     _mm_store_ps((float *)(dst +  8), a2); 
     _mm_store_ps((float *)(dst + 12), a3); 
}

__forceinline void
transposeSOAToAOS_4x4(const uint32_t *__restrict src, uint32_t *__restrict dstRows[4])
{
    __m128 a0 = _mm_load_ps((const float *)(src +  0));
    __m128 a1 = _mm_load_ps((const float *)(src +  4));
    __m128 a2 = _mm_load_ps((const float *)(src +  8));
    __m128 a3 = _mm_load_ps((const float *)(src + 12));

    _MM_TRANSPOSE4_PS(a0, a1, a2, a3);

    _mm_store_ps((float *)(dstRows[0]), a0);
    _mm_store_ps((float *)(dstRows[1]), a1);
    _mm_store_ps((float *)(dstRows[2]), a2);
    _mm_store_ps((float *)(dstRows[3]), a3);
}


//----------------------------------------------------------------------------

} // namespace math
} // namespace scene_rdl2


