// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file TestCol3.cc

#include "TestCol3.h"

#include "TestCol3_ispc_stubs.h"

using namespace scene_rdl2;
using scene_rdl2::common::math::ispc::unittest::TestCol3;

void
TestCol3::create()
{
    CPPUNIT_ASSERT(::ispc::Test_Col3_ctor() == 0);
}

void
TestCol3::clamp()
{
    CPPUNIT_ASSERT(::ispc::Test_Col3_clamp() == 0);
}

void
TestCol3::lerp()
{
    CPPUNIT_ASSERT(::ispc::Test_Col3_lerp() == 0);
}

void
TestCol3::isBlack()
{
    CPPUNIT_ASSERT(::ispc::Test_Col3_isBlack() == 0);
}

void
TestCol3::minus()
{
    CPPUNIT_ASSERT(::ispc::Test_Col3_minus() == 0);
}

void
TestCol3::mult()
{
    CPPUNIT_ASSERT(::ispc::Test_Col3_mult() == 0);
}

void
TestCol3::plus()
{
    CPPUNIT_ASSERT(::ispc::Test_Col3_plus() == 0);
}

void
TestCol3::sPostMult()
{
    CPPUNIT_ASSERT(::ispc::Test_Col3_sPostMult() == 0);
}

void
TestCol3::sPreMult()
{
    CPPUNIT_ASSERT(::ispc::Test_Col3_sPreMult() == 0);
}

void
TestCol3::isEqual()
{
    CPPUNIT_ASSERT(::ispc::Test_Col3_isEqual() == 0);
}

void
TestCol3::isEqualFixedEps()
{
    CPPUNIT_ASSERT(::ispc::Test_Col3_isEqualFixedEps() == 0);
}

void
TestCol3::max()
{
    CPPUNIT_ASSERT(::ispc::Test_Col3_max() == 0);
}

void
TestCol3::rcp()
{
    CPPUNIT_ASSERT(::ispc::Test_Col3_rcp() == 0);
}

void
TestCol3::sqrt()
{
    CPPUNIT_ASSERT(::ispc::Test_Col3_sqrt() == 0);
}

void
TestCol3::luminance()
{
    CPPUNIT_ASSERT(::ispc::Test_Col3_luminance() == 0);
}

