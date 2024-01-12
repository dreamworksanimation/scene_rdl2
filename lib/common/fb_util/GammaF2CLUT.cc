// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#include "GammaF2CLUT.h"
#include "GammaF2C.h"           // for verify table

#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace scene_rdl2 {
namespace fb_util {

std::string
GammaF2CLUT::show(const std::string &hd) const
{
    std::ostringstream ostr;
    ostr << hd << "singleFloat f:" << std::setw(20) << std::fixed << std::setprecision(15) << mUni.mF << " {\n";
    ostr << showBit(hd + "  ") << '\n';
    ostr << hd << "}";
    return ostr.str();
}

//------------------------------------------------------------------------------    

void
GammaF2CLUT::testReconstructSingleFloat()
{
    double sign = (mUni.mI & 0x80000000)? -1.0: 1.0;

    int exp = (mUni.mI >> 23) & 0xff;
    int man = mUni.mI & 0x7fffff;

    double expVal = calcExponentVal();
    double manVal = calcMantissaVal();
    double val = sign * manVal * expVal;

    std::cout << "sig:" << sign << std::endl;
    std::cout << "exp:" << exp << " mask:" << showMask(7, 0, exp) << " expVal:" << expVal << std::endl;
    std::cout << "man:" << man << " mask:" << showMask(22, 0, man) << " manVal:" << manVal << std::endl;
    std::cout << "Val:" << val << std::endl;
}

float
GammaF2CLUT::testGamma22()
{
    double expVal = calcExponentVal();
    double manVal = calcMantissaVal();

    double gamma = pow(expVal, 2.2) * pow(manVal, 2.2);
    return gamma;
}

//------------------------------------------------------------------------------    

void
GammaF2CLUT::tbl1024()
//
// test lookup table dump function. size = 1024 (10bit id)
//
{
    std::ostringstream ostr;

    float prevVal = 0.0f;
    for (unsigned int tblId = 0; tblId < 1024; ++tblId) {
        // id2f_tbl37(tblId); // 10bit
        // id2f_tbl46(tblId); // 10bit
        id2f_tbl55(tblId); // 10bit

        float delta = getF() - prevVal;
        float delta255 = delta * 255.0f;

        int gamma255 = static_cast<int>(powf(getF(), 1.0f/2.2f) * 255.0f);

        /*
        ostr.str("");
        ostr << std::setw(4) << tblId << " ";
        std::cout << show(ostr.str()) << " prev:" << prevVal << " delta:" << delta << " delta255:" << delta255 << std::endl;
        */
        std::cout << "id:" << tblId;
        std::cout << " f:" << getF() << " delta255:" << delta255 << " gamma255:" << gamma255 << std::endl;

        prevVal = getF();
    }
}

void
GammaF2CLUT::tbl2048()
//
// test loopup table dump function. size = 2048 (11bit id)
//
{
    std::ostringstream ostr;

    float prevVal = 0.0f;
    for (unsigned int tblId = 0; tblId < 2048; ++tblId) {
        // id2f_tbl47(tblId); // 11bit
        id2f_tbl57(tblId); // 11bit

        float delta = getF() - prevVal;
        float delta255 = delta * 255.0f;

        int gamma255 = static_cast<int>(powf(getF(), 1.0f/2.2f) * 255.0f);

        /*
        ostr.str("");
        ostr << std::setw(4) << tblId << " ";
        std::cout << show(ostr.str()) << " prev:" << prevVal << " delta:" << delta << " delta255:" << delta255
                  << " gamma255:" << gamma255 << std::endl;
        */
        std::cout << "id:" << tblId;
        std::cout << " f:" << getF() << " delta255:" << delta255 << " gamma255:" << gamma255 << std::endl;

        prevVal = getF();
    }
}

void
GammaF2CLUT::id2f_tbl37(const int tblId) // 10bit table
{
    unsigned int extId = (tblId >> 7) & 0x7; // 3bit : exponent : 0 ~ 7
    unsigned int manId = tblId & 0x7f;       // 7bit : mantissa
    unsigned int extVal = extId + 119;       // exponent range  : 119 ~ 126
    unsigned int manVal = manId << 16;
    set(0, extVal, manVal);
}

void
GammaF2CLUT::id2f_tbl38(const int tblId) // 11bit table
{
    unsigned int extId = (tblId >> 8) & 0x7; // 3bit : exponent : 0 ~ 7
    unsigned int manId = tblId & 0xff;       // 8bit : mantissa
    unsigned int extVal = extId + 119;       // exponent range  : 119 ~ 126
    unsigned int manVal = manId << 15;
    set(0, extVal, manVal);
}

void
GammaF2CLUT::id2f_tbl46(const int tblId) // 10bit table
{
    unsigned int extId = (tblId >> 6) & 0xf; // 4bit : exponent : 0 ~ 15
    unsigned int manId = tblId & 0x3f;       // 6bit : mantissa
    unsigned int extVal = extId + 111;       // exponent range  : 111 ~ 126
    unsigned int manVal = manId << 17;
    set(0, extVal, manVal);
}

void
GammaF2CLUT::id2f_tbl47(const int tblId) // 11bit table
{
    unsigned int extId = (tblId >> 7) & 0xf; // 4bit : exponent : 0 ~ 15
    unsigned int manId = tblId & 0x7f;       // 7bit : mantissa 
    unsigned int extVal = extId + 111;       // exponent range  : 111 ~ 126
    unsigned int manVal = manId << 16;
    set(0, extVal, manVal);
}

void
GammaF2CLUT::id2f_tbl55(const int tblId) // 10bit table
{
    unsigned int extId = (tblId >> 5) & 0x1f; // 5bit : exponent : 0 ~ 31
    unsigned int manId = tblId & 0x1f;        // 5bit : mantissa
    unsigned int extVal = extId + 96;         // exponent range  : 96 ~ 127
    unsigned int manVal = manId << 18;
    set(0, extVal, manVal);
}

void
GammaF2CLUT::id2f_tbl56(const int tblId) // 11bit table
{
    unsigned int extId = (tblId >> 6) & 0x1f; // 5bit : exponent : 0 ~ 31
    unsigned int manId = tblId & 0x3f;        // 6bit : mantissa
    unsigned int extVal = extId + 109;        // exponent range : 109 ~ 140
    unsigned int manVal = manId << 17;
    set(0, extVal, manVal);
}

void
GammaF2CLUT::id2f_tbl57(const int tblId) // 12bit table : best so far
{
    unsigned int extId = (tblId >> 7) & 0x1f; // 5bit : exponent : 0 ~ 31
    unsigned int manId = tblId & 0x7f;        // 7bit : mantissa
    unsigned int extVal = extId + 109;        // exponent range : 109 ~ 140
    unsigned int manVal = manId << 16;
    set(0, extVal, manVal);
}

//------------------------------------------------------------------------------

void
GammaF2CLUT::testTblId()
{
    std::ostringstream ostr;

    for (unsigned int tblId = 0; tblId < 1024; ++tblId) {
        id2f_tbl46(tblId); // 10bit << best

        /*
        float delta = getF() - prevVal;
        float delta255 = delta * 255.0f;

        int gamma255 = static_cast<int>(powf(getF(), 1.0f/2.2f) * 255.0f);
        */

        {
            unsigned currTblId = GammaF2CLUT::calcTblId(getF());
            std::cout << "tblId:" << tblId << " currTblId:" << currTblId << std::endl;
        }
    }
}

// static function
unsigned
GammaF2CLUT::calcTblId(const float f)
{
    union Uni {
        float f;
        unsigned u;
    };
    const Uni *uni = reinterpret_cast<const Uni *>(&f);

    unsigned expMask = (uni->u >> 23) & 0xff;
    unsigned manMaskShifted = (uni->u >> 16);

    return calcTblId(expMask, manMaskShifted);
}

// static function
unsigned
GammaF2CLUT::calcTblId(const unsigned expMask, const unsigned manMaskShifted)
//
// This function computes id for lookup table which created by GammaF2CLUT::tblGen()
//
{
    //
    // -- High speed float value to gamma 2.2 8bit conversion by lookup table --
    //
    // Regarding to the computation of gamma correction and quantized to 8bit range by lookup table,
    // I directory generate lookup table index from input float value's bit pattern.
    //
    // First of all, we consider following 4 values (f1 ~ f4).
    //
    //                            -- IEEE singleFloat bit pattern --
    //                            s|   exp  |         mantissa             expVal | gamma255
    //   f1 = 0.000005077049536 : 0 01101101 01010100101101110000110    =>   109        0
    //   f2 = 0.000005077049991 : 0 01101101 01010100101101110000111    =>   109        1
    //   f3 = 0.999999940395355 : 0 01111110 11111111111111111111111    =>   126      254
    //   f4 = 1.000000000000000 : 0 01111111 00000000000000000000000    =>   127      255
    //                            ^|<---+-->|<----------+---------->|       
    //                            |     |               |
    //                            |     |               +-- mantissaBit
    //                            |     +- exponentBit
    //                            +-- signeBit
    //
    // f1 is maxinum single float value which converted to 0 after gamma/8bit conversion.
    // f2 is minimum single float value which converted to 1.
    // f3 is maxinum single float value which converted to 254.
    // f4 is minimum single float value which converted to 255.
    //
    // This means all the float value equal or smaller than f1 should be converted to 0. And
    // all the float value equal or bigger than f4 should be converted to 255. We only need to consider between f2 to f3.
    // (There is no float value between f1 and f2 also f3 and f4).
    //
    // Exponent value range for f2 ~ f3 is 109(=0x6d,=01101101) ~ 126(=0x7e,=01111110).
    // In order to keep all range from 109 to 126, we need at least 5bit. And 5bit can store value range from 109 to 140.
    // This can save entire range from f1 to f4 (We only use 109 to 126. not using 127 to 140 actually).
    //
    // For mantissa, basically we don't need all 23bit of precision. After several tests, I decided to use
    // top left 7bit for mantissa and final tableId bit size is 12bit.
    //
    //                       expPart    manPart
    //   tableId (12bit) = 1 0 9 8 7 6 5 4 3 2 1 0
    //                    |<---+--->|<-----+----->|
    //                         |           |
    //                         |           +-- from mantissaBit : 7bit 0~127 : top left 7bit of single float mantissa
    //                         |
    //                         +-- from exponentBit : 5bit 0~31 : exponent range from 109(=0x60) to 140(=0x8c)
    //                                                But only use from 109 to 126
    // 
    // Following is a table index and it's input float value and output gamma255 table value (0~255)
    //
    //   tableId | expPart | manPart | expMask |  manMask |    inputF    | g255
    //         0         0         0       109   0x000000   0.0000038147      0
    //         1         0         1       109   0x010000   0.0000038445      0
    //         2         0         2       109   0x020000   0.0000038743      0
    //         3         0         3       109   0x030000   0.0000039041      0
    //         4         0         4       109   0x040000   0.0000039339      0
    //         5         0         5       109   0x050000   0.0000039637      0
    //         6         0         6       109   0x060000   0.0000039935      0
    //       ...
    //      2298        17       122       126   0x7a0000   0.9765625000    252
    //      2299        17       123       126   0x7b0000   0.9804687500    252
    //      2300        17       124       126   0x7c0000   0.9843750000    253
    //      2301        17       125       126   0x7d0000   0.9882812500    253
    //      2302        17       126       126   0x7e0000   0.9921875000    254
    //      2303        17       127       126   0x7f0000   0.9960937500    254
    //      2304        18         0       127   0x000000   1.0000000000    255
    // 
    // expPart is a value of exponent part of tableId.             expPart = (tableId >> 7) & 0x1f;
    // manPart is a value of mantissa part of tableId.             manPart = tableId & 0x7f;
    // expMask is input value's exponent bitMask based on expPart. expMask = expPart + 109
    // manMask is input value's mantissa bitMask based on manPart. manMask = manPart << 16
    // inputF is reconstruct float value based on tableId.         inputF  = (expMask << 23) | manMask
    // g255 is gamma corrected 8bit value of inputF                g255    = pow(inputF, 1.0/2.2) * 255
    //
    // tableId is 12bit and can support 4096 table size. However, we only use 0 ~ 2304.
    // (max exponent is 126 and exp(126) w/ man(0x7f0000 = full mantissa bitMask) goes to tblId 2303.
    // But we need index 2304 as to get max value (i.e. 255). This is why max tbl id is 2304 instead of 2303)
    //
    // Following logic is a IEEE single float to tablId direct conversion logic
    // 
    // int convFloatToTblId(const float f)
    // {
    //     union Uni {
    //         float f;
    //         unsigned u;
    //     };
    //     const Uni *uni = (const Uni *)&f;
    //     unsigned expMask = (uni->u >> 23) & 0xff;        // ... (a)
    //     unsigned manMaskShifted = (uni->u >> 16) & 0x7f; // ... (b)
    //
    //     unsigned tblId = 0;
    //     if (expMask < 109) {
    //         tblId = 0;    // ... (c)
    //     } else if (expMask < 127) {
    //         tblId = (((expMask - 109) & 0x1f) << 7) | manMaskShifted; // ... (d)
    //     } else {
    //         tblId = 2304; // ... (e)
    //     }
    //     return tblId;
    // }
    //
    // (a) : expMask is bitmask pattern of exponent : 8bit
    // (b) : manMaskShifted is shifted bitmask pattern of mantissa (only consider top left 7bit of mantissa bit)
    // (c) : if exponent value is less than 109, pick tableId as 0 (i.e. final out value is 0)
    // (d) : if exponent value is between 109~127, generated tblId is 0~2303
    // (e) : if exponent value is equal or bigger than 127, tableId is 2304.
    //    
    // Let's remove branch flow by bitMask operation
    //
    /* naive version
    unsigned tblId = 0;
    if (expMask < 109) {
        tblId = 0;
    } else if (expMask < 127) {
        tblId = (((expMask - 109) & 0x1f) << 7) | (manMaskShifted & 0x7f);
    } else {
        tblId = 2304;
    }
    return tblId;
    */

    int tblId = ((((expMask - 109) & 0x1f) << 7) | (manMaskShifted & 0x7f)); // ... (A)

    int expMaxMask = (static_cast<int>(expMask) - 127) >> 31;                // ... (B)
    int tblId2 = (tblId & expMaxMask) | (2304 & ~expMaxMask);    // ... (C)

    int expMinMask = ~((static_cast<int>(expMask) - 109) >> 31); // ... (D)
    int tblId3 = tblId2 & expMinMask;                            // ... (E)

    return tblId3;

    //
    // (A) : calculate base tblId based on exponent range (start 109 & 5bit) and left 7 bit of mantissa
    // (B) : mask for exponent max (127) : expMaxMask = (expMask >= 127)? 0x0: 0xffffffff
    // (C) : tblId (exponent 109~) is now clipped to 0 ~ 2304.
    // (D) : mask for exponent min (109) : expMinMask = (expMask < 109)? 0x0: 0xffffffff
    // (E) : tblId3 (all exponent) is now clipped to 0 ~ 2304. This is result tblId
    //
}

// static function
unsigned
GammaF2CLUT::calcTblId2(const unsigned manMaskShifted)
//
// This is a optimized version of GammaF2CLUT::calcTblId().
// Performance is slightly faster but potentially still we need 2 if judgements.
// Basic idea of this enhancement was suggestion by Mike Day. Thanks Mike!
// (See detail : http://github.anim.dreamworks.com/arras/moonray/pull/224)
// Toshi (Feb/28/2018)
//
{
    //
    // Basic logic and idea is same as GammaF2CLUT::calcTblId().
    // We processed expPart and manPart separately on GammaF2CLUT::calcTblId() but
    // we can process together because these are 12 consecutive bits.
    //
    // This is a sudo code to compute tblId
    //
    //   unsigned u = uni->u >> 16
    //   u = clamp(u, 0x36aa, 0x3f80) ... (a)
    //   int tblId = u - 0x3680 ... (b)
    //
    // Current LUT start from float value = (expPart=109, manPart=0).
    // 16bit right shifted value of this float is 00110110 10000000 = 0x3680.
    // This is why (b) subtracts 0x3680
    //
    // (a) needs to clamp by 0x36aa and 0x3f80 because.
    // 0x36aa is a 16bit right shifted value of f1 value (00110110 10101010 = 0x36aa)
    // 0x3f80 is a 16bit right shifted value of f4 value (00111111 10000000 = 0x3f80)
    //
    // Actually first 43 entries of LUT which generated by GammaF2CLUT::calcTblId() is ZERO
    // So we can reduce the table size a bit and accessed by following sudo code.
    //
    //   unsigned u = uni->u >> 16
    //   u = clamp(u, 0x36aa, 0x3f80)
    //   int tblId = u - 0x36aa ... (c)
    //
    // We changed offset of LUT and now we should substruct 0x36aa at (c).
    //
    // Following code is for reduce LUT size version (i.e. You should remove first 42 entries
    // from LUT which generated by GammaF2CLUT::tblGen() to use this function).
    //
    // -- Performance --
    // Based on my test and compare with GammaF2CLUT::calcTblId(), this calcTblid2() is slightly faster.
    // untile w/ gamma correction + 8bit quantize test
    //   calcTblId()  : 9.5~10.0 ms range
    //   calcTblId2() : 8.5~9.3 ms range
    // Speed itself is totally depend on test data itself but I could find calcTblId2() is relatively faster.
    // Toshi (Feb/28/2018)
    //

    int tblId = manMaskShifted - 0x36aa;
    if (tblId < 0) {
        tblId = 0;
    } else if (tblId > 2262) {
        tblId = 2262;
    }

    //
    // This is a convert if branch by bit mask operation but somehow this code is slower than
    // naive c++ if coding. (compiler is very smart :-)
    //
    /*
    int tblId1 = manMaskShifted - 0x36aa;

    int minMask = ~(tblId1 >> 31);
    int tblId2 = tblId1 & minMask;

    int maxMask = (tblId1 - 2262) >> 31;
    int tblId = (tblId2 & maxMask) | (2262 & ~maxMask);
    */

    return tblId;
}

// static function
unsigned
GammaF2CLUT::calcTblId3(const unsigned manMaskShifted)
//
// This id computation function is based on different ideas and
// used special LUT which generated by GammaF2CLUT::tblGen15bit()
// Main idea of this id computation and LUT is reduce branch at id computation stage.
// This is trade off of table size and id computation complexity. Now GammaF2CLUT::tblGen15bit()
// creates 32768 bytes (=32KByte) table. However id computation does not include
// any branch operation and just use 16bit shifted value as tblId.
// Toshi (Mar/01/2018)
//
{
    // -- Performance --
    // Compare performance between 2 of them.
    // untile w/ gamma correction + 8bit quantize test
    //   calcTblId()  : 9.5~10.0 ms range
    //   calcTblId2() : 8.5~9.3 ms range
    //   calcTblId3() : 7.0~7.2 ms range
    // Speed itself is totally depend on test data itself but I could find calcTblId3() is fastest.
    // Toshi (Mar/01/2018)
    // 
    return manMaskShifted; // uni->u >> 16
}

//------------------------------------------------------------------------------

// static function
std::string
GammaF2CLUT::tblGen()
{
    std::ostringstream ostr;

    int max = 144 * 16;         // We already know total size of table by experimental run
    
    ostr << "unsigned char f2g255[" << max + 1 << "] = {\n"; // You may change array name if you want

    GammaF2CLUT gLUT;
    bool topST = true;
    for (int tblId = 0; tblId <= max; ++tblId) {
        if (topST) {
            ostr << "  /* tblId:" << std::setw(4) << tblId << " */ ";
        }

        gLUT.id2f_tbl57(tblId);

        int g255 = static_cast<int>(pow(gLUT.getF(), 1.0/2.2) * 255);
        ostr << std::setw(3) << g255;

        if (tblId != max) ostr << ',';

        topST = false;
        if (!((tblId + 1) % 16)) {
            ostr << '\n';
            topST = true;
        }
    }
    ostr << '\n';
    ostr << "};";
    return ostr.str();
}

bool
GammaF2CLUT::verifyTbl22()
{
    unsigned startExp = 108;
    unsigned endExp   = 127;
    for (unsigned exp = startExp; exp <= endExp; ++exp) {
        for (unsigned man = 0x0; man <= 0x7ff000; man += 0x1000) {
            set(0, exp, man);

            float f = getF();

            std::cout << "  exp:" << std::dec << exp
                      << " man:0x" << std::setw(6) << std::hex << std::setfill('0') << man << std::setfill(' ')
                      << " f:" << std::setw(15) << std::fixed << std::setprecision(10) << f;

            int g255tbl = GammaF2C::g22(f);

            std::cout << " g255tbl:" << std::dec << g255tbl;

            int g255 = static_cast<int>(pow(f, 1.0/2.2) * 255.0);
            if (g255 < 0) g255 = 0;
            if (g255 > 255) g255 = 255;
            std::cout << " g255:" << std::dec << g255;

            if (g255tbl != g255) {
                if (g255tbl + 1 != g255) {
                    std::cout << " NG f:" << f << " {\n";
                    std::cout << "  exp:" << exp << " man:0x" << std::hex << man << std::endl;
                    std::cout << "  g255tbl:" << std::dec << g255tbl << std::endl;
                    std::cout << "  g255:" << std::dec << g255 << std::endl;
                    std::cout << "}\n";
                    return false;
                } else {
                    std::cout << " OK+1\n";
                }
            } else {
                std::cout << " OK\n";
            }
        }
    }
    return true;
}

//------------------------------------------------------------------------------

// static function
std::string
GammaF2CLUT::tblGen15bit()
{
    std::ostringstream ostr;

    int max = 32768;
    
    ostr << "unsigned char f2g255[" << max << "] = {\n"; // You may change array name if you want

    GammaF2CLUT gLUT;
    bool topST = true;
    for (int tblId = 0; tblId < max; ++tblId) {
        if (topST) {
            ostr << "  /* tblId:" << std::setw(5) << tblId << " */ ";
        }

        int exponent = (tblId >> 7) & 0xff;
        int mantissa = (tblId & 0x7f) << 16;
        gLUT.set(0, exponent, mantissa); // re-construct float value based on exponent and mantissa

        int g255;
        if (tblId > 32640) {
            // tblId > 32640 : nan
            g255 = 0; // we can not compute value -> just set 0
        } else if (tblId == 32640) {
            // tblId = 32640 : inf
            g255 = 255;
        } else if (tblId >= 16256) {
            // getF() is more than 1.0
            g255 = 255;
        } else {
            g255 = static_cast<int>(pow(gLUT.getF(), 1.0/2.2) * 255);
            if (g255 > 255) g255 = 255; // just in case
        }
        ostr << std::setw(3) << g255;

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
GammaF2CLUT::showBit(const std::string &hd) const
{
    std::ostringstream ostr;
    ostr << hd << "s|   exp  |         mantissa\n";
    ostr << hd << showSignBit() << ' ' << showExponentBit() << ' ' << showMantissaBit();
    return ostr.str();
}

std::string
GammaF2CLUT::showMask(const int left, const int right, const unsigned int d) const
{
    std::ostringstream ostr;
    for (int i = left; i >= right; --i) {
        ostr << ((d >> i) & 0x1);
    }
    return ostr.str();
}

double
GammaF2CLUT::calcExponentVal() const
{
    int exp = (mUni.mI >> 23) & 0xff;
    double val = pow(2.0, (double)(exp - 127));
    return val;
}

double
GammaF2CLUT::calcMantissaVal() const
{
    int man = mUni.mI & 0x7fffff;

    double val = 0.0;
    double currVal = 0.5;
    for (int i = 22; i >= 0; --i) {
        if ((man >> i) & 0x1) {
            val += currVal;
        }
        currVal *= 0.5;
    }
    return val + 1.0;
}

} // namespace fb_util
} // namespace scene_rdl2

