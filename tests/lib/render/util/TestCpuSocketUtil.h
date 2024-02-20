// Copyright 2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>

namespace scene_rdl2 {
namespace cpuSocketUtil {
namespace unittest {

class TestCpuSocketUtil : public CppUnit::TestFixture
{
public:
    void setUp() override {};
    void tearDown() override {};

    void testCpuIdDef();
    void testShowCpuIdTbl();
    void testSetupCpuInfo();

    CPPUNIT_TEST_SUITE(TestCpuSocketUtil);
    CPPUNIT_TEST(testCpuIdDef);
    CPPUNIT_TEST(testShowCpuIdTbl);
    CPPUNIT_TEST(testSetupCpuInfo);
    CPPUNIT_TEST_SUITE_END();
};

} // namespace unittest
} // namespace cpuSocketUtil
} // namespace scene_rdl2

