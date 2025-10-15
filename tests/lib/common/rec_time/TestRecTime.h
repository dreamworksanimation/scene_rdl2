// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>

#include <functional> 

namespace scene_rdl2 {
namespace grid_util {
namespace unittest {

class TestRecTime : public CppUnit::TestFixture
{
public:
    void setUp() {}
    void tearDown() {}

    void testRecTime();
    void testRecTimeVDSO();
#ifndef PLATFORM_APPLE
    void testRecTimeRDTSC();
#endif // end of Not PLATFORM_APPLE

    CPPUNIT_TEST_SUITE(TestRecTime);
    CPPUNIT_TEST(testRecTime);
    CPPUNIT_TEST(testRecTimeVDSO);
#ifndef PLATFORM_APPLE
    CPPUNIT_TEST(testRecTimeRDTSC);
#endif // end of Not PLATFORM_APPLE
    CPPUNIT_TEST_SUITE_END();

private:
    double recTimeLoop(const float intervalSec,
                       const unsigned maxLoop,
                       const std::function<void()>& timeStartFunc,
                       const std::function<double()>& timeEndFunc) const;
};

} // namespace unittest
} // namespace grid_util
} // namespace scene_rdl2
