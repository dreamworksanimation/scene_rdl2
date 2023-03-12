// Copyright 2023 DreamWorks Animation LLC and Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

// MoonRay: begin *****
#include <scene_rdl2/common/platform/HybridUniformData.h>
#include <scene_rdl2/common/platform/Platform.h>
#include "Math.h"
#include "Vec3.h"
#include <type_traits>

// Forward declaration of the ISPC types
namespace ispc {
    struct Vec2f;
}
// MoonRay: end *****

// Intel: #include "sys/platform.h"
// Intel: #include "math/math.h"

namespace scene_rdl2 {
namespace math {
// Intel: namespace embree {

  ////////////////////////////////////////////////////////////////////////////////
  /// Generic 2D vector Class
  ////////////////////////////////////////////////////////////////////////////////

  template<typename T> struct Vec2
  {
    T x, y;

    typedef T Scalar;
    enum { N = 2 };

    ////////////////////////////////////////////////////////////////////////////////
    /// Construction
    ////////////////////////////////////////////////////////////////////////////////

    // MoonRay: begin *****
    // using "= default" allows the compiler to assume trivially constructible/copyable
    __forceinline Vec2     ( ) = default;
    __forceinline Vec2     ( const Vec2& other ) = default;
    // MoonRay: end *****
    // Intel: __forceinline Vec2     ( )                  { }
    // Intel: __forceinline Vec2     ( const Vec2& other ) { x = other.x; y = other.y; }

    template<typename T1> __forceinline Vec2( const Vec2<T1>& a ) : x(T(a.x)), y(T(a.y)) {}
    template<typename T1> __forceinline Vec2& operator =( const Vec2<T1>& other ) { x = other.x; y = other.y; return *this; }

    __forceinline explicit Vec2( const T& a             ) : x(a), y(a) {}
    // MoonRay: added underscores to fix compile warning
    __forceinline explicit Vec2( const T& _x, const T& _y ) : x(_x), y(_y) {}
    __forceinline explicit Vec2( const T* const a, const ssize_t stride = 1 ) : x(a[0]), y(a[stride]) {}

    ////////////////////////////////////////////////////////////////////////////////
    /// Constants
    ////////////////////////////////////////////////////////////////////////////////

    __forceinline Vec2( ZeroTy   ) : x(zero), y(zero) {}
    __forceinline Vec2( OneTy    ) : x(one),  y(one) {}
    __forceinline Vec2( PosInfTy ) : x(pos_inf), y(pos_inf) {}
    __forceinline Vec2( NegInfTy ) : x(neg_inf), y(neg_inf) {}

    // MoonRay: use MNRY_ASSERT() instead of assert()
    __forceinline const T& operator []( const size_t axis ) const { MNRY_ASSERT(axis < 2); return (&x)[axis]; }
    __forceinline       T& operator []( const size_t axis )       { MNRY_ASSERT(axis < 2); return (&x)[axis]; }

    // MoonRay: begin *****
    ////////////////////////////////////////////////////////////////////////////////
    /// Convenience functions
    ////////////////////////////////////////////////////////////////////////////////

    __forceinline T length() const;
    __forceinline T lengthSqr() const;
    __forceinline Vec2<T> &normalize();
    __forceinline Vec2<T> & safeNormalize(T eps = T(epsilon));
    // MoonRay: end *****
  };

  ////////////////////////////////////////////////////////////////////////////////
  /// Unary Operators
  ////////////////////////////////////////////////////////////////////////////////

  template<typename T> __forceinline Vec2<T> operator +( const Vec2<T>& a ) { return Vec2<T>(+a.x, +a.y); }
  template<typename T> __forceinline Vec2<T> operator -( const Vec2<T>& a ) { return Vec2<T>(-a.x, -a.y); }
  template<typename T> __forceinline Vec2<T> abs       ( const Vec2<T>& a ) { return Vec2<T>(abs  (a.x), abs  (a.y)); }
  template<typename T> __forceinline Vec2<T> rcp       ( const Vec2<T>& a ) { return Vec2<T>(rcp  (a.x), rcp  (a.y)); }
  template<typename T> __forceinline Vec2<T> rsqrt     ( const Vec2<T>& a ) { return Vec2<T>(rsqrt(a.x), rsqrt(a.y)); }
  template<typename T> __forceinline Vec2<T> sqrt      ( const Vec2<T>& a ) { return Vec2<T>(sqrt (a.x), sqrt (a.y)); }

  // MoonRay: begin *****
  template<typename T> __forceinline bool    isFinite  ( const Vec2<T>& a ) { return math::isfinite(a.x) && math::isfinite(a.y); }
  template<typename T> __forceinline bool    isNormalized( const Vec2<T>& a, float eps ) { return fabsf((lengthSqr(a) - T(1)) - (eps * eps)) < (eps * T(2)); }
  template<typename T> __forceinline bool isNormalized(const Vec2<T>& a) {
      const float l = lengthSqr(a);
      return (l > sNormalizedLengthSqrMin  &&  l < sNormalizedLengthSqrMax);
  }
  // MoonRay: end *****

