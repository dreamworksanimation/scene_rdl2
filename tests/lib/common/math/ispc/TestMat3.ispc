// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file TestMat3.ispc

#include "TestMat3.isph"

export uniform int
Test_Mat3_ctor()
{
    return testCtor();
}

export uniform int
Test_Mat3_add()
{
    return testAdd();
}

export uniform int
Test_Mat3_minus()
{
    return testMinus();
}

export uniform int
Test_Mat3_scalarMultMat()
{
    return testScalarMultMat();
}

export uniform int
Test_Mat3_matMultScalar()
{
    return testMatMultScalar();
}

export uniform int
Test_Mat3_vecMultMat()
{
    return testVecMultMat();
}

export uniform int
Test_Mat3_matMultVec()
{
    return testMatMultVec();
}

export uniform int
Test_Mat3_matMultMat()
{
    return testMatMultMat();
}

export uniform int
Test_Mat3_isEqual()
{
    return testIsEqual();
}

export uniform int
Test_Mat3_isEqualFixedEps()
{
    return testIsEqualFixedEps();
}

export uniform int
Test_Mat3_det()
{
    return testDet();
}

export uniform int
Test_Mat3_transpose()
{
    return testTranspose();
}

export uniform int
Test_Mat3_adjoint()
{
    return testAdjoint();
}

export uniform int
Test_Mat3_inverse()
{
    return testInverse();
}

export uniform int
Test_Mat3_setToIdentity()
{
    return testSetToIdentity();
}

export uniform int
Test_Mat3_setToScale()
{
    return testSetToScale();
}

export uniform int
Test_Mat3_setToRotation()
{
    return testSetToRotation();
}

export uniform int
Test_Mat3_transform()
{
    return testTransform();
}

export uniform int
Test_Mat3_transformNormal()
{
    return testTransformNormal();
}

export uniform int
Test_Mat3_medley()
{
    return testMedley();
}

