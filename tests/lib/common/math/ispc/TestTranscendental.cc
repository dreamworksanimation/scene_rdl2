// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include <scene_rdl2/common/math/Math.h>
#include <scene_rdl2/common/math/Transcendental.h>

#include "../PeakErrs.h"
#include "TestTranscendental.h"

#include "TestTranscendental_ispc_stubs.h"

using namespace scene_rdl2;
using namespace scene_rdl2::math;
using scene_rdl2::common::math::ispc::unittest::TestTranscendental;


void
TestTranscendental::testRcp()
{
    printf("\n");
    printf("ispc::TestTranscendental::testRcp()\n");
    printf("===================================\n");

    // Test ispc::rcp(x) against value computed using 1/x.
    // Note that ispc::rcp() doesn't support denormals, either in its argument or in its result.
    // So the test iterates over all normal float inputs that produce normal float outputs.

    PeakErrs rcpPeakErrs;

    float sign = 1.0f;
    for (int i = 0; i < 2; ++i) {
        for (int X = 0x00800000; X <= 0x7E7FFFFF; ++X) {
            float x = sign * AsFloat(X);
            float rcpGood = 1.0f/x;
            float rcpDwa  = ::ispc::exported_rcp(x);
            rcpPeakErrs.update(x, rcpDwa, rcpGood);
        }
        sign = -sign;
    }

    printf("rcp tested over all normal float inputs that produce normal float outputs\n");
    rcpPeakErrs.print();
    printf("\n");
}

void
TestTranscendental::testDwAcos()
{
    printf("\n");
    printf("ispc::TestTranscendental::testAcos()\n");
    printf("====================================\n");

    // Test local ispc acos implementation, single precision ispc library version, and double precision
    // library version against double precision C++ library version (which is assumed to generate the nearest
    // float to the result after rounding to single precision).

    PeakErrs f32PeakErrs;
    PeakErrs f64PeakErrs;
    PeakErrs dwaPeakErrs;

    float sign = 1.0f;
    for (int i = 0; i < 2; ++i) {
        for (int X = 0x00000000; X <= 0x3F800000; ++X) {
            float x = sign * AsFloat(X);

            float acosGood = (float)scene_rdl2::math::acos((double)x);
            float acosf32  = ::ispc::exported_acos_f32(x);
            float acosf64  = ::ispc::exported_acos_f64(x);
            float acosDwa  = ::ispc::exported_dw_acos(x);

            f32PeakErrs.update(x, acosf32, acosGood);
            f64PeakErrs.update(x, acosf64, acosGood);
            dwaPeakErrs.update(x, acosDwa, acosGood);
        }
        sign = -sign;
    }

    printf("acos() f32, acos() f64, and dw_acos() tested over [-1.0f, 1.0f]\n");
    printf("\nPeak errors for library f32 acos():\n");
    f32PeakErrs.print();
    printf("\nPeak errors for library f64 acos():\n");
    f64PeakErrs.print();
    printf("\nPeak errors for dw_acos():\n");
    dwaPeakErrs.print();
    printf("\n");
}
