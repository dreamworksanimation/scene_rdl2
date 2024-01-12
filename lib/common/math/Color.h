// Copyright 2023-2024 DreamWorks Animation LLC and Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

// Include this before any other includes!
#include <scene_rdl2/common/platform/HybridUniformData.h>
#include <scene_rdl2/common/platform/Platform.h>

#include "Col3.h"
#include "Col4.h"
#include "Math.h"


// Forward declaration of the ISPC types
namespace ispc {
    struct Col3f;
    typedef Col3f Color;
}


namespace scene_rdl2 {
namespace math {
  ////////////////////////////////////////////////////////////////////////////////
  /// RGBA Color Class
  ////////////////////////////////////////////////////////////////////////////////
  
  // Note - If you edit this class, make sure to also bring the changes over to
  //        Color_sse.h also.

  struct Color4
  {
    float r, g, b, a;

    typedef float Scalar;

    ////////////////////////////////////////////////////////////////////////////////
    /// Construction
    ////////////////////////////////////////////////////////////////////////////////

    __forceinline Color4 () {}

    __forceinline explicit Color4 (const float& v) : r(v), g(v), b(v), a(v) {}
    __forceinline          Color4 (const float& rParam, const float& gParam, const float& bParam, const float& aParam) : r(rParam), g(gParam), b(bParam), a(aParam) {}

    __forceinline explicit Color4 ( const Col3c& other ) { r = other.r*sOneOver255; g = other.g*sOneOver255; b = other.b*sOneOver255; a = 1.0f; }
    __forceinline explicit Color4 ( const Col3f& other ) { r = other.r; g = other.g; b = other.b; a = 1.0f; }
    __forceinline explicit Color4 ( const Col4c& other ) { r = other.r*sOneOver255; g = other.g*sOneOver255; b = other.b*sOneOver255; a = other.a*sOneOver255; }
    __forceinline explicit Color4 ( const Col4f& other ) { r = other.r; g = other.g; b = other.b; a = other.a; }
    
    __forceinline Color4           ( const Color4& other ) { r = other.r; g = other.g; b = other.b; a = other.a; }
    __forceinline Color4& operator=( const Color4& other ) { r = other.r; g = other.g; b = other.b; a = other.a; return *this; }

    ////////////////////////////////////////////////////////////////////////////////
    /// Set
    ////////////////////////////////////////////////////////////////////////////////

    __forceinline void set(Col3f& d) const { d.r = r; d.g = g; d.b = b; }
    __forceinline void set(Col4f& d) const { d.r = r; d.g = g; d.b = b; d.a = a; }
    __forceinline void set(Col3c& d) const { d.r = char(clamp(r)*255.0f); d.g = char(clamp(g)*255.0f); d.b = char(clamp(b)*255.0f); }
    __forceinline void set(Col4c& d) const { d.r = char(clamp(r)*255.0f); d.g = char(clamp(g)*255.0f); d.b = char(clamp(b)*255.0f); d.a = char(clamp(a)*255.0f); }

    ////////////////////////////////////////////////////////////////////////////////
    /// Constants
    ////////////////////////////////////////////////////////////////////////////////

    __forceinline Color4 (ZeroTy)   : r(zero)   , g(zero)   , b(zero)   , a(zero)    {}
    __forceinline Color4 (OneTy)    : r(one)    , g(one)    , b(one)    , a(one)     {}
    __forceinline Color4 (PosInfTy) : r(pos_inf), g(pos_inf), b(pos_inf), a(pos_inf) {}
    __forceinline Color4 (NegInfTy) : r(neg_inf), g(neg_inf), b(neg_inf), a(neg_inf) {}

    __forceinline const float & operator []( const size_t idx ) const { MNRY_ASSERT(idx < 4); return (&r)[idx]; }
    __forceinline       float & operator []( const size_t idx )       { MNRY_ASSERT(idx < 4); return (&r)[idx]; }
  };

  ////////////////////////////////////////////////////////////////////////////////
  /// Binary Operators
  ////////////////////////////////////////////////////////////////////////////////

