// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file Test.ispc

#include "Test.isph"

export uniform int
Test_isEqual()
{
    return testIsEqual();
}

export uniform int
Test_isEqualFixedEps()
{
    return testIsEqualFixedEps();
}

export uniform int
Test_isZero()
{
    return testIsZero();
}

export uniform int
Test_isOne()
{
    return testIsOne();
}

export uniform int
Test_isNormalizedLengthSqr()
{
    return testIsNormalizedLengthSqr();
}

export uniform int
Test_isFinite()
{
    return testIsFinite();
}

export uniform int
Test_isInf()
{
    return testIsInf();
}

export uniform int
Test_isNormal()
{
    return testIsNormal();
}

export uniform int
Test_lerp()
{
    return testLerp();
}

export uniform int
Test_deg2rad()
{
    return testDeg2rad();
}

export uniform int
Test_rad2deg()
{
    return testRad2deg();
}

export uniform int
Test_bias()
{
    return testBias();
}

export uniform int
Test_gain()
{
    return testGain();
}

export uniform int
Test_trunc()
{
    return testTrunc();
}

export uniform int
Test_fmod()
{
    return testFmod();
}

export uniform int
Test_saturate()
{
    return testSaturate();
}

