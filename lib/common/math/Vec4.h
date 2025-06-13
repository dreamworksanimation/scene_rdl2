// Copyright 2023-2024 DreamWorks Animation LLC and Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

// MoonRay added begin *****
#include <scene_rdl2/common/platform/Platform.h>
#include <scene_rdl2/common/platform/HybridUniformData.h>
#include "Math.h"
#include "Vec2.h"
#include "Vec3.h"
#include <type_traits>

// Forward declaration of the ISPC types
namespace ispc {
    struct Vec4f;
}
// MoonRay added end *****
// Intel: #include "sys/platform.h"

namespace scene_rdl2 {
namespace math {
// Intel: namespace embree {

  ////////////////////////////////////////////////////////////////////////////////
  /// Generic 4D vector Class
  ////////////////////////////////////////////////////////////////////////////////

  template<typename T> struct Vec4
  {
    T x, y, z, w;

    typedef T Scalar;
    enum { N = 4 };

    ////////////////////////////////////////////////////////////////////////////////
    /// Construction
    ////////////////////////////////////////////////////////////////////////////////

// MoonRay begin *****
    // using "= default" allows the compiler to assume trivially constructible/copyable      
    __forceinline Vec4    ( ) = default;
    __forceinline Vec4    ( const Vec4& other ) = default;
    __forceinline Vec4    ( const Vec2<T>& other ) { x = other.x; y = other.y; z = 0; w = 0; }
    __forceinline Vec4    ( const Vec3<T>& other ) { x = other.x; y = other.y; z = other.z; w = 0; }
    __forceinline Vec4    ( const Vec3<T>& other, float inW ) { x = other.x; y = other.y; z = other.z; w = inW; }
    __forceinline Vec4    ( const Vec3fa& other );

    template<typename T1> __forceinline Vec4( const Vec4<T1>& a ) : x(T(a.x)), y(T(a.y)), z(T(a.z)), w(T(a.w)) {}

    __forceinline Vec4& operator =(const Vec4& other) = default;
    template<typename T1> __forceinline Vec4& operator =(const Vec4<T1>& other) { x = other.x; y = other.y; z = other.z; w = other.w; return *this; }

    __forceinline explicit Vec4( const T& a                                     ) : x(a), y(a), z(a), w(a) {}
    __forceinline explicit Vec4( const T& aX, const T& aY, const T& aZ, const T& aW ) : x(aX), y(aY), z(aZ), w(aW) {}
    __forceinline explicit Vec4( const T* const a, const size_t stride = 1     ) : x(a[0]), y(a[stride]), z(a[2*stride]), w(a[3*stride]) {}
// MoonRay end *****
// Intel begin *****
/*
    __forceinline Vec4    ( )                  { }
    __forceinline Vec4    ( const Vec4& other ) { x = other.x; y = other.y; z = other.z; w = other.w; }
    template<typename T1> __forceinline Vec4( const Vec4<T1>& a ) : x(T(a.x)), y(T(a.y)), z(T(a.z)), w(T(a.w)) {}
    template<typename T1> __forceinline Vec4& operator =(const Vec4<T1>& other) { x = other.x; y = other.y; z = other.z; w = other.w; return *this; }

    __forceinline explicit Vec4( const T& a                                     ) : x(a), y(a), z(a), w(a) {}
    __forceinline explicit Vec4( const T& x, const T& y, const T& z, const T& w ) : x(x), y(y), z(z), w(w) {}
    __forceinline explicit Vec4( const T* const a, const size_t stride = 1     ) : x(a[0]), y(a[stride]), z(a[2*stride]), w(a[3*stride]) {}
*/
// Intel end *****

    ////////////////////////////////////////////////////////////////////////////////
    /// Constants
    ////////////////////////////////////////////////////////////////////////////////

    __forceinline Vec4( ZeroTy   ) : x(zero), y(zero), z(zero), w(zero) {}
    __forceinline Vec4( OneTy    ) : x(one),  y(one),  z(one),  w(one) {}
    __forceinline Vec4( PosInfTy ) : x(pos_inf), y(pos_inf), z(pos_inf), w(pos_inf) {}
    __forceinline Vec4( NegInfTy ) : x(neg_inf), y(neg_inf), z(neg_inf), w(neg_inf) {}

// MoonRay begin *****
    __forceinline const T& operator []( const size_t axis ) const { MNRY_ASSERT(axis < 4); return (&x)[axis]; }
    __forceinline       T& operator []( const size_t axis )       { MNRY_ASSERT(axis < 4); return (&x)[axis]; }

    ////////////////////////////////////////////////////////////////////////////////
    /// Convenience functions
    ////////////////////////////////////////////////////////////////////////////////

    __forceinline T length() const;
    __forceinline T lengthSqr() const;
    __forceinline Vec4<T> &normalize();
    __forceinline Vec4<T> & safeNormalize(T eps = T(epsilon));
    __forceinline Vec3<T> toVec3(const uint32 i0 = 0,
                                 const uint32 i1 = 1,
                                 const uint32 i2 = 2) const { return Vec3<T>((&x)[i0], (&x)[i1], (&x)[i2]); }
// MoonRay end *****
// Intel begin *****
/*
    __forceinline const T& operator []( const size_t axis ) const { assert(axis < 4); return (&x)[axis]; }
    __forceinline       T& operator []( const size_t axis )       { assert(axis < 4); return (&x)[axis]; }
*/
// Intel end *****
  };

