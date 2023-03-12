// Copyright 2023 DreamWorks Animation LLC and Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Constants.h"
#include <algorithm>
#include <cmath>
#include <emmintrin.h>
#include <float.h>
#include <immintrin.h>

// Intel: begin *****
/*
#include "sys/platform.h"
#include "sys/intrinsics.h"

#include <math.h>
#include <cmath>
#include <float.h>
#include <emmintrin.h>
#include <xmmintrin.h>
*/
// Intel: end *****

// MoonRay: begin *****
// GCC, Clang, and ICC all define __FMA__ if the compilation instruction set
// specifies fma. MSVC does not, but it is supported if there is AVX2 support.
#if !defined(__FMA__) && defined(__AVX2__)
    #define __FMA__ 1
#endif
// MoonRay: end *****

namespace scene_rdl2 {
namespace math {
// Intel: namespace embree {
#if defined(__WIN32__)
  __forceinline bool finite ( const float x ) { return _finite(x) != 0; }
#endif

// MoonRay: begin *****
  // In C, isfinite is a macro, so there's no explicit float version. In C++11,
  // isfinite is overloaded.
  __forceinline bool isfinite ( const float x ) { return std::isfinite(x); }
  __forceinline bool isfinite ( const double x ) { return std::isfinite(x); }

  // In C, isnormal is a macro, so there's no explicit float version. In C++11,
  // isnormal is overloaded.
  __forceinline bool isnormal ( const float x ) { return std::isnormal(x); }
  __forceinline bool isnormal ( const double x ) { return std::isnormal(x); }

  __forceinline float sign ( const float x ) { return x<0?-1.0f:1.0f; }
  __forceinline float sqr  ( const float x ) { return x*x; }
// MoonRay: end *****

  __forceinline float rcp  ( const float x )
  {
    const __m128 a = _mm_set1_ps(x);
    const __m128 r = _mm_rcp_ps(a);
#if defined(__AVX2__)
    return _mm_cvtss_f32(_mm_mul_ps(r,_mm_fnmadd_ps(r, a, _mm_set_ss(2.0f))));
#else
    return _mm_cvtss_f32(_mm_mul_ps(r,_mm_sub_ps(_mm_set_ss(2.0f), _mm_mul_ps(r, a))));
#endif
  }
// Intel: begin *****
/*
#ifndef __MIC__
  __forceinline float rcp  ( const float x ) 
  { 
    const __m128 vx = _mm_set_ss(x);
    const __m128 r = _mm_rcp_ps(vx);
    return _mm_cvtss_f32(_mm_sub_ps(_mm_add_ps(r, r), _mm_mul_ps(_mm_mul_ps(r, r), vx)));
  }
*/
// Intel: end *****

