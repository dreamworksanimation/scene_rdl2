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
  /// SSE Vec3ia Type
  ////////////////////////////////////////////////////////////////////////////////

  struct __align(16) Vec3ia  // Intel: __aligned(16)
  {
    union {
      __m128i m128;
      struct { int x,y,z; int a; };
    };

    typedef int Scalar;
    enum { N = 3 };

    ////////////////////////////////////////////////////////////////////////////////
    /// Constructors, Assignment & Cast Operators
    ////////////////////////////////////////////////////////////////////////////////

    __forceinline Vec3ia( ) {}
    // MoonRay: added underscores to fix compile warning
    __forceinline Vec3ia( const __m128i _a ) : m128(_a) {}
    __forceinline Vec3ia( const Vec3ia& other ) : m128(other.m128) {}
    __forceinline Vec3ia& operator =(const Vec3ia& other) { m128 = other.m128; return *this; }

    // MoonRay: added underscores to fix compile warning
    __forceinline Vec3ia( const int _a ) : m128(_mm_set1_epi32(_a)) {}
    __forceinline explicit Vec3ia( const int _x, const int _y, const int _z) : m128(_mm_set_epi32(_z, _z, _y, _x)) {}
    __forceinline explicit Vec3ia( const __m128 _a ) : m128(_mm_cvtps_epi32(_a)) {}

    __forceinline operator const __m128i&( void ) const { return m128; }
    __forceinline operator       __m128i&( void )       { return m128; }

    ////////////////////////////////////////////////////////////////////////////////
    /// Constants
    ////////////////////////////////////////////////////////////////////////////////

    __forceinline Vec3ia( ZeroTy   ) : m128(_mm_setzero_si128()) {}
    __forceinline Vec3ia( OneTy    ) : m128(_mm_set1_epi32(1)) {}
    __forceinline Vec3ia( PosInfTy ) : m128(_mm_set1_epi32(pos_inf)) {}
    __forceinline Vec3ia( NegInfTy ) : m128(_mm_set1_epi32(neg_inf)) {}

    ////////////////////////////////////////////////////////////////////////////////
    /// Array Access
    ////////////////////////////////////////////////////////////////////////////////

    __forceinline const int& operator []( const size_t index ) const { assert(index < 3); return (&x)[index]; }
    __forceinline       int& operator []( const size_t index )       { assert(index < 3); return (&x)[index]; }
  };


  ////////////////////////////////////////////////////////////////////////////////
  /// Unary Operators
  ////////////////////////////////////////////////////////////////////////////////

  __forceinline const Vec3ia operator +( const Vec3ia& a ) { return a; }
  __forceinline const Vec3ia operator -( const Vec3ia& a ) { return _mm_sub_epi32(_mm_setzero_si128(), a.m128); }
#if defined(__SSSE3__)
  __forceinline const Vec3ia abs       ( const Vec3ia& a ) { return _mm_abs_epi32(a.m128); }
#endif

  ////////////////////////////////////////////////////////////////////////////////
  /// Binary Operators
  ////////////////////////////////////////////////////////////////////////////////

  __forceinline const Vec3ia operator +( const Vec3ia& a, const Vec3ia& b ) { return _mm_add_epi32(a.m128, b.m128); }
  __forceinline const Vec3ia operator +( const Vec3ia& a, const int        b ) { return a+Vec3ia(b); }
  __forceinline const Vec3ia operator +( const int        a, const Vec3ia& b ) { return Vec3ia(a)+b; }

  __forceinline const Vec3ia operator -( const Vec3ia& a, const Vec3ia& b ) { return _mm_sub_epi32(a.m128, b.m128); }
  __forceinline const Vec3ia operator -( const Vec3ia& a, const int        b ) { return a-Vec3ia(b); }
  __forceinline const Vec3ia operator -( const int        a, const Vec3ia& b ) { return Vec3ia(a)-b; }

#if defined(__SSE4_1__)
  __forceinline const Vec3ia operator *( const Vec3ia& a, const Vec3ia& b ) { return _mm_mullo_epi32(a.m128, b.m128); }
  __forceinline const Vec3ia operator *( const Vec3ia& a, const int        b ) { return a * Vec3ia(b); }
  __forceinline const Vec3ia operator *( const int        a, const Vec3ia& b ) { return Vec3ia(a) * b; }
#endif

  __forceinline const Vec3ia operator &( const Vec3ia& a, const Vec3ia& b ) { return _mm_and_si128(a.m128, b.m128); }
  __forceinline const Vec3ia operator &( const Vec3ia& a, const int        b ) { return a & Vec3ia(b); }
  __forceinline const Vec3ia operator &( const int        a, const Vec3ia& b ) { return Vec3ia(a) & b; }

  __forceinline const Vec3ia operator |( const Vec3ia& a, const Vec3ia& b ) { return _mm_or_si128(a.m128, b.m128); }
  __forceinline const Vec3ia operator |( const Vec3ia& a, const int        b ) { return a | Vec3ia(b); }
  __forceinline const Vec3ia operator |( const int        a, const Vec3ia& b ) { return Vec3ia(a) | b; }

  __forceinline const Vec3ia operator ^( const Vec3ia& a, const Vec3ia& b ) { return _mm_xor_si128(a.m128, b.m128); }
  __forceinline const Vec3ia operator ^( const Vec3ia& a, const int        b ) { return a ^ Vec3ia(b); }
  __forceinline const Vec3ia operator ^( const int        a, const Vec3ia& b ) { return Vec3ia(a) ^ b; }

  __forceinline const Vec3ia operator <<( const Vec3ia& a, const int n ) { return _mm_slli_epi32(a.m128, n); }
  __forceinline const Vec3ia operator >>( const Vec3ia& a, const int n ) { return _mm_srai_epi32(a.m128, n); }

  __forceinline const Vec3ia sra ( const Vec3ia& a, const int b ) { return _mm_srai_epi32(a.m128, b); }
  __forceinline const Vec3ia srl ( const Vec3ia& a, const int b ) { return _mm_srli_epi32(a.m128, b); }

  ////////////////////////////////////////////////////////////////////////////////
  /// Assignment Operators
  ////////////////////////////////////////////////////////////////////////////////

  __forceinline Vec3ia& operator +=( Vec3ia& a, const Vec3ia& b ) { return a = a + b; }
  __forceinline Vec3ia& operator +=( Vec3ia& a, const int32&     b ) { return a = a + b; }
  
  __forceinline Vec3ia& operator -=( Vec3ia& a, const Vec3ia& b ) { return a = a - b; }
  __forceinline Vec3ia& operator -=( Vec3ia& a, const int32&     b ) { return a = a - b; }
  