  ////////////////////////////////////////////////////////////////////////////////
  /// Unary Operators
  ////////////////////////////////////////////////////////////////////////////////

  template<typename T> __forceinline Vec4<T> operator +( const Vec4<T>& a ) { return Vec4<T>(+a.x, +a.y, +a.z, +a.w); }
  template<typename T> __forceinline Vec4<T> operator -( const Vec4<T>& a ) { return Vec4<T>(-a.x, -a.y, -a.z, -a.w); }
  template<typename T> __forceinline Vec4<T> abs       ( const Vec4<T>& a ) { return Vec4<T>(abs  (a.x), abs  (a.y), abs  (a.z), abs  (a.w)); }
  template<typename T> __forceinline Vec4<T> rcp       ( const Vec4<T>& a ) { return Vec4<T>( 1.0f/a.x,   1.0f/a.y,   1.0f/a.z,   1.0f/a.w);  }
  template<typename T> __forceinline Vec4<T> rsqrt     ( const Vec4<T>& a ) { return Vec4<T>(rsqrt(a.x), rsqrt(a.y), rsqrt(a.z), rsqrt(a.w)); }
  template<typename T> __forceinline Vec4<T> sqrt      ( const Vec4<T>& a ) { return Vec4<T>(sqrt (a.x), sqrt (a.y), sqrt (a.z), sqrt (a.w)); }

// MoonRay begin *****
  template<typename T> __forceinline bool    isFinite  ( const Vec4<T>& a ) { return math::isfinite(a.x) && math::isfinite(a.y) && math::isfinite(a.z) && math::isfinite(a.w); }
  template<typename T> __forceinline bool    isNormalized( const Vec4<T>& a, float eps ) { return fabsf((lengthSqr(a) - T(1)) - (eps * eps)) < (eps * T(2)); }
  template<typename T> __forceinline bool isNormalized(const Vec4<T>& a) {
      const float l = lengthSqr(a);
      return (l > sNormalizedLengthSqrMin  &&  l < sNormalizedLengthSqrMax);
  }
// MoonRay end *****

  ////////////////////////////////////////////////////////////////////////////////
  /// Binary Operators
  ////////////////////////////////////////////////////////////////////////////////

  template<typename T> __forceinline Vec4<T> operator +( const Vec4<T>& a, const Vec4<T>& b ) { return Vec4<T>(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w); }
  template<typename T> __forceinline Vec4<T> operator -( const Vec4<T>& a, const Vec4<T>& b ) { return Vec4<T>(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w); }
  template<typename T> __forceinline Vec4<T> operator *( const Vec4<T>& a, const Vec4<T>& b ) { return Vec4<T>(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w); }
  template<typename T> __forceinline Vec4<T> operator *( const       T& a, const Vec4<T>& b ) { return Vec4<T>(a   * b.x, a   * b.y, a   * b.z, a   * b.w); }
  template<typename T> __forceinline Vec4<T> operator *( const Vec4<T>& a, const       T& b ) { return Vec4<T>(a.x * b  , a.y * b  , a.z * b  , a.w * b  ); }
  template<typename T> __forceinline Vec4<T> operator /( const Vec4<T>& a, const Vec4<T>& b ) { return Vec4<T>(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w); }
  template<typename T> __forceinline Vec4<T> operator /( const Vec4<T>& a, const       T& b ) { return Vec4<T>(a.x / b  , a.y / b  , a.z / b  , a.w / b  ); }
  template<typename T> __forceinline Vec4<T> operator /( const       T& a, const Vec4<T>& b ) { return Vec4<T>(a   / b.x, a   / b.y, a   / b.z, a   / b.w); }

