// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file TestAsA.cc

#include "TestAsA.h"

#include "TestAsA_ispc_stubs.h"

using namespace scene_rdl2;
using scene_rdl2::common::math::ispc::unittest::TestAsA;

void
TestAsA::asAVec2()
{
    CPPUNIT_ASSERT(::ispc::Test_AsA_Vec2() == 0);
}

void
TestAsA::asAVec3()
{
    CPPUNIT_ASSERT(::ispc::Test_AsA_Vec3() == 0);
}

void
TestAsA::asAVec4()
{
    CPPUNIT_ASSERT(::ispc::Test_AsA_Vec4() == 0);
}

void
TestAsA::asACol3()
{
    CPPUNIT_ASSERT(::ispc::Test_AsA_Col3() == 0);
}

void
TestAsA::asAColor()
{
    CPPUNIT_ASSERT(::ispc::Test_AsA_Color() == 0);
}

void
TestAsA::asACol4()
{
    CPPUNIT_ASSERT(::ispc::Test_AsA_Col4() == 0);
}

void
TestAsA::asArray()
{
    CPPUNIT_ASSERT(::ispc::Test_AsArray() == 0);
}

