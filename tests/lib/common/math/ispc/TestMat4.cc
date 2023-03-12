// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file TestMat4.cc

#include "TestMat4.h"

#include "TestMat4_ispc_stubs.h"

using namespace scene_rdl2;
using scene_rdl2::common::math::ispc::unittest::TestMat4;

void
TestMat4::testCreate()
{
    CPPUNIT_ASSERT(::ispc::Test_Mat4_ctor() == 0);
}

void
TestMat4::testAdd()
{
    CPPUNIT_ASSERT(::ispc::Test_Mat4_add() == 0);
}

void
TestMat4::testMinus()
{
    CPPUNIT_ASSERT(::ispc::Test_Mat4_minus() == 0);
}

void
TestMat4::testScalarMultMat()
{
    CPPUNIT_ASSERT(::ispc::Test_Mat4_scalarMultMat() == 0);
}

void
TestMat4::testMatMultScalar()
{
    CPPUNIT_ASSERT(::ispc::Test_Mat4_matMultScalar() == 0);
}

void
TestMat4::testVecMultMat()
{
    CPPUNIT_ASSERT(::ispc::Test_Mat4_vecMultMat() == 0);
}

void
TestMat4::testMatMultVec()
{
    CPPUNIT_ASSERT(::ispc::Test_Mat4_matMultVec() == 0);
}

void
TestMat4::testMatMultMat()
{
    CPPUNIT_ASSERT(::ispc::Test_Mat4_matMultMat() == 0);
}

void
TestMat4::testIsEqual()
{
    CPPUNIT_ASSERT(::ispc::Test_Mat4_isEqual() == 0);
}

void
TestMat4::testIsEqualFixedEps()
{
    CPPUNIT_ASSERT(::ispc::Test_Mat4_isEqualFixedEps() == 0);
}

void
TestMat4::testDet()
{
    CPPUNIT_ASSERT(::ispc::Test_Mat4_det() == 0);
}

void
TestMat4::testTranspose()
{
    CPPUNIT_ASSERT(::ispc::Test_Mat4_transpose() == 0);
}

void
TestMat4::testAdjoint()
{
    CPPUNIT_ASSERT(::ispc::Test_Mat4_adjoint() == 0);
}

void
TestMat4::testInverse()
{
    CPPUNIT_ASSERT(::ispc::Test_Mat4_inverse() == 0);
}

void
TestMat4::testSetToIdentity()
{
    CPPUNIT_ASSERT(::ispc::Test_Mat4_setToIdentity() == 0);
}

void
TestMat4::testSetToScale()
{
    CPPUNIT_ASSERT(::ispc::Test_Mat4_setToScale() == 0);
}

void
TestMat4::testSetToRotation()
{
    CPPUNIT_ASSERT(::ispc::Test_Mat4_setToRotation() == 0);
}

void
TestMat4::testTransform()
{
    CPPUNIT_ASSERT(::ispc::Test_Mat4_transform() == 0);
}

void
TestMat4::testTransformNormal()
{
    CPPUNIT_ASSERT(::ispc::Test_Mat4_transformNormal() == 0);
}

void
TestMat4::testTransformH()
{
    CPPUNIT_ASSERT(::ispc::Test_Mat4_transformH() == 0);
}

void
TestMat4::testTransformPoint()
{
    CPPUNIT_ASSERT(::ispc::Test_Mat4_transformPoint() == 0);
}

