// Copyright 2023-2024 DreamWorks Animation LLC and Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

namespace simd
// Intel: namespace embree
{
  /*! 4-wide SSE float type. */
  struct ssef
  {
    typedef sseb Mask;                    // mask type
    typedef ssei Int;                     // int type
    typedef ssef Float;                   // float type
    
    enum   { size = 4 };  // number of SIMD elements
    union { __m128 m128; float f[4]; int i[4]; }; // data

    ////////////////////////////////////////////////////////////////////////////////
    /// Constructors, Assignment & Cast Operators
    ////////////////////////////////////////////////////////////////////////////////
    
    __forceinline ssef           ( ) {}
    __forceinline ssef           ( const ssef& other ) { m128 = other.m128; }
    __forceinline ssef& operator=( const ssef& other ) { m128 = other.m128; return *this; }

    __forceinline ssef( const __m128 a ) : m128(a) {}
    __forceinline operator const __m128&( void ) const { return m128; }
    __forceinline operator       __m128&( void )       { return m128; }

    __forceinline ssef           ( float  a ) : m128(_mm_set1_ps(a)) {}
    __forceinline ssef           ( float  a, float  b, float  c, float  d) : m128(_mm_set_ps(d, c, b, a)) {}

    __forceinline explicit ssef( const __m128i a ) : m128(_mm_cvtepi32_ps(a)) {}

    ////////////////////////////////////////////////////////////////////////////////
    /// Constants
    ////////////////////////////////////////////////////////////////////////////////

    // MoonRay: added namespace scene_rdl2::math
    __forceinline ssef( scene_rdl2::math::ZeroTy   ) : m128(_mm_setzero_ps()) {}
    __forceinline ssef( scene_rdl2::math::OneTy    ) : m128(_mm_set1_ps(1.0f)) {}
    __forceinline ssef( scene_rdl2::math::PosInfTy ) : m128(_mm_set1_ps(scene_rdl2::math::pos_inf)) {}
    __forceinline ssef( scene_rdl2::math::NegInfTy ) : m128(_mm_set1_ps(scene_rdl2::math::neg_inf)) {}
    __forceinline ssef( scene_rdl2::math::StepTy   ) : m128(_mm_set_ps(3.0f, 2.0f, 1.0f, 0.0f)) {}
    __forceinline ssef( scene_rdl2::math::NaNTy    ) : m128(_mm_set1_ps(scene_rdl2::math::nan)) {}

    ////////////////////////////////////////////////////////////////////////////////
    /// Loads and Stores
    ////////////////////////////////////////////////////////////////////////////////

#if defined(__AVX__)
    static __forceinline ssef broadcast( const void* const a ) { return _mm_broadcast_ss((float*)const_cast<void*>(a)); }
    // Intel: static __forceinline ssef broadcast( const void* const a ) { return _mm_broadcast_ss((float*)a); }
#else
    static __forceinline ssef broadcast( const void* const a ) { return _mm_set1_ps(*(float*)a); }
#endif

    ////////////////////////////////////////////////////////////////////////////////
    /// Array Access
    ////////////////////////////////////////////////////////////////////////////////

    // MoonRay: changed assert() to MNRY_ASSERT()
    __forceinline const float& operator []( const size_t aI ) const { MNRY_ASSERT(aI < 4); return f[aI]; }
    __forceinline       float& operator []( const size_t aI )       { MNRY_ASSERT(aI < 4); return f[aI]; }
  };


  ////////////////////////////////////////////////////////////////////////////////
  /// Unary Operators
  ////////////////////////////////////////////////////////////////////////////////

  __forceinline const ssef cast      (const __m128i& a) { return _mm_castsi128_ps(a); }
  __forceinline const ssef operator +( const ssef& a ) { return a; }
  __forceinline const ssef operator -( const ssef& a ) { return _mm_xor_ps(a.m128, _mm_castsi128_ps(_mm_set1_epi32(0x80000000))); }
  __forceinline const ssef abs       ( const ssef& a ) { return _mm_and_ps(a.m128, _mm_castsi128_ps(_mm_set1_epi32(0x7fffffff))); }
  // MoonRay: added namespace scene_rdl2::math
  __forceinline const ssef sign      ( const ssef& a ) { return _mm_blendv_ps(ssef(scene_rdl2::math::one), -ssef(scene_rdl2::math::one), _mm_cmplt_ps (a,ssef(scene_rdl2::math::zero))); }
  __forceinline const ssef signmsk   ( const ssef& a ) { return _mm_and_ps(a.m128,_mm_castsi128_ps(_mm_set1_epi32(0x80000000))); }
  
  __forceinline const ssef rcp  ( const ssef& a ) {
    const ssef r = _mm_rcp_ps(a.m128);
    return _mm_sub_ps(_mm_add_ps(r, r), _mm_mul_ps(_mm_mul_ps(r, r), a));
  }
  __forceinline const ssef sqr  ( const ssef& a ) { return _mm_mul_ps(a,a); }
  __forceinline const ssef sqrt ( const ssef& a ) { return _mm_sqrt_ps(a.m128); }
  __forceinline const ssef rsqrt( const ssef& a ) {
    const ssef r = _mm_rsqrt_ps(a.m128);
    return _mm_add_ps(_mm_mul_ps(_mm_set_ps(1.5f, 1.5f, 1.5f, 1.5f), r),
                      _mm_mul_ps(_mm_mul_ps(_mm_mul_ps(a, _mm_set_ps(-0.5f, -0.5f, -0.5f, -0.5f)), r), _mm_mul_ps(r, r)));
  }

  ////////////////////////////////////////////////////////////////////////////////
  /// Binary Operators
  ////////////////////////////////////////////////////////////////////////////////

  __forceinline const ssef operator +( const ssef& a, const ssef& b ) { return _mm_add_ps(a.m128, b.m128); }
  __forceinline const ssef operator +( const ssef& a, const float& b ) { return a + ssef(b); }
  __forceinline const ssef operator +( const float& a, const ssef& b ) { return ssef(a) + b; }

  __forceinline const ssef operator -( const ssef& a, const ssef& b ) { return _mm_sub_ps(a.m128, b.m128); }
  __forceinline const ssef operator -( const ssef& a, const float& b ) { return a - ssef(b); }
  __forceinline const ssef operator -( const float& a, const ssef& b ) { return ssef(a) - b; }

  __forceinline const ssef operator *( const ssef& a, const ssef& b ) { return _mm_mul_ps(a.m128, b.m128); }
  __forceinline const ssef operator *( const ssef& a, const float& b ) { return a * ssef(b); }
  __forceinline const ssef operator *( const float& a, const ssef& b ) { return ssef(a) * b; }

  __forceinline const ssef operator /( const ssef& a, const ssef& b ) { return _mm_div_ps(a.m128,b.m128); }
  __forceinline const ssef operator /( const ssef& a, const float& b ) { return a/ssef(b); }
  __forceinline const ssef operator /( const float& a, const ssef& b ) { return ssef(a)/b; }

  __forceinline const ssef operator^( const ssef& a, const ssef& b ) { return _mm_xor_ps(a.m128,b.m128); }
  __forceinline const ssef operator^( const ssef& a, const ssei& b ) { return _mm_xor_ps(a.m128,_mm_castsi128_ps(b.m128)); }

  __forceinline const ssef min( const ssef& a, const ssef& b ) { return _mm_min_ps(a.m128,b.m128); }
  __forceinline const ssef min( const ssef& a, const float& b ) { return _mm_min_ps(a.m128,ssef(b)); }
  __forceinline const ssef min( const float& a, const ssef& b ) { return _mm_min_ps(ssef(a),b.m128); }

  __forceinline const ssef max( const ssef& a, const ssef& b ) { return _mm_max_ps(a.m128,b.m128); }
  __forceinline const ssef max( const ssef& a, const float& b ) { return _mm_max_ps(a.m128,ssef(b)); }
  __forceinline const ssef max( const float& a, const ssef& b ) { return _mm_max_ps(ssef(a),b.m128); }

#if defined(__SSE4_1__)
    __forceinline ssef mini(const ssef& a, const ssef& b) {
      const ssei ai = _mm_castps_si128(a);
      const ssei bi = _mm_castps_si128(b);
      const ssei ci = _mm_min_epi32(ai,bi);
      return _mm_castsi128_ps(ci);
    }
#endif
    
#if defined(__SSE4_1__)
    __forceinline ssef maxi(const ssef& a, const ssef& b) {
      const ssei ai = _mm_castps_si128(a);
      const ssei bi = _mm_castps_si128(b);
      const ssei ci = _mm_max_epi32(ai,bi);
      return _mm_castsi128_ps(ci);
    }
#endif

  ////////////////////////////////////////////////////////////////////////////////
  /// Ternary Operators
  ////////////////////////////////////////////////////////////////////////////////

#if defined(__AVX2__)
  __forceinline const ssef madd  ( const ssef& a, const ssef& b, const ssef& c) { return _mm_fmadd_ps(a,b,c); }
  __forceinline const ssef msub  ( const ssef& a, const ssef& b, const ssef& c) { return _mm_fmsub_ps(a,b,c); }
  __forceinline const ssef nmadd ( const ssef& a, const ssef& b, const ssef& c) { return _mm_fnmadd_ps(a,b,c); }
  __forceinline const ssef nmsub ( const ssef& a, const ssef& b, const ssef& c) { return _mm_fnmsub_ps(a,b,c); }
#else
  __forceinline const ssef madd  ( const ssef& a, const ssef& b, const ssef& c) { return a*b+c; }
  __forceinline const ssef msub  ( const ssef& a, const ssef& b, const ssef& c) { return a*b-c; }
  __forceinline const ssef nmadd ( const ssef& a, const ssef& b, const ssef& c) { return -a*b-c;}
  __forceinline const ssef nmsub ( const ssef& a, const ssef& b, const ssef& c) { return c-a*b; }
#endif

  ////////////////////////////////////////////////////////////////////////////////
  /// Assignment Operators
  ////////////////////////////////////////////////////////////////////////////////

  __forceinline ssef& operator +=( ssef& a, const ssef& b ) { return a = a + b; }
  __forceinline ssef& operator +=( ssef& a, const float& b ) { return a = a + b; }

  __forceinline ssef& operator -=( ssef& a, const ssef& b ) { return a = a - b; }
  __forceinline ssef& operator -=( ssef& a, const float& b ) { return a = a - b; }

  __forceinline ssef& operator *=( ssef& a, const ssef& b ) { return a = a * b; }
  __forceinline ssef& operator *=( ssef& a, const float& b ) { return a = a * b; }

  __forceinline ssef& operator /=( ssef& a, const ssef& b ) { return a = a / b; }
  __forceinline ssef& operator /=( ssef& a, const float& b ) { return a = a / b; }

  ////////////////////////////////////////////////////////////////////////////////
  /// Comparison Operators + Select
  ////////////////////////////////////////////////////////////////////////////////

  __forceinline const sseb operator ==( const ssef& a, const ssef& b ) { return _mm_cmpeq_ps (a.m128, b.m128); }
  __forceinline const sseb operator ==( const ssef& a, const float& b ) { return a == ssef(b); }
  __forceinline const sseb operator ==( const float& a, const ssef& b ) { return ssef(a) == b; }

  __forceinline const sseb operator !=( const ssef& a, const ssef& b ) { return _mm_cmpneq_ps(a.m128, b.m128); }
  __forceinline const sseb operator !=( const ssef& a, const float& b ) { return a != ssef(b); }
  __forceinline const sseb operator !=( const float& a, const ssef& b ) { return ssef(a) != b; }

  __forceinline const sseb operator < ( const ssef& a, const ssef& b ) { return _mm_cmplt_ps (a.m128, b.m128); }
  __forceinline const sseb operator < ( const ssef& a, const float& b ) { return a <  ssef(b); }
  __forceinline const sseb operator < ( const float& a, const ssef& b ) { return ssef(a) <  b; }

  __forceinline const sseb operator >=( const ssef& a, const ssef& b ) { return _mm_cmpnlt_ps(a.m128, b.m128); }
  __forceinline const sseb operator >=( const ssef& a, const float& b ) { return a >= ssef(b); }
  __forceinline const sseb operator >=( const float& a, const ssef& b ) { return ssef(a) >= b; }

  __forceinline const sseb operator > ( const ssef& a, const ssef& b ) { return _mm_cmpnle_ps(a.m128, b.m128); }
  __forceinline const sseb operator > ( const ssef& a, const float& b ) { return a >  ssef(b); }
  __forceinline const sseb operator > ( const float& a, const ssef& b ) { return ssef(a) >  b; }

  __forceinline const sseb operator <=( const ssef& a, const ssef& b ) { return _mm_cmple_ps (a.m128, b.m128); }
  __forceinline const sseb operator <=( const ssef& a, const float& b ) { return a <= ssef(b); }
  __forceinline const sseb operator <=( const float& a, const ssef& b ) { return ssef(a) <= b; }

 __forceinline const ssef select( const sseb& m, const ssef& t, const ssef& f ) { 
#if defined(__SSE4_1__)
    return _mm_blendv_ps(f, t, m); 
#else
    return _mm_or_ps(_mm_and_ps(m, t), _mm_andnot_ps(m, f)); 
#endif
 }

#if defined(__SSE4_1__) 
#if defined(__clang__) || defined(_MSC_VER) && !defined(__INTEL_COMPILER)
__forceinline const ssef select(const int mask, const ssef& t, const ssef& f) {
 return select(sseb(mask), t, f);
}
#else
 __forceinline const ssef select(const int mask, const ssef& t, const ssef& f) {
	 return _mm_blend_ps(f, t, mask);
 }
#endif
#endif

  ////////////////////////////////////////////////////////////////////////////////
  /// Rounding Functions
  ////////////////////////////////////////////////////////////////////////////////

#if defined (__SSE4_1__)
  __forceinline const ssef round_even( const ssef& a ) { return _mm_round_ps(a, _MM_FROUND_TO_NEAREST_INT); }
  __forceinline const ssef round_down( const ssef& a ) { return _mm_round_ps(a, _MM_FROUND_TO_NEG_INF    ); }
  __forceinline const ssef round_up  ( const ssef& a ) { return _mm_round_ps(a, _MM_FROUND_TO_POS_INF    ); }
  __forceinline const ssef round_zero( const ssef& a ) { return _mm_round_ps(a, _MM_FROUND_TO_ZERO       ); }
  __forceinline const ssef floor     ( const ssef& a ) { return _mm_round_ps(a, _MM_FROUND_TO_NEG_INF    ); }
  __forceinline const ssef ceil      ( const ssef& a ) { return _mm_round_ps(a, _MM_FROUND_TO_POS_INF    ); }
#endif

  __forceinline ssei floori (const ssef& a) {
#if defined (__SSE4_1__)
    return ssei(floor(a));
#else
    return ssei(a-ssef(0.5f));
#endif
  }

  ////////////////////////////////////////////////////////////////////////////////
  /// Movement/Shifting/Shuffling Functions
  ////////////////////////////////////////////////////////////////////////////////

  __forceinline ssef unpacklo( const ssef& a, const ssef& b ) { return _mm_unpacklo_ps(a.m128, b.m128); }
  __forceinline ssef unpackhi( const ssef& a, const ssef& b ) { return _mm_unpackhi_ps(a.m128, b.m128); }

  template<size_t i0, size_t i1, size_t i2, size_t i3> __forceinline const ssef shuffle( const ssef& b ) {
    return _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(b), _MM_SHUFFLE(i3, i2, i1, i0)));
  }

  template<size_t i0, size_t i1, size_t i2, size_t i3> __forceinline const ssef shuffle( const ssef& a, const ssef& b ) {
    return _mm_shuffle_ps(a, b, _MM_SHUFFLE(i3, i2, i1, i0));
  }

