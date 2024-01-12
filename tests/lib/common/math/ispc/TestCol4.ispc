// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file TestCol4.ispc

#include "TestCol4.isph"

export uniform int
Test_Col4_ctor()
{
    return testCtor();
}

export uniform int
Test_Col4_clamp()
{
    return testClamp();
}

export uniform int
Test_Col4_lerp()
{
    return testLerp();
}

export uniform int
Test_Col4_minus()
{
    return testMinus();
}

export uniform int
Test_Col4_mult()
{
    return testMult();
}


export uniform int
Test_Col4_plus()
{
    return testPlus();
}

export uniform int
Test_Col4_sPostMult()
{
    return testScalarPostMult();
}

export uniform int
Test_Col4_sPreMult()
{
    return testScalarPreMult();
}

export uniform int
Test_Col4_isEqual()
{
    return testIsEqual();
}

export uniform int
Test_Col4_isEqualFixedEps()
{
    return testIsEqualFixedEps();
}

