// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file Test.cc

#include "Test.h"

#include "Test_ispc_stubs.h"

using namespace scene_rdl2;
using scene_rdl2::common::math::ispc::unittest::Test;

void
Test::isEqual()
{
    CPPUNIT_ASSERT(::ispc::Test_isEqual() == 0);
}

void
Test::isEqualFixedEps()
{
    CPPUNIT_ASSERT(::ispc::Test_isEqualFixedEps() == 0);
}

void
Test::isZero()
{
    CPPUNIT_ASSERT(::ispc::Test_isZero() == 0);
}

void
Test::isOne()
{
    CPPUNIT_ASSERT(::ispc::Test_isOne() == 0);
}

void
Test::isNormalizedLengthSqr()
{
    CPPUNIT_ASSERT(::ispc::Test_isNormalizedLengthSqr() == 0);
}

void
Test::isFinite()
{
    CPPUNIT_ASSERT(::ispc::Test_isFinite() == 0);
}

void
Test::isInf()
{
    CPPUNIT_ASSERT(::ispc::Test_isInf() == 0);
}

void
Test::isNormal()
{
    CPPUNIT_ASSERT(::ispc::Test_isNormal() == 0);
}

void
Test::lerp()
{
    CPPUNIT_ASSERT(::ispc::Test_lerp() == 0);
}

void
Test::deg2rad()
{
    CPPUNIT_ASSERT(::ispc::Test_deg2rad() == 0);
}

void
Test::rad2deg()
{
    CPPUNIT_ASSERT(::ispc::Test_rad2deg() == 0);
}

void
Test::bias()
{
    CPPUNIT_ASSERT(::ispc::Test_bias() == 0);
}

void
Test::gain()
{
    CPPUNIT_ASSERT(::ispc::Test_gain() == 0);
}

void
Test::trunc()
{
    CPPUNIT_ASSERT(::ispc::Test_trunc() == 0);
}

void
Test::fmod()
{
    CPPUNIT_ASSERT(::ispc::Test_fmod() == 0);
}

void
Test::saturate()
{
    CPPUNIT_ASSERT(::ispc::Test_saturate() == 0);
}

