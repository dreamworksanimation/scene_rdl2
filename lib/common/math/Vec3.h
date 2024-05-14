// Copyright 2023-2024 DreamWorks Animation LLC and Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

// MoonRay: begin *****
#include <scene_rdl2/common/platform/HybridUniformData.h>
#include <scene_rdl2/common/platform/Platform.h>

#include "Math.h"

#if defined __SSE__
#include "sse.h"
#endif

#if defined __AVX__
#include "avx.h"
#endif

#if defined __AVX512F__
#include "mic.h"
#endif

#include <type_traits>

// Forward declaration of the ISPC types
namespace ispc {
    struct Vec3f;
}
// MoonRay: end *****
// Intel: begin *****
/*
#include "sys/platform.h"
#include "math/math.h"

#if defined __SSE__
#include "simd/sse.h"
#endif

#if defined __AVX__
#include "simd/avx.h"
#endif

#if defined __MIC__
#include "simd/sse_mic.h"
#endif

#if defined __MIC__
#include "simd/mic.h"
#endif
*/
// Intel: end *****

namespace scene_rdl2 {
namespace math {
// Intel: namespace embree

  struct Vec3fa;

  ////////////////////////////////////////////////////////////////////////////////
  /// Generic 3D vector Class
  ////////////////////////////////////////////////////////////////////////////////

  template<typename T> struct Vec3
  {
    T x, y, z;

    typedef T Scalar;
    enum { N  = 3 };

    ////////////////////////////////////////////////////////////////////////////////
    /// Construction
    ////////////////////////////////////////////////////////////////////////////////

    // MoonRay: using "= default" allows the compiler to assume trivially constructible/copyable
    __forceinline Vec3 ( ) = default;
    // Intel: __forceinline Vec3 ( ) {}
    __forceinline Vec3 ( const T& a                         ) : x(a), y(a), z(a) {}
    // MoonRay: added underscores to fix compile warning
    __forceinline Vec3 ( const T& _x, const T& _y, const T& _z ) : x(_x), y(_y), z(_z) {}

    __forceinline Vec3     ( const Vec3& other ) = default;
    // Intel: __forceinline Vec3     ( const Vec3& other ) { x = other.x; y = other.y; z = other.z; }
    __forceinline Vec3     ( const Vec3fa& other );

    template<typename T1> __forceinline Vec3( const Vec3<T1>& a ) : x(T(a.x)), y(T(a.y)), z(T(a.z)) {}
    template<typename T1> __forceinline Vec3& operator =(const Vec3<T1>& other) { x = other.x; y = other.y; z = other.z; return *this; }
  
    // MoonRay: added
    __forceinline explicit Vec3( const T* const a, const size_t stride = 1 ) : x(a[0]), y(a[stride]), z(a[2*stride]) {}
    
    ////////////////////////////////////////////////////////////////////////////////
    /// Constants
    ////////////////////////////////////////////////////////////////////////////////

    __forceinline Vec3( ZeroTy   ) : x(zero), y(zero), z(zero) {}
    __forceinline Vec3( OneTy    ) : x(one),  y(one),  z(one) {}
    __forceinline Vec3( PosInfTy ) : x(pos_inf), y(pos_inf), z(pos_inf) {}
    __forceinline Vec3( NegInfTy ) : x(neg_inf), y(neg_inf), z(neg_inf) {}

    // MoonRay: use MNRY_ASSERT() instead of assert()
    __forceinline const T& operator []( const size_t axis ) const { MNRY_ASSERT(axis < 3); return (&x)[axis]; }
    __forceinline       T& operator []( const size_t axis )       { MNRY_ASSERT(axis < 3); return (&x)[axis]; }

    // MoonRay:: begin *****
    ////////////////////////////////////////////////////////////////////////////////
    /// Convenience functions
    ////////////////////////////////////////////////////////////////////////////////

    __forceinline T length() const;
    __forceinline T lengthSqr() const;
    __forceinline Vec3<T> &normalize();
    __forceinline Vec3<T> & safeNormalize(T eps = T(epsilon));
    // MoonRay: end *****
  };

  ////////////////////////////////////////////////////////////////////////////////
  /// Unary Operators
  ////////////////////////////////////////////////////////////////////////////////