#if defined(__SSE4_1__)
  __forceinline Vec3ia& operator *=( Vec3ia& a, const Vec3ia& b ) { return a = a * b; }
  __forceinline Vec3ia& operator *=( Vec3ia& a, const int32&     b ) { return a = a * b; }
#endif
  
  __forceinline Vec3ia& operator &=( Vec3ia& a, const Vec3ia& b ) { return a = a & b; }
  __forceinline Vec3ia& operator &=( Vec3ia& a, const int32&     b ) { return a = a & b; }
  
  __forceinline Vec3ia& operator |=( Vec3ia& a, const Vec3ia& b ) { return a = a | b; }
  __forceinline Vec3ia& operator |=( Vec3ia& a, const int32&     b ) { return a = a | b; }
  
  __forceinline Vec3ia& operator <<=( Vec3ia& a, const int32&    b ) { return a = a << b; }
  __forceinline Vec3ia& operator >>=( Vec3ia& a, const int32&    b ) { return a = a >> b; }

  ////////////////////////////////////////////////////////////////////////////////
  /// Reductions
  ////////////////////////////////////////////////////////////////////////////////

  __forceinline int reduce_add(const Vec3ia& v) { return v.x+v.y+v.z; }
  __forceinline int reduce_mul(const Vec3ia& v) { return v.x*v.y*v.z; }
  __forceinline int reduce_min(const Vec3ia& v) { return min(v.x,v.y,v.z); }
  __forceinline int reduce_max(const Vec3ia& v) { return max(v.x,v.y,v.z); }

  ////////////////////////////////////////////////////////////////////////////////
  /// Comparison Operators
  ////////////////////////////////////////////////////////////////////////////////

  __forceinline bool operator ==( const Vec3ia& a, const Vec3ia& b ) { return (_mm_movemask_ps(_mm_castsi128_ps(_mm_cmpeq_epi32(a.m128, b.m128))) & 7) == 7; }
  __forceinline bool operator !=( const Vec3ia& a, const Vec3ia& b ) { return (_mm_movemask_ps(_mm_castsi128_ps(_mm_cmpeq_epi32(a.m128, b.m128))) & 7) != 7; }
  __forceinline bool operator < ( const Vec3ia& a, const Vec3ia& b ) {
    if (a.x != b.x) return a.x < b.x;
    if (a.y != b.y) return a.y < b.y;
    if (a.z != b.z) return a.z < b.z;
    return false;
  }

  __forceinline Vec3ba eq_mask( const Vec3ia& a, const Vec3ia& b ) { return _mm_castsi128_ps(_mm_cmpeq_epi32 (a.m128, b.m128)); }
  __forceinline Vec3ba lt_mask( const Vec3ia& a, const Vec3ia& b ) { return _mm_castsi128_ps(_mm_cmplt_epi32 (a.m128, b.m128)); }
  __forceinline Vec3ba gt_mask( const Vec3ia& a, const Vec3ia& b ) { return _mm_castsi128_ps(_mm_cmpgt_epi32 (a.m128, b.m128)); }

  ////////////////////////////////////////////////////////////////////////////////
  /// Select
  ////////////////////////////////////////////////////////////////////////////////

  __forceinline const Vec3ia select( const Vec3ba& m, const Vec3ia& t, const Vec3ia& f ) {
#if defined(__SSE4_1__)
    return _mm_castps_si128(_mm_blendv_ps(_mm_castsi128_ps(f), _mm_castsi128_ps(t), m));
#else
    return _mm_or_si128(_mm_and_si128(_mm_castps_si128(m), t), _mm_andnot_si128(_mm_castps_si128(m), f)); 
#endif
  }

#if defined(__SSE4_1__)
  __forceinline const Vec3ia min( const Vec3ia& a, const Vec3ia& b ) { return _mm_min_epi32(a.m128,b.m128); }
  __forceinline const Vec3ia max( const Vec3ia& a, const Vec3ia& b ) { return _mm_max_epi32(a.m128,b.m128); }
#else
  __forceinline const Vec3ia min( const Vec3ia& a, const Vec3ia& b ) { return select(lt_mask(a,b),a,b); }
  __forceinline const Vec3ia max( const Vec3ia& a, const Vec3ia& b ) { return select(gt_mask(a,b),a,b); }
#endif

  ////////////////////////////////////////////////////////////////////////////////
  /// Output Operators
  ////////////////////////////////////////////////////////////////////////////////

  inline std::ostream& operator<<(std::ostream& cout, const Vec3ia& a) {
    return cout << "(" << a.x << ", " << a.y << ", " << a.z << ")";
  }
}
}

