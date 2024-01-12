// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file TestVec4.ispc

#include "TestVec4.isph"

export uniform int
Test_Vec4_ctor()
{
    return testCtor();
}

export uniform int
Test_Vec4_plus()
{
    return testPlus();
}

export uniform int
Test_Vec4_minus()
{
    return testMinus();
}

export uniform int
Test_Vec4_scalarPreMult()
{
    return testScalarPreMult();
}

export uniform int
Test_Vec4_scalarPostMult()
{
    return testScalarPostMult();
}

export uniform int
Test_Vec4_mult()
{
    return testMult();
}

export uniform int
Test_Vec4_scalarPreDiv()
{
    return testScalarPreDiv();
}

export uniform int
Test_Vec4_scalarPostDiv()
{
    return testScalarPostDiv();
}

export uniform int
Test_Vec4_div()
{
    return testDiv();
}

export uniform int
Test_Vec4_isEqual()
{
    return testIsEqual();
}

export uniform int
Test_Vec4_isEqualFixedEps()
{
    return testIsEqualFixedEps();
}

export uniform int
Test_Vec4_isZero()
{
    return testIsZero();
}

export uniform int
Test_Vec4_dot()
{
    return testDot();
}

export uniform int
Test_Vec4_length()
{
    return testLength();
}

export uniform int
Test_Vec4_lengthSqr()
{
    return testLengthSqr();
}

export uniform int
Test_Vec4_normalize()
{
    return testNormalize();
}

export uniform int
Test_Vec4_abs()
{
    return testAbs();
}

export uniform int
Test_Vec4_neg()
{
    return testNeg();
}

export uniform int
Test_Vec4_rcp()
{
    return testRcp();
}

export uniform int
Test_Vec4_isNormalized()
{
    return testIsNormalized();
}

export uniform int
Test_Vec4_lerp()
{
    return testLerp();
}

