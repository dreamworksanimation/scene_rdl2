// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <scene_rdl2/common/math/Math.h>


float AsFloat(int32_t X);
int32_t AsInt(float x);

// A utility struct for maintaining the peak errors (ULP, absolute, and relative) for a given function
struct PeakErrs
{
    int   minUlpErr;
    float xMinUlpErr;
    float approxMinUlpErr;
    float funcMinUlpErr;

    int   maxUlpErr;
    float xMaxUlpErr;
    float approxMaxUlpErr;
    float funcMaxUlpErr;

    float minAbsErr;
    float xMinAbsErr;
    float approxMinAbsErr;
    float funcMinAbsErr;

    float maxAbsErr;
    float xMaxAbsErr;
    float approxMaxAbsErr;
    float funcMaxAbsErr;

    float minRelErr;
    float xMinRelErr;
    float approxMinRelErr;
    float funcMinRelErr;

    float maxRelErr;
    float xMaxRelErr;
    float approxMaxRelErr;
    float funcMaxRelErr;

    PeakErrs();
    void update(float x, float approx, float func);
    void print();
};

