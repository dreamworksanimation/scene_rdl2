// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file TestQuaternion.cc

#include "TestQuaternion.h"

#include "TestQuaternion_ispc_stubs.h"

using namespace scene_rdl2;
using scene_rdl2::common::math::ispc::unittest::TestQuaternion;

void
TestQuaternion::testCreate()
{
    CPPUNIT_ASSERT(::ispc::Test_Quaternion_ctor() == 0);
}

void
TestQuaternion::testGetV()
{
    CPPUNIT_ASSERT(::ispc::Test_Quaternion_getV() == 0);
}

void
TestQuaternion::testConjugate()
{
    CPPUNIT_ASSERT(::ispc::Test_Quaternion_conjugate() == 0);
}

void
TestQuaternion::testScalarPrePlus()
{
    CPPUNIT_ASSERT(::ispc::Test_Quaternion_scalarPrePlus() == 0);
}

void
TestQuaternion::testScalarPostPlus()
{
    CPPUNIT_ASSERT(::ispc::Test_Quaternion_scalarPostPlus() == 0);
}

void
TestQuaternion::testPlus()
{
    CPPUNIT_ASSERT(::ispc::Test_Quaternion_plus() == 0);
}

void
TestQuaternion::testScalarPreMinus()
{
    CPPUNIT_ASSERT(::ispc::Test_Quaternion_scalarPreMinus() == 0);
}

void
TestQuaternion::testScalarPostMinus()
{
    CPPUNIT_ASSERT(::ispc::Test_Quaternion_scalarPostMinus() == 0);
}

void
TestQuaternion::testMinus()
{
    CPPUNIT_ASSERT(::ispc::Test_Quaternion_minus() == 0);
}

void
TestQuaternion::testScalarPreMult()
{
    CPPUNIT_ASSERT(::ispc::Test_Quaternion_scalarPreMult() == 0);
}

void
TestQuaternion::testScalarPostMult()
{
    CPPUNIT_ASSERT(::ispc::Test_Quaternion_scalarPostMult() == 0);
}

void
TestQuaternion::testMult()
{
    CPPUNIT_ASSERT(::ispc::Test_Quaternion_mult() == 0);
}

void
TestQuaternion::testVecPostMult()
{
    CPPUNIT_ASSERT(::ispc::Test_Quaternion_vecPostMult() == 0);
}

void
TestQuaternion::testTransform()
{
    CPPUNIT_ASSERT(::ispc::Test_Quaternion_transform() == 0);
}

void
TestQuaternion::testScalarPreDiv()
{
    CPPUNIT_ASSERT(::ispc::Test_Quaternion_scalarPreDiv() == 0);
}

void
TestQuaternion::testScalarPostDiv()
{
    CPPUNIT_ASSERT(::ispc::Test_Quaternion_scalarPostDiv() == 0);
}

void
TestQuaternion::testDiv()
{
    CPPUNIT_ASSERT(::ispc::Test_Quaternion_div() == 0);
}

void
TestQuaternion::testDot()
{
    CPPUNIT_ASSERT(::ispc::Test_Quaternion_dot() == 0);
}

void
TestQuaternion::testNormalize()
{
    CPPUNIT_ASSERT(::ispc::Test_Quaternion_normalize() == 0);
}

void
TestQuaternion::testIsNormalized()
{
    CPPUNIT_ASSERT(::ispc::Test_Quaternion_isNormalized() == 0);
}

void
TestQuaternion::testRcp()
{
    CPPUNIT_ASSERT(::ispc::Test_Quaternion_rcp() == 0);
}

void
TestQuaternion::testIsEqual()
{
    CPPUNIT_ASSERT(::ispc::Test_Quaternion_isEqual() == 0);
}

void
TestQuaternion::testIsEqualFixedEps()
{
    CPPUNIT_ASSERT(::ispc::Test_Quaternion_isEqualFixedEps() == 0);
}

void
TestQuaternion::testSlerp()
{
    CPPUNIT_ASSERT(::ispc::Test_Quaternion_slerp() == 0);
}