  __forceinline float signmsk ( const float x ) { 
    return _mm_cvtss_f32(_mm_and_ps(_mm_set_ss(x),_mm_castsi128_ps(_mm_set1_epi32(0x80000000))));
  }
  __forceinline float xorf( const float x, const float y ) { 
    return _mm_cvtss_f32(_mm_xor_ps(_mm_set_ss(x),_mm_set_ss(y)));
  }
  __forceinline float andf( const float x, const unsigned y ) { 
    return _mm_cvtss_f32(_mm_and_ps(_mm_set_ss(x),_mm_castsi128_ps(_mm_set1_epi32(y))));
  }
  __forceinline float rsqrt( const float x ) {
    const __m128 a = _mm_set1_ps(x);
    // Intel: const __m128 a = _mm_set_ss(x);
    const __m128 r = _mm_rsqrt_ps(a);
    const __m128 c = _mm_add_ps(_mm_mul_ps(_mm_set_ps(1.5f, 1.5f, 1.5f, 1.5f), r),
                                _mm_mul_ps(_mm_mul_ps(_mm_mul_ps(a, _mm_set_ps(-0.5f, -0.5f, -0.5f, -0.5f)), r), _mm_mul_ps(r, r)));
    return _mm_cvtss_f32(c);
  }
// Intel: begin *****
/*
#else
  __forceinline float rcp  ( const float x ) { return 1.0f/x; }
  __forceinline float signmsk ( const float x ) { return cast_i2f(cast_f2i(x)&0x80000000); }
  __forceinline float xorf( const float x, const float y ) { return cast_i2f(cast_f2i(x) ^ cast_f2i(y)); }
  __forceinline float andf( const float x, const float y ) { return cast_i2f(cast_f2i(x) & cast_f2i(y)); }
  __forceinline float rsqrt( const float x ) { return 1.0f/sqrtf(x); }
#endif
*/
// Intel: end *****

#if !defined(__WIN32__)
// MoonRay: added exp2(), log2() and sincos()
  __forceinline float abs  ( const float x ) { return ::fabsf(x); }
  __forceinline float acos ( const float x ) { return ::acosf (x); }
  __forceinline float asin ( const float x ) { return ::asinf (x); }
  __forceinline float atan ( const float x ) { return ::atanf (x); }
  __forceinline float atan2( const float y, const float x ) { return ::atan2f(y, x); }
  __forceinline float cos  ( const float x ) { return ::cosf  (x); }
  __forceinline float cosh ( const float x ) { return ::coshf (x); }
  __forceinline float exp  ( const float x ) { return ::expf  (x); }
  __forceinline float exp2 ( const float x ) { return ::exp2f (x); }
  __forceinline float fmod ( const float x, const float y ) { return ::fmodf (x, y); }
  __forceinline float log  ( const float x ) { return ::logf  (x); }
  __forceinline float log2 ( const float x ) { return ::log2f (x); }
  __forceinline float log10( const float x ) { return ::log10f(x); }
  __forceinline float pow  ( const float x, const float y ) { return ::powf  (x, y); }
  __forceinline float sin  ( const float x ) { return ::sinf  (x); }
  __forceinline void  sincos( const float a, float *y, float *x ) { return ::sincosf(a, y, x); }
  __forceinline float sinh ( const float x ) { return ::sinhf (x); }
  __forceinline float sqrt ( const float x ) { return ::sqrtf (x); }
  __forceinline float tan  ( const float x ) { return ::tanf  (x); }
  __forceinline float tanh ( const float x ) { return ::tanhf (x); }
  __forceinline float floor( const float x ) { return ::floorf (x); }
  __forceinline float ceil ( const float x ) { return ::ceilf (x); }
#endif

// MoonRay: added exp2(), log2() and sincos()
  __forceinline double abs  ( const double x ) { return ::fabs(x); }
  __forceinline double sign ( const double x ) { return x<0?-1.0:1.0; }
  __forceinline double acos ( const double x ) { return ::acos (x); }
  __forceinline double asin ( const double x ) { return ::asin (x); }
  __forceinline double atan ( const double x ) { return ::atan (x); }
  __forceinline double atan2( const double y, const double x ) { return ::atan2(y, x); }
  __forceinline double cos  ( const double x ) { return ::cos  (x); }
  __forceinline double cosh ( const double x ) { return ::cosh (x); }
  __forceinline double exp  ( const double x ) { return ::exp  (x); }
  __forceinline double exp2 ( const double x ) { return ::exp2 (x); }
  __forceinline double fmod ( const double x, const double y ) { return ::fmod (x, y); }
  __forceinline double log  ( const double x ) { return ::log  (x); }
  __forceinline double log2 ( const double x ) { return ::log2 (x); }
  __forceinline double log10( const double x ) { return ::log10(x); }
  __forceinline double pow  ( const double x, const double y ) { return ::pow  (x, y); }
  __forceinline double rcp  ( const double x ) { return 1.0/x; }
  __forceinline double rsqrt( const double x ) { return 1.0/::sqrt(x); }
  __forceinline double sin  ( const double x ) { return ::sin  (x); }
  __forceinline void   sincos( const double a, double *y, double *x ) { return ::sincos(a, y, x); }
  __forceinline double sinh ( const double x ) { return ::sinh (x); }
  __forceinline double sqr  ( const double x ) { return x*x; }
  __forceinline double sqrt ( const double x ) { return ::sqrt (x); }
  __forceinline double tan  ( const double x ) { return ::tan  (x); }
  __forceinline double tanh ( const double x ) { return ::tanh (x); }
  __forceinline double floor( const double x ) { return ::floor (x); }
  __forceinline double ceil ( const double x ) { return ::ceil (x); }

#if defined(__SSE4_1__)
  __forceinline float mini(float a, float b) { 
    const __m128i ai = _mm_castps_si128(_mm_set_ss(a));
    const __m128i bi = _mm_castps_si128(_mm_set_ss(b));
    const __m128i ci = _mm_min_epi32(ai,bi);
    return _mm_cvtss_f32(_mm_castsi128_ps(ci));
  }
#endif

#if defined(__SSE4_1__)
  __forceinline float maxi(float a, float b) { 
    const __m128i ai = _mm_castps_si128(_mm_set_ss(a));
    const __m128i bi = _mm_castps_si128(_mm_set_ss(b));
    const __m128i ci = _mm_max_epi32(ai,bi);
    return _mm_cvtss_f32(_mm_castsi128_ps(ci));
  }
#endif

// MoonRay: added these two abs() functions
  __forceinline                  int32 abs(int32  a)                                                   { return a<0? -a:a; }
  __forceinline                  int64 abs(int64  a)                                                   { return a<0? -a:a; }

// MoonRay: added
  __forceinline                 uint32 min(uint32 a, uint32 b)                                         { return a<b? a:b; }
// MoonRay: size_t -> uint64
  __forceinline                 uint64 min(uint64 a, uint64 b)                                         { return a<b? a:b; }
  __forceinline                    int min(int     a, int     b)                                       { return a<b? a:b; }
  __forceinline                  int64 min(int64   a, int64   b)                                       { return a<b? a:b; }
// Intel begin *****
/*
  __forceinline                 size_t min(size_t  a, size_t  b)                                       { return a<b? a:b; }
#if !defined(__WIN32__)
// Intel:  __forceinline                ssize_t min(ssize_t a, ssize_t b)                                       { return a<b? a:b; }
#endif
*/
// Intel end *****
  __forceinline                  float min(float   a, float   b)                                       { return a<b? a:b; }
  __forceinline                 double min(double  a, double  b)                                       { return a<b? a:b; }
  template<typename T> __forceinline T min(const T& a, const T& b, const T& c)                         { return min(min(a,b),c); }
  template<typename T> __forceinline T min(const T& a, const T& b, const T& c, const T& d)             { return min(min(a,b),min(c,d)); }
  template<typename T> __forceinline T min(const T& a, const T& b, const T& c, const T& d, const T& e) { return min(min(min(a,b),min(c,d)),e); }

// MoonRay: added
  __forceinline                 uint32 max(uint32 a, uint32 b)                                         { return a<b? b:a; }
// MoonRay: size_t -> uint64
  __forceinline                 uint64 max(uint64 a, uint64 b)                                         { return a<b? b:a; }
  __forceinline                    int max(int     a, int     b)                                       { return a<b? b:a; }
  __forceinline                  int64 max(int64   a, int64   b)                                       { return a<b? b:a; }
// Intel begin *****
/*
  __forceinline                 size_t max(size_t  a, size_t  b)                                       { return a<b? b:a; }
#if !defined(__WIN32__)
// Intel:  __forceinline                ssize_t max(ssize_t a, ssize_t b)                                       { return a<b? b:a; }
#endif
*/
// Intel end *****
  __forceinline                  float max(float   a, float   b)                                       { return a<b? b:a; }
  __forceinline                 double max(double  a, double  b)                                       { return a<b? b:a; }
  template<typename T> __forceinline T max(const T& a, const T& b, const T& c)                         { return max(max(a,b),c); }
  template<typename T> __forceinline T max(const T& a, const T& b, const T& c, const T& d)             { return max(max(a,b),max(c,d)); }
  template<typename T> __forceinline T max(const T& a, const T& b, const T& c, const T& d, const T& e) { return max(max(max(a,b),max(c,d)),e); }

