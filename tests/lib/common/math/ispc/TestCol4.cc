// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file TestCol4.cc

#include "TestCol4.h"

#include "TestCol4_ispc_stubs.h"

using namespace scene_rdl2;
using scene_rdl2::common::math::ispc::unittest::TestCol4;

void
TestCol4::create()
{
    CPPUNIT_ASSERT(::ispc::Test_Col4_ctor() == 0);
}

void
TestCol4::clamp()
{
    CPPUNIT_ASSERT(::ispc::Test_Col4_clamp() == 0);
}

void
TestCol4::lerp()
{
    CPPUNIT_ASSERT(::ispc::Test_Col4_lerp() == 0);
}

void
TestCol4::minus()
{
    CPPUNIT_ASSERT(::ispc::Test_Col4_minus() == 0);
}

void
TestCol4::mult()
{
    CPPUNIT_ASSERT(::ispc::Test_Col4_mult() == 0);
}

void
TestCol4::plus()
{
    CPPUNIT_ASSERT(::ispc::Test_Col4_plus() == 0);
}

void
TestCol4::sPostMult()
{
    CPPUNIT_ASSERT(::ispc::Test_Col4_sPostMult() == 0);
}

void
TestCol4::sPreMult()
{
    CPPUNIT_ASSERT(::ispc::Test_Col4_sPreMult() == 0);
}

void
TestCol4::isEqual()
{
    CPPUNIT_ASSERT(::ispc::Test_Col4_isEqual() == 0);
}

void
TestCol4::isEqualFixedEps()
{
    CPPUNIT_ASSERT(::ispc::Test_Col4_isEqualFixedEps() == 0);
}

