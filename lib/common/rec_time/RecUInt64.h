// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#pragma once

namespace scene_rdl2 {
namespace rec_time {

class __attribute__((aligned(64))) RecUInt64Log
//
// Simple logging for uint64 value
//
{
public:
    RecUInt64Log() { reset(); }
    ~RecUInt64Log() {}

    void reset() { mAll = 0; mLast = 0; mTotal = 0; }
    bool isReset() const { return (mTotal == 0)? true: false; }
    void add(const uint64_t v) { mLast = v; mAll += v; ++mTotal; }
    uint64_t getAll() const { return mAll; }
    uint64_t getLast() const { return mLast; }
    double getAverage() const { return (mTotal)? (double)mAll / (double)mTotal: 0.0; }
    uint64_t getTotal() const { return mTotal; }

protected:
    uint64_t mAll;
    uint64_t mLast;
    uint64_t mTotal;
};

class RecUInt64ManualInterval
//
// set uint64 interval value and logging
//
{
public:
    RecUInt64ManualInterval() {}
    RecUInt64ManualInterval(std::string &&name) : mName(std::move(name)) {}

    const std::string &getName() const { return mName; }

    void add(const uint64_t v) { mLog.add(v); } // add interval double value to log

    double getAverage() const { return mLog.getAverage(); } // return average value
    uint64_t getAll() const { return mLog.getAll(); } // return all value
    uint64_t getLast() const { return mLog.getLast(); } // return last value

    void reset() { mLog.reset(); } // reset internal log
    bool isReset() const { return mLog.isReset(); }
    
protected:
    std::string mName;

    RecUInt64Log mLog;
};

} // namespace rec_time
} // namespace scene_rdl2

