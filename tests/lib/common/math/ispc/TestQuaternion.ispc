// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file TestQuaternion.ispc

#include "TestQuaternion.isph"

export uniform int
Test_Quaternion_ctor()
{
    return testCtor();
}

export uniform int
Test_Quaternion_getV()
{
    return testGetV();
}

export uniform int
Test_Quaternion_conjugate()
{
    return testConjugate();
}

export uniform int
Test_Quaternion_scalarPrePlus()
{
    return testScalarPrePlus();
}

export uniform int
Test_Quaternion_scalarPostPlus()
{
    return testScalarPostPlus();
}

export uniform int
Test_Quaternion_plus()
{
    return testPlus();
}

export uniform int
Test_Quaternion_scalarPreMinus()
{
    return testScalarPreMinus();
}

export uniform int
Test_Quaternion_scalarPostMinus()
{
    return testScalarPostMinus();
}

export uniform int
Test_Quaternion_minus()
{
    return testMinus();
}

export uniform int
Test_Quaternion_scalarPreMult()
{
    return testScalarPreMult();
}

export uniform int
Test_Quaternion_scalarPostMult()
{
    return testScalarPostMult();
}

export uniform int
Test_Quaternion_mult()
{
    return testMult();
}

export uniform int
Test_Quaternion_vecPostMult()
{
    return testVecPostMult();
}

export uniform int
Test_Quaternion_transform()
{
    return testTransform();
}

export uniform int
Test_Quaternion_scalarPreDiv()
{
    return testScalarPreDiv();
}

export uniform int
Test_Quaternion_scalarPostDiv()
{
    return testScalarPostDiv();
}

export uniform int
Test_Quaternion_div()
{
    return testDiv();
}

export uniform int
Test_Quaternion_dot()
{
    return testDot();
}

export uniform int
Test_Quaternion_normalize()
{
    return testNormalize();
}

export uniform int
Test_Quaternion_isNormalized()
{
    return testIsNormalized();
}

export uniform int
Test_Quaternion_isEqual()
{
    return testIsEqual();
}

export uniform int
Test_Quaternion_isEqualFixedEps()
{
    return testIsEqualFixedEps();
}

export uniform int
Test_Quaternion_rcp()
{
    return testRcp();
}

export uniform int
Test_Quaternion_slerp()
{
    return testSlerp();
}


