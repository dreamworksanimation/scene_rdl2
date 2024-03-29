// Copyright 2023-2024 DreamWorks Animation LLC and Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Math.h"
// Intel: #include "math/math.h"

namespace simd
// Intel: namespace embree
{
  /*! 4-wide SSE integer type. */
  struct ssei
  {
    typedef sseb Mask;                    // mask type
    typedef ssei Int;                     // int type
    typedef ssef Float;                   // float type

    enum   { size = 4 };                  // number of SIMD elements
    union  { __m128i m128; int32 i[4]; }; // data

    ////////////////////////////////////////////////////////////////////////////////
    /// Constructors, Assignment & Cast Operators
    ////////////////////////////////////////////////////////////////////////////////
    
    __forceinline ssei           ( ) {}
    __forceinline ssei           ( const ssei& a ) { m128 = a.m128; }
    __forceinline ssei& operator=( const ssei& a ) { m128 = a.m128; return *this; }

    __forceinline ssei( const __m128i a ) : m128(a) {}
    __forceinline operator const __m128i&( void ) const { return m128; }
    __forceinline operator       __m128i&( void )       { return m128; }

    // MoonRay: added const_casts
    __forceinline ssei           ( const int32&  a ) : m128(_mm_shuffle_epi32(_mm_castps_si128(_mm_load_ss((float*)const_cast<int32*>(&a))), _MM_SHUFFLE(0, 0, 0, 0))) {}
    __forceinline ssei           ( const uint32& a ) : m128(_mm_shuffle_epi32(_mm_castps_si128(_mm_load_ss((float*)const_cast<uint32*>(&a))), _MM_SHUFFLE(0, 0, 0, 0))) {}
#if defined(__X86_64__)
    __forceinline ssei           ( const size_t a  ) : m128(_mm_set1_epi32((int)a)) {}
#endif
    __forceinline ssei           ( int32  a, int32  b, int32  c, int32  d) : m128(_mm_set_epi32(d, c, b, a)) {}

    __forceinline explicit ssei( const __m128 a ) : m128(_mm_cvtps_epi32(a)) {}

    ////////////////////////////////////////////////////////////////////////////////
    /// Constants
    ////////////////////////////////////////////////////////////////////////////////

    // MoonRay: added namespace scene_rdl2::math
    __forceinline ssei( scene_rdl2::math::ZeroTy   ) : m128(_mm_setzero_si128()) {}
    __forceinline ssei( scene_rdl2::math::OneTy    ) : m128(_mm_set_epi32(1, 1, 1, 1)) {}
    __forceinline ssei( scene_rdl2::math::PosInfTy ) : m128(_mm_set_epi32(scene_rdl2::math::pos_inf, scene_rdl2::math::pos_inf, scene_rdl2::math::pos_inf, scene_rdl2::math::pos_inf)) {}
    __forceinline ssei( scene_rdl2::math::NegInfTy ) : m128(_mm_set_epi32(scene_rdl2::math::neg_inf, scene_rdl2::math::neg_inf, scene_rdl2::math::neg_inf, scene_rdl2::math::neg_inf)) {}
    __forceinline ssei( scene_rdl2::math::StepTy )   : m128(_mm_set_epi32(3, 2, 1, 0)) {}

    ////////////////////////////////////////////////////////////////////////////////
    /// Array Access
    ////////////////////////////////////////////////////////////////////////////////

    // MoonRay: changed assert() to MNRY_ASSERT()
    __forceinline const int32& operator []( const size_t index ) const { MNRY_ASSERT(index < 4); return i[index]; }
    __forceinline       int32& operator []( const size_t index )       { MNRY_ASSERT(index < 4); return i[index]; }
  };

  ////////////////////////////////////////////////////////////////////////////////
  /// Unary Operators
  ////////////////////////////////////////////////////////////////////////////////

  __forceinline const ssei cast      ( const __m128& a ) { return _mm_castps_si128(a); }
  __forceinline const ssei operator +( const ssei& a ) { return a; }
  __forceinline const ssei operator -( const ssei& a ) { return _mm_sub_epi32(_mm_setzero_si128(), a.m128); }
#if defined(__SSSE3__)
  __forceinline const ssei abs       ( const ssei& a ) { return _mm_abs_epi32(a.m128); }
#endif

  ////////////////////////////////////////////////////////////////////////////////
  /// Binary Operators
  ////////////////////////////////////////////////////////////////////////////////

  __forceinline const ssei operator +( const ssei& a, const ssei& b ) { return _mm_add_epi32(a.m128, b.m128); }
  __forceinline const ssei operator +( const ssei& a, const int32&  b ) { return a + ssei(b); }
  __forceinline const ssei operator +( const int32&  a, const ssei& b ) { return ssei(a) + b; }

  __forceinline const ssei operator -( const ssei& a, const ssei& b ) { return _mm_sub_epi32(a.m128, b.m128); }
  __forceinline const ssei operator -( const ssei& a, const int32&  b ) { return a - ssei(b); }
  __forceinline const ssei operator -( const int32&  a, const ssei& b ) { return ssei(a) - b; }

#if defined(__SSE4_1__)
  __forceinline const ssei operator *( const ssei& a, const ssei& b ) { return _mm_mullo_epi32(a.m128, b.m128); }
  __forceinline const ssei operator *( const ssei& a, const int32&  b ) { return a * ssei(b); }
  __forceinline const ssei operator *( const int32&  a, const ssei& b ) { return ssei(a) * b; }
#endif

  __forceinline const ssei operator &( const ssei& a, const ssei& b ) { return _mm_and_si128(a.m128, b.m128); }
  __forceinline const ssei operator &( const ssei& a, const int32&  b ) { return a & ssei(b); }
  __forceinline const ssei operator &( const int32& a, const ssei& b ) { return ssei(a) & b; }

  __forceinline const ssei operator |( const ssei& a, const ssei& b ) { return _mm_or_si128(a.m128, b.m128); }
  __forceinline const ssei operator |( const ssei& a, const int32&  b ) { return a | ssei(b); }
  __forceinline const ssei operator |( const int32& a, const ssei& b ) { return ssei(a) | b; }

  __forceinline const ssei operator ^( const ssei& a, const ssei& b ) { return _mm_xor_si128(a.m128, b.m128); }
  __forceinline const ssei operator ^( const ssei& a, const int32&  b ) { return a ^ ssei(b); }
  __forceinline const ssei operator ^( const int32& a, const ssei& b ) { return ssei(a) ^ b; }

  __forceinline const ssei operator <<( const ssei& a, const int32& n ) { return _mm_slli_epi32(a.m128, n); }
  __forceinline const ssei operator >>( const ssei& a, const int32& n ) { return _mm_srai_epi32(a.m128, n); }

  __forceinline const ssei sra ( const ssei& a, const int32& b ) { return _mm_srai_epi32(a.m128, b); }
  __forceinline const ssei srl ( const ssei& a, const int32& b ) { return _mm_srli_epi32(a.m128, b); }
  
#if defined(__SSE4_1__)
  __forceinline const ssei min( const ssei& a, const ssei& b ) { return _mm_min_epi32(a.m128, b.m128); }
  __forceinline const ssei max( const ssei& a, const ssei& b ) { return _mm_max_epi32(a.m128, b.m128); }
#else
  __forceinline const ssei min( const ssei& a, const ssei& b ) { return ssei(std::min(a[0],b[0]),std::min(a[1],b[1]),std::min(a[2],b[2]),std::min(a[3],b[3])); }
  __forceinline const ssei max( const ssei& a, const ssei& b ) { return ssei(std::max(a[0],b[0]),std::max(a[1],b[1]),std::max(a[2],b[2]),std::max(a[3],b[3])); }
#endif

  __forceinline const ssei min( const ssei& a, const int32&  b ) { return min(a,ssei(b)); }
  __forceinline const ssei min( const int32&  a, const ssei& b ) { return min(ssei(a),b); }
  __forceinline const ssei max( const ssei& a, const int32&  b ) { return max(a,ssei(b)); }
  __forceinline const ssei max( const int32&  a, const ssei& b ) { return max(ssei(a),b); }

  ////////////////////////////////////////////////////////////////////////////////
  /// Assignment Operators
  ////////////////////////////////////////////////////////////////////////////////

  __forceinline ssei& operator +=( ssei& a, const ssei& b ) { return a = a + b; }
  __forceinline ssei& operator +=( ssei& a, const int32&  b ) { return a = a + b; }
  
  __forceinline ssei& operator -=( ssei& a, const ssei& b ) { return a = a - b; }
  __forceinline ssei& operator -=( ssei& a, const int32&  b ) { return a = a - b; }

#if defined(__SSE4_1__)
  __forceinline ssei& operator *=( ssei& a, const ssei& b ) { return a = a * b; }
  __forceinline ssei& operator *=( ssei& a, const int32&  b ) { return a = a * b; }
#endif
  
  __forceinline ssei& operator &=( ssei& a, const ssei& b ) { return a = a & b; }
  __forceinline ssei& operator &=( ssei& a, const int32&  b ) { return a = a & b; }
  
  __forceinline ssei& operator |=( ssei& a, const ssei& b ) { return a = a | b; }
  __forceinline ssei& operator |=( ssei& a, const int32&  b ) { return a = a | b; }
  
  __forceinline ssei& operator <<=( ssei& a, const int32&  b ) { return a = a << b; }
  __forceinline ssei& operator >>=( ssei& a, const int32&  b ) { return a = a >> b; }

  ////////////////////////////////////////////////////////////////////////////////
  /// Comparison Operators + Select
  ////////////////////////////////////////////////////////////////////////////////

  __forceinline const sseb operator ==( const ssei& a, const ssei& b ) { return _mm_castsi128_ps(_mm_cmpeq_epi32 (a.m128, b.m128)); }
  __forceinline const sseb operator ==( const ssei& a, const int32& b ) { return a == ssei(b); }
  __forceinline const sseb operator ==( const int32& a, const ssei& b ) { return ssei(a) == b; }
  
  __forceinline const sseb operator !=( const ssei& a, const ssei& b ) { return !(a == b); }
  __forceinline const sseb operator !=( const ssei& a, const int32& b ) { return a != ssei(b); }
  __forceinline const sseb operator !=( const int32& a, const ssei& b ) { return ssei(a) != b; }
  
  __forceinline const sseb operator < ( const ssei& a, const ssei& b ) { return _mm_castsi128_ps(_mm_cmplt_epi32 (a.m128, b.m128)); }
  __forceinline const sseb operator < ( const ssei& a, const int32& b ) { return a <  ssei(b); }
  __forceinline const sseb operator < ( const int32& a, const ssei& b ) { return ssei(a) <  b; }
  
  __forceinline const sseb operator >=( const ssei& a, const ssei& b ) { return !(a <  b); }
  __forceinline const sseb operator >=( const ssei& a, const int32& b ) { return a >= ssei(b); }
  __forceinline const sseb operator >=( const int32& a, const ssei& b ) { return ssei(a) >= b; }

  __forceinline const sseb operator > ( const ssei& a, const ssei& b ) { return _mm_castsi128_ps(_mm_cmpgt_epi32 (a.m128, b.m128)); }
  __forceinline const sseb operator > ( const ssei& a, const int32& b ) { return a >  ssei(b); }
  __forceinline const sseb operator > ( const int32& a, const ssei& b ) { return ssei(a) >  b; }

  __forceinline const sseb operator <=( const ssei& a, const ssei& b ) { return !(a >  b); }
  __forceinline const sseb operator <=( const ssei& a, const int32& b ) { return a <= ssei(b); }
  __forceinline const sseb operator <=( const int32& a, const ssei& b ) { return ssei(a) <= b; }

  __forceinline const ssei select( const sseb& m, const ssei& t, const ssei& f ) { 
#if defined(__SSE4_1__)
    return _mm_castps_si128(_mm_blendv_ps(_mm_castsi128_ps(f), _mm_castsi128_ps(t), m)); 
#else
    return _mm_or_si128(_mm_and_si128(m, t), _mm_andnot_si128(m, f)); 
#endif
  }

#if defined(__SSE4_1__) 
#if defined(__clang__) || defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  __forceinline const ssei select(const int mask, const ssei& t, const ssei& f) {
	  return select(sseb(mask), t, f);
  }
