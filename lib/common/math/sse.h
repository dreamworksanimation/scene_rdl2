// Copyright 2023-2024 DreamWorks Animation LLC and Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

// MoonRay added includes
#include <scene_rdl2/common/platform/Platform.h>
#include <scene_rdl2/common/platform/Intrinsics.h>

// Intel: #include "sys/platform.h"
// Intel: #include "sys/intrinsics.h"
// Intel: #include "sse_special.h"

/* Workaround for Compiler bug in VS2008 */
#if !defined(__SSE4_1__) || defined(_MSC_VER) && (_MSC_VER < 1600) && !defined(__INTEL_COMPILER)
  #define _mm_blendv_ps __emu_mm_blendv_ps
  __forceinline __m128 _mm_blendv_ps( __m128 f, __m128 t, __m128 mask ) { 
    return _mm_or_ps(_mm_and_ps(mask, t), _mm_andnot_ps(mask, f)); 
  }
#endif

/* Workaround for Compiler bug in VS2008 */
#if defined(_MSC_VER) && (_MSC_VER < 1600) && !defined(__INTEL_COMPILER)
  #define _mm_extract_epi32 __emu_mm_extract_epi32
  __forceinline int _mm_extract_epi32( __m128i input, const int i ) {
    return input.m128i_i32[i];
  }
#endif

namespace simd
// Intel: namespace embree
{
  extern const __m128 _mm_lookupmask_ps[16];

  struct sseb;
  struct ssei;
  struct ssef;

#if !defined(__MIC__)
  typedef ssef ssef_t;
  typedef ssei ssei_t;

  typedef ssef ssef_m;
  typedef ssei ssei_m;
#endif
}

#include "sseb.h"
#include "ssei.h"
#include "ssef.h"
// Intel: #include "simd/sseb.h"
// Intel: #include "simd/ssei.h"
// Intel: #include "simd/ssef.h"

