// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "Math.h"
#include "Transcendental.h"

namespace scene_rdl2 {
namespace math {


// Minimax coefficients computed by Mathematica
const float c0_acos =  1.5707963039207744f;
const float c1_acos = -0.2145986966117427f;
const float c2_acos =  0.08897731218991513f;
const float c3_acos = -0.05016418844436275f;
const float c4_acos =  0.030862751448600963f;
const float c5_acos = -0.017045102007693682f;
const float c6_acos =  0.006638618338665405f;
const float c7_acos = -0.0012534570550072417f;

// A couple of tweaked values to reduce evaluation error.
// The resulting error is at most 2 ULP. It may be possible to improve on this by further tweaking.
const float c0_acos_tweaked = 1.5707964;
const float c3_acos_tweaked = -0.0501641f;

float dw_acos(const float x)
{
    if (x >= 0.0f) {
        float p = c0_acos + x * (c1_acos + x * (c2_acos + x * (c3_acos_tweaked
                          + x * (c4_acos + x * (c5_acos + x * (c6_acos + x * c7_acos))))));
        float s = sqrt(1.0f - x);
        return p*s;
    } else {
        float p = c0_acos_tweaked - x * (c1_acos - x * (c2_acos - x * (c3_acos
                                  - x * (c4_acos - x * (c5_acos - x * (c6_acos - x * c7_acos))))));
        float s = sqrt(1.0f + x);
        return 3.14159265f - p*s;
    }
}


} // namespace math
} // namespace scene_rdl2