#else
  __forceinline const ssei select(const int m, const ssei& t, const ssei& f) {
	  return _mm_castps_si128(_mm_blend_ps(_mm_castsi128_ps(f), _mm_castsi128_ps(t), m));
  }
#endif
#endif

  ////////////////////////////////////////////////////////////////////////////////
  // Movement/Shifting/Shuffling Functions
  ////////////////////////////////////////////////////////////////////////////////

  __forceinline ssei unpacklo( const ssei& a, const ssei& b ) { return _mm_castps_si128(_mm_unpacklo_ps(_mm_castsi128_ps(a.m128), _mm_castsi128_ps(b.m128))); }
  __forceinline ssei unpackhi( const ssei& a, const ssei& b ) { return _mm_castps_si128(_mm_unpackhi_ps(_mm_castsi128_ps(a.m128), _mm_castsi128_ps(b.m128))); }

  template<size_t i0, size_t i1, size_t i2, size_t i3> __forceinline const ssei shuffle( const ssei& a ) {
    return _mm_shuffle_epi32(a, _MM_SHUFFLE(i3, i2, i1, i0));
  }

  template<size_t i0, size_t i1, size_t i2, size_t i3> __forceinline const ssei shuffle( const ssei& a, const ssei& b ) {
    return _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(a), _mm_castsi128_ps(b), _MM_SHUFFLE(i3, i2, i1, i0)));
  }

