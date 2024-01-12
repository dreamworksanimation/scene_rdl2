// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "PeakErrs.h"
#include "TestTranscendental.h"

#include <scene_rdl2/common/math/Math.h>
#include <scene_rdl2/common/math/Transcendental.h>

using namespace scene_rdl2;
using namespace scene_rdl2::math;


void TestCommonMathTranscendental::testRcp()
{
    printf("\n");
    printf("TestCommonMathTranscendental::testRcp()\n");
    printf("=======================================\n");

    // Test rcp(x) against value computed using 1/x.
    // Note that rcp() doesn't support denormals, either in its argument or in its result.
    // So the test iterates over all normal float inputs that produce normal float outputs.

    PeakErrs rcpPeakErrs;

    float sign = 1.0f;
    for (int i = 0; i < 2; ++i) {
        for (int X = 0x00800000; X <= 0x7E7FFFFF; ++X) {
            float x = sign * AsFloat(X);
            float rcpGood = 1.0f/x;
            float rcpDwa  = rcp(x);
            rcpPeakErrs.update(x, rcpDwa, rcpGood);
        }
        sign = -sign;
    }

    printf("rcp tested over all normal float inputs that produce normal float outputs\n");
    rcpPeakErrs.print();
    printf("\n");
}


void TestCommonMathTranscendental::testAcos()
{
    printf("\n");
    printf("TestCommonMathTranscendental::testAcos()\n");
    printf("========================================\n");

    // Test local acos implementation and single precision library version against double precision library version.
    // Note that we're assuming sufficient accuracy of the double precision C++ library version - this assumption
    // has yet to be tested, and it is worth testing because it doesn't hold for the ISPC library.

    PeakErrs libPeakErrs;
    PeakErrs dwaPeakErrs;

    float sign = 1.0f;
    for (int i = 0; i < 2; ++i) {
        for (int X = 0x00000000; X <= 0x3F800000; ++X) {
            float x = sign * AsFloat(X);

            float acosGood = (float)scene_rdl2::math::acos((double)x);
            float acosLib  = scene_rdl2::math::acos(x);
            float acosDwa  = dw_acos(x);

            libPeakErrs.update(x, acosLib, acosGood);
            dwaPeakErrs.update(x, acosDwa, acosGood);
        }
        sign = -sign;
    }

    printf("acos() and dw_acos() tested over [-1.0f, 1.0f]\n");
    printf("\nPeak errors for library acos():\n");
    libPeakErrs.print();
    printf("\nPeak errors for dw_acos():\n");
    dwaPeakErrs.print();
    printf("\n");
}

