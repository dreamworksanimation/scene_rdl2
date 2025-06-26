// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>

namespace scene_rdl2 {
namespace grid_util {
namespace unittest {

class TestShmAffInfo : public CppUnit::TestFixture
{
public:
    void setUp() {}
    void tearDown() {}

    void testAffInfoDataSize();
    void testAffInfo();
    void testAffInfoManager();
    void testCoreAllocation();

    CPPUNIT_TEST_SUITE(TestShmAffInfo);
    CPPUNIT_TEST(testAffInfoDataSize);
    CPPUNIT_TEST(testAffInfo);
    CPPUNIT_TEST(testAffInfoManager);
    CPPUNIT_TEST(testCoreAllocation);
    CPPUNIT_TEST_SUITE_END();

protected:

    bool testAffInfoMain() const;

    void testAffInfoManagerMain(const int dataTypeId) const;

    int testCoreAllocationLoop(const std::string& modeStr,
                               const int randMaxLoopCount) const;
    int testCoreAllocationMain(const std::string& modeStr,
                               const int randMaxSize,
                               const int myPidUpdateInterval) const;

    bool rmOldShmAffInfo(const std::string& headMsg) const;
};

} // namespace unittest
} // namespace grid_util
} // namespace scene_rdl2