#if defined(__SSE3__)
  template<> __forceinline const ssei shuffle<0, 0, 2, 2>( const ssei& a ) { return _mm_castps_si128(_mm_moveldup_ps(_mm_castsi128_ps(a))); }
  template<> __forceinline const ssei shuffle<1, 1, 3, 3>( const ssei& a ) { return _mm_castps_si128(_mm_movehdup_ps(_mm_castsi128_ps(a))); }
  template<> __forceinline const ssei shuffle<0, 1, 0, 1>( const ssei& a ) { return _mm_castpd_si128(_mm_movedup_pd (_mm_castsi128_pd(a))); }
#endif

  template<size_t i0> __forceinline const ssei shuffle( const ssei& b ) {
    return shuffle<i0,i0,i0,i0>(b);
  }

#if defined(__SSE4_1__)
  template<size_t src> __forceinline int extract( const ssei& b ) { return _mm_extract_epi32(b, src); }
  template<size_t dst> __forceinline const ssei insert( const ssei& a, const int32 b ) { return _mm_insert_epi32(a, b, dst); }
#else
  template<size_t src> __forceinline int extract( const ssei& b ) { return b[src]; }
  template<size_t dst> __forceinline const ssei insert( const ssei& a, const int32 b ) { ssei c = a; c[dst] = b; return c; }
