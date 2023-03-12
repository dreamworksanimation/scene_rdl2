// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

namespace scene_rdl2 {
namespace fb_util {
namespace unittest {

class TestPixelBuffer : public CppUnit::TestFixture
{
public:
    void setUp();
    void tearDown();

    void testClear();

    CPPUNIT_TEST_SUITE(TestPixelBuffer);
    CPPUNIT_TEST(testClear);
    CPPUNIT_TEST_SUITE_END();
};

} // namespace unittest
} // namespace rndr
} // namespace scene_rdl2