  template<typename T> __forceinline T clamp(const T& x, const T& lower = T(zero), const T& upper = T(one)) { return max(min(x,upper),lower); }
  template<typename T> __forceinline T clampz(const T& x, const T& upper) { return max(T(zero), min(x,upper)); }

// MoonRay: begin *****
  template<typename T, typename U> __forceinline T lerp(const T& a, const T& b, const U t) { return (b - a) * t + a; }
  template<typename T, typename U> __forceinline T bilerp(const T& s0t0, const T& s1t0, const T& s0t1, const T& s1t1,
                                                          const U s, const U t)
  {
      return lerp(lerp(s0t0, s1t0, s), lerp(s0t1, s1t1, s), t);
  }

  // given the result, find t such that result = lerp(start, end, t)
  template<typename T, typename U> __forceinline void invLerp(const T &start, const T &end, const T &result, U *t) { *t = (result - start) / (end - start); }
// MoonRay: end *****

  template<typename T> __forceinline T  deg2rad ( const T& x )  { return x * T(1.74532925199432957692e-2f); }
  template<typename T> __forceinline T  rad2deg ( const T& x )  { return x * T(5.72957795130823208768e1f); }
  template<typename T> __forceinline T  sin2cos ( const T& x )  { return sqrt(max(T(zero),T(one)-x*x)); }
  template<typename T> __forceinline T  cos2sin ( const T& x )  { return sin2cos(x); }

// MoonRay: begin *****
#if defined(__FMA__)
    inline float madd  (const float a, const float b, const float c) { return std::fma( a,  b,  c); }
    inline float msub  (const float a, const float b, const float c) { return std::fma( a,  b, -c); }
    inline float nmadd (const float a, const float b, const float c) { return std::fma(-a,  b,  c); }
    inline float nmsub (const float a, const float b, const float c) { return std::fma(-a,  b, -c); }
#else
    inline float madd  (const float a, const float b, const float c) { return  a * b + c; }
    inline float msub  (const float a, const float b, const float c) { return  a * b - c; }
    inline float nmadd (const float a, const float b, const float c) { return -a * b + c; }
    inline float nmsub (const float a, const float b, const float c) { return -a * b - c; }
#endif
// MoonRay: end *****

// Intel: begin *****
/*
#if defined(__AVX2__)
  __forceinline float madd  ( const float a, const float b, const float c) { return _mm_cvtss_f32(_mm_fmadd_ss(_mm_set_ss(a),_mm_set_ss(b),_mm_set_ss(c))); }
  __forceinline float msub  ( const float a, const float b, const float c) { return _mm_cvtss_f32(_mm_fmsub_ss(_mm_set_ss(a),_mm_set_ss(b),_mm_set_ss(c))); }
  __forceinline float nmadd ( const float a, const float b, const float c) { return _mm_cvtss_f32(_mm_fnmadd_ss(_mm_set_ss(a),_mm_set_ss(b),_mm_set_ss(c))); }
  __forceinline float nmsub ( const float a, const float b, const float c) { return _mm_cvtss_f32(_mm_fnmsub_ss(_mm_set_ss(a),_mm_set_ss(b),_mm_set_ss(c))); }
#else
  __forceinline float madd  ( const float a, const float b, const float c) { return a*b+c; }
  __forceinline float msub  ( const float a, const float b, const float c) { return a*b-c; }
  __forceinline float nmadd ( const float a, const float b, const float c) { return -a*b-c;}
  __forceinline float nmsub ( const float a, const float b, const float c) { return c-a*b; }
#endif
*/
// Intel: end *****