  template<typename T> __forceinline Vec4<T> min(const Vec4<T>& a, const Vec4<T>& b) { return Vec4<T>(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z), min(a.w, b.w)); }
  template<typename T> __forceinline Vec4<T> max(const Vec4<T>& a, const Vec4<T>& b) { return Vec4<T>(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z), max(a.w, b.w)); }

  ////////////////////////////////////////////////////////////////////////////////
  /// Assignment Operators
  ////////////////////////////////////////////////////////////////////////////////

  template<typename T> __forceinline Vec4<T>& operator +=( Vec4<T>& a, const Vec4<T>& b ) { a.x += b.x; a.y += b.y; a.z += b.z; a.w += b.w; return a; }
  template<typename T> __forceinline Vec4<T>& operator -=( Vec4<T>& a, const Vec4<T>& b ) { a.x -= b.x; a.y -= b.y; a.z -= b.z; a.w -= b.w; return a; }
  template<typename T> __forceinline Vec4<T>& operator *=( Vec4<T>& a, const       T& b ) { a.x *= b  ; a.y *= b  ; a.z *= b  ; a.w *= b  ; return a; }
  template<typename T> __forceinline Vec4<T>& operator /=( Vec4<T>& a, const       T& b ) { a.x /= b  ; a.y /= b  ; a.z /= b  ; a.w /= b  ; return a; }

  ////////////////////////////////////////////////////////////////////////////////
  /// Reduction Operators
  ////////////////////////////////////////////////////////////////////////////////

  template<typename T> __forceinline T reduce_add( const Vec4<T>& a ) { return a.x + a.y + a.z + a.w; }
  template<typename T> __forceinline T reduce_mul( const Vec4<T>& a ) { return a.x * a.y * a.z * a.w; }
  template<typename T> __forceinline T reduce_min( const Vec4<T>& a ) { return min(a.x, a.y, a.z, a.w); }
  template<typename T> __forceinline T reduce_max( const Vec4<T>& a ) { return max(a.x, a.y, a.z, a.w); }

  ////////////////////////////////////////////////////////////////////////////////
  /// Comparison Operators
  ////////////////////////////////////////////////////////////////////////////////

  template<typename T> __forceinline bool operator ==( const Vec4<T>& a, const Vec4<T>& b ) { return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w; }
  template<typename T> __forceinline bool operator !=( const Vec4<T>& a, const Vec4<T>& b ) { return a.x != b.x || a.y != b.y || a.z != b.z || a.w != b.w; }
  template<typename T> __forceinline bool operator < ( const Vec4<T>& a, const Vec4<T>& b ) {
    if (a.x != b.x) return a.x < b.x;
    if (a.y != b.y) return a.y < b.y;
    if (a.z != b.z) return a.z < b.z;
    if (a.w != b.w) return a.w < b.w;
    return false;
  }

// MoonRay begin *****
  // default eps is taken from gmath
  template<typename T> __forceinline bool isEqual(const Vec4<T>& a, const Vec4<T>& b, T eps = T(epsilon))        { return isEqual(a.x, b.x, eps) && isEqual(a.y, b.y, eps) && isEqual(a.z, b.z, eps) && isEqual(a.w, b.w, eps); }

  template<typename T> __forceinline bool isEqualFixedEps(const Vec4<T>& a, const Vec4<T>& b, T eps = T(epsilon))        { return isEqualFixedEps(a.x, b.x, eps) && isEqualFixedEps(a.y, b.y, eps) && isEqualFixedEps(a.z, b.z, eps) && isEqualFixedEps(a.w, b.w, eps); }
