// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "TestRecTime.h"

#include <scene_rdl2/common/rec_time/RecTime.h>
#include <scene_rdl2/render/util/StrUtil.h>

#include <unistd.h>
#include <iostream>

namespace scene_rdl2 {
namespace grid_util {
namespace unittest {

constexpr double threshRatio = 5.0;

void
TestRecTime::testRecTime()
{
    std::cerr << ">> testRecTime()\n";
    rec_time::RecTime recTime;
    const bool flag = recTimeOverheadEstimationLoop(threshRatio,
                                                    [&]() { recTime.start(); },
                                                    [&]() { return static_cast<double>(recTime.end()); });
    CPPUNIT_ASSERT_MESSAGE("testRecTime", flag);
}

void
TestRecTime::testRecTimeVDSO()
{
    std::cerr << ">> testRecTimeVDSO()\n";
    rec_time::RecTimeVDSO recTime;
    const bool flag = recTimeOverheadEstimationLoop(threshRatio,
                                                    [&]() { recTime.start(); },
                                                    [&]() { return recTime.end(); });
    CPPUNIT_ASSERT_MESSAGE("testRecTimeVDSO", flag);
}

#ifndef PLATFORM_APPLE
void
TestRecTime::testRecTimeRDTSC()
{
    std::cerr << ">> testRecTimeRDTSC()\n";
    const double secPerCycle = rec_time::RecTimeRDTSC::getSecPerCycle();

    rec_time::RecTimeRDTSC recTime;
    const bool flag =
        recTimeOverheadEstimationLoop(threshRatio,
                                      [&]() {
                                          // Timing measurement start TSC is saved inside the
                                          // RecTimeRDTSC object, and it is processed as counter
                                          // value itself.
                                          recTime.start();
                                      },
                                      [&]() {
                                          // end() returns the delta TSC value and needs to be
                                          // converted to seconds here.
                                          return static_cast<double>(recTime.end()) * secPerCycle;
                                      });
    CPPUNIT_ASSERT_MESSAGE("testRecTimeRDTSC", flag);
}
#endif // end of Not PLATFORM_APPLE

//------------------------------------------------------------------------------------------

double
TestRecTime::recTimeLoop(const double intervalSec,
                         const unsigned maxLoop,
                         const std::function<void()>& timeStartFunc,
                         const std::function<double()>& timeEndFunc) const
// timeEndFunc returns 0.0 or a negative if an error
{
    double total = 0.0;
    const useconds_t us = static_cast<useconds_t>(intervalSec * 1000000);
    for (unsigned i = 0; i < maxLoop; ++i) {
        timeStartFunc();
        usleep(us);
        const double sec = timeEndFunc();
        if (sec <= 0.0) return -1.0; // error
        total += sec;
    }
    return total / static_cast<double>(maxLoop);
}

double
TestRecTime::recTimeOverheadEstimation(const std::function<void()>& timeStartFunc,
                                       const std::function<double()>& timeEndFunc) const
//
// In this unit test, we measure the overhead of recTime while gradually varying the interval and
// the number of trials. For each trial, we check how far the overhead deviates from the mean and
// verify that it falls within five times the average. With this approach, even if the overhead is
// large, we consider it acceptable as long as measurements across various patterns show a reasonably
// consistent overhead. As a criterion for "reasonably consistent," we use up to five times the mean.
// However, if an interval measurement is 0.0, it is treated as an immediate error.
//
{
    //                id :   0  1  2  3  4  5  6   7
    // iteration (count) : 128 64 32 16  8  4  2   1
    //  interval (ms)    :   1  2  4  8 16 32 64 128
    constexpr int maxIteration = 5;
    std::vector<double> tbl(maxIteration, 0.0);

    double intervalSec = 0.001; // 1 ms
    unsigned maxLoop = 128;
    double totalOverheadSec = 0.0;
    for (int loopId = 0; loopId < maxIteration; ++loopId) {
        const double recTimeLoopResult = recTimeLoop(intervalSec, maxLoop, timeStartFunc, timeEndFunc);
        if (recTimeLoopResult < 0.0) {
            std::cerr << "ERROR : recTimeLoop returned negative value\n";
            return -1.0; // error
        }
        const double overheadSec = recTimeLoopResult - intervalSec;
        tbl[loopId] = overheadSec;
        totalOverheadSec += overheadSec;
        intervalSec *= 2.0;
        maxLoop /= 2;
    }
    const double overheadAvg = totalOverheadSec / static_cast<double>(maxIteration);

    std::cerr << "avg:" << overheadAvg << '\n';
    double maxDeltaRatio = 0.0;
    for (int loopId = 0; loopId < maxIteration; ++loopId) {
        const double delta = std::abs(tbl[loopId] - overheadAvg);
        const double ratio = delta / overheadAvg;
        maxDeltaRatio = std::max(maxDeltaRatio, ratio);
        std::cerr << "loopId:" << loopId << " " << tbl[loopId] << " delta:" << delta
                  << " ratio:" << ratio << " maxDeltaRatio:" << maxDeltaRatio << '\n';
    }

    return maxDeltaRatio;
}

bool
TestRecTime::recTimeOverheadEstimationLoop(const double threshRatio,
                                           const std::function<void()>& timeStartFunc,
                                           const std::function<double()>& timeEndFunc) const
//
// Furthermore, even if an error occurs, the test will attempt to retry several times.
//
{
    constexpr int maxLoop = 10;
    for (int loopId = 0; loopId < maxLoop; ++loopId) {
        std::cerr << "testId:" << loopId << '\n';
        const double maxDeltaRatio = recTimeOverheadEstimation(timeStartFunc, timeEndFunc);
        if (maxDeltaRatio < 0.0) return false; // error

        // This is the retry logic used when recTimeOverheadEstimation returns a poor result.
        // If a valid value is returned, it is immediately considered a success.
        if (maxDeltaRatio <= threshRatio) return true;
    }
    return false;
}

} // namespace unittest
} // namespace grid_util
} // namespace scene_rdl2
