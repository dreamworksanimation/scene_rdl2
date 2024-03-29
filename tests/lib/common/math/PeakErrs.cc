// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "PeakErrs.h"

using namespace scene_rdl2;
using namespace scene_rdl2::math;


// Functions for reinterpeting between 32-bit floats and 32-bit ints
float   AsFloat(int32_t X) { return *(float *)&X; }
int32_t AsInt(float x) { return *(int32_t   *)&x; }


PeakErrs::PeakErrs()
{
    minUlpErr       = 0x7FFFFFFF;
    xMinUlpErr      = 0.0f;
    approxMinUlpErr = 0.0f;
    funcMinUlpErr   = 0.0f;

    maxUlpErr       = 0x80000000;
    xMaxUlpErr      = 0.0f;
    approxMaxUlpErr = 0.0f;
    funcMaxUlpErr   = 0.0f;

    minAbsErr       = pos_inf;
    xMinAbsErr      = 0.0f;
    approxMinAbsErr = 0.0f;
    funcMinAbsErr   = 0.0f;

    maxAbsErr       = neg_inf;
    xMaxAbsErr      = 0.0f;
    approxMaxAbsErr = 0.0f;
    funcMaxAbsErr   = 0.0f;

    minRelErr       = pos_inf;
    xMinRelErr      = 0.0f;
    approxMinRelErr = 0.0f;
    funcMinRelErr   = 0.0f;

    maxRelErr       = neg_inf;
    xMaxRelErr      = 0.0f;
    approxMaxRelErr = 0.0f;
    funcMaxRelErr   = 0.0f;
}


// Test a newly generated result against the peak errors and update as necessary.
// x = argument at which function is being evaluated
// approx = value generated by function being tested
// func = closest single precision float to the true function value
void PeakErrs::update(float x, float approx, float func)
{
    int ulpErr = AsInt(approx) - AsInt(func);
    if (ulpErr < minUlpErr) {
        minUlpErr       = ulpErr;
        xMinUlpErr      = x;
        approxMinUlpErr = approx;
        funcMinUlpErr   = func;
    }
    if (ulpErr > maxUlpErr) {
        maxUlpErr       = ulpErr;
        xMaxUlpErr      = x;
        approxMaxUlpErr = approx;
        funcMaxUlpErr   = func;
    }

    float absErr = approx - func;
    if (absErr < minAbsErr) {
        minAbsErr       = absErr;
        xMinAbsErr      = x;
        approxMinAbsErr = approx;
        funcMinAbsErr   = func;
    }
    if (absErr > maxAbsErr) {
        maxAbsErr       = absErr;
        xMaxAbsErr      = x;
        approxMaxAbsErr = approx;
        funcMaxAbsErr   = func;
    }

    if (func != 0.0f) {
        float relErr = absErr / func;
        if (relErr < minRelErr) {
            minRelErr       = relErr;
            xMinRelErr      = x;
            approxMinRelErr = approx;
            funcMinRelErr   = func;
        }
        if (relErr > maxRelErr) {
            maxRelErr       = relErr;
            xMaxRelErr      = x;
            approxMaxRelErr = approx;
            funcMaxRelErr   = func;
        }
    }
}


void PeakErrs::print()
{
    printf("Peak ULP errors [%d, %d]\n", minUlpErr, maxUlpErr);
    printf("Peak absolute errors [%g, %g]\n", minAbsErr, maxAbsErr);
    printf("Peak relative errors [%g, %g]\n", minRelErr, maxRelErr);
    printf("First argument values where peak errors occurred:\n");

    printf("x=%g(0x%08X) -> %g(0x%08X), true value %g(0x%08X), ulp err %d\n",
           xMinUlpErr,      AsInt(xMinUlpErr),
           approxMinUlpErr, AsInt(approxMinUlpErr),
           funcMinUlpErr,   AsInt(funcMinUlpErr),
           minUlpErr);

    printf("x=%g(0x%08X) -> %g(0x%08X), true value %g(0x%08X), ulp err %d\n",
           xMaxUlpErr,      AsInt(xMaxUlpErr),
           approxMaxUlpErr, AsInt(approxMaxUlpErr),
           funcMaxUlpErr,   AsInt(funcMaxUlpErr),
           maxUlpErr);

    printf("x=%g(0x%08X) -> %g(0x%08X), true value %g(0x%08X), abs err %g\n",
           xMinAbsErr,      AsInt(xMinAbsErr),
           approxMinAbsErr, AsInt(approxMinAbsErr),
           funcMinAbsErr,   AsInt(funcMinAbsErr),
           minAbsErr);

    printf("x=%g(0x%08X) -> %g(0x%08X), true value %g(0x%08X), abs err %g\n",
           xMaxAbsErr,      AsInt(xMaxAbsErr),
           approxMaxAbsErr, AsInt(approxMaxAbsErr),
           funcMaxAbsErr,   AsInt(funcMaxAbsErr),
           maxAbsErr);

    printf("x=%g(0x%08X) -> %g(0x%08X), true value %g(0x%08X), rel err %g\n",
           xMinRelErr,      AsInt(xMinRelErr),
           approxMinRelErr, AsInt(approxMinRelErr),
           funcMinRelErr,   AsInt(funcMinRelErr),
           minRelErr);

    printf("x=%g(0x%08X) -> %g(0x%08X), true value %g(0x%08X), rel err %g\n",
           xMaxRelErr,      AsInt(xMaxRelErr),
           approxMaxRelErr, AsInt(approxMaxRelErr),
           funcMaxRelErr,   AsInt(funcMaxRelErr),
           maxRelErr);
}

