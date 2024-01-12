// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file TestVec3.cc

#include "TestVec3.h"

#include "TestVec3_ispc_stubs.h"

using namespace scene_rdl2;
using scene_rdl2::common::math::ispc::unittest::TestVec3;

void
TestVec3::testCreate()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec3_ctor() == 0);
}

void
TestVec3::testPlus()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec3_plus() == 0);
}

void
TestVec3::testMinus()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec3_minus() == 0);
}

void
TestVec3::testScalarPreMult()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec3_scalarPreMult() == 0);
}

void
TestVec3::testScalarPostMult()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec3_scalarPostMult() == 0);
}

void
TestVec3::tetsMult()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec3_mult() == 0);
}

void
TestVec3::testScalarPreDiv()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec3_scalarPreDiv() == 0);
}

void
TestVec3::testScalarPostDiv()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec3_scalarPostDiv() == 0);
}

void
TestVec3::testDiv()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec3_div() == 0);
}

void
TestVec3::testIsEqual()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec3_isEqual() == 0);
}

void
TestVec3::testIsEqualFixedEps()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec3_isEqualFixedEps() == 0);
}

void
TestVec3::testIsZero()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec3_isZero() == 0);
}

void
TestVec3::testDot()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec3_dot() == 0);
}

void
TestVec3::testCross()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec3_cross() == 0);
}

void
TestVec3::testLength()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec3_length() == 0);
}

void
TestVec3::testLengthSqr()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec3_lengthSqr() == 0);
}

void
TestVec3::testNormalize()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec3_normalize() == 0);
}

void
TestVec3::testAbs()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec3_abs() == 0);
}

void
TestVec3::testNeg()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec3_neg() == 0);
}

void
TestVec3::testRcp()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec3_rcp() == 0);
}

void
TestVec3::testIsNormalized()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec3_isNormalized() == 0);
}

void
TestVec3::testLerp()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec3_lerp() == 0);
}