// MoonRay end *****

  ////////////////////////////////////////////////////////////////////////////////
  /// Euclidian Space Operators
  ////////////////////////////////////////////////////////////////////////////////

  template<typename T> __forceinline T       dot      ( const Vec4<T>& a, const Vec4<T>& b ) { return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w; }
  template<typename T> __forceinline T       length   ( const Vec4<T>& a )                   { return sqrt(dot(a,a)); }
  template<typename T> __forceinline Vec4<T> normalize( const Vec4<T>& a )                   { return a*rsqrt(dot(a,a)); }
  template<typename T> __forceinline T       distance ( const Vec4<T>& a, const Vec4<T>& b ) { return length(a-b); }

// MoonRay begin *****
  template<typename T> __forceinline T       lengthSqr( const Vec4<T>& a )                   { return dot(a,a); }

  template <typename T> __forceinline Vec4<T>
  safeNormalize(const Vec4<T> & a, const T eps = T(epsilon)) 
  {
    const T lengthSqr = dot(a, a);
    return (lengthSqr <= eps * eps) ? Vec4<T>(T(0)) : a * rsqrt(lengthSqr);
  }
// MoonRay end *****

  ////////////////////////////////////////////////////////////////////////////////
  /// Select
  ////////////////////////////////////////////////////////////////////////////////

  template<typename T> __forceinline Vec4<T> select ( bool s, const Vec4<T>& t, const Vec4<T>& f ) {
    return Vec4<T>(select(s,t.x,f.x),select(s,t.y,f.y),select(s,t.z,f.z),select(s,t.w,f.w));
  }

  ////////////////////////////////////////////////////////////////////////////////
  /// Output Operators
  ////////////////////////////////////////////////////////////////////////////////

  template<typename T> inline std::ostream& operator<<(std::ostream& cout, const Vec4<T>& a) {
    return cout << "(" << a.x << ", " << a.y << ", " << a.z << ", " << a.w << ")";
  }

// MoonRay begin *****
  ////////////////////////////////////////////////////////////////////////////////
  /// Convenience functions
  ////////////////////////////////////////////////////////////////////////////////

  template<typename T> __forceinline T Vec4<T>::length() const      { return math::length(*this); }
  template<typename T> __forceinline T Vec4<T>::lengthSqr() const   { return math::lengthSqr(*this); }
  template<typename T> __forceinline Vec4<T> &Vec4<T>::normalize()  { return *this = math::normalize(*this); }
  template<typename T> __forceinline Vec4<T> &
  Vec4<T>::safeNormalize(const T eps)
  { return *this = math::safeNormalize(*this, eps); }

  // Helper functions to treat a Vec4 as a Vec3 without having to do a copy.
  template<typename T> __forceinline Vec3<T> &asVec3(Vec4<T> &v)                { return reinterpret_cast<Vec3<T> &>(v); }
  template<typename T> __forceinline const Vec3<T> &asVec3(const Vec4<T> &v)    { return reinterpret_cast<const Vec3<T> &>(v); }
// MoonRay end *****

  ////////////////////////////////////////////////////////////////////////////////
  /// Default template instantiations
  ////////////////////////////////////////////////////////////////////////////////

  typedef Vec4<bool         > Vec4b;
  typedef Vec4<unsigned char> Vec4uc;
  typedef Vec4<int          > Vec4i;
  typedef Vec4<float        > Vec4f;
}
}

// Intel begin *****
/*
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
*/
// Intel end *****

// MoonRay begin *****

#include "Vec3ba.h"
#include "Vec3ia.h"
#include "Vec3fa.h"

namespace scene_rdl2 {
namespace math {

  typedef Vec4<double       > Vec4d;