  /*! random functions */
  template<typename T> T          random() { return T(0); }
  template<> __forceinline int    random() { return int(rand()); }
  template<> __forceinline uint32 random() { return uint32(rand()); }
  template<> __forceinline float  random() { return float(random<uint32>())/float(RAND_MAX); }
  // Intel: template<> __forceinline float  random() { return random<uint32>()/float(RAND_MAX); }
  template<> __forceinline double random() { return random<uint32>()/double(RAND_MAX); }

  /*! selects */
  __forceinline bool  select(bool s, bool  t , bool f) { return s ? t : f; }
  __forceinline int   select(bool s, int   t,   int f) { return s ? t : f; }
  __forceinline float select(bool s, float t, float f) { return s ? t : f; }

  /*! exchange */
  template<typename T> __forceinline void xchg ( T& a, T& b ) { const T tmp = a; a = b; b = tmp; }

  /*! bit reverse operation */
  template<class T>
    __forceinline T bitReverse(const T& vin)
  {
    T v = vin;
    v = ((v >> 1) & 0x55555555) | ((v & 0x55555555) << 1);
    v = ((v >> 2) & 0x33333333) | ((v & 0x33333333) << 2);
    v = ((v >> 4) & 0x0F0F0F0F) | ((v & 0x0F0F0F0F) << 4);
    v = ((v >> 8) & 0x00FF00FF) | ((v & 0x00FF00FF) << 8);
    v = ( v >> 16             ) | ( v               << 16);
    return v;
  }

  /*! bit interleave operation */
  template<class T>
    __forceinline T bitInterleave(const T& xin, const T& yin, const T& zin)
  {
	T x = xin, y = yin, z = zin;
    x = (x | (x << 16)) & 0x030000FF; 
    x = (x | (x <<  8)) & 0x0300F00F; 
    x = (x | (x <<  4)) & 0x030C30C3; 
    x = (x | (x <<  2)) & 0x09249249; 
    
    y = (y | (y << 16)) & 0x030000FF; 
    y = (y | (y <<  8)) & 0x0300F00F; 
    y = (y | (y <<  4)) & 0x030C30C3; 
    y = (y | (y <<  2)) & 0x09249249; 
    
    z = (z | (z << 16)) & 0x030000FF; 
    z = (z | (z <<  8)) & 0x0300F00F; 
    z = (z | (z <<  4)) & 0x030C30C3; 
    z = (z | (z <<  2)) & 0x09249249; 
    
    return x | (y << 1) | (z << 2);
  }

