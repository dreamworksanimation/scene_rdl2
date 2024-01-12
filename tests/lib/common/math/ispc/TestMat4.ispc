// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file TestMat4.ispc

#include "TestMat4.isph"

export uniform int
Test_Mat4_ctor()
{
    return testCtor();
}

export uniform int
Test_Mat4_add()
{
    return testAdd();
}

export uniform int
Test_Mat4_minus()
{
    return testMinus();
}

export uniform int
Test_Mat4_scalarMultMat()
{
    return testScalarMultMat();
}

export uniform int
Test_Mat4_matMultScalar()
{
    return testMatMultScalar();
}

export uniform int
Test_Mat4_vecMultMat()
{
    return testVecMultMat();
}

export uniform int
Test_Mat4_matMultVec()
{
    return testMatMultVec();
}

export uniform int
Test_Mat4_matMultMat()
{
    return testMatMultMat();
}

export uniform int
Test_Mat4_isEqual()
{
    return testIsEqual();
}

export uniform int
Test_Mat4_isEqualFixedEps()
{
    return testIsEqualFixedEps();
}

export uniform int
Test_Mat4_det()
{
    return testDet();
}

export uniform int
Test_Mat4_transpose()
{
    return testTranspose();
}

export uniform int
Test_Mat4_adjoint()
{
    return testAdjoint();
}

export uniform int
Test_Mat4_inverse()
{
    return testInverse();
}

export uniform int
Test_Mat4_setToIdentity()
{
    return testSetToIdentity();
}

export uniform int
Test_Mat4_setToScale()
{
    return testSetToScale();
}

export uniform int
Test_Mat4_setToRotation()
{
    return testSetToRotation();
}

export uniform int
Test_Mat4_transform()
{
    return testTransform();
}

export uniform int
Test_Mat4_transformNormal()
{
    return testTransformNormal();
}

export uniform int
Test_Mat4_transformH()
{
    return testTransformH();
}

export uniform int
Test_Mat4_transformPoint()
{
    return testTransformPoint();
}

