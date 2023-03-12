// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file TestVec2.cc

#include "TestVec2.h"

#include "TestVec2_ispc_stubs.h"

using namespace scene_rdl2;
using scene_rdl2::common::math::ispc::unittest::TestVec2;

void
TestVec2::testCreate()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec2_ctor() == 0);
}

void
TestVec2::testPlus()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec2_plus() == 0);
}

void
TestVec2::testMinus()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec2_minus() == 0);
}

void
TestVec2::testScalarPreMult()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec2_scalarPreMult() == 0);
}

void
TestVec2::testScalarPostMult()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec2_scalarPostMult() == 0);
}

void
TestVec2::tetsMult()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec2_mult() == 0);
}

void
TestVec2::testScalarPreDiv()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec2_scalarPreDiv() == 0);
}

void
TestVec2::testScalarPostDiv()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec2_scalarPostDiv() == 0);
}

void
TestVec2::testDiv()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec2_div() == 0);
}

void
TestVec2::testIsEqual()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec2_isEqual() == 0);
}

void
TestVec2::testIsEqualFixedEps()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec2_isEqualFixedEps() == 0);
}

void
TestVec2::testIsZero()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec2_isZero() == 0);
}

void
TestVec2::testDot()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec2_dot() == 0);
}

void
TestVec2::testLength()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec2_length() == 0);
}

void
TestVec2::testLengthSqr()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec2_lengthSqr() == 0);
}

void
TestVec2::testNormalize()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec2_normalize() == 0);
}

void
TestVec2::testAbs()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec2_abs() == 0);
}

void
TestVec2::testNeg()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec2_neg() == 0);
}

void
TestVec2::testRcp()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec2_rcp() == 0);
}

void
TestVec2::testIsNormalized()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec2_isNormalized() == 0);
}

void
TestVec2::testLerp()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec2_lerp() == 0);
}