  __forceinline const Color4 operator+(const Color4& a, const Color4& b) { return Color4(a.r + b.r, a.g + b.g, a.b + b.b, a.a + b.a); }
  __forceinline const Color4 operator-(const Color4& a, const Color4& b) { return Color4(a.r - b.r, a.g - b.g, a.b - b.b, a.a - b.a); }
  __forceinline const Color4 operator*(const float&  a, const Color4& b) { return Color4(a * b.r, a * b.g, a * b.b, a * b.a); }
  __forceinline const Color4 operator*(const Color4& a, const float&  b) { return Color4(a.r * b, a.g * b, a.b * b, a.a * b); }
  __forceinline const Color4 operator*(const Color4& a, const Color4& b) { return Color4(a.r * b.r, a.g * b.g, a.b * b.b, a.a * b.a); }

  ////////////////////////////////////////////////////////////////////////////////
  /// Assignment Operators
  ////////////////////////////////////////////////////////////////////////////////

  __forceinline const Color4 operator+=(Color4& a, const Color4& b) { return a = a + b; }
  __forceinline const Color4 operator-=(Color4& a, const Color4& b) { return a = a - b; }
  __forceinline const Color4 operator*=(Color4& a, const Color4& b) { return a = a * b; }
  __forceinline const Color4 operator*=(Color4& a, const float b  ) { return a = a * b; }

  ////////////////////////////////////////////////////////////////////////////////
  /// Comparison Operators
  ////////////////////////////////////////////////////////////////////////////////

  __forceinline bool
  isEqual(const Color4 & a, const Color4 & b, const float eps = float(epsilon))
  {
      return isEqual(a.r, b.r, eps) &&
             isEqual(a.g, b.g, eps) &&
             isEqual(a.b, b.b, eps) &&
             isEqual(a.a, b.a, eps);
  }


  ////////////////////////////////////////////////////////////////////////////////
  /// RGB Color Class
  ////////////////////////////////////////////////////////////////////////////////

  // Note - If you edit this class, make sure to also bring the changes over to
  //        Color_sse.h also.

  struct Color
  {
    float r, g, b;

    typedef float Scalar;

    ////////////////////////////////////////////////////////////////////////////////
    /// Construction
    ////////////////////////////////////////////////////////////////////////////////

    __forceinline Color () {}

    __forceinline explicit Color (const float& v) : r(v), g(v), b(v) {}
    __forceinline          Color (const float& rParam, const float& gParam, const float& bParam) : r(rParam), g(gParam), b(bParam) {}

    __forceinline Color           ( const Color& other ) { r = other.r; g = other.g; b = other.b; }
    __forceinline Color& operator=( const Color& other ) { r = other.r; g = other.g; b = other.b; return *this; }

    __forceinline Color           ( const Color4& other ) { r = other.r; g = other.g; b = other.b; }
    __forceinline Color& operator=( const Color4& other ) { r = other.r; g = other.g; b = other.b; return *this; }

    __forceinline operator const Col3f&( void ) const { return *reinterpret_cast<const Col3f *>(this); }
    __forceinline operator       Col3f&( void )       { return *reinterpret_cast<Col3f *>(this); }

    ////////////////////////////////////////////////////////////////////////////////
    /// Set
    ////////////////////////////////////////////////////////////////////////////////

    __forceinline void set(Col3f& d) const { d.r = r; d.g = g; d.b = b; }
    __forceinline void set(Col4f& d) const { d.r = r; d.g = g; d.b = b; d.a = 1.0f; }
    __forceinline void set(Col3c& d) const { d.r = char(clamp(r)*255.0f); d.g = char(clamp(g)*255.0f); d.b = char(clamp(b)*255.0f); }
    __forceinline void set(Col4c& d) const { d.r = char(clamp(r)*255.0f); d.g = char(clamp(g)*255.0f); d.b = char(clamp(b)*255.0f); d.a = 1.0f; }

    ////////////////////////////////////////////////////////////////////////////////
    /// Constants
    ////////////////////////////////////////////////////////////////////////////////

