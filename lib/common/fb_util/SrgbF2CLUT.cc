// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#include "SrgbF2CLUT.h"
#include <scene_rdl2/render/util/StrUtil.h>

#include <cmath>
#include <iostream>
#include <sstream>

namespace scene_rdl2 {
namespace fb_util {

std::string
SrgbF2CLUT::show() const
{
    std::ostringstream ostr;
    ostr << "singleFloat f:" << std::setw(20) << std::fixed << std::setprecision(15) << mUni.mF
         << " {\n"
         << str_util::addIndent(showBit()) << '\n'
         << "}";
    return ostr.str();
}

//------------------------------------------------------------------------------    

// static function
std::string
SrgbF2CLUT::tblGen15bit()
{
    auto linearToSrgb = [](float f) -> float {
        // https://en.wikipedia.org/wiki/SRGB
        return (f <= 0.0031308f)? (f * 12.92f): (1.055f * powf(f, 1.0f / 2.4f) - 0.055f);
    };

    std::ostringstream ostr;

    int max = 32768;
    
    ostr << "unsigned char f2c255[" << max << "] = {\n"; // You may change array name if you want

    SrgbF2CLUT gLUT;
    bool topST = true;
    for (int tblId = 0; tblId < max; ++tblId) {
        if (topST) {
            ostr << "  /* tblId:" << std::setw(5) << tblId << " */ ";
        }

        int exponent = (tblId >> 7) & 0xff;
        int mantissa = (tblId & 0x7f) << 16;
        gLUT.set(0, exponent, mantissa); // re-construct float value based on exponent and mantissa

        int uc;
        if (tblId > 32640) {
            // tblId > 32640 : nan
            uc = 0; // we can not compute value -> just set 0
        } else if (tblId == 32640) {
            // tblId = 32640 : inf
            uc = 255;
        } else if (tblId >= 16256) {
            // getF() is more than 1.0
            uc = 255;
        } else {
            uc = static_cast<int>(linearToSrgb(gLUT.getF()) * 255);
            if (uc > 255) uc = 255; // just in case
        }
        ostr << std::setw(3) << uc;

        if (tblId != (max - 1)) ostr << ',';

        topST = false;
        if (!((tblId + 1) % 16)) {
            ostr << '\n';
            topST = true;
        }
    }
    ostr << "};";
    return ostr.str();
}

//------------------------------------------------------------------------------

std::string
SrgbF2CLUT::showBit() const
{
    std::ostringstream ostr;
    ostr << "s|   exp  |         mantissa\n";
    ostr << showSignBit() << ' ' << showExponentBit() << ' ' << showMantissaBit();
    return ostr.str();
}

std::string
SrgbF2CLUT::showMask(const int left, const int right, const unsigned int d) const
{
    std::ostringstream ostr;
    for (int i = left; i >= right; --i) {
        ostr << ((d >> i) & 0x1);
    }
    return ostr.str();
}

} // namespace fb_util
} // namespace scene_rdl2

