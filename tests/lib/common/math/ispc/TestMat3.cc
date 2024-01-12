// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file TestMat3.cc

#include "TestMat3.h"

#include "TestMat3_ispc_stubs.h"

using namespace scene_rdl2;
using scene_rdl2::common::math::ispc::unittest::TestMat3;

void
TestMat3::testCreate()
{
    CPPUNIT_ASSERT(::ispc::Test_Mat3_ctor() == 0);
}

void
TestMat3::testAdd()
{
    CPPUNIT_ASSERT(::ispc::Test_Mat3_add() == 0);
}

void
TestMat3::testMinus()
{
    CPPUNIT_ASSERT(::ispc::Test_Mat3_minus() == 0);
}

void
TestMat3::testScalarMultMat()
{
    CPPUNIT_ASSERT(::ispc::Test_Mat3_scalarMultMat() == 0);
}

void
TestMat3::testMatMultScalar()
{
    CPPUNIT_ASSERT(::ispc::Test_Mat3_matMultScalar() == 0);
}

void
TestMat3::testVecMultMat()
{
    CPPUNIT_ASSERT(::ispc::Test_Mat3_vecMultMat() == 0);
}

void
TestMat3::testMatMultVec()
{
    CPPUNIT_ASSERT(::ispc::Test_Mat3_matMultVec() == 0);
}

void
TestMat3::testMatMultMat()
{
    CPPUNIT_ASSERT(::ispc::Test_Mat3_matMultMat() == 0);
}

void
TestMat3::testIsEqual()
{
    CPPUNIT_ASSERT(::ispc::Test_Mat3_isEqual() == 0);
}

void
TestMat3::testIsEqualFixedEps()
{
    CPPUNIT_ASSERT(::ispc::Test_Mat3_isEqualFixedEps() == 0);
}

void
TestMat3::testDet()
{
    CPPUNIT_ASSERT(::ispc::Test_Mat3_det() == 0);
}

void
TestMat3::testTranspose()
{
    CPPUNIT_ASSERT(::ispc::Test_Mat3_transpose() == 0);
}

void
TestMat3::testAdjoint()
{
    CPPUNIT_ASSERT(::ispc::Test_Mat3_adjoint() == 0);
}

void
TestMat3::testInverse()
{
    CPPUNIT_ASSERT(::ispc::Test_Mat3_inverse() == 0);
}

void
TestMat3::testSetToIdentity()
{
    CPPUNIT_ASSERT(::ispc::Test_Mat3_setToIdentity() == 0);
}

void
TestMat3::testSetToScale()
{
    CPPUNIT_ASSERT(::ispc::Test_Mat3_setToScale() == 0);
}

void
TestMat3::testSetToRotation()
{
    CPPUNIT_ASSERT(::ispc::Test_Mat3_setToRotation() == 0);
}

void
TestMat3::testTransform()
{
    CPPUNIT_ASSERT(::ispc::Test_Mat3_transform() == 0);
}

void
TestMat3::testTransformNormal()
{
    CPPUNIT_ASSERT(::ispc::Test_Mat3_transformNormal() == 0);
}

void
TestMat3::testMedley()
{
    CPPUNIT_ASSERT(::ispc::Test_Mat3_medley() == 0);
}