  template<typename T> __forceinline Vec3<T> operator +( const Vec3<T>& a ) { return Vec3<T>(+a.x, +a.y, +a.z); }
  template<typename T> __forceinline Vec3<T> operator -( const Vec3<T>& a ) { return Vec3<T>(-a.x, -a.y, -a.z); }
  template<typename T> __forceinline Vec3<T> abs       ( const Vec3<T>& a ) { return Vec3<T>(abs  (a.x), abs  (a.y), abs  (a.z)); }
  template<typename T> __forceinline Vec3<T> rcp       ( const Vec3<T>& a ) { return Vec3<T>( 1.0f/a.x,   1.0f/a.y,   1.0f/a.z);  }
  template<typename T> __forceinline Vec3<T> rsqrt     ( const Vec3<T>& a ) { return Vec3<T>(rsqrt(a.x), rsqrt(a.y), rsqrt(a.z)); }
  template<typename T> __forceinline Vec3<T> sqrt      ( const Vec3<T>& a ) { return Vec3<T>(sqrt (a.x), sqrt (a.y), sqrt (a.z)); }

  // MoonRay: begin *****
  template<typename T> __forceinline bool    isFinite  ( const Vec3<T>& a ) { return math::isfinite(a.x) && math::isfinite(a.y) && math::isfinite(a.z); }
  template<typename T> __forceinline bool    isNormalized( const Vec3<T>& a, float eps ) { return fabsf((lengthSqr(a) - T(1)) - (eps * eps)) < (eps * T(2)); }
  template<typename T> __forceinline bool isNormalized(const Vec3<T>& a) {
      const float l = lengthSqr(a);
      return (l > sNormalizedLengthSqrMin  &&  l < sNormalizedLengthSqrMax);
  }
  // MoonRay: end *****

  template<typename T> __forceinline Vec3<T> zero_fix( const Vec3<T>& a ) {
    return Vec3<T>(select(a.x==0.0f,T(1E-10f),a.x),select(a.y==0.0f,T(1E-10f),a.y),select(a.z==0.0f,T(1E-10f),a.z));
  }
  template<typename T> __forceinline const Vec3<T> rcp_safe(const Vec3<T>& a) { return rcp(zero_fix(a)); }

  ////////////////////////////////////////////////////////////////////////////////
  /// Binary Operators
  ////////////////////////////////////////////////////////////////////////////////

  template<typename T> __forceinline Vec3<T> operator +( const Vec3<T>& a, const Vec3<T>& b ) { return Vec3<T>(a.x + b.x, a.y + b.y, a.z + b.z); }
  template<typename T> __forceinline Vec3<T> operator -( const Vec3<T>& a, const Vec3<T>& b ) { return Vec3<T>(a.x - b.x, a.y - b.y, a.z - b.z); }
  template<typename T> __forceinline Vec3<T> operator *( const Vec3<T>& a, const Vec3<T>& b ) { return Vec3<T>(a.x * b.x, a.y * b.y, a.z * b.z); }
  template<typename T> __forceinline Vec3<T> operator *( const       T& a, const Vec3<T>& b ) { return Vec3<T>(a   * b.x, a   * b.y, a   * b.z); }
  template<typename T> __forceinline Vec3<T> operator *( const Vec3<T>& a, const       T& b ) { return Vec3<T>(a.x * b  , a.y * b  , a.z * b  ); }
  template<typename T> __forceinline Vec3<T> operator /( const Vec3<T>& a, const       T& b ) { return Vec3<T>(a.x / b  , a.y / b  , a.z / b  ); }
  template<typename T> __forceinline Vec3<T> operator /( const       T& a, const Vec3<T>& b ) { return Vec3<T>(a   / b.x, a   / b.y, a   / b.z); }
  template<typename T> __forceinline Vec3<T> operator /( const Vec3<T>& a, const Vec3<T>& b ) { return Vec3<T>(a.x / b.x, a.y / b.y, a.z / b.z); }

