// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


// Minimax coefficients computed by Mathematica
const uniform float c0_acos =  1.5707963039207744f;
const uniform float c1_acos = -0.2145986966117427f;
const uniform float c2_acos =  0.08897731218991513f;
const uniform float c3_acos = -0.05016418844436275f;
const uniform float c4_acos =  0.030862751448600963f;
const uniform float c5_acos = -0.017045102007693682f;
const uniform float c6_acos =  0.006638618338665405f;
const uniform float c7_acos = -0.0012534570550072417f;

// A couple of tweaked values to reduce evaluation error.
// The resulting error is at most 2 ULP. It may be possible to improve on this by further tweaking.
const uniform float c0_acos_tweaked = 1.5707964;
const uniform float c3_acos_tweaked = -0.0501641f;

uniform float dw_acos(uniform float x)
{
    if (x >= 0.0f) {
        uniform float p = c0_acos + x * (c1_acos + x * (c2_acos + x * (c3_acos_tweaked
                                  + x * (c4_acos + x * (c5_acos + x * (c6_acos + x * c7_acos))))));
        uniform float s = sqrt(1.0f - x);
        return p*s;
    } else {
        uniform float p = c0_acos_tweaked - x * (c1_acos - x * (c2_acos - x * (c3_acos
                                          - x * (c4_acos - x * (c5_acos - x * (c6_acos - x * c7_acos))))));
        uniform float s = sqrt(1.0f + x);
        return 3.14159265f - p*s;
    }
}

varying float dw_acos(varying float x)
{
    if (x >= 0.0f) {
        varying float p = c0_acos + x * (c1_acos + x * (c2_acos + x * (c3_acos_tweaked
                                  + x * (c4_acos + x * (c5_acos + x * (c6_acos + x * c7_acos))))));
        varying float s = sqrt(1.0f - x);
        return p*s;
    } else {
        varying float p = c0_acos_tweaked - x * (c1_acos - x * (c2_acos - x * (c3_acos
                                          - x * (c4_acos - x * (c5_acos - x * (c6_acos - x * c7_acos))))));
        varying float s = sqrt(1.0f + x);
        return 3.14159265f - p*s;
    }
}

