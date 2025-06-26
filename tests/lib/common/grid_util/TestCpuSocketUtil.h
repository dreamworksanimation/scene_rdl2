// Copyright 2024-2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>

namespace scene_rdl2 {
namespace grid_util {
namespace unittest {

class TestCpuSocketUtil : public CppUnit::TestFixture
{
public:
    void setUp() override {};
    void tearDown() override {};

    void testCpuIdDef();
    void testShowCpuIdTbl();
    void testSetupCpuInfo();
    void testIdTblToDefStr();

    CPPUNIT_TEST_SUITE(TestCpuSocketUtil);
    CPPUNIT_TEST(testCpuIdDef);
    CPPUNIT_TEST(testShowCpuIdTbl);
    CPPUNIT_TEST(testSetupCpuInfo);
    CPPUNIT_TEST(testIdTblToDefStr);
    CPPUNIT_TEST_SUITE_END();

private:
    bool testIdTblToDefStrMain(const std::string& idTblDefStr) const;
};

} // namespace unittest
} // namespace grid_util
} // namespace scene_rdl2
