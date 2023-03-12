// Copyright 2023 DreamWorks Animation LLC and Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

// MoonRay added
#include "Math.h"

namespace simd
// Intel: namespace embree
{
  /* memory representation as 4 aligned ints */
  struct __align(16) ssei_m  // Intel: __aligned(16)
  {
    typedef sseb_m Mask;
    typedef ssei_m Int;
    typedef ssef_m Float;
    enum { size = 4 };  // number of SIMD elements
    int i[4]; 
    
    ////////////////////////////////////////////////////////////////////////////////
    /// Constructors, Assignment & Cast Operators
    ////////////////////////////////////////////////////////////////////////////////
  
    __forceinline ssei_m ( ) {}
    __forceinline ssei_m ( int a                     ) { i[0] = a;  i[1] = a; i[2] = a; i[3] = a; }
    __forceinline ssei_m ( int a, int b, int c, int d) { i[0] = a;  i[1] = b; i[2] = c; i[3] = d; }

    __forceinline ssei_m           ( const ssei_t& other ); 
    __forceinline ssei_m& operator=( const ssei_t& other );

    ////////////////////////////////////////////////////////////////////////////////
    /// Constants
    ////////////////////////////////////////////////////////////////////////////////

    // MoonRay: added math namespace
    __forceinline ssei_m( math::ZeroTy   ) { i[0] = i[1] = i[2] = i[3] = math::zero; }
    __forceinline ssei_m( math::OneTy    ) { i[0] = i[1] = i[2] = i[3] = math::one; }
    __forceinline ssei_m( math::NegInfTy ) { i[0] = i[1] = i[2] = i[3] = math::neg_inf; }
    __forceinline ssei_m( math::PosInfTy ) { i[0] = i[1] = i[2] = i[3] = math::pos_inf; }
    __forceinline ssei_m( math::StepTy   ) { i[0] = 0; i[1] = 1; i[2] = 2; i[3] = 3; }

    ////////////////////////////////////////////////////////////////////////////////
    /// Array Access
    ////////////////////////////////////////////////////////////////////////////////
    
    // MoonRay: changed assert() to MNRY_ASSERT()
    __forceinline const int& operator []( const size_t index ) const { MNRY_ASSERT(index < 4); return i[index]; }
    __forceinline       int& operator []( const size_t index )       { MNRY_ASSERT(index < 4); return i[index]; }
  };
    
  /*! 4-wide SSE integer type emulated with 16-wide vectors. */
  struct __aligned(64) ssei_t 
  {
    union {
      __m512i m512; 
      float  f[4];
      int    i[4];
    };
    
    ////////////////////////////////////////////////////////////////////////////////
    /// Constructors, Assignment & Cast Operators
    ////////////////////////////////////////////////////////////////////////////////
    
    __forceinline ssei_t( const __m512i a ) : m512(a) {}
    __forceinline ssei_t           ( const ssei_t& other ) { m512 = other.m512; }
    __forceinline ssei_t& operator=( const ssei_t& other ) { m512 = other.m512; return *this; }
        

    __forceinline operator const __m512i&( void ) const { return m512; }
    __forceinline operator       __m512i&( void )       { return m512; }

    __forceinline ssei_t ( const ssef_t& other );
    
  public:
    __forceinline explicit ssei_t ( int  a ) 
      : m512(_mm512_set1_epi32(a)) {}

    // MoonRay: added math namespace
    __forceinline explicit ssei_t ( math::PosInfTy )
      : m512(_mm512_set1_epi32(math::pos_inf)) {}

    // MoonRay: added math namespace
    __forceinline explicit ssei_t ( math::NegInfTy )
      : m512(_mm512_set1_epi32(math::neg_inf)) {}

    __forceinline ssei_t ( const ssei_m& other ) { 
      m512 = _mm512_extload_epi32(&other,_MM_UPCONV_EPI32_NONE,_MM_BROADCAST_4X16,_MM_HINT_NONE); 
    }
  };

  ////////////////////////////////////////////////////////////////////////////////
  /// Assignment Operators
  ////////////////////////////////////////////////////////////////////////////////

  __forceinline ssei_m::ssei_m ( const ssei_t& other ) { 
    _mm512_mask_extpackstorelo_epi32(this,0xf,other.m512,_MM_DOWNCONV_EPI32_NONE, _MM_HINT_NONE); 
  }
  
  __forceinline ssei_m& ssei_m::operator=( const ssei_t& other ) { 
    _mm512_mask_extpackstorelo_epi32(this,0xf,other.m512,_MM_DOWNCONV_EPI32_NONE, _MM_HINT_NONE); 
    return *this; 
  }
  
  ////////////////////////////////////////////////////////////////////////////////
  /// Unary Operators
  ////////////////////////////////////////////////////////////////////////////////
  
  __forceinline const ssei_t operator +( const ssei_t& a ) { return a; }
  __forceinline const ssei_t operator -( const ssei_t& a ) { return _mm512_sub_epi32(_mm512_setzero_epi32(), a.m512); }
  
  ////////////////////////////////////////////////////////////////////////////////
  /// Binary Operators
  ////////////////////////////////////////////////////////////////////////////////
  
  __forceinline const ssei_t operator +( const ssei_t& a, const ssei_t& b ) { return _mm512_add_epi32(a.m512, b.m512); }
  __forceinline const ssei_t operator +( const ssei_t& a, const int32&  b ) { return a + ssei_t(b); }
  __forceinline const ssei_t operator +( const int32&  a, const ssei_t& b ) { return ssei_t(a) + b; }
  
  __forceinline const ssei_t operator -( const ssei_t& a, const ssei_t& b ) { return _mm512_sub_epi32(a.m512, b.m512); }
  __forceinline const ssei_t operator -( const ssei_t& a, const int32&  b ) { return a - ssei_t(b); }
  __forceinline const ssei_t operator -( const int32&  a, const ssei_t& b ) { return ssei_t(a) - b; }
  
  __forceinline const ssei_t operator *( const ssei_t& a, const ssei_t& b ) { return _mm512_mullo_epi32(a.m512, b.m512); }
  __forceinline const ssei_t operator *( const ssei_t& a, const int32&  b ) { return a * ssei_t(b); }
  __forceinline const ssei_t operator *( const int32&  a, const ssei_t& b ) { return ssei_t(a) * b; }
  
  __forceinline const ssei_t operator &( const ssei_t& a, const ssei_t& b ) { return _mm512_and_epi32(a.m512, b.m512); }
  __forceinline const ssei_t operator &( const ssei_t& a, const int32&  b ) { return a & ssei_t(b); }
  __forceinline const ssei_t operator &( const int32& a, const ssei_t& b ) { return ssei_t(a) & b; }
  
  __forceinline const ssei_t operator |( const ssei_t& a, const ssei_t& b ) { return _mm512_or_epi32(a.m512, b.m512); }
  __forceinline const ssei_t operator |( const ssei_t& a, const int32&  b ) { return a | ssei_t(b); }
  __forceinline const ssei_t operator |( const int32& a, const ssei_t& b ) { return ssei_t(a) | b; }
  
  __forceinline const ssei_t operator ^( const ssei_t& a, const ssei_t& b ) { return _mm512_xor_epi32(a.m512, b.m512); }
  __forceinline const ssei_t operator ^( const ssei_t& a, const int32&  b ) { return a ^ ssei_t(b); }
  __forceinline const ssei_t operator ^( const int32& a, const ssei_t& b ) { return ssei_t(a) ^ b; }
  
  __forceinline const ssei_t operator <<( const ssei_t& a, const int32& n ) { return _mm512_slli_epi32(a.m512, n); }
  __forceinline const ssei_t operator >>( const ssei_t& a, const int32& n ) { return _mm512_srai_epi32(a.m512, n); }
  
  __forceinline const ssei_t sra ( const ssei_t& a, const int32& b ) { return _mm512_srai_epi32(a.m512, b); }
  __forceinline const ssei_t srl ( const ssei_t& a, const int32& b ) { return _mm512_srli_epi32(a.m512, b); }
  
  __forceinline const ssei_t min( const ssei_t& a, const ssei_t& b ) { return _mm512_min_epi32(a.m512, b.m512); }
  __forceinline const ssei_t min( const ssei_t& a, const int32&  b ) { return min(a,ssei_t(b)); }
  __forceinline const ssei_t min( const int32&  a, const ssei_t& b ) { return min(ssei_t(a),b); }
  
  __forceinline const ssei_t max( const ssei_t& a, const ssei_t& b ) { return _mm512_max_epi32(a.m512, b.m512); }
  __forceinline const ssei_t max( const ssei_t& a, const int32&  b ) { return max(a,ssei_t(b)); }
  __forceinline const ssei_t max( const int32&  a, const ssei_t& b ) { return max(ssei_t(a),b); }
  
  ////////////////////////////////////////////////////////////////////////////////
  /// Assignment Operators
  ////////////////////////////////////////////////////////////////////////////////
  
  __forceinline ssei_t& operator +=( ssei_t& a, const ssei_t& b ) { return a = a + b; }
  __forceinline ssei_t& operator +=( ssei_t& a, const int32&  b ) { return a = a + b; }
  
  __forceinline ssei_t& operator -=( ssei_t& a, const ssei_t& b ) { return a = a - b; }
  __forceinline ssei_t& operator -=( ssei_t& a, const int32&  b ) { return a = a - b; }
  
  __forceinline ssei_t& operator *=( ssei_t& a, const ssei_t& b ) { return a = a * b; }
  __forceinline ssei_t& operator *=( ssei_t& a, const int32&  b ) { return a = a * b; }
  
  __forceinline ssei_t& operator &=( ssei_t& a, const ssei_t& b ) { return a = a & b; }
  __forceinline ssei_t& operator &=( ssei_t& a, const int32&  b ) { return a = a & b; }
  
  __forceinline ssei_t& operator |=( ssei_t& a, const ssei_t& b ) { return a = a | b; }
  __forceinline ssei_t& operator |=( ssei_t& a, const int32&  b ) { return a = a | b; }
  
  __forceinline ssei_t& operator <<=( ssei_t& a, const int32&  b ) { return a = a << b; }
  __forceinline ssei_t& operator >>=( ssei_t& a, const int32&  b ) { return a = a >> b; }
  
  ////////////////////////////////////////////////////////////////////////////////
  /// Comparison Operators + Select
  ////////////////////////////////////////////////////////////////////////////////
  
  __forceinline const sseb_t operator ==( const ssei_t& a, const ssei_t& b ) { return _mm512_cmp_epi32_mask(a,b,_MM_CMPINT_EQ); }
  __forceinline const sseb_t operator ==( const ssei_t& a, const int32& b ) { return a == ssei_t(b); }
  __forceinline const sseb_t operator ==( const int32& a, const ssei_t& b ) { return ssei_t(a) == b; }
  
  __forceinline const sseb_t operator !=( const ssei_t& a, const ssei_t& b ) { return _mm512_cmp_epi32_mask(a,b,_MM_CMPINT_NE); }
  __forceinline const sseb_t operator !=( const ssei_t& a, const int32& b ) { return a != ssei_t(b); }
  __forceinline const sseb_t operator !=( const int32& a, const ssei_t& b ) { return ssei_t(a) != b; }
  
  __forceinline const sseb_t operator < ( const ssei_t& a, const ssei_t& b ) { return _mm512_cmp_epi32_mask(a,b,_MM_CMPINT_LT); }
  __forceinline const sseb_t operator < ( const ssei_t& a, const int32& b ) { return a <  ssei_t(b); }
  __forceinline const sseb_t operator < ( const int32& a, const ssei_t& b ) { return ssei_t(a) <  b; }
  
  __forceinline const sseb_t operator >=( const ssei_t& a, const ssei_t& b ) { return _mm512_cmp_epi32_mask(a,b,_MM_CMPINT_GE); }
  __forceinline const sseb_t operator >=( const ssei_t& a, const int32& b ) { return a >= ssei_t(b); }
  __forceinline const sseb_t operator >=( const int32& a, const ssei_t& b ) { return ssei_t(a) >= b; }
  
  __forceinline const sseb_t operator > ( const ssei_t& a, const ssei_t& b ) { return _mm512_cmp_epi32_mask(a,b,_MM_CMPINT_GT); }
  __forceinline const sseb_t operator > ( const ssei_t& a, const int32& b ) { return a >  ssei_t(b); }
  __forceinline const sseb_t operator > ( const int32& a, const ssei_t& b ) { return ssei_t(a) >  b; }
  
  __forceinline const sseb_t operator <=( const ssei_t& a, const ssei_t& b ) { return _mm512_cmp_epi32_mask(a,b,_MM_CMPINT_LE); }
  __forceinline const sseb_t operator <=( const ssei_t& a, const int32& b ) { return a <= ssei_t(b); }
  __forceinline const sseb_t operator <=( const int32& a, const ssei_t& b ) { return ssei_t(a) <= b; }
  
  __forceinline const ssei_t select( const sseb_t& mask, const ssei_t& t, const ssei_t& f ) { return _mm512_mask_blend_epi32(mask.v, f, t); }
  __forceinline const ssei_t select( const sseb_t& mask, const int     t, const ssei_t& f ) { return _mm512_mask_blend_epi32(mask.v, f, ssei_t(t)); }
  __forceinline const ssei_t select( const sseb_t& mask, const ssei_t& t, const int     f ) { return _mm512_mask_blend_epi32(mask.v, ssei_t(f), t); }
   
  
  ////////////////////////////////////////////////////////////////////////////////
  // Movement/Shifting/Shuffling Functions
  ////////////////////////////////////////////////////////////////////////////////
    
  template<size_t i0, size_t i1, size_t i2, size_t i3> __forceinline const ssei_t shuffle( const ssei_t& a ) {
    return _mm512_permute4f128_epi32(a, (_MM_PERM_ENUM)_MM_SHUFFLE(i3, i2, i1, i0));
  }
  
  template<size_t i0> __forceinline const ssei_m shuffle( const ssei_m& b ) {
    return shuffle<i0,i0,i0,i0>(b);
  }
  
  template<size_t src> __forceinline int    extract( const ssei_t& a                ) { return a.i[src]; }
  template<size_t dst> __forceinline ssei_t insert ( const ssei_t& a, const float b ) { ssei_t c = a; c.i[dst] = b; return c; }
  
  ////////////////////////////////////////////////////////////////////////////////
  /// Reductions
  ////////////////////////////////////////////////////////////////////////////////
  
  __forceinline const ssei_t vreduce_min(const ssei_t& v) { ssei_t h = min(shuffle<1,0,3,2>(v),v); return min(shuffle<2,3,0,1>(h),h); }
  __forceinline const ssei_t vreduce_max(const ssei_t& v) { ssei_t h = max(shuffle<1,0,3,2>(v),v); return max(shuffle<2,3,0,1>(h),h); }
  __forceinline const ssei_t vreduce_add(const ssei_t& v) { ssei_t h = shuffle<1,0,3,2>(v)   + v ; return shuffle<2,3,0,1>(h)   + h ; }
  
  __forceinline int reduce_min(const ssei_t& v) { return extract<0>(vreduce_min(v)); }
  __forceinline int reduce_max(const ssei_t& v) { return extract<0>(vreduce_max(v)); }
  __forceinline int reduce_add(const ssei_t& v) { return extract<0>(vreduce_add(v)); }
  
  __forceinline size_t select_min(const ssei_t& v) { return bitscan(movemask(v == vreduce_min(v))); }
  __forceinline size_t select_max(const ssei_t& v) { return bitscan(movemask(v == vreduce_max(v))); }
  
  // MoonRay: added math namespace
  __forceinline size_t select_min(const sseb_t& valid, const ssei_t& v) { const ssei_t a = select(valid,v,ssei_t(math::pos_inf)); return bitscan(movemask(valid & (a == vreduce_min(a)))); }
  __forceinline size_t select_max(const sseb_t& valid, const ssei_t& v) { const ssei_t a = select(valid,v,ssei_t(math::neg_inf)); return bitscan(movemask(valid & (a == vreduce_max(a)))); }
  

  ////////////////////////////////////////////////////////////////////////////////
  /// Memory load and store operations
  ////////////////////////////////////////////////////////////////////////////////

  __forceinline ssei_t load4i( const void* const ptr ) { 
    return _mm512_extload_epi32(ptr,_MM_UPCONV_EPI32_NONE,_MM_BROADCAST_4X16,_MM_HINT_NONE); 
  }

  __forceinline void store4i(void* ptr, const ssei_t& v) {
    // MoonRay: changed assert() to MNRY_ASSERT()
    MNRY_ASSERT((size_t)ptr % 16 == 0);
    _mm512_mask_extpackstorelo_epi32(ptr,0xf,v,_MM_DOWNCONV_EPI32_NONE, _MM_HINT_NONE); 
  }

  __forceinline void storeu4i(void* ptr, const ssei_t& v) {
    _mm512_mask_extpackstorelo_epi32(ptr,0xf,v,_MM_DOWNCONV_EPI32_NONE, _MM_HINT_NONE); 
    _mm512_mask_extpackstorehi_epi32(ptr,0xf,v,_MM_DOWNCONV_EPI32_NONE, _MM_HINT_NONE); 
  }
  
  __forceinline void store4i( const sseb_t& mask, void* ptr, const ssei_t& i ) { 
    store4i(ptr,select(mask,i,*(ssei_m*)ptr));
  }

  __forceinline void store4i( const sseb_t& mask, void* ptr, const int i ) { 
    store4i(ptr,select(mask,(ssei_t)i,*(ssei_m*)ptr));
  }

  __forceinline ssei_t load4i_nt (void* ptr) { 
    return _mm512_extload_epi32(ptr,_MM_UPCONV_EPI32_NONE,_MM_BROADCAST_4X16,_MM_HINT_NT); 
  }

  __forceinline void store4i_nt(void* ptr, const ssei_t& v) { 
    // MoonRay: changed assert() to MNRY_ASSERT()
    MNRY_ASSERT((size_t)ptr % 16 == 0);
    _mm512_mask_extpackstorelo_epi32(ptr,0xf,v,_MM_DOWNCONV_EPI32_NONE, _MM_HINT_NT); 
  }


  ////////////////////////////////////////////////////////////////////////////////
  /// Output Operators
  ////////////////////////////////////////////////////////////////////////////////
  
  inline std::ostream& operator<<(std::ostream& cout, const ssei_m& a) {
    return cout << "<" << a[0] << ", " << a[1] << ", " << a[2] << ", " << a[3] << ">";
  }
}

