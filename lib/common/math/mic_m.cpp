// Copyright 2023-2024 DreamWorks Animation LLC and Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "mic.h"

namespace simd
// Intel: namespace embree
{

  __aligned(64) unsigned int mic_m::shift1[32] = {
    ((unsigned int)1 << 0),
    ((unsigned int)1 << 1),
    ((unsigned int)1 << 2),
    ((unsigned int)1 << 3),
    ((unsigned int)1 << 4),
    ((unsigned int)1 << 5),
    ((unsigned int)1 << 6),
    ((unsigned int)1 << 7),
    ((unsigned int)1 << 8),
    ((unsigned int)1 << 9),
    ((unsigned int)1 << 10),
    ((unsigned int)1 << 11),
    ((unsigned int)1 << 12),
    ((unsigned int)1 << 13),
    ((unsigned int)1 << 14),
    ((unsigned int)1 << 15),
    ((unsigned int)1 << 16),
    ((unsigned int)1 << 17),
    ((unsigned int)1 << 18),
    ((unsigned int)1 << 19),
    ((unsigned int)1 << 20),
    ((unsigned int)1 << 21),
    ((unsigned int)1 << 22),
    ((unsigned int)1 << 23),
    ((unsigned int)1 << 24),
    ((unsigned int)1 << 25),
    ((unsigned int)1 << 26),
    ((unsigned int)1 << 27),
    ((unsigned int)1 << 28),
    ((unsigned int)1 << 29),
    ((unsigned int)1 << 30),
    ((unsigned int)1 << 31)
  };

};