    __forceinline Color (ZeroTy)   : r(zero)   , g(zero)   , b(zero)    {}
    __forceinline Color (OneTy)    : r(one)    , g(one)    , b(one)     {}
    __forceinline Color (PosInfTy) : r(pos_inf), g(pos_inf), b(pos_inf) {}
    __forceinline Color (NegInfTy) : r(neg_inf), g(neg_inf), b(neg_inf) {}

    __forceinline const float & operator []( const size_t idx ) const { MNRY_ASSERT(idx < 3); return (&r)[idx]; }
    __forceinline       float & operator []( const size_t idx )       { MNRY_ASSERT(idx < 3); return (&r)[idx]; }
  };

  ////////////////////////////////////////////////////////////////////////////////
  /// Unary Operators
  ////////////////////////////////////////////////////////////////////////////////

  __forceinline const Color operator+ (const Color& v) { return Color(+v.r,+v.g,+v.b); }
  __forceinline const Color operator- (const Color& v) { return Color(-v.r,-v.g,-v.b); }
  __forceinline const Color abs       (const Color& a) { return Color(abs(a.r),abs(a.g),abs(a.b)); }
  __forceinline const Color rcp       (const Color& a) { return Color(rcp(a.r),rcp(a.g),rcp(a.b)); }
  __forceinline const Color rsqrt     (const Color& a) { return Color(rsqrt(a.r),rsqrt(a.g),rsqrt(a.b)); }
  __forceinline const Color sqrt      (const Color& a) { return Color(sqrt(a.r),sqrt(a.g),sqrt(a.b)); }
  __forceinline       bool  isFinite  (const Color& a) { return math::isfinite(a.r) && math::isfinite(a.g) && math::isfinite(a.b); }

  ////////////////////////////////////////////////////////////////////////////////
  /// Binary Operators
  ////////////////////////////////////////////////////////////////////////////////

  __forceinline const Color operator+(const Color& a, const Color& b) { return Color(a.r+b.r,a.g+b.g,a.b+b.b); }
  __forceinline const Color operator+(const Color& a, const float& b) { return Color(a.r+b,a.g+b,a.b+b); }
  __forceinline const Color operator+(const float& a, const Color& b) { return Color(a+b.r,a+b.g,a+b.b); }
  __forceinline const Color operator-(const Color& a, const Color& b) { return Color(a.r-b.r,a.g-b.g,a.b-b.b); }
  __forceinline const Color operator-(const Color& a, const float& b) { return Color(a.r-b,a.g-b,a.b-b); }
  __forceinline const Color operator-(const float& a, const Color& b) { return Color(a-b.r,a-b.g,a-b.b); }
  __forceinline const Color operator*(const float& a, const Color& b) { return Color(a*b.r,a*b.g,a*b.b); }
  __forceinline const Color operator*(const Color& a, const float& b) { return Color(a.r*b,a.g*b,a.b*b); }
  __forceinline const Color operator*(const Color& a, const Color& b) { return Color(a.r*b.r,a.g*b.g,a.b*b.b); }
  __forceinline const Color operator/(const Color& a, const Color& b) { return Color(a.r/b.r,a.g/b.g,a.b/b.b); }
  __forceinline const Color operator/(const Color& a, const float& b) { return Color(a.r/b,a.g/b,a.b/b); }
  __forceinline const Color operator/(const float& a, const Color& b) { return Color(a/b.r,a/b.g,a/b.b); }

  __forceinline const Color min(const Color& a, const Color& b) { return Color(min(a.r,b.r),min(a.g,b.g),min(a.b,b.b)); }
  __forceinline const Color max(const Color& a, const Color& b) { return Color(max(a.r,b.r),max(a.g,b.g),max(a.b,b.b)); }

  ////////////////////////////////////////////////////////////////////////////////
  /// Assignment Operators
  ////////////////////////////////////////////////////////////////////////////////

  __forceinline const Color operator+=(Color& a, const Color& b) { return a = a + b; }
  __forceinline const Color operator-=(Color& a, const Color& b) { return a = a - b; }
  __forceinline const Color operator*=(Color& a, const Color& b) { return a = a * b; }
  __forceinline const Color operator/=(Color& a, const Color& b) { return a = a / b; }
  __forceinline const Color operator*=(Color& a, const float b      ) { return a = a * b; }
  __forceinline const Color operator/=(Color& a, const float b      ) { return a = a / b; }

