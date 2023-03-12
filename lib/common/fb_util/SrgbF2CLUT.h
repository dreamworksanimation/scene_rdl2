// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#pragma once

#include <string>

namespace scene_rdl2 {
namespace fb_util {

//
// -- Generating lookup table for conversion from linear float to sRGB uchar
//
// This class is designed for generating lookup table for direct conversion from 32bit
// single float (linear space) to sRGB space 8bit unsigned char.
// Also includes several different test and verify functions.
//
// This class is not used runtime of moonray. But I would like to keep this code inside
// moonray because it's very useful to understand basic idea of direct float to uchar
// lookup table from linear to sRGB conversion. Toshi (Sep/25/2020)
//
class SrgbF2CLUT
{
public:
    SrgbF2CLUT() { mUni.mI = 0; }

    //------------------------------
    //
    // raw level set/get/show for single float value
    //
    void set(const float f) { mUni.mF = f; }
    void set(const int signBit, const int exponent, const int mantissa) {
        mUni.mI = (((signBit & 0x1) << 31) | ((exponent & 0xff) << 23) | (mantissa & 0x7fffff));        
    }
    
    float getF() const { return mUni.mF; }

    std::string show() const;

    //------------------------------
    //
    // table generation
    //
    static std::string tblGen15bit();

protected:
    union {
        float mF;
        unsigned int mI;
    } mUni;

    std::string showBit() const;
    std::string showSignBit() const { return showMask(31, 31, mUni.mI); }
    std::string showExponentBit() const { return showMask(30, 23, mUni.mI); }
    std::string showMantissaBit() const { return showMask(22, 0, mUni.mI); }
    std::string showAllBit() const { return showMask(31, 0, mUni.mI); }
    std::string showMask(const int left, const int right, const unsigned int d) const;
}; // SrgbF2CLUT

} // namespace fb_util
} // namespace scene_rdl2