  ////////////////////////////////////////////////////////////////////////////////
  /// Binary Operators
  ////////////////////////////////////////////////////////////////////////////////

  template<typename T> __forceinline Vec2<T> operator +( const Vec2<T>& a, const Vec2<T>& b ) { return Vec2<T>(a.x + b.x, a.y + b.y); }
  template<typename T> __forceinline Vec2<T> operator -( const Vec2<T>& a, const Vec2<T>& b ) { return Vec2<T>(a.x - b.x, a.y - b.y); }
  template<typename T> __forceinline Vec2<T> operator *( const Vec2<T>& a, const Vec2<T>& b ) { return Vec2<T>(a.x * b.x, a.y * b.y); }
  template<typename T> __forceinline Vec2<T> operator *( const       T& a, const Vec2<T>& b ) { return Vec2<T>(a   * b.x, a   * b.y); }
  template<typename T> __forceinline Vec2<T> operator *( const Vec2<T>& a, const       T& b ) { return Vec2<T>(a.x * b  , a.y * b  ); }
  template<typename T> __forceinline Vec2<T> operator /( const Vec2<T>& a, const Vec2<T>& b ) { return Vec2<T>(a.x / b.x, a.y / b.y); }
  template<typename T> __forceinline Vec2<T> operator /( const Vec2<T>& a, const       T& b ) { return Vec2<T>(a.x / b  , a.y / b  ); }
  template<typename T> __forceinline Vec2<T> operator /( const       T& a, const Vec2<T>& b ) { return Vec2<T>(a   / b.x, a   / b.y); }

  template<typename T> __forceinline Vec2<T> min(const Vec2<T>& a, const Vec2<T>& b) { return Vec2<T>(min(a.x, b.x), min(a.y, b.y)); }
  template<typename T> __forceinline Vec2<T> max(const Vec2<T>& a, const Vec2<T>& b) { return Vec2<T>(max(a.x, b.x), max(a.y, b.y)); }

  ////////////////////////////////////////////////////////////////////////////////
  /// Assignment Operators
  ////////////////////////////////////////////////////////////////////////////////

  template<typename T> __forceinline Vec2<T>& operator +=( Vec2<T>& a, const Vec2<T>& b ) { a.x += b.x; a.y += b.y; return a; }
  template<typename T> __forceinline Vec2<T>& operator -=( Vec2<T>& a, const Vec2<T>& b ) { a.x -= b.x; a.y -= b.y; return a; }
  template<typename T> __forceinline Vec2<T>& operator *=( Vec2<T>& a, const       T& b ) { a.x *= b  ; a.y *= b  ; return a; }
  template<typename T> __forceinline Vec2<T>& operator /=( Vec2<T>& a, const       T& b ) { a.x /= b  ; a.y /= b  ; return a; }

  ////////////////////////////////////////////////////////////////////////////////
  /// Reduction Operators
  ////////////////////////////////////////////////////////////////////////////////

  template<typename T> __forceinline T reduce_add( const Vec2<T>& a ) { return a.x + a.y; }
  template<typename T> __forceinline T reduce_mul( const Vec2<T>& a ) { return a.x * a.y; }
  template<typename T> __forceinline T reduce_min( const Vec2<T>& a ) { return min(a.x, a.y); }
  template<typename T> __forceinline T reduce_max( const Vec2<T>& a ) { return max(a.x, a.y); }

  ////////////////////////////////////////////////////////////////////////////////
  /// Comparison Operators
  ////////////////////////////////////////////////////////////////////////////////

  template<typename T> __forceinline bool operator ==( const Vec2<T>& a, const Vec2<T>& b ) { return a.x == b.x && a.y == b.y; }
  template<typename T> __forceinline bool operator !=( const Vec2<T>& a, const Vec2<T>& b ) { return a.x != b.x || a.y != b.y; }
  template<typename T> __forceinline bool operator < ( const Vec2<T>& a, const Vec2<T>& b ) {
    if (a.x != b.x) return a.x < b.x;
    if (a.y != b.y) return a.y < b.y;
    return false;
  }

  // MoonRay: begin *****
  template<typename T> __forceinline bool isEqual(const Vec2<T>& a, const Vec2<T>& b, T eps = T(epsilon))        { return isEqual(a.x, b.x, eps) && isEqual(a.y, b.y, eps); }
  template<typename T> __forceinline bool isEqualFixedEps(const Vec2<T>& a, const Vec2<T>& b, T eps = T(epsilon))        { return isEqualFixedEps(a.x, b.x, eps) && isEqualFixedEps(a.y, b.y, eps); }
  // MoonRay: end *****

  ////////////////////////////////////////////////////////////////////////////////
  /// Euclidian Space Operators
  ////////////////////////////////////////////////////////////////////////////////

