// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//

#pragma once

#include <cppunit/extensions/HelperMacros.h>

#include <scene_rdl2/common/math/ColorSpace.h>

class TestCommonColorSpace: public CppUnit::TestCase {
public:
    CPPUNIT_TEST_SUITE(TestCommonColorSpace);
    CPPUNIT_TEST(testRgbToHsv);
    CPPUNIT_TEST(testRgbToHsl);
    CPPUNIT_TEST(testHsvToRgb);
    CPPUNIT_TEST(testHslToRgb);
    CPPUNIT_TEST_SUITE_END();

    void testRgbToHsv();
    void testRgbToHsl();
    void testHsvToRgb();
    void testHslToRgb();
};