  /*! bit interleave operation for 64bit data types*/
  template<class T>
    __forceinline T bitInterleave64(const T& xin, const T& yin, const T& zin){
    T x = xin & 0x1fffff; 
    T y = yin & 0x1fffff; 
    T z = zin & 0x1fffff; 

    x = (x | x << 32) & 0x1f00000000ffff;  
    x = (x | x << 16) & 0x1f0000ff0000ff;  
    x = (x | x << 8) & 0x100f00f00f00f00f; 
    x = (x | x << 4) & 0x10c30c30c30c30c3; 
    x = (x | x << 2) & 0x1249249249249249;

    y = (y | y << 32) & 0x1f00000000ffff;  
    y = (y | y << 16) & 0x1f0000ff0000ff;  
    y = (y | y << 8) & 0x100f00f00f00f00f; 
    y = (y | y << 4) & 0x10c30c30c30c30c3; 
    y = (y | y << 2) & 0x1249249249249249;

    z = (z | z << 32) & 0x1f00000000ffff;  
    z = (z | z << 16) & 0x1f0000ff0000ff;  
    z = (z | z << 8) & 0x100f00f00f00f00f; 
    z = (z | z << 4) & 0x10c30c30c30c30c3; 
    z = (z | z << 2) & 0x1249249249249249;

    return x | (y << 1) | (z << 2);
  }

#if defined(_WIN32)
// Intel: #if _WIN32
  __forceinline double drand48() {
    return double(rand())/double(RAND_MAX);
  }
#endif

// MoonRay: begin added *****

// Equality with tolerance, fixed for |a| < 1, otherwise relative
template <typename T> __forceinline bool 
isEqual(const T a, const T b, const T eps = T(epsilon))
{ 
    return abs(a - b) <= (std::max<T>(abs(a), T(1)) * eps);
}

// Equality with fixed tolerance
template <typename T> __forceinline bool 
isEqualFixedEps(const T a, const T b, const T eps = T(epsilon))
{ 
    return abs(a - b) <= eps;
}

template <typename T> __forceinline bool 
isZero(const T a, const T eps = T(epsilon))
{ 
    return abs(a) <= eps;
}

template <typename T> __forceinline bool 
isOne(const T a, const T eps = T(epsilon))
{ 
    return abs(a - T(1)) <= eps;
}

  template<typename T> __forceinline bool isValidFloat(T x) { int type = std::fpclassify(x); return type == FP_ZERO || type == FP_NORMAL; }

// C++ implementation to match ISPC version of erf() in Math.isph.
__forceinline float erf(const float x) {
    // Algorithm 7.1.25 in Abromowitz and Stegun, Handbook of Mathematical
    // Functions With Formulas, Graphs, and Mathematical Tables (pg. 299)
    // (maximum error: 2.5×10−5)
    // http://www.math.ubc.ca/~cbm/aands/abramowitz_and_stegun.pdf (pg. 88)
    const float p  =  0.47047f;
    const float a1 =  0.3480242f;
    const float a2 = -0.0958798f;
    const float a3 =  0.7478556f;
    float t = 1.0f / (1.0f + p * abs(x));
    float result = 1.0f - t * (a1 + t * (a2 + t * a3)) * exp( -(x * x) );
    return copysignf(result, x);
}

// MoonRay: begin *****
// Return a*b - c*d
template <typename T, typename U, typename V, typename W>
inline auto differenceOfProductsFast(const T& a, const U& b, const V& c, const W& d) noexcept
{
    return msub(a, b, c*d);
}

#if defined(__FMA__)
// Return a*b - c*d in a way that avoids catastrophic cancellation.
// https://pharr.org/matt/blog/2019/11/03/difference-of-floats
template <typename T, typename U, typename V, typename W>
inline auto differenceOfProducts(const T& a, const U& b, const V& c, const W& d) noexcept
{
    const auto cd = c * d;
    const auto err = nmadd(c, d, cd); // Compute the error in computing c*d
    const auto dop =  msub(a, b, cd);
    return dop + err;
}
#else
// Return a*b - c*d
template <typename T, typename U, typename V, typename W>
inline auto differenceOfProducts(const T& a, const U& b, const V& c, const W& d) noexcept
{
    return a*b - c*d;
}
#endif

// MoonRay: end added *****
}
}

