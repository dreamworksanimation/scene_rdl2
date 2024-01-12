// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file TestCol3.ispc

#include "TestCol3.isph"

export uniform int
Test_Col3_ctor()
{
    return testCtor();
}

export uniform int
Test_Col3_clamp()
{
    return testClamp();
}

export uniform int
Test_Col3_lerp()
{
    return testLerp();
}

export uniform int
Test_Col3_isBlack()
{
    return testIsBlack();
}

export uniform int
Test_Col3_minus()
{
    return testMinus();
}

export uniform int
Test_Col3_mult()
{
    return testMult();
}


export uniform int
Test_Col3_plus()
{
    return testPlus();
}

export uniform int
Test_Col3_sPostMult()
{
    return testScalarPostMult();
}

export uniform int
Test_Col3_sPreMult()
{
    return testScalarPreMult();
}

export uniform int
Test_Col3_isEqual()
{
    return testIsEqual();
}

export uniform int
Test_Col3_isEqualFixedEps()
{
    return testIsEqualFixedEps();
}

export uniform int
Test_Col3_max()
{
    return testMax();
}

export uniform int
Test_Col3_rcp()
{
    return testRcp();
}

export uniform int
Test_Col3_sqrt()
{
    return testSqrt();
}

export uniform int
Test_Col3_luminance()
{
    return testLuminance();
}

