// Copyright 2023-2024 DreamWorks Animation LLC and Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "sse.h"
#include "Math.h"
// Intel: #include "simd/sse.h"
// Intel: #include "math.h"

namespace scene_rdl2 {
namespace math {
// Intel: namespace embree {

  ////////////////////////////////////////////////////////////////////////////////
  /// SSE Vec3ba Type
  ////////////////////////////////////////////////////////////////////////////////

  struct __align(16) Vec3ba // Intel: __aligned(16)
  {
    union {
      __m128 m128;
      struct { int x,y,z; int a; };
    };

    typedef int Scalar;
    enum { N = 3 };

    ////////////////////////////////////////////////////////////////////////////////
    /// Constructors, Assignment & Cast Operators
    ////////////////////////////////////////////////////////////////////////////////

    __forceinline Vec3ba( ) {}
    __forceinline Vec3ba( const __m128  input ) : m128(input) {}
    __forceinline Vec3ba( const Vec3ba& other ) : m128(other.m128) {}
    __forceinline Vec3ba& operator =(const Vec3ba& other) { m128 = other.m128; return *this; }

    // MoonRay: added simd namespace
    // MoonRay: added underscores to fix compile warning
    __forceinline explicit Vec3ba           ( bool  _a )
      : m128(simd::_mm_lookupmask_ps[(size_t(_a) << 3) | (size_t(_a) << 2) | (size_t(_a) << 1) | size_t(_a)]) {}
    __forceinline explicit Vec3ba           ( bool  _a, bool  b, bool  c)
      : m128(simd::_mm_lookupmask_ps[(size_t(c) << 2) | (size_t(b) << 1) | size_t(_a)]) {}

    __forceinline operator const __m128&( void ) const { return m128; }
    __forceinline operator       __m128&( void )       { return m128; }

    ////////////////////////////////////////////////////////////////////////////////
    /// Constants
    ////////////////////////////////////////////////////////////////////////////////

    // MoonRay: added util:: namespace
    __forceinline Vec3ba( util::FalseTy ) : m128(_mm_setzero_ps()) {}
    __forceinline Vec3ba( util::TrueTy  ) : m128(_mm_castsi128_ps(_mm_cmpeq_epi32(_mm_setzero_si128(), _mm_setzero_si128()))) {}

    ////////////////////////////////////////////////////////////////////////////////
    /// Array Access
    ////////////////////////////////////////////////////////////////////////////////

    __forceinline const int& operator []( const size_t index ) const { assert(index < 3); return (&x)[index]; }
    __forceinline       int& operator []( const size_t index )       { assert(index < 3); return (&x)[index]; }
  };


  ////////////////////////////////////////////////////////////////////////////////
  /// Unary Operators
  ////////////////////////////////////////////////////////////////////////////////

  // MoonRay: added util:: namespace
  __forceinline const Vec3ba operator !( const Vec3ba& a ) { return _mm_xor_ps(a.m128, Vec3ba(util::True)); }

  ////////////////////////////////////////////////////////////////////////////////
  /// Binary Operators
  ////////////////////////////////////////////////////////////////////////////////

  __forceinline const Vec3ba operator &( const Vec3ba& a, const Vec3ba& b ) { return _mm_and_ps(a.m128, b.m128); }
  __forceinline const Vec3ba operator |( const Vec3ba& a, const Vec3ba& b ) { return _mm_or_ps (a.m128, b.m128); }
  __forceinline const Vec3ba operator ^( const Vec3ba& a, const Vec3ba& b ) { return _mm_xor_ps(a.m128, b.m128); }
  
  ////////////////////////////////////////////////////////////////////////////////
  /// Assignment Operators
  ////////////////////////////////////////////////////////////////////////////////
  
  __forceinline const Vec3ba operator &=( Vec3ba& a, const Vec3ba& b ) { return a = a & b; }
  __forceinline const Vec3ba operator |=( Vec3ba& a, const Vec3ba& b ) { return a = a | b; }
  __forceinline const Vec3ba operator ^=( Vec3ba& a, const Vec3ba& b ) { return a = a ^ b; }
  
  ////////////////////////////////////////////////////////////////////////////////
  /// Comparison Operators + Select
  ////////////////////////////////////////////////////////////////////////////////
  
  __forceinline bool operator ==( const Vec3ba& a, const Vec3ba& b ) { 
    return (_mm_movemask_ps(_mm_castsi128_ps(_mm_cmpeq_epi32(_mm_castps_si128(a.m128), _mm_castps_si128(b.m128)))) & 7) == 7; 
  }
  __forceinline bool operator !=( const Vec3ba& a, const Vec3ba& b ) { 
    return (_mm_movemask_ps(_mm_castsi128_ps(_mm_cmpeq_epi32(_mm_castps_si128(a.m128), _mm_castps_si128(b.m128)))) & 7) != 7; 
  }
  __forceinline bool operator < ( const Vec3ba& a, const Vec3ba& b ) {
    if (a.x != b.x) return a.x < b.x;
    if (a.y != b.y) return a.y < b.y;
    if (a.z != b.z) return a.z < b.z;
    return false;
  }

  ////////////////////////////////////////////////////////////////////////////////
  /// Output Operators
  ////////////////////////////////////////////////////////////////////////////////

  inline std::ostream& operator<<(std::ostream& cout, const Vec3ba& a) {
    return cout << "(" << (a.x ? "1" : "0") << ", " << (a.y ? "1" : "0") << ", " << (a.z ? "1" : "0") << ")";
  }
}
}

