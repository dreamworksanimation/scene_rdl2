// Copyright 2023 DreamWorks Animation LLC and Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <scene_rdl2/common/platform/Platform.h>
#include "Vec3.h"
// Intel: #include "sys/platform.h"
// Intel: #include "math/vec3.h"

namespace scene_rdl2 {
namespace math {
// Intel: namespace embree {

  ////////////////////////////////////////////////////////////////
  // Quaterion Struct
  ////////////////////////////////////////////////////////////////

  template<typename T>
  struct QuaternionT
  {
    typedef Vec3<T> Vector;

    ////////////////////////////////////////////////////////////////////////////////
    /// Construction
    ////////////////////////////////////////////////////////////////////////////////

    __forceinline QuaternionT           ( void )                     { }
    __forceinline QuaternionT           ( const QuaternionT& other ) { r = other.r; i = other.i; j = other.j; k = other.k; }
    __forceinline QuaternionT& operator=( const QuaternionT& other ) { r = other.r; i = other.i; j = other.j; k = other.k; return *this; }

    // MoonRay: added underscores to fix compile warning
    __forceinline          QuaternionT( const T& _r        ) : r(_r), i(zero), j(zero), k(zero) {}
    __forceinline explicit QuaternionT( const Vec3<T>& v ) : r(zero), i(v.x), j(v.y), k(v.z) {}
    // MoonRay: added underscores to fix compile warning
    __forceinline          QuaternionT( const T& _r, const T& _i, const T& _j, const T& _k ) : r(_r), i(_i), j(_j), k(_k) {}
    // MoonRay: added underscores to fix compile warning
    __forceinline          QuaternionT( const T& _r, const Vec3<T>& v ) : r(_r), i(v.x), j(v.y), k(v.z) {}

    __inline QuaternionT( const Vec3<T>& vx, const Vec3<T>& vy, const Vec3<T>& vz );
    __inline QuaternionT( const T& yaw, const T& pitch, const T& roll );

    ////////////////////////////////////////////////////////////////////////////////
    /// Constants
    ////////////////////////////////////////////////////////////////////////////////

    __forceinline QuaternionT( ZeroTy ) : r(zero), i(zero), j(zero), k(zero) {}
    __forceinline QuaternionT( OneTy  ) : r( one), i(zero), j(zero), k(zero) {}

    /*! return quaternion for rotation around arbitrary axis */
    static __forceinline QuaternionT rotate(const Vec3<T>& u, const T& r) {
      return QuaternionT<T>(cos(T(0.5)*r),sin(T(0.5)*r)*normalize(u));
    }

    /*! returns the rotation axis of the quaternion as a vector */
    __forceinline const Vec3<T> v( ) const { return Vec3<T>(i, j, k); }

  public:
    T r, i, j, k;
  };

  template<typename T> __forceinline QuaternionT<T> operator *( const T             & a, const QuaternionT<T>& b ) { return QuaternionT<T>(a * b.r, a * b.i, a * b.j, a * b.k); }
  template<typename T> __forceinline QuaternionT<T> operator *( const QuaternionT<T>& a, const T             & b ) { return QuaternionT<T>(a.r * b, a.i * b, a.j * b, a.k * b); }

  // MoonRay added
  template<typename T> __forceinline T dot(const QuaternionT<T>& a, const QuaternionT<T>& b) { return a.r*b.r+a.i*b.i+a.j*b.j+a.k*b.k; }

  ////////////////////////////////////////////////////////////////
  // Unary Operators
  ////////////////////////////////////////////////////////////////

  template<typename T> __forceinline QuaternionT<T> operator +( const QuaternionT<T>& a ) { return QuaternionT<T>(+a.r, +a.i, +a.j, +a.k); }
  template<typename T> __forceinline QuaternionT<T> operator -( const QuaternionT<T>& a ) { return QuaternionT<T>(-a.r, -a.i, -a.j, -a.k); }
  template<typename T> __forceinline QuaternionT<T> conj      ( const QuaternionT<T>& a ) { return QuaternionT<T>(a.r, -a.i, -a.j, -a.k); }
  template<typename T> __forceinline T              abs       ( const QuaternionT<T>& a ) { return sqrt(a.r*a.r + a.i*a.i + a.j*a.j + a.k*a.k); }
  template<typename T> __forceinline QuaternionT<T> rcp       ( const QuaternionT<T>& a ) { return conj(a)*rcp(a.r*a.r + a.i*a.i + a.j*a.j + a.k*a.k); }
  template<typename T> __forceinline QuaternionT<T> normalize ( const QuaternionT<T>& a ) { return a*rsqrt(a.r*a.r + a.i*a.i + a.j*a.j + a.k*a.k); }

  ////////////////////////////////////////////////////////////////
  // Binary Operators
  ////////////////////////////////////////////////////////////////