#endif

  ////////////////////////////////////////////////////////////////////////////////
  /// Reductions
  ////////////////////////////////////////////////////////////////////////////////

#if defined(__SSE4_1__)
  __forceinline const ssei vreduce_min(const ssei& v) { ssei h = min(shuffle<1,0,3,2>(v),v); return min(shuffle<2,3,0,1>(h),h); }
  __forceinline const ssei vreduce_max(const ssei& v) { ssei h = max(shuffle<1,0,3,2>(v),v); return max(shuffle<2,3,0,1>(h),h); }
  __forceinline const ssei vreduce_add(const ssei& v) { ssei h = shuffle<1,0,3,2>(v)   + v ; return shuffle<2,3,0,1>(h)   + h ; }

  __forceinline int reduce_min(const ssei& v) { return extract<0>(vreduce_min(v)); }
  __forceinline int reduce_max(const ssei& v) { return extract<0>(vreduce_max(v)); }
  __forceinline int reduce_add(const ssei& v) { return extract<0>(vreduce_add(v)); }

  __forceinline size_t select_min(const ssei& v) { return __bsf(movemask(v == vreduce_min(v))); }
  __forceinline size_t select_max(const ssei& v) { return __bsf(movemask(v == vreduce_max(v))); }

  // MoonRay: added namespace scene_rdl2::math
  __forceinline size_t select_min(const sseb& valid, const ssei& v) { const ssei a = select(valid,v,ssei(scene_rdl2::math::pos_inf)); return __bsf(movemask(valid & (a == vreduce_min(a)))); }
  __forceinline size_t select_max(const sseb& valid, const ssei& v) { const ssei a = select(valid,v,ssei(scene_rdl2::math::neg_inf)); return __bsf(movemask(valid & (a == vreduce_max(a)))); }

#else

  // MoonRay: added namespace scene_rdl2::math
  __forceinline int reduce_min(const ssei& v) { return scene_rdl2::math::min(v[0],v[1],v[2],v[3]); }
  __forceinline int reduce_max(const ssei& v) { return scene_rdl2::math::max(v[0],v[1],v[2],v[3]); }
  __forceinline int reduce_add(const ssei& v) { return v[0]+v[1]+v[2]+v[3]; }

#endif

  ////////////////////////////////////////////////////////////////////////////////
  /// Memory load and store operations
  ////////////////////////////////////////////////////////////////////////////////

  __forceinline ssei load4i( const void* const a ) { 
    return _mm_load_si128((__m128i*)const_cast<void*>(a)); 
    // Intel: return _mm_load_si128((__m128i*)a); 
  }

  __forceinline void store4i(void* ptr, const ssei& v) {
    _mm_store_si128((__m128i*)ptr,v);
  }

  __forceinline void storeu4i(void* ptr, const ssei& v) {
    _mm_storeu_si128((__m128i*)ptr,v);
  }
  
  __forceinline void store4i( const sseb& mask, void* ptr, const ssei& i ) { 
#if defined (__AVX__)
    _mm_maskstore_ps((float*)ptr,(__m128i)mask,_mm_castsi128_ps(i));
#else
    *(ssei*)ptr = select(mask,i,*(ssei*)ptr);
#endif
  }

  __forceinline ssei load4i_nt (void* ptr) { 
#if defined(__SSE4_1__)
    return _mm_stream_load_si128((__m128i*)ptr); 
#else
    return _mm_load_si128((__m128i*)ptr); 
#endif
  }

  __forceinline void store4i_nt(void* ptr, const ssei& v) { 
#if defined(__SSE4_1__)
    _mm_stream_ps((float*)ptr,_mm_castsi128_ps(v)); 
#else
    _mm_store_si128((__m128i*)ptr,v);
#endif
  }

  ////////////////////////////////////////////////////////////////////////////////
  /// Output Operators
  ////////////////////////////////////////////////////////////////////////////////

  inline std::ostream& operator<<(std::ostream& cout, const ssei& a) {
    return cout << "<" << a[0] << ", " << a[1] << ", " << a[2] << ", " << a[3] << ">";
  }
}

