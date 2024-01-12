// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//

#include "RecTime.h"

#include <sstream>

namespace scene_rdl2 {
namespace rec_time {

void
RecTimeAutoInterval::showInterval(const std::string &msg,
                                  const float msgIntervalSec,
                                  void (*msgOutFunc)(const std::string &))
{
    static const float MINIMUM_INTERVAL = 0.0f; // sec
    static const float MAXIMUM_INTERVAL = 5.0f; // sec

    float lap = mLap.end();
    if (MINIMUM_INTERVAL < lap && lap < MAXIMUM_INTERVAL) {
        mLog.add(lap);
    }
    mLap.start();

    if (mLog.getAll() > msgIntervalSec) {
        float interval = mLog.getAverage();
        float fps = 1.0f / interval;

        std::ostringstream ostr;
        ostr << msg << " interval:" << fps << " fps";
        if (msgOutFunc) {
            (*msgOutFunc)(ostr.str());
        }

        mLog.reset();
    }
}

} // namespace rec_time
} // namespace scene_rdl2

