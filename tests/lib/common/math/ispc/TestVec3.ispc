// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file TestVec3.ispc

#include "TestVec3.isph"

export uniform int
Test_Vec3_ctor()
{
    return testCtor();
}

export uniform int
Test_Vec3_plus()
{
    return testPlus();
}

export uniform int
Test_Vec3_minus()
{
    return testMinus();
}

export uniform int
Test_Vec3_scalarPreMult()
{
    return testScalarPreMult();
}

export uniform int
Test_Vec3_scalarPostMult()
{
    return testScalarPostMult();
}

export uniform int
Test_Vec3_mult()
{
    return testMult();
}

export uniform int
Test_Vec3_scalarPreDiv()
{
    return testScalarPreDiv();
}

export uniform int
Test_Vec3_scalarPostDiv()
{
    return testScalarPostDiv();
}

export uniform int
Test_Vec3_div()
{
    return testDiv();
}

export uniform int
Test_Vec3_isEqual()
{
    return testIsEqual();
}

export uniform int
Test_Vec3_isEqualFixedEps()
{
    return testIsEqualFixedEps();
}

export uniform int
Test_Vec3_isZero()
{
    return testIsZero();
}

export uniform int
Test_Vec3_dot()
{
    return testDot();
}

export uniform int
Test_Vec3_cross()
{
    return testCross();
}

export uniform int
Test_Vec3_length()
{
    return testLength();
}

export uniform int
Test_Vec3_lengthSqr()
{
    return testLengthSqr();
}

export uniform int
Test_Vec3_normalize()
{
    return testNormalize();
}

export uniform int
Test_Vec3_abs()
{
    return testAbs();
}

export uniform int
Test_Vec3_neg()
{
    return testNeg();
}

export uniform int
Test_Vec3_rcp()
{
    return testRcp();
}

export uniform int
Test_Vec3_isNormalized()
{
    return testIsNormalized();
}

export uniform int
Test_Vec3_lerp()
{
    return testLerp();
}

