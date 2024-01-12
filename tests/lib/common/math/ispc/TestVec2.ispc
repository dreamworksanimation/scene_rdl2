// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file TestVec2.ispc

#include "TestVec2.isph"

export uniform int
Test_Vec2_ctor()
{
    return testCtor();
}

export uniform int
Test_Vec2_plus()
{
    return testPlus();
}

export uniform int
Test_Vec2_minus()
{
    return testMinus();
}

export uniform int
Test_Vec2_scalarPreMult()
{
    return testScalarPreMult();
}

export uniform int
Test_Vec2_scalarPostMult()
{
    return testScalarPostMult();
}

export uniform int
Test_Vec2_mult()
{
    return testMult();
}

export uniform int
Test_Vec2_scalarPreDiv()
{
    return testScalarPreDiv();
}

export uniform int
Test_Vec2_scalarPostDiv()
{
    return testScalarPostDiv();
}

export uniform int
Test_Vec2_div()
{
    return testDiv();
}

export uniform int
Test_Vec2_isEqual()
{
    return testIsEqual();
}

export uniform int
Test_Vec2_isEqualFixedEps()
{
    return testIsEqualFixedEps();
}

export uniform int
Test_Vec2_isZero()
{
    return testIsZero();
}

export uniform int
Test_Vec2_dot()
{
    return testDot();
}

export uniform int
Test_Vec2_length()
{
    return testLength();
}

export uniform int
Test_Vec2_lengthSqr()
{
    return testLengthSqr();
}

export uniform int
Test_Vec2_normalize()
{
    return testNormalize();
}

export uniform int
Test_Vec2_abs()
{
    return testAbs();
}

export uniform int
Test_Vec2_neg()
{
    return testNeg();
}

export uniform int
Test_Vec2_rcp()
{
    return testRcp();
}

export uniform int
Test_Vec2_isNormalized()
{
    return testIsNormalized();
}

export uniform int
Test_Vec2_lerp()
{
    return testLerp();
}