#if defined (__SSSE3__)
  __forceinline const ssef shuffle8(const ssef& a, const ssei& shuf) { 
    return _mm_castsi128_ps(_mm_shuffle_epi8(_mm_castps_si128(a), shuf)); 
  }
#endif

#if defined(__SSE3__)
  template<> __forceinline const ssef shuffle<0, 0, 2, 2>( const ssef& b ) { return _mm_moveldup_ps(b); }
  template<> __forceinline const ssef shuffle<1, 1, 3, 3>( const ssef& b ) { return _mm_movehdup_ps(b); }
  template<> __forceinline const ssef shuffle<0, 1, 0, 1>( const ssef& b ) { return _mm_castpd_ps(_mm_movedup_pd(_mm_castps_pd(b))); }
#endif

  template<size_t i0> __forceinline const ssef shuffle( const ssef& b ) {
    return shuffle<i0,i0,i0,i0>(b);
  }

#if defined (__SSE4_1__) && !defined(__GNUC__)
  template<size_t i> __forceinline float extract   ( const ssef& a ) { return _mm_cvtss_f32(_mm_extract_ps(a,i)); }
#else
  template<size_t i> __forceinline float extract   ( const ssef& a ) { return _mm_cvtss_f32(shuffle<i,i,i,i>(a)); }