  template<typename T> __forceinline Vec3<T> min(const Vec3<T>& a, const Vec3<T>& b) { return Vec3<T>(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z)); }
  template<typename T> __forceinline Vec3<T> max(const Vec3<T>& a, const Vec3<T>& b) { return Vec3<T>(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z)); }

  template<typename T> __forceinline Vec3<T> operator >>( const Vec3<T>& a, const int b ) { return Vec3<T>(a.x >> b, a.y >> b, a.z >> b); }
  template<typename T> __forceinline Vec3<T> operator <<( const Vec3<T>& a, const int b ) { return Vec3<T>(a.x << b, a.y << b, a.z << b); }

  ////////////////////////////////////////////////////////////////////////////////
  /// Ternary Operators
  ////////////////////////////////////////////////////////////////////////////////

  // MoonRay: begin *****
  template<typename T> inline Vec3<T> madd( const Vec3<T>& a, const Vec3<T>& b, const Vec3<T>& c )
  {
      return Vec3<T>(madd(a.x, b.x, c.x), madd(a.y, b.y, c.y), madd(a.z, b.z, c.z));
  }

  template<typename T> inline Vec3<T> madd( const T& a, const Vec3<T>& b, const Vec3<T>& c )
  {
      return Vec3<T>(madd(a, b.x, c.x), madd(a, b.y, c.y), madd(a, b.z, c.z));
  }

  template<typename T> inline Vec3<T> madd( const Vec3<T>& a, const T& b, const Vec3<T>& c )
  {
      return Vec3<T>(madd(a.x, b, c.x), madd(a.y, b, c.y), madd(a.z, b, c.z));
  }

  template<typename T> inline Vec3<T> msub( const Vec3<T>& a, const Vec3<T>& b, const Vec3<T>& c )
  {
      return madd(a, b, -c);
  }

  template<typename T> inline Vec3<T> msub( const T& a, const Vec3<T>& b, const Vec3<T>& c )
  {
      return madd(a, b, -c);
  }

  template<typename T> inline Vec3<T> msub( const Vec3<T>& a, const T& b, const Vec3<T>& c )
  {
      return madd(a, b, -c);
  }

  template<typename T> inline Vec3<T> nmadd( const Vec3<T>& a, const Vec3<T>& b, const Vec3<T>& c )
  {
      return madd(-a, b, c);
  }

  template<typename T> inline Vec3<T> nmadd( const T& a, const Vec3<T>& b, const Vec3<T>& c )
  {
      return madd(-a, b, c);
  }

  template<typename T> inline Vec3<T> nmadd( const Vec3<T>& a, const T& b, const Vec3<T>& c )
  {
      return madd(-a, b, c);
  }

  template<typename T> inline Vec3<T> nmsub( const Vec3<T>& a, const Vec3<T>& b, const Vec3<T>& c )
  {
      return madd(-a, b, -c);
  }

  template<typename T> inline Vec3<T> nmsub( const T& a, const Vec3<T>& b, const Vec3<T>& c )
  {
      return madd(-a, b, -c);
  }

  template<typename T> inline Vec3<T> nmsub( const Vec3<T>& a, const T& b, const Vec3<T>& c )
  {
      return madd(-a, b, -c);
  }
  // MoonRay: end *****

  ////////////////////////////////////////////////////////////////////////////////
  /// Assignment Operators
  ////////////////////////////////////////////////////////////////////////////////

  template<typename T> __forceinline const Vec3<T>& operator +=( Vec3<T>& a, const T        b ) { a.x += b;   a.y += b;   a.z += b;   return a; }
  template<typename T> __forceinline const Vec3<T>& operator +=( Vec3<T>& a, const Vec3<T>& b ) { a.x += b.x; a.y += b.y; a.z += b.z; return a; }
  template<typename T> __forceinline const Vec3<T>& operator -=( Vec3<T>& a, const Vec3<T>& b ) { a.x -= b.x; a.y -= b.y; a.z -= b.z; return a; }
  template<typename T> __forceinline const Vec3<T>& operator *=( Vec3<T>& a, const       T& b ) { a.x *= b  ; a.y *= b  ; a.z *= b  ; return a; }
  template<typename T> __forceinline const Vec3<T>& operator /=( Vec3<T>& a, const       T& b ) { a.x /= b  ; a.y /= b  ; a.z /= b  ; return a; }

  ////////////////////////////////////////////////////////////////////////////////
  /// Reduction Operators
  ////////////////////////////////////////////////////////////////////////////////

  template<typename T> __forceinline T reduce_add( const Vec3<T>& a ) { return a.x + a.y + a.z; }
  template<typename T> __forceinline T reduce_mul( const Vec3<T>& a ) { return a.x * a.y * a.z; }
  template<typename T> __forceinline T reduce_min( const Vec3<T>& a ) { return min(a.x, a.y, a.z); }
  template<typename T> __forceinline T reduce_max( const Vec3<T>& a ) { return max(a.x, a.y, a.z); }

  ////////////////////////////////////////////////////////////////////////////////
  /// Comparison Operators
  ////////////////////////////////////////////////////////////////////////////////

  template<typename T> __forceinline bool operator ==( const Vec3<T>& a, const Vec3<T>& b ) { return a.x == b.x && a.y == b.y && a.z == b.z; }
  template<typename T> __forceinline bool operator !=( const Vec3<T>& a, const Vec3<T>& b ) { return a.x != b.x || a.y != b.y || a.z != b.z; }
  template<typename T> __forceinline bool operator < ( const Vec3<T>& a, const Vec3<T>& b ) {
    if (a.x != b.x) return a.x < b.x;
    if (a.y != b.y) return a.y < b.y;
    if (a.z != b.z) return a.z < b.z;
    return false;
  }

  // MoonRay: begin *****
  template<typename T> __forceinline bool isZero(const Vec3<T>& a)                                                { return (a.x == T(zero)) && (a.y == T(zero)) && (a.z == T(zero)); }
  template<typename T> __forceinline bool isEqual(const Vec3<T>& a, const Vec3<T>& b, T eps = T(epsilon))         { return isEqual(a.x, b.x, eps) && isEqual(a.y, b.y, eps) && isEqual(a.z, b.z, eps); }
  template<typename T> __forceinline bool isEqualFixedEps(const Vec3<T>& a, const Vec3<T>& b, T eps = T(epsilon)) { return isEqualFixedEps(a.x, b.x, eps) && isEqualFixedEps(a.y, b.y, eps) && isEqualFixedEps(a.z, b.z, eps); }
  // MoonRay: end *****

  ////////////////////////////////////////////////////////////////////////////////
  /// Euclidian Space Operators
  ////////////////////////////////////////////////////////////////////////////////

  template <typename T>
  inline T dot (const Vec3<T>& a, const Vec3<T>& b)
  {
      return madd(a.x, b.x, madd(a.y, b.y, a.z*b.z));
  }

  // MoonRay: begin *****
  template <typename T>
  inline Vec3<T> cross(const Vec3<T>& a, const Vec3<T>& b)
  {
      // Accurate Differences of Products with Kahan's Algorithm
      // https://pharr.org/matt/blog/2019/11/03/difference-of-floats
      // This version of cross may be a touch slower with increased precision.
      // Profiling did not demonstrate a noticeable slowdown.
      return Vec3<T>(differenceOfProducts(a.y, b.z, a.z, b.y),
                     differenceOfProducts(a.z, b.x, a.x, b.z),
                     differenceOfProducts(a.x, b.y, a.y, b.x));
  }
  // MoonRay: end *****

  template<typename T> __forceinline T       length    ( const Vec3<T>& a )                   { return sqrt(dot(a,a)); }
  template<typename T> __forceinline Vec3<T> normalize ( const Vec3<T>& a )                   { return a*rsqrt(dot(a,a)); }
  template<typename T> __forceinline T       distance  ( const Vec3<T>& a, const Vec3<T>& b ) { return length(a-b); }

  // MoonRay: begin *****
  template<typename T> __forceinline T       lengthSqr( const Vec3<T>& a )                   { return dot(a,a); }

  template <typename T> __forceinline Vec3<T>
  safeNormalize(const Vec3<T> & a, const T eps = T(epsilon))
  {
      const T lengthSqr = dot(a, a);
      return (lengthSqr <= eps * eps) ? Vec3<T>(T(0)) : a * rsqrt(lengthSqr);
  }
  // MoonRay: end *****

  ////////////////////////////////////////////////////////////////////////////////
  /// Select
  ////////////////////////////////////////////////////////////////////////////////

  template<typename T> __forceinline Vec3<T> select ( bool s, const Vec3<T>& t, const Vec3<T>& f ) {
    return Vec3<T>(select(s,t.x,f.x),select(s,t.y,f.y),select(s,t.z,f.z));
  }

  template<typename T> __forceinline Vec3<T> select ( const Vec3<bool>& s, const Vec3<T>& t, const Vec3<T>& f ) {
    return Vec3<T>(select(s.x,t.x,f.x),select(s.y,t.y,f.y),select(s.z,t.z,f.z));
  }

  template<typename T> __forceinline Vec3<T> select ( const typename T::Mask& s, const Vec3<T>& t, const Vec3<T>& f ) {
    return Vec3<T>(select(s,t.x,f.x),select(s,t.y,f.y),select(s,t.z,f.z));
  }

  template<typename T> __forceinline int maxDim ( const Vec3<T>& a ) 
  { 
    if (a.x > a.y) {
      if (a.x > a.z) return 0; else return 2;
    } else {
      if (a.y > a.z) return 1; else return 2;
    }
  }

  // MoonRay: begin *****
  ////////////////////////////////////////////////////////////////////////////////
  /// Convenience functions
  ////////////////////////////////////////////////////////////////////////////////

  template<typename T> __forceinline T Vec3<T>::length() const      { return math::length(*this); }
  template<typename T> __forceinline T Vec3<T>::lengthSqr() const   { return math::lengthSqr(*this); }
  template<typename T> __forceinline Vec3<T> &Vec3<T>::normalize()  { return *this = math::normalize(*this); }
  
  template<typename T> __forceinline Vec3<T> &
  Vec3<T>::safeNormalize(const T eps)  
  { return *this = math::safeNormalize(*this, eps); }
  // MoonRay: end *****

  ////////////////////////////////////////////////////////////////////////////////
  /// Comparison Operators
  ////////////////////////////////////////////////////////////////////////////////
  template<typename T> __forceinline Vec3<bool> eq_mask( const Vec3<T>& a, const Vec3<T>& b ) { return Vec3<bool>(a.x==b.x,a.y==b.y,a.z==b.z); }
  template<typename T> __forceinline Vec3<bool> neq_mask(const Vec3<T>& a, const Vec3<T>& b ) { return Vec3<bool>(a.x!=b.x,a.y!=b.y,a.z!=b.z); }
  template<typename T> __forceinline Vec3<bool> lt_mask( const Vec3<T>& a, const Vec3<T>& b ) { return Vec3<bool>(a.x< b.x,a.y< b.y,a.z< b.z); }
  template<typename T> __forceinline Vec3<bool> le_mask( const Vec3<T>& a, const Vec3<T>& b ) { return Vec3<bool>(a.x<=b.x,a.y<=b.y,a.z<=b.z); }
  template<typename T> __forceinline Vec3<bool> gt_mask( const Vec3<T>& a, const Vec3<T>& b ) { return Vec3<bool>(a.x> b.x,a.y> b.y,a.z> b.z); }
  template<typename T> __forceinline Vec3<bool> ge_mask( const Vec3<T>& a, const Vec3<T>& b ) { return Vec3<bool>(a.x>=b.x,a.y>=b.y,a.z>=b.z); }

  ////////////////////////////////////////////////////////////////////////////////
  /// Output Operators
  ////////////////////////////////////////////////////////////////////////////////

  template<typename T> inline std::ostream& operator<<(std::ostream& cout, const Vec3<T>& a) {
    return cout << "(" << a.x << ", " << a.y << ", " << a.z << ")";
  }

  typedef Vec3<bool > Vec3b;
  typedef Vec3<int  > Vec3i;
  typedef Vec3<float> Vec3f;