  ////////////////////////////////////////////////////////////////////////////////
  /// Reductions
  ////////////////////////////////////////////////////////////////////////////////

  __forceinline float reduce_add(const Color& v) { return v.r+v.g+v.b; }
  __forceinline float reduce_mul(const Color& v) { return v.r*v.g*v.b; }
  __forceinline float reduce_min(const Color& v) { return min(v.r,v.g,v.b); }
  __forceinline float reduce_max(const Color& v) { return max(v.r,v.g,v.b); }
  __forceinline float reduce_avg(const Color& a) { return (a.r + a.g + a.b) / 3.0f; }

  ////////////////////////////////////////////////////////////////////////////////
  /// Comparison Operators
  ////////////////////////////////////////////////////////////////////////////////

  __forceinline bool operator ==(const Color& a, const Color& b) { return a.r == b.r && a.g == b.g && a.b == b.b; }
  __forceinline bool operator !=(const Color& a, const Color& b) { return a.r != b.r || a.g != b.g || a.b != b.b; }

  __forceinline bool
  operator<(const Color & a, const Color & b)
  {
      const float aa = a.r * a.r + a.g * a.g + a.b * a.b;
      const float bb = b.r * b.r + b.g * b.g + b.b * b.b;
      return aa < bb;
  }

  __forceinline bool
  isEqual(const Color & a, const Color & b, const float eps = float(epsilon))
  {
      return isEqual(a.r, b.r, eps) &&
             isEqual(a.g, b.g, eps) &&
             isEqual(a.b, b.b, eps);
  }

  __forceinline bool
  isEqualFixedEps(const Color & a, const Color & b, const float eps = float(epsilon))
  {
      return isEqualFixedEps(a.r, b.r, eps) &&
             isEqualFixedEps(a.g, b.g, eps) &&
             isEqualFixedEps(a.b, b.b, eps);
  }

  // Convenience wrappers
  __forceinline bool isBlack(const Color &c)        {  return isEqual(c, Color(zero));  }
  __forceinline bool isExactlyZero(const Color &c)  {  return isEqual(c, Color(zero), 0.0f);  }

  ////////////////////////////////////////////////////////////////////////////////
  /// Select
  ////////////////////////////////////////////////////////////////////////////////

  __forceinline Color select ( bool s, const Color& t, const Color& f ){ 
    return Color(select(s,t.r,f.r),select(s,t.g,f.g),select(s,t.b,f.b)); 
  }

  ////////////////////////////////////////////////////////////////////////////////
  /// Special Operators
  ////////////////////////////////////////////////////////////////////////////////

  /*! computes luminance of a color */
  __forceinline float relativeLuminance (const Color& a) { return 0.212671f*a.r + 0.715160f*a.g + 0.072169f*a.b; }
  __forceinline float luminance (const Color& a) { return 0.299f*a.r + 0.587f*a.g + 0.114f*a.b; }

  __forceinline Color exp (const Color& a) { return Color(exp(a.r),exp(a.g),exp(a.b)); }
  __forceinline Color log (const Color& a) { return Color(log(a.r),log(a.g),log(a.b)); }
  __forceinline Color pow (const Color& a, float e) { return exp(log(max(Color(1E-10f),a))*e); }

  /*! output operator */
  inline std::ostream& operator<<(std::ostream& cout, const Color& a) {
    return cout << '(' << a.r << ", " << a.g << ", " << a.b << ')';
  }
  inline std::ostream& operator<<(std::ostream& cout, const Color4& a) {
    return cout << '(' << a.r << ", " << a.g << ", " << a.b << ", " << a.a << ')';
  }


  ////////////////////////////////////////////////////////////////////////////////
  /// asIspc() and asCpp() C++ <--> ISPC Type-casting functions
  ////////////////////////////////////////////////////////////////////////////////
  HUD_AS_ISPC_FUNCTIONS(Color);
  HUD_AS_CPP_FUNCTIONS(Color);


  // Convenience constants
  static const Color sBlack = Color(math::zero);
  static const Color sWhite = Color(math::one);

}
}

