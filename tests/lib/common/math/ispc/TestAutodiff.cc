// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file TestAutodiff.cc

#include "TestAutodiff.h"

#include "TestAutodiff_ispc_stubs.h"

using namespace scene_rdl2;
using scene_rdl2::common::math::ispc::unittest::TestAutodiff;

void
TestAutodiff::ctor()
{
    CPPUNIT_ASSERT(::ispc::Test_Autodiff_ctor() == 0);
}

void
TestAutodiff::comparison()
{
    CPPUNIT_ASSERT(::ispc::Test_Autodiff_comparison() == 0);
}

void
TestAutodiff::opOverloads()
{
    CPPUNIT_ASSERT(::ispc::Test_Autodiff_opOverloads() == 0);
}

void
TestAutodiff::evaluation()
{
    CPPUNIT_ASSERT(::ispc::Test_Autodiff_evaluation() == 0);
}

void
TestAutodiff::algebra3()
{
    CPPUNIT_ASSERT(::ispc::Test_Autodiff_algebra3() == 0);
}

void
TestAutodiff::acos()
{
    CPPUNIT_ASSERT(::ispc::Test_Autodiff_acos() == 0);
}

void
TestAutodiff::asin()
{
    CPPUNIT_ASSERT(::ispc::Test_Autodiff_asin() == 0);
}

void
TestAutodiff::atan()
{
    CPPUNIT_ASSERT(::ispc::Test_Autodiff_atan() == 0);
}

void
TestAutodiff::atan2()
{
    CPPUNIT_ASSERT(::ispc::Test_Autodiff_atan2() == 0);
}

void
TestAutodiff::bias()
{
    CPPUNIT_ASSERT(::ispc::Test_Autodiff_bias() == 0);
}

void
TestAutodiff::ceil()
{
    CPPUNIT_ASSERT(::ispc::Test_Autodiff_ceil() == 0);
}

void
TestAutodiff::cos()
{
    CPPUNIT_ASSERT(::ispc::Test_Autodiff_cos() == 0);
}

void
TestAutodiff::exp()
{
    CPPUNIT_ASSERT(::ispc::Test_Autodiff_exp() == 0);
}

void
TestAutodiff::floor()
{
    CPPUNIT_ASSERT(::ispc::Test_Autodiff_floor() == 0);
}

void
TestAutodiff::fmod()
{
    CPPUNIT_ASSERT(::ispc::Test_Autodiff_fmod() == 0);
}

void
TestAutodiff::gain()
{
    CPPUNIT_ASSERT(::ispc::Test_Autodiff_gain() == 0);
}

void
TestAutodiff::log()
{
    CPPUNIT_ASSERT(::ispc::Test_Autodiff_log() == 0);
}

void
TestAutodiff::matrix()
{
    CPPUNIT_ASSERT(::ispc::Test_Autodiff_matrix() == 0);
}

void
TestAutodiff::pow()
{
    CPPUNIT_ASSERT(::ispc::Test_Autodiff_pow() == 0);
}

void
TestAutodiff::rcp()
{
    CPPUNIT_ASSERT(::ispc::Test_Autodiff_rcp() == 0);
}

void
TestAutodiff::rsqrt()
{
    CPPUNIT_ASSERT(::ispc::Test_Autodiff_rsqrt() == 0);
}

void
TestAutodiff::min()
{
    CPPUNIT_ASSERT(::ispc::Test_Autodiff_min() == 0);
}

void
TestAutodiff::max()
{
    CPPUNIT_ASSERT(::ispc::Test_Autodiff_max() == 0);
}

void
TestAutodiff::saturate()
{
    CPPUNIT_ASSERT(::ispc::Test_Autodiff_saturate() == 0);
}

void
TestAutodiff::sin()
{
    CPPUNIT_ASSERT(::ispc::Test_Autodiff_sin() == 0);
}

void
TestAutodiff::sincos()
{
    CPPUNIT_ASSERT(::ispc::Test_Autodiff_sincos() == 0);
}

void
TestAutodiff::sqrt()
{
    CPPUNIT_ASSERT(::ispc::Test_Autodiff_sqrt() == 0);
}

void
TestAutodiff::tan()
{
    CPPUNIT_ASSERT(::ispc::Test_Autodiff_tan() == 0);
}

void
TestAutodiff::trunc()
{
    CPPUNIT_ASSERT(::ispc::Test_Autodiff_trunc() == 0);
}

void
TestAutodiff::col3()
{
    CPPUNIT_ASSERT(::ispc::Test_Autodiff_col3() == 0);
}

void
TestAutodiff::vec2()
{
    CPPUNIT_ASSERT(::ispc::Test_Autodiff_vec2() == 0);
}

void
TestAutodiff::vec3()
{
    CPPUNIT_ASSERT(::ispc::Test_Autodiff_vec3() == 0);
}

void
TestAutodiff::vec4()
{
    CPPUNIT_ASSERT(::ispc::Test_Autodiff_vec4() == 0);
}

