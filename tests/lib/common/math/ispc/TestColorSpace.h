// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file TestColorSpace.h
#pragma once

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

namespace scene_rdl2 {
namespace common {
namespace math {
namespace ispc {
namespace unittest {

class TestColorSpace : public CppUnit::TestFixture
{
public:
    void testRgbToHsv();
    void testRgbToHsl();
    void testHsvToRgb();
    void testHslToRgb();

    CPPUNIT_TEST_SUITE(TestColorSpace);
    CPPUNIT_TEST(testRgbToHsv);
    CPPUNIT_TEST(testRgbToHsl);
    CPPUNIT_TEST(testHsvToRgb);
    CPPUNIT_TEST(testHslToRgb);
    CPPUNIT_TEST_SUITE_END();
};

} // namespace scene_rdl2
} // namespace common
} // namespace math
} // namespace ispc
} // namespace unittest

