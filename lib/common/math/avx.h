// Copyright 2023-2024 DreamWorks Animation LLC and Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "sse.h"
// Intel: #include "simd/sse.h"

namespace simd
// Intel: namespace embree 
{
  struct avxb;
  struct avxi;
  struct avxf;
}

#include "avxb.h"
// Intel: #include "simd/avxb.h"
#if defined (__AVX2__)
#include "avxi.h"
// Intel: #include "simd/avxi.h"
#else
#include "avxi_emu.h"
// Intel: #include "simd/avxi_emu.h"
#endif
#include "avxf.h"
// Intel: #include "simd/avxf.h"