  MNRY_STATIC_ASSERT(std::is_trivially_default_constructible<Vec4b>::value);
  MNRY_STATIC_ASSERT(std::is_trivially_default_constructible<Vec4i>::value);
  MNRY_STATIC_ASSERT(std::is_trivially_default_constructible<Vec4f>::value);
  MNRY_STATIC_ASSERT(std::is_trivially_default_constructible<Vec4d>::value);
  MNRY_STATIC_ASSERT(std::is_trivially_copyable<Vec4b>::value);
  MNRY_STATIC_ASSERT(std::is_trivially_copyable<Vec4i>::value);
  MNRY_STATIC_ASSERT(std::is_trivially_copyable<Vec4f>::value);
  MNRY_STATIC_ASSERT(std::is_trivially_copyable<Vec4d>::value);
  MNRY_STATIC_ASSERT(std::is_trivially_destructible<Vec4b>::value);
  MNRY_STATIC_ASSERT(std::is_trivially_destructible<Vec4i>::value);
  MNRY_STATIC_ASSERT(std::is_trivially_destructible<Vec4f>::value);
  MNRY_STATIC_ASSERT(std::is_trivially_destructible<Vec4d>::value);

  ////////////////////////////////////////////////////////////////////////////////
  /// asIspc() and asCpp() C++ <--> ISPC Type-casting functions
  ////////////////////////////////////////////////////////////////////////////////
  HUD_AS_ISPC_FUNCTIONS(Vec4f);
  HUD_AS_CPP_FUNCTIONS(Vec4f);

  /// down-cast a Vec4d to a Vec4f
  __forceinline Vec4f toFloat(const Vec4d &v) {
      return Vec4f(static_cast<float>(v.x), static_cast<float>(v.y),
                   static_cast<float>(v.z), static_cast<float>(v.w));
  }

  /// up-cast a Vec4f to a Vec4d
  __forceinline Vec4d toDouble(const Vec4f &v) {
      return Vec4d(static_cast<double>(v.x), static_cast<double>(v.y),
                   static_cast<double>(v.z), static_cast<double>(v.w));
  }

  using simd::shuffle;
// MoonRay end *****
  template<> __forceinline Vec4<float>::Vec4( const Vec3fa& a ) { x = a.x; y = a.y; z = a.z; w = a.w; }

#if defined (__SSE__)
  template<> __forceinline Vec4<ssef>::Vec4( const Vec3fa& a ) { 
    const ssef v = ssef(a); x = shuffle<0,0,0,0>(v); y = shuffle<1,1,1,1>(v); z = shuffle<2,2,2,2>(v); w = shuffle<3,3,3,3>(v); 
  }
  __forceinline Vec4<ssef> broadcast4f( const Vec4<ssef>& a, const size_t k ) {  
    return Vec4<ssef>(ssef::broadcast(&a.x[k]), ssef::broadcast(&a.y[k]), ssef::broadcast(&a.z[k]), ssef::broadcast(&a.w[k]));
  }
#endif

#if defined(__AVX__)
  typedef simd::avxf avxf; // MoonRay added
  template<> __forceinline Vec4<avxf>::Vec4( const Vec3fa& a ) {  
    x = a.x; y = a.y; z = a.z; w = a.w; 
  }
  __forceinline Vec4<ssef> broadcast4f( const Vec4<avxf>& a, const size_t k ) {  
    return Vec4<ssef>(ssef::broadcast(&a.x[k]), ssef::broadcast(&a.y[k]), ssef::broadcast(&a.z[k]), ssef::broadcast(&a.w[k]));
  }
  __forceinline Vec4<avxf> broadcast8f( const Vec4<ssef>& a, const size_t k ) {  
    return Vec4<avxf>(avxf::broadcast(&a.x[k]), avxf::broadcast(&a.y[k]), avxf::broadcast(&a.z[k]), avxf::broadcast(&a.w[k]));
  }
  __forceinline Vec4<avxf> broadcast8f( const Vec4<avxf>& a, const size_t k ) {  
    return Vec4<avxf>(avxf::broadcast(&a.x[k]), avxf::broadcast(&a.y[k]), avxf::broadcast(&a.z[k]), avxf::broadcast(&a.w[k]));
  }
#endif

#if defined(__MIC__)
  template<> __forceinline Vec4<ssef>::Vec4( const Vec3fa& a ) : x(a.x), y(a.y), z(a.z), w(a.w) {}
  template<> __forceinline Vec4<mic_f>::Vec4( const Vec3fa& a ) : x(a.x), y(a.y), z(a.z), w(a.w) {}
#endif
}
}