// MoonRay: begin *****
  typedef Vec3<double> Vec3d;

  MNRY_STATIC_ASSERT(std::is_trivially_default_constructible<Vec3b>::value);
  MNRY_STATIC_ASSERT(std::is_trivially_default_constructible<Vec3i>::value);
  MNRY_STATIC_ASSERT(std::is_trivially_default_constructible<Vec3f>::value);
  MNRY_STATIC_ASSERT(std::is_trivially_default_constructible<Vec3d>::value);
  MNRY_STATIC_ASSERT(std::is_trivially_copyable<Vec3b>::value);
  MNRY_STATIC_ASSERT(std::is_trivially_copyable<Vec3i>::value);
  MNRY_STATIC_ASSERT(std::is_trivially_copyable<Vec3f>::value);
  MNRY_STATIC_ASSERT(std::is_trivially_copyable<Vec3d>::value);
  MNRY_STATIC_ASSERT(std::is_trivially_destructible<Vec3b>::value);
  MNRY_STATIC_ASSERT(std::is_trivially_destructible<Vec3i>::value);
  MNRY_STATIC_ASSERT(std::is_trivially_destructible<Vec3f>::value);
  MNRY_STATIC_ASSERT(std::is_trivially_destructible<Vec3d>::value);

  ////////////////////////////////////////////////////////////////////////////////
  /// asIspc() and asCpp() C++ <--> ISPC Type-casting functions
  ////////////////////////////////////////////////////////////////////////////////
  HUD_AS_ISPC_FUNCTIONS(Vec3f);
  HUD_AS_CPP_FUNCTIONS(Vec3f);

} // namespace math
} // namespace scene_rdl2

