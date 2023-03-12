// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#pragma once

#include <cstdint>

#include <emmintrin.h>          // SSE2
#include <immintrin.h>          // AVX

namespace scene_rdl2 {
namespace fb_util {

class GammaF2C
{
public:

    //
    // -- High speed float value to gamma 2.2 8bit conversion by lookup table --
    //
    // Only works for positive float f or zero.
    // Negative f returns zero. Inf f returns 255 and nan f returns zero.
    //
    // g22() returns same result of following code. (f is always zero or positive in this case)
    //
    //   uint8_t g22(const float f) const {
    //     if (isnan(f)) return 0;
    //     if (isinf(f)) return 255;
    //     if (f <= 0.0f) return 0;
    //     else if (f < 1.0f) return (uint8_t)(pow(f, 1.0/2.2) * 255.0);
    //     else return 255;
    //   }
    //
    static uint8_t g22(const float f); // gamma 2.2 correction and 8bit quantization from single float

#   ifdef TEST
    // Following functions are test for SIMD version of id computation.
    // However not support negative value return 0 functionality yet. Toshi (04/Oct/20)

    // -- performance result --
    //   Tested by untile + gamma correction + 8bit quantization on my test scene (Timmy's Bedroom)
    //   naive (u_char)(pow(f, 1/2.2) * 255) : 57   ~ 63   ms
    //   g22() (no SIMD)                     :  7.0 ~  7.2 ms
    //   g22c4() (SSE2)                      :  4.5 ~  5.0 ms
    //   g22c4x2() (AVX2)                    :  3.3 ~  3.4 ms
    //   
    // SIMD(SSE) version of single pixel value conversion
    // rgba contains 1 pixel data as __m128 (=RGBA)
    // and return out[3] as gamma corrected RGB
    static void g22c4(const __m128 *rgba, uint8_t out[3]);

    // SIMD(AVX) version of double pixel value conversion
    // rgba2 contains 2 pixels data as __m256 (=RGBARGBA)
    // and return out[6] as gamma corrected RGBRGB
    static void g22c4x2(const __m256 *rgba2, uint8_t out[6]);
#   endif // end TEST
}; // GammaF2C

} // namespace fb_util
} // namespace scene_rdl2

