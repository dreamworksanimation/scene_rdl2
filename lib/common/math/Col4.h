// Copyright 2023 DreamWorks Animation LLC and Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <scene_rdl2/common/platform/Platform.h>
#include "Math.h"
// Intel: #include "sys/platform.h"
// Intel: "math/math.h"

namespace scene_rdl2 {
namespace math {
// Intel: namespace embree

  ////////////////////////////////////////////////////////////////////////////////
  /// RGBA Color Class
  ////////////////////////////////////////////////////////////////////////////////

  template<typename T> struct Col4
  {
    T r, g, b, a;

    ////////////////////////////////////////////////////////////////////////////////
    /// Construction
    ////////////////////////////////////////////////////////////////////////////////

    __forceinline Col4           ( )                   { }
    __forceinline Col4           ( const Col4& other ) { r = other.r; g = other.g; b = other.b; a = other.a; }
    __forceinline Col4& operator=( const Col4& other ) { r = other.r; g = other.g; b = other.b; a = other.a; return *this; }

    __forceinline explicit Col4 (const T& v) : r(v), g(v), b(v), a(v) {}
    __forceinline          Col4 (const T& r, const T& g, const T& b, const T& a) : r(r), g(g), b(b), a(a) {}

    ////////////////////////////////////////////////////////////////////////////////
    /// Constants
    ////////////////////////////////////////////////////////////////////////////////

    __forceinline Col4 (ZeroTy)   : r(zero)   , g(zero)   , b(zero)   , a(zero) {}
    __forceinline Col4 (OneTy)    : r(one)    , g(one)    , b(one)    , a(one) {}
    __forceinline Col4 (PosInfTy) : r(pos_inf), g(pos_inf), b(pos_inf), a(pos_inf) {}
    __forceinline Col4 (NegInfTy) : r(neg_inf), g(neg_inf), b(neg_inf), a(neg_inf) {}
  };

  /*! output operator */
  template<typename T> inline std::ostream& operator<<(std::ostream& cout, const Col4<T>& a) {
    return cout << "(" << a.r << ", " << a.g << ", " << a.b << ", " << a.a << ")";
  }

  /*! default template instantiations */
  typedef Col4<unsigned char> Col4c;
  typedef Col4<float        > Col4f;
}
}

