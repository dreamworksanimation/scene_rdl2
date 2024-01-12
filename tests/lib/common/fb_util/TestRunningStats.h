// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

namespace scene_rdl2 {
namespace fb_util {
namespace unittest {

class TestRunningStats : public CppUnit::TestFixture
{
public:
    void setUp();
    void tearDown();

    void testRunningStats();

    CPPUNIT_TEST_SUITE(TestRunningStats);
    CPPUNIT_TEST(testRunningStats);
    CPPUNIT_TEST_SUITE_END();
};

} // namespace unittest
} // namespace fb_util
} // namespace scene_rdl2

