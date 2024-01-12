// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#pragma once

#include "RecTime.h"

#include <string>

namespace scene_rdl2 {
namespace rec_time {

class RecTick
//
// Simple get tick interval
//
{
public:
    RecTick() : mStartTick(0), mEndTick(0) {}

    inline void start();
    inline uint64_t end();

protected:
    uint64_t mStartTick;
    uint64_t mEndTick;
};

inline void
RecTick::start()
{
    uint32_t high, low;

    asm volatile("CPUID\n\t"
                 "RDTSC\n\t"
                 "mov %%edx, %0\n\t"
                 "mov %%eax, %1\n\t": "=r" (high), "=r" (low):: 
                 "%rax", "%rbx", "%rcx", "%rdx");

    mStartTick = ((uint64_t(high)) << 32) | low;
}

inline uint64_t
RecTick::end()
{
    uint32_t high, low;

    asm volatile("RDTSCP\n\t"
                 "mov %%edx, %0\n\t"
                 "mov %%eax, %1\n\t"
                 "CPUID\n\t": "=r" (high), "=r" (low):: 
                 "%rax", "%rbx", "%rcx", "%rdx");

    mEndTick = ((uint64_t(high)) << 32) | low;

    return mEndTick - mStartTick;
}

//------------------------------------------------------------------------------

class __attribute__((aligned(64))) RecTickLog
//
// Simple logging for tick value
//
{
public:
    RecTickLog() { reset(); }
    ~RecTickLog() {}

    void reset() { mAllTick = 0; mLastTick = 0; mTotal = 0; }
    bool isReset() const { return (mTotal == 0)? true: false; }
    void add(const uint64_t i) { mLastTick = i; mAllTick += i; ++mTotal; }
    uint64_t getAll() const { return mAllTick; }
    uint64_t getLast() const { return mLastTick; }
    uint64_t getAverage() const { return (mTotal)? mAllTick / mTotal: 0; }
    uint64_t getTotal() const { return mTotal; }

protected:
    uint64_t mAllTick;
    uint64_t mLastTick;
    uint64_t mTotal;
};

//------------------------------------------------------------------------------

class RecTickManualInterval
//
// get interval by tick and logging
//
{
public:
    RecTickManualInterval() {}
    RecTickManualInterval(std::string &&name) : mName(std::move(name)) {}

    const std::string &getName() const { return mName; }

    void start() { mLap.start(); }          // start interval
    void endAdd() { mLog.add(mLap.end()); } // end interval and add interval-tick to log

    uint64_t getAverage() const { return mLog.getAverage(); } // return average tick
    uint64_t getLast() const { return mLog.getLast(); } // return last tick

    void reset() { mLog.reset(); } // reset internal log
    bool isReset() const { return mLog.isReset(); }

protected:
    std::string mName;

    RecTick    mLap;
    RecTickLog mLog;
};

//------------------------------------------------------------------------------

class RecTickTimeManualInterval
//
// get interval by tick also time (sec) and logging
//
{
public:
    void reset() { mLogTime.reset(); mLogTick.reset(); }
    bool isReset() const { return mLogTick.isReset(); }
    void start() { mLapTime.start(); mLapTick.start(); }
    void endAddStart() {
        static const float MINIMUM_INTERVAL = 0.0f; // sec
        static const float MAXIMUM_INTERVAL = 5.0f; // sec
        float    time_interval = mLapTime.end();
        uint64_t tick_interval = mLapTick.end();
        if (MINIMUM_INTERVAL < time_interval && time_interval < MAXIMUM_INTERVAL) {
            mLogTime.add(time_interval);
            mLogTick.add(tick_interval);
        }
        mLapTime.start();
        mLapTick.start();
    }

    float getTimeAll() const { return mLogTime.getAll(); }
    float getTimeAverage() const { return mLogTime.getAverage(); }
    uint64_t getTickAverage() const { return mLogTick.getAverage(); }

protected:
    RecTime    mLapTime;
    RecTimeLog mLogTime;
    
    RecTick    mLapTick;
    RecTickLog mLogTick;
};

} // namespace rec_time
} // namespace scene_rdl2

