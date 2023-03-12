// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#pragma once

#include <cstdint>

namespace scene_rdl2 {
namespace fb_util {

class SrgbF2C
{
public:

    //
    // -- High speed float value to sRGB 8bit conversion by lookup table --
    //
    // Only works for positive float f or zero.
    // Negative f returns zero. Inf f returns 255 and nan f returns zero.
    //
    // sRGB() returns same result of following code. f is linear space value.
    //
    //   uint8_t sRGB(const float f) const {
    //     if (isnan(f)) return 0;
    //     if (isinf(f)) return 255;
    //     if (f <= 0.0f) return 0;
    //     else if (f < 1.0f) return (uint8_t)(linearFloatToSrgb(f);
    //     else return 255;
    //   }
    //
    //   float linearFloatToSrgb(float f) {
    //      // https://en.wikipedia.org/wiki/SRGB
    //      return (f <= 0.0031308f)? (f * 12.92f): (1.055f * powf(f, 1.0f / 2.4f) - 0.055f);
    //   }
    //
    static uint8_t sRGB(const float f); // convert to sRGB space and 8bit quantization from linear float

}; // SrgbF2C

} // namespace fb_util
} // namespace scene_rdl2

