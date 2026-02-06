// Copyright 2026 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>

namespace scene_rdl2 {
namespace str_util {

class TestStrUtil : public CppUnit::TestFixture
{
public:
    void setUp() {}
    void tearDown() {}

    void testOctal3DigitsStr();
    void testPermissionStr();
    void testPermissionStrMacSysVSemaphore();

    CPPUNIT_TEST_SUITE(TestStrUtil);
    CPPUNIT_TEST(testOctal3DigitsStr);
    CPPUNIT_TEST(testPermissionStr);
    CPPUNIT_TEST(testPermissionStrMacSysVSemaphore);
    CPPUNIT_TEST_SUITE_END();
};

} // namespace str_util
} // namespace scene_rdl2
    