  template<typename T> __forceinline T       dot      ( const Vec2<T>& a, const Vec2<T>& b ) { return a.x*b.x + a.y*b.y; }
  template<typename T> __forceinline T       length   ( const Vec2<T>& a )                   { return sqrt(dot(a,a)); }
  template<typename T> __forceinline Vec2<T> normalize( const Vec2<T>& a )                   { return a*rsqrt(dot(a,a)); }
  template<typename T> __forceinline T       distance ( const Vec2<T>& a, const Vec2<T>& b ) { return length(a-b); }

  // MoonRay: begin *****
  template<typename T> __forceinline T       lengthSqr( const Vec2<T>& a )                   { return dot(a,a); }

  template <typename T> __forceinline Vec2<T>
  safeNormalize(const Vec2<T> & a, const T eps = T(epsilon))
  {
      const T lengthSqr = dot(a, a);
      return (lengthSqr <= eps * eps) ? Vec2<T>(T(0)) : a * rsqrt(lengthSqr);
  }
  // MoonRay: end *****

  ////////////////////////////////////////////////////////////////////////////////
  /// Select
  ////////////////////////////////////////////////////////////////////////////////

  template<typename T> __forceinline Vec2<T> select ( bool s, const Vec2<T>& t, const Vec2<T>& f ) {
    return Vec2<T>(select(s,t.x,f.x),select(s,t.y,f.y));
  }

  template<typename T> __forceinline Vec2<T> select ( const typename T::Mask& s, const Vec2<T>& t, const Vec2<T>& f ) {
    return Vec2<T>(select(s,t.x,f.x),select(s,t.y,f.y));
  }

  ////////////////////////////////////////////////////////////////////////////////
  /// Output Operators
  ////////////////////////////////////////////////////////////////////////////////

  template<typename T> inline std::ostream& operator<<(std::ostream& cout, const Vec2<T>& a) {
    return cout << "(" << a.x << ", " << a.y << ")";
  }

  // MoonRay: begin *****
  ////////////////////////////////////////////////////////////////////////////////
  /// Convenience functions
  ////////////////////////////////////////////////////////////////////////////////

  template<typename T> __forceinline T Vec2<T>::length() const      { return math::length(*this); }
  template<typename T> __forceinline T Vec2<T>::lengthSqr() const   { return math::lengthSqr(*this); }
  template<typename T> __forceinline Vec2<T> &Vec2<T>::normalize()  { return *this = math::normalize(*this); }
  
  template<typename T> __forceinline Vec2<T> & 
  Vec2<T>::safeNormalize(const T eps)
  { return *this = math::safeNormalize(*this, eps); }

  // Helper functions to treat a Vec3 as a Vec2 without having to do a copy.
  template<typename T> __forceinline Vec2<T> &asVec2(Vec3<T> &v)                { return reinterpret_cast<Vec2<T> &>(v); }
  template<typename T> __forceinline const Vec2<T> &asVec2(const Vec3<T> &v)    { return reinterpret_cast<const Vec2<T> &>(v); }
  // MoonRay: end *****

  ////////////////////////////////////////////////////////////////////////////////
  /// Default template instantiations
  ////////////////////////////////////////////////////////////////////////////////

  typedef Vec2<bool > Vec2b;
  typedef Vec2<int  > Vec2i;
  typedef Vec2<float> Vec2f;

  // MoonRay: begin *****
  typedef Vec2<double> Vec2d;

  MNRY_STATIC_ASSERT(std::is_trivially_default_constructible<Vec2b>::value);
  MNRY_STATIC_ASSERT(std::is_trivially_default_constructible<Vec2i>::value);
  MNRY_STATIC_ASSERT(std::is_trivially_default_constructible<Vec2f>::value);
  MNRY_STATIC_ASSERT(std::is_trivially_default_constructible<Vec2d>::value);
  MNRY_STATIC_ASSERT(std::is_trivially_copyable<Vec2b>::value);
  MNRY_STATIC_ASSERT(std::is_trivially_copyable<Vec2i>::value);
  MNRY_STATIC_ASSERT(std::is_trivially_copyable<Vec2f>::value);
  MNRY_STATIC_ASSERT(std::is_trivially_copyable<Vec2d>::value);
  MNRY_STATIC_ASSERT(std::is_trivially_destructible<Vec2b>::value);
  MNRY_STATIC_ASSERT(std::is_trivially_destructible<Vec2i>::value);
  MNRY_STATIC_ASSERT(std::is_trivially_destructible<Vec2f>::value);
  MNRY_STATIC_ASSERT(std::is_trivially_destructible<Vec2d>::value);

  ////////////////////////////////////////////////////////////////////////////////
  /// asIspc() and asCpp() C++ <--> ISPC Type-casting functions
  ////////////////////////////////////////////////////////////////////////////////
  HUD_AS_ISPC_FUNCTIONS(Vec2f);
  HUD_AS_CPP_FUNCTIONS(Vec2f);
  // MoonRay: end *****
}
}

