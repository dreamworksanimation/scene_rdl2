// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file TestVec4.cc

#include "TestVec4.h"

#include "TestVec4_ispc_stubs.h"

using namespace scene_rdl2;
using scene_rdl2::common::math::ispc::unittest::TestVec4;

void
TestVec4::testCreate()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec4_ctor() == 0);
}

void
TestVec4::testPlus()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec4_plus() == 0);
}

void
TestVec4::testMinus()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec4_minus() == 0);
}

void
TestVec4::testScalarPreMult()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec4_scalarPreMult() == 0);
}

void
TestVec4::testScalarPostMult()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec4_scalarPostMult() == 0);
}

void
TestVec4::tetsMult()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec4_mult() == 0);
}

void
TestVec4::testScalarPreDiv()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec4_scalarPreDiv() == 0);
}

void
TestVec4::testScalarPostDiv()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec4_scalarPostDiv() == 0);
}

void
TestVec4::testDiv()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec4_div() == 0);
}

void
TestVec4::testIsEqual()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec4_isEqual() == 0);
}

void
TestVec4::testIsEqualFixedEps()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec4_isEqualFixedEps() == 0);
}

void
TestVec4::testIsZero()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec4_isZero() == 0);
}

void
TestVec4::testDot()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec4_dot() == 0);
}

void
TestVec4::testLength()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec4_length() == 0);
}

void
TestVec4::testLengthSqr()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec4_lengthSqr() == 0);
}

void
TestVec4::testNormalize()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec4_normalize() == 0);
}

void
TestVec4::testAbs()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec4_abs() == 0);
}

void
TestVec4::testNeg()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec4_neg() == 0);
}

void
TestVec4::testRcp()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec4_rcp() == 0);
}

void
TestVec4::testIsNormalized()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec4_isNormalized() == 0);
}

void
TestVec4::testLerp()
{
    CPPUNIT_ASSERT(::ispc::Test_Vec4_lerp() == 0);
}