  template<typename T> __forceinline QuaternionT<T> operator +( const T             & a, const QuaternionT<T>& b ) { return QuaternionT<T>(a + b.r,  b.i,  b.j,  b.k); }
  template<typename T> __forceinline QuaternionT<T> operator +( const QuaternionT<T>& a, const T             & b ) { return QuaternionT<T>(a.r + b, a.i, a.j, a.k); }
  template<typename T> __forceinline QuaternionT<T> operator +( const QuaternionT<T>& a, const QuaternionT<T>& b ) { return QuaternionT<T>(a.r + b.r, a.i + b.i, a.j + b.j, a.k + b.k); }
  template<typename T> __forceinline QuaternionT<T> operator -( const T             & a, const QuaternionT<T>& b ) { return QuaternionT<T>(a - b.r, -b.i, -b.j, -b.k); }
  template<typename T> __forceinline QuaternionT<T> operator -( const QuaternionT<T>& a, const T             & b ) { return QuaternionT<T>(a.r - b, a.i, a.j, a.k); }
  template<typename T> __forceinline QuaternionT<T> operator -( const QuaternionT<T>& a, const QuaternionT<T>& b ) { return QuaternionT<T>(a.r - b.r, a.i - b.i, a.j - b.j, a.k - b.k); }

  template<typename T> __forceinline Vec3<T>       operator *( const QuaternionT<T>& a, const Vec3<T>      & b ) { return (a*QuaternionT<T>(b)*conj(a)).v(); }
  template<typename T> __forceinline QuaternionT<T> operator *( const QuaternionT<T>& a, const QuaternionT<T>& b ) {
    return QuaternionT<T>(a.r*b.r - a.i*b.i - a.j*b.j - a.k*b.k,
                          a.r*b.i + a.i*b.r + a.j*b.k - a.k*b.j,
                          a.r*b.j - a.i*b.k + a.j*b.r + a.k*b.i,
                          a.r*b.k + a.i*b.j - a.j*b.i + a.k*b.r);
  }
  template<typename T> __forceinline QuaternionT<T> operator /( const T             & a, const QuaternionT<T>& b ) { return a*rcp(b); }
  template<typename T> __forceinline QuaternionT<T> operator /( const QuaternionT<T>& a, const T             & b ) { return a*rcp(b); }
  template<typename T> __forceinline QuaternionT<T> operator /( const QuaternionT<T>& a, const QuaternionT<T>& b ) { return a*rcp(b); }

  template<typename T> __forceinline QuaternionT<T>& operator +=( QuaternionT<T>& a, const T             & b ) { return a = a+b; }
  template<typename T> __forceinline QuaternionT<T>& operator +=( QuaternionT<T>& a, const QuaternionT<T>& b ) { return a = a+b; }
  template<typename T> __forceinline QuaternionT<T>& operator -=( QuaternionT<T>& a, const T             & b ) { return a = a-b; }
  template<typename T> __forceinline QuaternionT<T>& operator -=( QuaternionT<T>& a, const QuaternionT<T>& b ) { return a = a-b; }
  template<typename T> __forceinline QuaternionT<T>& operator *=( QuaternionT<T>& a, const T             & b ) { return a = a*b; }
  template<typename T> __forceinline QuaternionT<T>& operator *=( QuaternionT<T>& a, const QuaternionT<T>& b ) { return a = a*b; }
  template<typename T> __forceinline QuaternionT<T>& operator /=( QuaternionT<T>& a, const T             & b ) { return a = a*rcp(b); }
  template<typename T> __forceinline QuaternionT<T>& operator /=( QuaternionT<T>& a, const QuaternionT<T>& b ) { return a = a*rcp(b); }

  // MoonRay: begin added *****
  template<typename T> __forceinline Vec3<T> transformPoint ( const QuaternionT<T>& a, const Vec3<T>&       b ) { return (a*QuaternionT<T>(b)*conj(a)).v(); }
  template<typename T> __forceinline Vec3<T> transformVector( const QuaternionT<T>& a, const Vec3<T>&       b ) { return (a*QuaternionT<T>(b)*conj(a)).v(); }
  template<typename T> __forceinline Vec3<T> transformNormal( const QuaternionT<T>& a, const Vec3<T>&       b ) { return (a*QuaternionT<T>(b)*conj(a)).v(); }

  template<typename T> __forceinline QuaternionT<T> slerp(const QuaternionT<T>& a, const QuaternionT<T>& b, const T t) {
    T angle = T(zero);
    T sineAngle = T(zero);
    T cosAngle = dot(a,b);
    if (abs(cosAngle) < T(one)) { // if angle between a & b is neither 0 or 180 degree.
        angle = acos(cosAngle);
        sineAngle = sin(angle);
    }
    if (abs(sineAngle) < T(0.00001f)) {
      return (T(one)-t)*a + static_cast<T>(t)*b;
    }
    T rcpSine = T(one)/sineAngle;
    T rA = sin((T(one)-t)*angle)*rcpSine;
    T rB = sin(t*angle)*rcpSine;
    return a*rA + b*rB;
  }