#endif
  template<>         __forceinline float extract<0>( const ssef& a ) { return _mm_cvtss_f32(a); }

#if defined (__SSE4_1__)
  template<size_t dst, size_t src, size_t clr> __forceinline const ssef insert( const ssef& a, const ssef& b ) { return _mm_insert_ps(a, b, (dst << 4) | (src << 6) | clr); }
  template<size_t dst, size_t src> __forceinline const ssef insert( const ssef& a, const ssef& b ) { return insert<dst, src, 0>(a, b); }
  template<size_t dst>             __forceinline const ssef insert( const ssef& a, const float b ) { return insert<dst,      0>(a, _mm_set_ss(b)); }
#else
  template<size_t dst>             __forceinline const ssef insert( const ssef& a, const float b ) { ssef c = a; c[dst] = b; return c; }
#endif

  ////////////////////////////////////////////////////////////////////////////////
  /// Transpose
  ////////////////////////////////////////////////////////////////////////////////

  __forceinline void transpose(const ssef& r0, const ssef& r1, const ssef& r2, const ssef& r3, ssef& c0, ssef& c1, ssef& c2, ssef& c3) 
  {
    ssef l02 = unpacklo(r0,r2);
    ssef h02 = unpackhi(r0,r2);
    ssef l13 = unpacklo(r1,r3);
    ssef h13 = unpackhi(r1,r3);
    c0 = unpacklo(l02,l13);
    c1 = unpackhi(l02,l13);
    c2 = unpacklo(h02,h13);
    c3 = unpackhi(h02,h13);
  }

  __forceinline void transpose(const ssef& r0, const ssef& r1, const ssef& r2, const ssef& r3, ssef& c0, ssef& c1, ssef& c2) 
  {
    ssef l02 = unpacklo(r0,r2);
    ssef h02 = unpackhi(r0,r2);
    ssef l13 = unpacklo(r1,r3);
    ssef h13 = unpackhi(r1,r3);
    c0 = unpacklo(l02,l13);
    c1 = unpackhi(l02,l13);
    c2 = unpacklo(h02,h13);
  }

  ////////////////////////////////////////////////////////////////////////////////
  /// Reductions
  ////////////////////////////////////////////////////////////////////////////////

  __forceinline const ssef vreduce_min(const ssef& v) { ssef h = min(shuffle<1,0,3,2>(v),v); return min(shuffle<2,3,0,1>(h),h); }
  __forceinline const ssef vreduce_max(const ssef& v) { ssef h = max(shuffle<1,0,3,2>(v),v); return max(shuffle<2,3,0,1>(h),h); }
  __forceinline const ssef vreduce_add(const ssef& v) { ssef h = shuffle<1,0,3,2>(v)   + v ; return shuffle<2,3,0,1>(h)   + h ; }

  __forceinline float reduce_min(const ssef& v) { return _mm_cvtss_f32(vreduce_min(v)); }
  __forceinline float reduce_max(const ssef& v) { return _mm_cvtss_f32(vreduce_max(v)); }
  __forceinline float reduce_add(const ssef& v) { return _mm_cvtss_f32(vreduce_add(v)); }

  __forceinline size_t select_min(const ssef& v) { return __bsf(movemask(v == vreduce_min(v))); }
  __forceinline size_t select_max(const ssef& v) { return __bsf(movemask(v == vreduce_max(v))); }

  // MoonRay: added namespace scene_rdl2::math
  __forceinline size_t select_min(const sseb& valid, const ssef& v) { const ssef a = select(valid,v,ssef(scene_rdl2::math::pos_inf)); return __bsf(movemask(valid & (a == vreduce_min(a)))); }
  __forceinline size_t select_max(const sseb& valid, const ssef& v) { const ssef a = select(valid,v,ssef(scene_rdl2::math::neg_inf)); return __bsf(movemask(valid & (a == vreduce_max(a)))); }

  ////////////////////////////////////////////////////////////////////////////////
  /// Memory load and store operations
  ////////////////////////////////////////////////////////////////////////////////

  __forceinline ssef load4f( const void* const a ) {
    return _mm_load_ps((float*)const_cast<void*>(a)); 
    // Intel: return _mm_load_ps((float*)a); 
  }

  __forceinline void store4f ( void* ptr, const ssef& v ) {
    _mm_store_ps((float*)ptr,v);
  }

  __forceinline ssef loadu4f( const void* const a ) {
    return _mm_loadu_ps((float*)const_cast<void*>(a));
    // Intel: return _mm_loadu_ps((float*)a);
  }

  __forceinline void storeu4f ( void* ptr, const ssef& v ) {
    _mm_storeu_ps((float*)ptr,v);
  }

  __forceinline void store4f ( const sseb& mask, void* ptr, const ssef& f ) { 
#if defined (__AVX__)
    _mm_maskstore_ps((float*)ptr,(__m128i)mask,f);
#else
    *(ssef*)ptr = select(mask,f,*(ssef*)ptr);
#endif
  }

  __forceinline ssef load4f_nt (void* ptr) {
#if defined (__SSE4_1__)
    return _mm_castsi128_ps(_mm_stream_load_si128((__m128i*)ptr));
#else
    return _mm_load_ps((float*)ptr); 
#endif
  }

  __forceinline void store4f_nt (void* ptr, const ssef& v) {
#if defined (__SSE4_1__)
    _mm_stream_ps((float*)ptr,v);
#else
    _mm_store_ps((float*)ptr,v);
#endif
  }
  
  ////////////////////////////////////////////////////////////////////////////////
  /// Euclidian Space Operators
  ////////////////////////////////////////////////////////////////////////////////

  __forceinline float dot ( const ssef& a, const ssef& b ) {
    return reduce_add(a*b);
  }

  __forceinline ssef cross ( const ssef& a, const ssef& b ) 
  {
    const ssef a0 = a;
    const ssef b0 = shuffle<1,2,0,3>(b);
    const ssef a1 = shuffle<1,2,0,3>(a);
    const ssef b1 = b;
    return shuffle<1,2,0,3>(msub(a0,b0,a1*b1));
  }