#include "Vec3ba.h"
#include "Vec3ia.h"
#include "Vec3fa.h"

namespace scene_rdl2 {
namespace math {

  typedef simd::ssef ssef;

  template<> __forceinline Vec3<float>::Vec3( const Vec3fa& a ) { x = a.x; y = a.y; z = a.z; }

#if defined (__SSE__)
  template<> __forceinline Vec3<ssef>::Vec3( const Vec3fa& a ) { 
    const ssef v = ssef(a); x = simd::shuffle<0,0,0,0>(v); y = simd::shuffle<1,1,1,1>(v); z = simd::shuffle<2,2,2,2>(v);
  }
  __forceinline Vec3<ssef> broadcast4f( const Vec3<ssef>& a, const size_t k ) {  
    return Vec3<ssef>(ssef::broadcast(&a.x[k]), ssef::broadcast(&a.y[k]), ssef::broadcast(&a.z[k]));
  }
#endif
// MoonRay: end *****
// Intel: begin *****
/*
}

#if defined(__MIC__)
#include "vec3ba_mic.h"
#include "vec3ia_mic.h"
#include "vec3fa_mic.h"
#else
#include "vec3ba.h" 
#include "vec3ia.h" 
#include "vec3fa.h" 
#endif

namespace embree 
{ 
  template<> __forceinline Vec3<float>::Vec3( const Vec3fa& a ) { x = a.x; y = a.y; z = a.z; }

#if defined (__SSE__)
  template<> __forceinline Vec3<ssef>::Vec3( const Vec3fa& a ) { 
    const ssef v = ssef(a); x = shuffle<0,0,0,0>(v); y = shuffle<1,1,1,1>(v); z = shuffle<2,2,2,2>(v); 
  }
  __forceinline Vec3<ssef> broadcast4f( const Vec3<ssef>& a, const size_t k ) {  
    return Vec3<ssef>(ssef::broadcast(&a.x[k]), ssef::broadcast(&a.y[k]), ssef::broadcast(&a.z[k]));
  }
#endif
*/
// Intel: end *****

#if defined(__AVX__)
  // MoonRay: added
  typedef simd::avxf avxf;

