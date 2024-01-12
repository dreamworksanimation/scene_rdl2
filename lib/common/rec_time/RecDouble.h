// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#pragma once

namespace scene_rdl2 {
namespace rec_time {

class __attribute__((aligned(64))) RecDoubleLog
//
// Simple logging for double value
//
{
public:
    RecDoubleLog() { reset(); }
    ~RecDoubleLog() {}

    void reset() { mAll = 0.0; mLast = 0.0; mTotal = 0; }
    bool isReset() const { return (mTotal == 0)? true: false; }
    void add(const double v) { mLast = v; mAll += v; ++mTotal; }
    double getAll() const { return mAll; }
    double getLast() const { return mLast; }
    double getAverage() const { return (mTotal)? mAll / (double)mTotal: 0.0; }
    double getTotal() const { return (double)mTotal; }
    
protected:
    double mAll;
    double mLast;
    uint64_t mTotal;
};

class RecDoubleManualInterval
//
// set double interval value and logging
//
{
public:
    RecDoubleManualInterval() {}
    RecDoubleManualInterval(std::string &&name) : mName(std::move(name)) {}

    const std::string &getName() const { return mName; }

    void add(const double v) { mLog.add(v); } // add interval double value to log

    double getAverage() const { return mLog.getAverage(); } // return average value
    double getLast() const { return mLog.getLast(); } // return last value

    void reset() { mLog.reset(); } // reset internal log
    bool isReset() const { return mLog.isReset(); }
    
protected:
    std::string mName;

    RecDoubleLog mLog;
};

} // namespace rec_time
} // namespace scene_rdl2

