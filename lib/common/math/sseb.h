// Copyright 2023-2024 DreamWorks Animation LLC and Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once


namespace simd
// Intel: namespace embree
{
  /*! 4-wide SSE bool type. */
  struct sseb
  {
    typedef sseb Mask;                    // mask type
    typedef ssei Int;                     // int type
    typedef ssef Float;                   // float type

    enum   { size = 4 };                  // number of SIMD elements
    union  { __m128 m128; int32 v[4]; };  // data

    ////////////////////////////////////////////////////////////////////////////////
    /// Constructors, Assignment & Cast Operators
    ////////////////////////////////////////////////////////////////////////////////
    
    __forceinline sseb           ( ) {}
    __forceinline sseb           ( const sseb& other ) { m128 = other.m128; }
    __forceinline sseb& operator=( const sseb& other ) { m128 = other.m128; return *this; }

    __forceinline sseb( const __m128  input ) : m128(input) {}
    __forceinline operator const __m128&( void ) const { return m128; }
    __forceinline operator const __m128i( void ) const { return _mm_castps_si128(m128); }
    __forceinline operator const __m128d( void ) const { return _mm_castps_pd(m128); }
    
    __forceinline sseb           ( bool  a )
      : m128(_mm_lookupmask_ps[(size_t(a) << 3) | (size_t(a) << 2) | (size_t(a) << 1) | size_t(a)]) {}
    __forceinline sseb           ( bool  a, bool  b) 
      : m128(_mm_lookupmask_ps[(size_t(b) << 3) | (size_t(a) << 2) | (size_t(b) << 1) | size_t(a)]) {}
    __forceinline sseb           ( bool  a, bool  b, bool  c, bool  d)
      : m128(_mm_lookupmask_ps[(size_t(d) << 3) | (size_t(c) << 2) | (size_t(b) << 1) | size_t(a)]) {}
    __forceinline sseb(int mask) {
      assert(mask >= 0 && mask < 16);
      m128 = _mm_lookupmask_ps[mask];
    }

    ////////////////////////////////////////////////////////////////////////////////
    /// Constants
    ////////////////////////////////////////////////////////////////////////////////

    // MoonRay: added scene_rdl2::util namespace
    __forceinline sseb( scene_rdl2::util::FalseTy ) : m128(_mm_setzero_ps()) {}
    __forceinline sseb( scene_rdl2::util::TrueTy  ) : m128(_mm_castsi128_ps(_mm_cmpeq_epi32(_mm_setzero_si128(), _mm_setzero_si128()))) {}

    ////////////////////////////////////////////////////////////////////////////////
    /// Array Access
    ////////////////////////////////////////////////////////////////////////////////

    // MoonRay: use MNRY_ASSERT() instead of assert()
    __forceinline bool   operator []( const size_t i ) const { MNRY_ASSERT(i < 4); return (_mm_movemask_ps(m128) >> i) & 1; }
    __forceinline int32& operator []( const size_t i )       { MNRY_ASSERT(i < 4); return v[i]; }
  };

  ////////////////////////////////////////////////////////////////////////////////
  /// Unary Operators
  ////////////////////////////////////////////////////////////////////////////////
  
  // MoonRay: added scene_rdl2::util namespace
  __forceinline const sseb operator !( const sseb& a ) { return _mm_xor_ps(a, sseb(scene_rdl2::util::True)); }
  
  ////////////////////////////////////////////////////////////////////////////////
  /// Binary Operators
  ////////////////////////////////////////////////////////////////////////////////
  
  __forceinline const sseb operator &( const sseb& a, const sseb& b ) { return _mm_and_ps(a, b); }
  __forceinline const sseb operator |( const sseb& a, const sseb& b ) { return _mm_or_ps (a, b); }
  __forceinline const sseb operator ^( const sseb& a, const sseb& b ) { return _mm_xor_ps(a, b); }
  
  ////////////////////////////////////////////////////////////////////////////////
  /// Assignment Operators
  ////////////////////////////////////////////////////////////////////////////////
  
  __forceinline const sseb operator &=( sseb& a, const sseb& b ) { return a = a & b; }
  __forceinline const sseb operator |=( sseb& a, const sseb& b ) { return a = a | b; }
  __forceinline const sseb operator ^=( sseb& a, const sseb& b ) { return a = a ^ b; }
  
  ////////////////////////////////////////////////////////////////////////////////
  /// Comparison Operators + Select
  ////////////////////////////////////////////////////////////////////////////////
  
  __forceinline const sseb operator !=( const sseb& a, const sseb& b ) { return _mm_xor_ps(a, b); }
  __forceinline const sseb operator ==( const sseb& a, const sseb& b ) { return _mm_castsi128_ps(_mm_cmpeq_epi32(a, b)); }
  
  __forceinline const sseb select( const sseb& m, const sseb& t, const sseb& f ) { 
#if defined(__SSE4_1__)
    return _mm_blendv_ps(f, t, m); 
#else
    return _mm_or_ps(_mm_and_ps(m, t), _mm_andnot_ps(m, f)); 
#endif
  }
  
  ////////////////////////////////////////////////////////////////////////////////
  /// Movement/Shifting/Shuffling Functions
  ////////////////////////////////////////////////////////////////////////////////
  
  __forceinline const sseb unpacklo( const sseb& a, const sseb& b ) { return _mm_unpacklo_ps(a, b); }
  __forceinline const sseb unpackhi( const sseb& a, const sseb& b ) { return _mm_unpackhi_ps(a, b); }

  template<size_t i0, size_t i1, size_t i2, size_t i3> __forceinline const sseb shuffle( const sseb& a ) {
    return sseb(_mm_shuffle_epi32(a, _MM_SHUFFLE(i3, i2, i1, i0)));
// Intel:    return _mm_shuffle_epi32(a, _MM_SHUFFLE(i3, i2, i1, i0));
  }

  template<size_t i0, size_t i1, size_t i2, size_t i3> __forceinline const sseb shuffle( const sseb& a, const sseb& b ) {
    return _mm_shuffle_ps(a, b, _MM_SHUFFLE(i3, i2, i1, i0));
  }

#if defined(__SSE3__)
  template<> __forceinline const sseb shuffle<0, 0, 2, 2>( const sseb& a ) { return _mm_moveldup_ps(a); }
  template<> __forceinline const sseb shuffle<1, 1, 3, 3>( const sseb& a ) { return _mm_movehdup_ps(a); }
  template<> __forceinline const sseb shuffle<0, 1, 0, 1>( const sseb& a ) { return _mm_castpd_ps(_mm_movedup_pd (a)); }
#endif

#if defined(__SSE4_1__)
  template<size_t dst, size_t src, size_t clr> __forceinline const sseb insert( const sseb& a, const sseb& b ) { return _mm_insert_ps(a, b, (dst << 4) | (src << 6) | clr); }
  template<size_t dst, size_t src> __forceinline const sseb insert( const sseb& a, const sseb& b ) { return insert<dst, src, 0>(a, b); }
  template<size_t dst>             __forceinline const sseb insert( const sseb& a, const bool b ) { return insert<dst,0>(a, sseb(b)); }
#endif
  
  ////////////////////////////////////////////////////////////////////////////////
  /// Reduction Operations
  ////////////////////////////////////////////////////////////////////////////////
  
#if defined(__SSE4_2__)
  __forceinline size_t popcnt( const sseb& a ) { return __popcnt(_mm_movemask_ps(a)); }
#else
  __forceinline size_t popcnt( const sseb& a ) { return bool(a[0])+bool(a[1])+bool(a[2])+bool(a[3]); }
#endif
  
  __forceinline bool reduce_and( const sseb& a ) { return _mm_movemask_ps(a) == 0xf; }
  __forceinline bool reduce_or ( const sseb& a ) { return _mm_movemask_ps(a) != 0x0; }
  __forceinline bool all       ( const sseb& b ) { return _mm_movemask_ps(b) == 0xf; }
  __forceinline bool any       ( const sseb& b ) { return _mm_movemask_ps(b) != 0x0; }
  __forceinline bool none      ( const sseb& b ) { return _mm_movemask_ps(b) == 0x0; }
  
  __forceinline size_t movemask( const sseb& a ) { return _mm_movemask_ps(a); }
  
  ////////////////////////////////////////////////////////////////////////////////
  /// Output Operators
  ////////////////////////////////////////////////////////////////////////////////
  
  inline std::ostream& operator<<(std::ostream& cout, const sseb& a) {
    return cout << "<" << a[0] << ", " << a[1] << ", " << a[2] << ", " << a[3] << ">";
  }
}