  template<typename T> __forceinline bool isEqual( const QuaternionT<T>& a, const QuaternionT<T>& b ) {
    return isEqual(a.r, b.r) && isEqual(a.i, b.i) && isEqual(a.j, b.j) && isEqual(a.k, b.k);
  }
  // MoonRay: end added *****

  // Intel:   template<typename T> __forceinline Vec3<T> xfmPoint ( const QuaternionT<T>& a, const Vec3<T>&       b ) { return (a*QuaternionT<T>(b)*conj(a)).v(); }
  // Intel:   template<typename T> __forceinline Vec3<T> xfmVector( const QuaternionT<T>& a, const Vec3<T>&       b ) { return (a*QuaternionT<T>(b)*conj(a)).v(); }
  // Intel:   template<typename T> __forceinline Vec3<T> xfmNormal( const QuaternionT<T>& a, const Vec3<T>&       b ) { return (a*QuaternionT<T>(b)*conj(a)).v(); }

  ////////////////////////////////////////////////////////////////////////////////
  /// Comparison Operators
  ////////////////////////////////////////////////////////////////////////////////

  template<typename T> __forceinline bool operator ==( const QuaternionT<T>& a, const QuaternionT<T>& b ) { return a.r == b.r && a.i == b.i && a.j == b.j && a.k == b.k; }
  template<typename T> __forceinline bool operator !=( const QuaternionT<T>& a, const QuaternionT<T>& b ) { return a.r != b.r || a.i != b.i || a.j != b.j || a.k != b.k; }


  ////////////////////////////////////////////////////////////////////////////////
  /// Orientation Functions
  ////////////////////////////////////////////////////////////////////////////////

  template<typename T> QuaternionT<T>::QuaternionT( const Vec3<T>& vx, const Vec3<T>& vy, const Vec3<T>& vz )
  {
    if ( vx.x + vy.y + vz.z >= T(zero) )
    {
      const T t = T(one) + (vx.x + vy.y + vz.z);
      const T s = rsqrt(t)*T(0.5f);
      r = t*s;
      i = (vy.z - vz.y)*s;
      j = (vz.x - vx.z)*s;
      k = (vx.y - vy.x)*s;
    }
    else if ( vx.x >= max(vy.y, vz.z) )
    {
      const T t = (T(one) + vx.x) - (vy.y + vz.z);
      const T s = rsqrt(t)*T(0.5f);
      r = (vy.z - vz.y)*s;
      i = t*s;
      j = (vx.y + vy.x)*s;
      k = (vz.x + vx.z)*s;
    }
    else if ( vy.y >= vz.z ) // if ( vy.y >= max(vz.z, vx.x) )
    {
      const T t = (T(one) + vy.y) - (vz.z + vx.x);
      const T s = rsqrt(t)*T(0.5f);
      r = (vz.x - vx.z)*s;
      i = (vx.y + vy.x)*s;
      j = t*s;
      k = (vy.z + vz.y)*s;
    }
    else //if ( vz.z >= max(vy.y, vx.x) )
    {
      const T t = (T(one) + vz.z) - (vx.x + vy.y);
      const T s = rsqrt(t)*T(0.5f);
      r = (vx.y - vy.x)*s;
      i = (vz.x + vx.z)*s;
      j = (vy.z + vz.y)*s;
      k = t*s;
    }
  }

  template<typename T> QuaternionT<T>::QuaternionT( const T& yaw, const T& pitch, const T& roll )
  {
    const T cya = cos(yaw  *T(0.5f));
    const T cpi = cos(pitch*T(0.5f));
    const T cro = cos(roll *T(0.5f));
    const T sya = sin(yaw  *T(0.5f));
    const T spi = sin(pitch*T(0.5f));
    const T sro = sin(roll *T(0.5f));
    r = cro*cya*cpi + sro*sya*spi;
    i = cro*cya*spi + sro*sya*cpi;
    j = cro*sya*cpi - sro*cya*spi;
    k = sro*cya*cpi - cro*sya*spi;
  }

  ////////////////////////////////////////////////////////////////////////////////
  /// Output Operators
  ////////////////////////////////////////////////////////////////////////////////

  template<typename T> static std::ostream& operator<<(std::ostream& cout, const QuaternionT<T>& q) {
    return cout << "{ r = " << q.r << ", i = " << q.i << ", j = " << q.j << ", k = " << q.k << " }";
  }

  /*! default template instantiations */
  typedef QuaternionT<float>  Quaternion3f;
  typedef QuaternionT<double> Quaternion3d;
}
}

