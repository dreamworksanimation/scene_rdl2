// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "TestRunningStats.h"
#include <scene_rdl2/common/fb_util/RunningStats.h>

namespace scene_rdl2 {
namespace fb_util {
namespace unittest {

void
TestRunningStats::setUp()
{
}

void
TestRunningStats::tearDown()
{
}

void
TestRunningStats::testRunningStats()
{
    RunningStatsLightWeight<double> stats;
    stats.push(0.2f);
    stats.push(0.3f);
    stats.push(0.9f);
    stats.push(1.9f);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.609167, stats.variance(), 0.00001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.825, stats.mean(), 0.00001);
}

} // namespace unittest
} // namespace fb_util
} // namespace scene_rdl2