  template<> __forceinline Vec3<avxf>::Vec3( const Vec3fa& a ) {  
    x = a.x; y = a.y; z = a.z; 
  }
  __forceinline Vec3<ssef> broadcast4f( const Vec3<avxf>& a, const size_t k ) {  
    return Vec3<ssef>(ssef::broadcast(&a.x[k]), ssef::broadcast(&a.y[k]), ssef::broadcast(&a.z[k]));
  }
  __forceinline Vec3<avxf> broadcast8f( const Vec3<ssef>& a, const size_t k ) {  
    return Vec3<avxf>(avxf::broadcast(&a.x[k]), avxf::broadcast(&a.y[k]), avxf::broadcast(&a.z[k]));
  }
  __forceinline Vec3<avxf> broadcast8f( const Vec3<avxf>& a, const size_t k ) {  
    return Vec3<avxf>(avxf::broadcast(&a.x[k]), avxf::broadcast(&a.y[k]), avxf::broadcast(&a.z[k]));
  }
#endif

#if defined(__MIC__)
  template<> __forceinline Vec3<ssef>::Vec3( const Vec3fa& a ) : x(a.x), y(a.y), z(a.z) {}
  template<> __forceinline Vec3<mic_f>::Vec3( const Vec3fa& a ) : x(a.x), y(a.y), z(a.z) {}
#endif
}
}

