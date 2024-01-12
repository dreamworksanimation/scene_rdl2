// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#include "ReGammaC2FLUT.h"

#include <iomanip>
#include <sstream>

#include <math.h>

namespace scene_rdl2 {
namespace fb_util {

// static function
std::string
ReGammaC2FLUT::tblGen()
//
// Generating lookup table for conversion from 8bit quantized value to 32bit single float
//
{
    std::ostringstream ostr;

    ostr << "float g255Tof[255] = {\n"; // You may change array name if you want.

    for (unsigned int tblId = 0; tblId < 256; ++tblId) {


        float v = (float)tblId / 255.0f;
        float regamma = powf(v, 2.2f);

        ostr << "  /* tblid:" << std::setw(3) << tblId << " */ " << regamma;

        if (tblId != 255) ostr << ',';
        ostr << '\n';
    }

    ostr << "}";
    return ostr.str();
}

} // namespace fb_util
} // namespace scene_rdl2

