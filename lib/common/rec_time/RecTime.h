// Copyright 2023-2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <sstream>

#include <sys/time.h>
#include <stdint.h>
#include <time.h> // clock_gettime()

#ifdef PLATFORM_APPLE
#include <mach/mach_time.h> // mach_timebase_info(), mach_absolute_time()
#else // else of PLATFORM_APPLE
#include <unistd.h> // usleep()
#include <x86intrin.h> // __rdtscp()
#endif // end of Non PLATFORM_APPLE

namespace scene_rdl2 {
namespace rec_time {

class RecTime
//
// Simple get time interval
//
// This class uses gettimeofday (kernel call), but it is converted as a VDSO version around glibc 2.17
// (around the end of 2012, RHEL7/CentOS7/Rocky7). On Rocky9, gettimeofday is a VDSO call and much faster
// than a Kernel call.
//
{
public:
    RecTime() : mStartTime(0) {}

    inline void reset() { mStartTime = 0; }
    inline bool isInit() const { return (mStartTime == 0)? true: false; }

    inline void start() { mStartTime = getCurrentMicroSec(); }
    inline float end() const { return (float)(getCurrentMicroSec() - mStartTime) * 0.000001f; } // return sec

    static uint64_t getCurrentMicroSec() // MTsafe
    {
        struct timeval tv;
        gettimeofday(&tv, 0x0);
        return static_cast<uint64_t>(tv.tv_sec) * 1000000 + static_cast<uint64_t>(tv.tv_usec);
    }

protected:
    uint64_t mStartTime;
};

class RecTimeVDSO
//
// VDSO = Virtual Dynamic Shared Object
//
// Performance-wise, this is the equivalent of the above RecTime version.
//
{
public:
    RecTimeVDSO() {}

    void reset() { mStartTime = 0; }
    bool isInit() { return mStartTime == 0; }

    void start() { mStartTime = getCurrentNanoSec(); }
    double end() { return static_cast<double>(getCurrentNanoSec() - mStartTime) * 0.000000001; } // return sec

    static uint64_t getCurrentNanoSec()
    {
        timespec ts;
        clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
        return static_cast<uint64_t>(ts.tv_sec) * 1000000000ull + static_cast<uint64_t>(ts.tv_nsec); // return ns
    }

protected:
    uint64_t mStartTime {0};
};

#ifdef PLATFORM_APPLE

class RecTimeMach
{
public:
    RecTimeMach() {}

    void reset() { mStartTime = 0; }
    bool isInit() { return mStartTime == 0; }

    static double getSecPerCycle()
    {
        //
        // This secPerCycle is constant between single processes, so you run this API once and share that
        // information for all the conversions of counter to sec.
        //
        mach_timebase_info_data_t info;
        mach_timebase_info(&info);
        return static_cast<double>(info.numer) / static_cast<double>(info.denom) / 1e9;
    }

    void start() { mStartTime = mach_absolute_time(); }
    uint64_t end() { return mach_absolute_time() - mStartTime; } // timeSec = (double)end() * secPerCycle

protected:
    uint64_t mStartTime {0};
};

#else // end of PLATFORM_APPLE

class RecTimeRDTSC
//
// RDTSC : ReaD Time Stamp Counter
//
// This class is designed under the assumption that the CPU meets the following two conditions described below.
// You can check whether this class will function correctly by examining the CPU status flags obtained with the
// following command:
//
// > cat /proc/cpuinfo | grep tsc
//
// 1) The TSC must increment at a constant rate, independent of CPU clock frequency changes.
//    This can be verified by the "constant_tsc" flag.
// 2) The TSC must continue running even when the CPU is in a sleep state or C-state (CPU Idle state /
//    power-saving mode). This can be verified by the "nonstop_tsc" flag.
//
// This class is designed for interval time measurement using __rdtscp, and it is extremely fast
// (2ns ~ 10ns range). The condition described above are the minimum runtime requirements. However, even if
// those conditions are satisfied, on machines which multiple NUMA-nodes, you must assume that the TSCs
// across different NUMA nodes are not perfectly synchronized. Therefore, in a multi-threaded environment,
// if a particular thread is migrated to another core due to context switching, there is a possibility that
// the TSC values may differ between cores, leading to inaccurate measurements.
// There is a safe logic and end() returns 0 if cpuId does not match between the previous measurement timing
// and the current. This avoids the situation of wrong measurement for CPU migration during measurement.
// However, this also loses some measurement results and reduces the accuracy.
//
// To completely eliminate this risk, you should pin the measurement thread to a specific core. (in other words,
// use CPU affinity control to fix the thread to a particular core during measurement.)
//
{
public:
    RecTimeRDTSC() {}

    void reset() { mStartTime = 0; }
    bool isInit() { return mStartTime == 0; }

    static double getSecPerCycle() // Calculate TSC seconds per cycle
    {
        //
        // Since Nehalem (micro-architecture by Intel, around 2008), both Intel and AMD have designed
        // their CPUs so that the TSC operates as an independent, constant-rate clock source. Therefore,
        // the TSC frequency required for rdtsc only needs to be measured once, sometime after the process
        // starts.
        // Be aware that this function takes about 100ms to execute.
        // Due to the slowness of this API, you should run this API once and share this information for all
        // conversions of counter to sec.
        //
        //   double tscSecPerCycle = RecTimeRDTSC::getSecPerCycle();
        //     ...
        //   RecTimeRDTSC recTime;
        //   recTime.start(); // timing measurement start
        //     ...
        //   double sec = (double)recTime.end() * tscSecPerCycle; // timing measurement end
        //
        const uint64_t c0 = __rdtsc();
        const uint64_t ns0 = RecTimeVDSO::getCurrentNanoSec(); // ns
        usleep(100000); // 100ms
        const uint64_t c1 = __rdtsc();
        const uint64_t ns1 = RecTimeVDSO::getCurrentNanoSec(); // ns
        const double deltaNs = static_cast<double>(ns1 - ns0);
        const double tscFreq = static_cast<double>(c1 - c0) / deltaNs;
        return 1.0 / tscFreq / 1e9;
    }

    void start() { mStartTime = __rdtscp(&mCpuId); }
    uint64_t end() // timeSec = (double)end() * secPerCycle
    {
        unsigned cpuId;
        const uint64_t t = __rdtscp(&cpuId);
        if (cpuId != mCpuId) return 0ull;
        return t - mStartTime;
    }

protected:
    unsigned mCpuId {0};
    uint64_t mStartTime {0};
};

#endif // end of Not PLATFORM_APPLE

//------------------------------------------------------------------------------------------

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

    bool minBoundCheck(const float minMsec, void (*msgOutFunc)(const std::string &))
    {
        const float cLastMsec = getLastMsec();
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
