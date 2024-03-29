// Copyright 2023-2024 DreamWorks Animation LLC and Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

// MoonRay: added
#include <scene_rdl2/common/platform/Platform.h>

namespace simd
// Intel: namespace embree
{
   /*! 16-wide MIC bool type. */
  class SCENE_RDL2_SIMD_ALIGN mic_m
// Intel: class mic_m
  {
  public:
    __mmask v;
    
    ////////////////////////////////////////////////////////////////////////////////
    /// Constructors, Assignment & Cast Operators
    ////////////////////////////////////////////////////////////////////////////////
    
    __forceinline mic_m () {};
    __forceinline mic_m (const mic_m &t) { v = t.v; };
    __forceinline mic_m& operator=(const mic_m &f) { v = f.v; return *this; };

    __forceinline mic_m (const __mmask &t) { v = t; };
    __forceinline operator __mmask () const { return v; };
    
    __forceinline mic_m(bool b) { v = b ? 0xFFFF : 0x0000; };
    __forceinline mic_m(int t ) { v = (__mmask)t; };
    __forceinline mic_m(unsigned int t ) { v = (__mmask)t; };

    ////////////////////////////////////////////////////////////////////////////////
    /// Constants
    ////////////////////////////////////////////////////////////////////////////////

    // MoonRay: added util namespace
    __forceinline mic_m( util::FalseTy ) : v(0x0000) {}
    __forceinline mic_m( util::TrueTy  ) : v(0xffff) {}

    static unsigned int shift1[32];
  };

  ////////////////////////////////////////////////////////////////////////////////
  /// Unary Operators
  ////////////////////////////////////////////////////////////////////////////////
  
   __forceinline mic_m operator!(const mic_m &a) { return _mm512_knot(a); }
  
   ////////////////////////////////////////////////////////////////////////////////
   /// Binary Operators
   ////////////////////////////////////////////////////////////////////////////////
  
  __forceinline mic_m operator&(const mic_m &a, const mic_m &b) { return _mm512_kand(a,b); };
  __forceinline mic_m operator|(const mic_m &a, const mic_m &b) { return _mm512_kor(a,b); };
  __forceinline mic_m operator^(const mic_m &a, const mic_m &b) { return _mm512_kxor(a,b); };

    __forceinline mic_m andn(const mic_m &a, const mic_m &b) { return _mm512_kandn(b,a); }
  
  ////////////////////////////////////////////////////////////////////////////////
  /// Assignment Operators
  ////////////////////////////////////////////////////////////////////////////////
  
  __forceinline const mic_m operator &=( mic_m& a, const mic_m& b ) { return a = a & b; }
  __forceinline const mic_m operator |=( mic_m& a, const mic_m& b ) { return a = a | b; }
  __forceinline const mic_m operator ^=( mic_m& a, const mic_m& b ) { return a = a ^ b; }
  
  ////////////////////////////////////////////////////////////////////////////////
  /// Comparison Operators + Select
  ////////////////////////////////////////////////////////////////////////////////
  
  __forceinline const mic_m operator !=( const mic_m& a, const mic_m& b ) { return _mm512_kxor(a, b); }
  __forceinline const mic_m operator ==( const mic_m& a, const mic_m& b ) { return _mm512_kxnor(a, b); }
  
  __forceinline mic_m select (const mic_m &s, const mic_m &a, const mic_m &b) {
    return _mm512_kor(_mm512_kand(s,a),_mm512_kandn(s,b));
  }

  ////////////////////////////////////////////////////////////////////////////////
  /// Reduction Operations
  ////////////////////////////////////////////////////////////////////////////////
  
  __forceinline int all(const mic_m &a)  { return  _mm512_kortestc(a,a) != 0; }
  __forceinline int any(const mic_m &a)  { return  _mm512_kortestz(a,a) == 0; }
  __forceinline int none(const mic_m &a) { return  _mm512_kortestz(a,a) != 0; }
  
  __forceinline size_t movemask( const mic_m& a ) { return _mm512_kmov(a); }
  __forceinline size_t popcnt  ( const mic_m& a ) { return _mm_countbits_32(a.v); }
  
  ////////////////////////////////////////////////////////////////////////////////
  /// Convertion Operations
  ////////////////////////////////////////////////////////////////////////////////

  __forceinline unsigned int toInt (const mic_m &a) { return _mm512_mask2int(a); };
  __forceinline mic_m        toMask(const int &a)   { return _mm512_int2mask(a); };

  ////////////////////////////////////////////////////////////////////////////////
  /// Output Operators
  ////////////////////////////////////////////////////////////////////////////////
  
  inline std::ostream& operator<<(std::ostream& cout, const mic_m& a) 
  {
    cout << "<";
    for (size_t i=0; i<16; i++) {
      if ((a.v >> i) & 1) cout << "1"; else cout << "0";
    }
    return cout << ">";
  }
}

