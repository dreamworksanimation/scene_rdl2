// Copyright 2023-2024 DreamWorks Animation LLC and Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <scene_rdl2/common/platform/Platform.h>
#include <scene_rdl2/common/platform/Intrinsics.h>
// Intel: #include "sys/platform.h"
// Intel: #include "sys/intrinsics.h"

#include <immintrin.h>

namespace simd {
// Intel: namespace embree {
  struct sseb_t;
  struct ssei_t;
  struct ssef_t;
  struct sseb_m;
  struct ssei_m;
  struct ssef_m;
}

#include "sseb_mic.h"
#include "ssei_mic.h"
#include "ssef_mic.h"
// Intel:  #include "simd/sseb_mic.h"
// Intel:  #include "simd/ssei_mic.h"
// Intel:  #include "simd/ssef_mic.h"

namespace simd {
// Intel: namespace embree {
  typedef sseb_m sseb;
  typedef ssei_m ssei;
  typedef ssef_m ssef;

__forceinline const ssei cast( const ssef& a ) { return *(ssei*)&a; }
__forceinline const ssef cast( const ssei& a ) { return *(ssef*)&a; }

 __forceinline ssei_t floor_i( const ssef_t& other ) 
 { 
   const __m512i m512 = _mm512_cvtfxpnt_round_adjustps_epi32(other.m512,_MM_FROUND_FLOOR,_MM_EXPADJ_NONE); 
   return ssei_t(m512);
 }

}