#if defined (__INTEL_COMPILER)
  __forceinline
  ssef atan( const ssef& a)
  {
    return _mm_atan_ps(a.m128);
  }

  __forceinline
  ssef atan2( const ssef& a, const ssef& b)
  {
    return _mm_atan2_ps(a.m128, b.m128);
  }
#elif defined(__GNUG__) && !defined(__INTEL_COMPILER)

  // Atan() + Atan2() SSE Intrinsic Emulator
  // Intel provides an atan and atan2 intrinsic in their short vector math library.
  // In order to achieve the same functionality, an emulator was created:
  //
  // https://github.com/xoolive/geodesy/blob/master/src/sse2_math.h
  //
  // This emulator was appropriated from this implementation and modified
  // to achieve compatibility with our ssef type.

  // TODO: This code is under the MIT license:
  // https://github.com/xoolive/geodesy/blob/master/license.txt

  #define ALIGN16_BEG
  #define ALIGN16_END __attribute__((aligned(16)))

  // Masks
  static const ALIGN16_BEG int _mm_cst_sign_mask[4] ALIGN16_END =
  { static_cast<int>(0x80000000), static_cast<int>(0x80000000), static_cast<int>(0x80000000), static_cast<int>(0x80000000) };

  // Numerical constants
  static const __m128 _mm_cst_zero = _mm_setzero_ps();
  static const __m128 _mm_cst_one  = _mm_set1_ps( 1.0f);
  static const __m128 _mm_cst_mone = _mm_set1_ps(-1.0f);
  static const __m128 _mm_cst_two  = _mm_set1_ps( 2.0f);
  static const __m128 _mm_cst_mtwo = _mm_set1_ps(-2.0f);

  // Tangent-based numerical constants
  static const __m128 _mm_cst_tan3pio8 = _mm_set1_ps(2.414213562373095f);
  static const __m128 _mm_cst_tanpio8  = _mm_set1_ps(0.4142135623730950f);

  // Pi-based numerical constants
  static const __m128 _mm_cst_pi    = _mm_set1_ps(3.14159265358979f);
  static const __m128 _mm_cst_mpi   = _mm_set1_ps(-3.14159265358979f);
  static const __m128 _mm_cst_pio2  = _mm_set1_ps(1.5707963267948966f);
  static const __m128 _mm_cst_mpio2 = _mm_set1_ps(-1.5707963267948966f);
  static const __m128 _mm_cst_pio4  = _mm_set1_ps(0.7853981633974483f);

  // Minimax coefficients
  static const __m128 _mm_cst_atancof_p0 = _mm_set1_ps( 8.05374449538e-2f);
  static const __m128 _mm_cst_atancof_p1 = _mm_set1_ps(-1.38776856032e-1f);
  static const __m128 _mm_cst_atancof_p2 = _mm_set1_ps( 1.99777106478e-1f);
  static const __m128 _mm_cst_atancof_p3 = _mm_set1_ps(-3.33329491539e-1f);

  __forceinline
  ssef atan( __m128 x)
  {
    // Figure out if x is negative and if so flip it
    __m128 signbit = _mm_and_ps(x, *(const __m128*) _mm_cst_sign_mask); // Determine the signbit for each element
    x = _mm_andnot_ps(*(const __m128*) _mm_cst_sign_mask, x);           // Get the absolute value

    __m128 y = _mm_cst_zero;                            // y = <0, 0, 0, 0, 0, 0, 0, 0>

    // Range Reduction
    __m128 x2 = _mm_div_ps(_mm_cst_mone, x);            // x2[n] = (1/x[n])
    __m128 x3 = _mm_div_ps(_mm_sub_ps(x, _mm_cst_one),
                           _mm_add_ps(x, _mm_cst_one)); // x3[n] = (x[n] - 1)/(x[n] + 1)

    // if (x > tan(3*pi/8) {
    //     x = x2;
    //     y = pi/2;
    // }
    __m128 mask = _mm_cmp_ps(x, _mm_cst_tan3pio8, _CMP_GT_OQ); // mask[n] = (x[n] > 2.4142)
    x = _mm_blendv_ps(x, x2, mask);
    y = _mm_blendv_ps(y, _mm_cst_pio2, mask);

    // If (x > tan(pi/8) {
    //     x = x3;
    //     y = pi/4;
    // }
    mask = _mm_cmp_ps(x, _mm_cst_tanpio8, _CMP_GT_OQ); // mask[n] = (x[n] > 0.4142)
    x = _mm_blendv_ps(x, x3, mask);
    y = _mm_blendv_ps(y, _mm_cst_pio4, mask);

    // Polynomial Computation
    __m128 z = _mm_mul_ps(x, x);                    // z = x^2
    __m128 num;

#if defined (__FMA__)
    num = _mm_fmadd_ps(z, _mm_cst_atancof_p0, _mm_cst_atancof_p1); // num = (p0 * x^2) + p1
    num = _mm_fmadd_ps(num, z, _mm_cst_atancof_p2);                // num = (p0 * x^4) + (p1 * x^2) + p2
    num = _mm_fmadd_ps(num, z, _mm_cst_atancof_p3);                // num = (p0 * x^6) + (p1 * x^4) + (p2 * x^2) + p3
    num = _mm_mul_ps(num, z);                                      // num = (p0 * x^8) + (p1 * x^6) + (p2 * x^4) + (p3 * x^2)
    num = _mm_fmadd_ps(num, x, x);                                 // num = (p0 * x^9) + (p1 * x^7) + (p2 * x^5) + (p3 * x^3) + x
#else
    num = _mm_mul_ps(_mm_cst_atancof_p0, z);   // num = p0 * x^2
    num = _mm_add_ps(num, _mm_cst_atancof_p1); // num = (p0 * x^2) + p1
    num = _mm_mul_ps(num, z);                  // num = (p0 * x^4) + (p1 * x^2)
    num = _mm_add_ps(num, _mm_cst_atancof_p2); // num = (p0 * x^4) + (p1 * x^2) + p2
    num = _mm_mul_ps(num, z);                  // num = (p0 * x^6) + (p1 * x^4) + (p2 * x^2)
    num = _mm_add_ps(num, _mm_cst_atancof_p3); // num = (p0 * x^6) + (p1 * x^4) + (p2 * x^2) + p3
    num = _mm_mul_ps(num, z);                  // num = (p0 * x^8) + (p1 * x^6) + (p2 * x^4) + (p3 * x^2)
    num = _mm_mul_ps(num, x);                  // num = (p0 * x^9) + (p1 * x^7) + (p2 * x^5) + (p3 * x^3)
    num = _mm_add_ps(num, x);                  // num = (p0 * x^9) + (p1 * x^7) + (p2 * x^5) + (p3 * x^3) + x
#endif

    y = _mm_add_ps(y, num);     // y += num
    y = _mm_xor_ps(y, signbit); // y *= -1 (maybe)

    return y;
  }

  __forceinline
  ssef atan2 (const ssef& y, const ssef& x) {
    __m128 w    = _mm_blendv_ps(_mm_cst_pi, _mm_cst_mpi, y);
    w = _mm_blendv_ps(_mm_cst_zero, w, x);

    __m128 q = _mm_div_ps(y, x); // q = y/x - Possible NaN, handled via masks
    q = _mm_add_ps(w, atan(q));  // q = w + atan(q)

    __m128 mask;  // x-dependent mask
    __m128 mask2; // y-dependent mask

    // atan2(-y, 0) -> -pi/2
    mask  = _mm_cmp_ps(x, _mm_cst_zero, _CMP_EQ_OQ);                   // mask[n] = (x[n] == 0)
    mask2 = _mm_and_ps(mask, _mm_cmp_ps(y, _mm_cst_zero, _CMP_LT_OQ)); // mask2[n] = (mask[n] & (y[n] < 0))
    q     = _mm_blendv_ps(q, _mm_cst_mpio2, mask2);

    // atan2(+y, 0) -> pi/2
    mask2 = _mm_and_ps(mask, _mm_cmp_ps(y, _mm_cst_zero, _CMP_GT_OQ)); // mask2[n] = (mask[n] & (y[n] > 0))
    q     = _mm_blendv_ps(q, _mm_cst_pio2, mask2);

    // atan2(0, 0) -> 0
    mask2 = _mm_and_ps(mask, _mm_cmp_ps(y, _mm_cst_zero, _CMP_EQ_OQ)); // mask2[n] = (mask & y[n] == 0)
    q     = _mm_blendv_ps(q, _mm_cst_zero, mask2);

    // atan2(0, -x) -> pi
    mask  = _mm_cmp_ps(x, _mm_cst_zero, _CMP_LT_OQ);                   // mask[n] = (x[n] < 0)
    mask2 = _mm_and_ps(mask, _mm_cmp_ps(y, _mm_cst_zero, _CMP_EQ_OQ)); // mask2[n] = (mask[n] & y[n] == 0)
    q     = _mm_blendv_ps(q, _mm_cst_pi, mask2);

    return q;
  }

#endif

  ////////////////////////////////////////////////////////////////////////////////
  /// Output Operators
  ////////////////////////////////////////////////////////////////////////////////

  inline std::ostream& operator<<(std::ostream& cout, const ssef& a) {
    return cout << "<" << a[0] << ", " << a[1] << ", " << a[2] << ", " << a[3] << ">";
  }

}

