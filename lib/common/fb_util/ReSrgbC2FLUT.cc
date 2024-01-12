// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#include "ReSrgbC2FLUT.h"

#include <iomanip>
#include <sstream>

#include <math.h>

namespace scene_rdl2 {
namespace fb_util {

// static function
std::string
ReSrgbC2FLUT::tblGen()
//
// Generating lookup table for conversion from 8bit sRGB quantized value to 32bit single float
//
{
    auto sRGBtoLinear = [](float f01) -> float {
        // https://en.wikipedia.org/wiki/SRGB
        return (f01 <= 0.04045f)? (f01 / 12.92f): powf((f01 + 0.055f) / 1.055f, 2.4f);
    };

    std::ostringstream ostr;
    ostr << "float uc255Tof[255] = {\n"; // You may change array name if you want.

    for (unsigned int tblId = 0; tblId < 256; ++tblId) {

        float v = (float)tblId / 255.0f;
        float reSrgb = sRGBtoLinear(v);

        ostr << "  /* tblid:" << std::setw(3) << tblId << " */ " << reSrgb;

        if (tblId != 255) ostr << ',';
        ostr << '\n';
    }

    ostr << "};";
    return ostr.str();
}

} // namespace fb_util
} // namespace scene_rdl2

