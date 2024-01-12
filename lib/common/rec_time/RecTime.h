// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <sstream>

#include <sys/time.h>
#include <stdint.h>

namespace scene_rdl2 {
namespace rec_time {

class RecTime
//
// Simple get time interval
//
{
public:
    RecTime() : mStartTime(0) {}

    inline void reset() { mStartTime = 0; }
    inline bool isInit() const { return (mStartTime == 0)? true: false; }

    inline void start() { mStartTime = getCurrentMicroSec(); }
    inline float end() const { return (float)(getCurrentMicroSec() - mStartTime) * 0.000001f; } // return sec

    static long long getCurrentMicroSec() {
        struct timeval tv;
        gettimeofday(&tv, 0x0);
        long long cTime = (long long)tv.tv_sec * 1000 * 1000 + (long long)tv.tv_usec;
        return cTime;
    }

protected:
    long long mStartTime;
};

class __attribute__((aligned(64))) RecTimeLog
//
// Simple logging for time value
//
{
public:
    RecTimeLog() { reset(); }
    ~RecTimeLog() {}

    void reset() { mAll = 0.0f; mLast = 0.0f; mTotal = 0; }
    void add(const float sec) { mLast = sec; mAll += sec; ++mTotal; }
    float getAll() const { return mAll; } // return sec
    float getLast() const { return mLast; } // return sec
    float getAverage() const { return (mTotal)? mAll / (float)mTotal: 0; } // return sec
    uint64_t getTotal() const { return mTotal; }

protected:
    float mAll;                 // sec
    float mLast;                // sec
    uint64_t mTotal;
};

class RecTimeAutoInterval
//
// Show interval information by simple API
//
{
public:
    float getLastSec() const { return mLog.getLast(); } // return last sec
    float getLastMsec() const { return getLastSec() * 1000.0f; } // return last milli sec 

    bool minBoundCheck(const float minMsec, void (*msgOutFunc)(const std::string &)) {
        float cLastMsec = getLastMsec();
        if (cLastMsec < minMsec) {
            std::ostringstream ostr;
            ostr << "minBound error " << cLastMsec << " ms < min:" << minMsec << " ms";
            (*msgOutFunc)(ostr.str());
            return false;       // NG
        }
        return true;            // OK
    }

    void showInterval(const std::string &msg, const float msgIntervalSec, void (*msgOutFunc)(const std::string &));

protected:
    RecTime    mLap;
    RecTimeLog mLog;
};

} // namespace rec_time
} // namespace scene_rdl2
