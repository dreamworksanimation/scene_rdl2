// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file TestColorSpace.cc

#include "TestColorSpace.h"

#include "TestColorSpace_ispc_stubs.h"

using namespace scene_rdl2;
using scene_rdl2::common::math::ispc::unittest::TestColorSpace;

void
TestColorSpace::testRgbToHsv()
{
    CPPUNIT_ASSERT(::ispc::testRgbToHsv() == 0);
}

void
TestColorSpace::testRgbToHsl()
{
    CPPUNIT_ASSERT(::ispc::testRgbToHsl() == 0);
}

void
TestColorSpace::testHsvToRgb()
{
    CPPUNIT_ASSERT(::ispc::testHsvToRgb() == 0);
}

void
TestColorSpace::testHslToRgb()
{
    CPPUNIT_ASSERT(::ispc::testHslToRgb() == 0);
}


