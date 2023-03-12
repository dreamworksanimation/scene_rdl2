// Copyright 2023 DreamWorks Animation LLC and Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Math.h"
// Intel: #include "math/math.h"

/* include SSE emulation for Xeon Phi */
#if defined (__MIC__)
#include "sse_mic.h"
#include "mic.h"
// Intel: #  include "simd/sse_mic.h"
// Intel: #  include "simd/mic.h"
#endif

/* include SSE wrapper classes */
#if defined(__SSE__)
#  include "sse.h"
// Intel: #  include "simd/sse.h"
#endif

/* include AVX wrapper classes */
#if defined(__AVX__)
#include "avx.h"
// Intel: #include "simd/avx.h"
#endif

#if defined (__AVX__)
#define AVX_ZERO_UPPER() _mm256_zeroupper()
#else
#define AVX_ZERO_UPPER()
#endif

