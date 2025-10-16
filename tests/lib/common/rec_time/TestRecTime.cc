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

const float intervalSec = 0.001f;
const unsigned maxLoop = 512;
#ifdef PLATFORM_APPLE
// On Mac, we need to allow a bigger overhead than Linux.
// 1.5 is an experimental value based on several different tests on the M4 MacBook Pro
const double threshRatio = 1.5;
#else // else of PLATFORM_APPLE
const double threshRatio = 1.1;
#endif // end of Not PLATFORM_APPLE

void
TestRecTime::testRecTime()
{
    rec_time::RecTime recTime;
    const double s = recTimeLoop(intervalSec, maxLoop,
                                 [&]() { recTime.start(); },
                                 [&]() { return static_cast<double>(recTime.end()); });
    const double ratio = s / intervalSec;
    const bool flag = ratio < threshRatio;
    std::cerr << "testRecTime(): s:" << s << " ratio:" << ratio << " flag:" << scene_rdl2::str_util::boolStr(flag) << '\n';
    CPPUNIT_ASSERT_MESSAGE("testRecTime", flag);
}

void
TestRecTime::testRecTimeVDSO()
{
    rec_time::RecTimeVDSO recTime;
    const double s = recTimeLoop(intervalSec, maxLoop,
                                 [&]() { recTime.start(); },
                                 [&]() { return recTime.end(); });
    const double ratio = s / intervalSec;
    const bool flag = ratio < threshRatio;
    std::cerr << "testRecTimeVDSO(): s:" << s << " ratio:" << ratio << " flag:" << scene_rdl2::str_util::boolStr(flag) << '\n';
    CPPUNIT_ASSERT_MESSAGE("testRecTimeVDSO", flag);
}

#ifndef PLATFORM_APPLE
void
TestRecTime::testRecTimeRDTSC()
{
    const double secPerCycle = rec_time::RecTimeRDTSC::getSecPerCycle();

    rec_time::RecTimeRDTSC recTime;
    const double tick = recTimeLoop(intervalSec, maxLoop,
                                    [&]() { recTime.start(); },
                                    [&]() { return recTime.end(); });
    const double sec = tick * secPerCycle;
    const double ratio = sec / intervalSec;
    const bool flag = ratio < threshRatio;
    std::cerr << "testRecTimeRDTSC(): tick:" << tick << " secPerCycle:" << secPerCycle << " sec:" << sec << " ratio:" << ratio
              << " flag:" << scene_rdl2::str_util::boolStr(flag) << '\n';
    CPPUNIT_ASSERT_MESSAGE("testRecTimeRDTSC", flag);
}
#endif // end of Not PLATFORM_APPLE

double
TestRecTime::recTimeLoop(const float intervalSec, const unsigned maxLoop,
                         const std::function<void()>& timeStartFunc,
                         const std::function<double()>& timeEndFunc) const
{
    double total = 0.0;
    const useconds_t us = static_cast<useconds_t>(intervalSec * 1000000);
    for (unsigned i = 0; i < maxLoop; ++i) {
        timeStartFunc();
        usleep(us);
        total += timeEndFunc();
    }
    return total / static_cast<double>(maxLoop);
}

} // namespace unittest
} // namespace grid_util
} // namespace scene_rdl2
