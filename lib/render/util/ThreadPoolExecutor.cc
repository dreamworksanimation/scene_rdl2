// Copyright 2024-2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "CpuAffinityMask.h"
#include "ThreadPoolExecutor.h"

#include <scene_rdl2/common/except/exceptions.h>

#include <iostream>
#include <pthread.h> // pthread_setaffinity_np
#include <sstream>
#include <unistd.h> // usleep

//#define DEBUG_MSG_THREAD
//#define DEBUG_MSG_THREAD_CPUAFFINITY
//#define DEBUG_MSG_THREAD_POOL

// This directive computes the average time of multiple runs of shutdown() function execution.
// This directive should not commented out for the release version.
//#define DEBUG_MSG_SHUTDOWN_TIME

#ifdef DEBUG_MSG_SHUTDOWN_TIME
#include <scene_rdl2/common/rec_time/RecTime.h>

namespace {
    
class TimeMeasurementOfShutdown
{
public:
    ~TimeMeasurementOfShutdown() { if (!mTotal) std::cerr << "avg " << mTime / mTotal << '\n'; };
    void push(float time) { mTime += time; mTotal++; }
private:
    size_t mTotal {0};
    float mTime {0.0f};
};

static TimeMeasurementOfShutdown shutdownTime;

} // namespace

#endif // end DEBUG_MSG_SHUTDOWN_TIME

namespace scene_rdl2 {

ThreadExecutor::~ThreadExecutor()
{
    mThreadShutdown = true; // This is the only place mThreadShutdown is set to true
    if (mThread.joinable()) mThread.join();
}

void
ThreadExecutor::boot(size_t threadId, ThreadPoolExecutor* poolExecutor, int pinCpuId)
{
    mThreadId = threadId;
    mPinCpuId = pinCpuId;
    mPoolExecutor = poolExecutor;

#   ifdef DEBUG_MSG_THREAD
    std::ostringstream ostr;
    ostr << ">> ThreadPoolExecutor.cc boot threadId:" << threadId << " passA\n";
    std::cerr << ostr.str();
#   endif // end DEBUG_MSG_THREAD

    mThreadState = ThreadState::INIT; // just in case
    mThread = std::move(std::thread([&] { threadMain(); })); // We have to build thread

#   ifdef DEBUG_MSG_THREAD
    ostr.str("");
    ostr << ">> ThreadPoolExecutor.cc boot threadId:" << threadId << " passB\n";
    std::cerr << ostr.str();
#   endif // end DEBUG_MSG_THREAD

    { // Wait until thread is booted
        std::unique_lock<std::mutex> uqLock(mMutex);
        mCvBoot.wait(uqLock, [&]() {
                return (mThreadState != ThreadState::INIT); // Not wait if already non INIT condition
            });
    }

#   ifdef DEBUG_MSG_THREAD
    ostr.str("");
    ostr << ">> ThreadPoolExecutor.cc boot threadId:" << threadId << " passC\n";
    std::cerr << ostr.str();
#   endif // end DEBUG_MSG_THREAD
}

// static function
std::string
ThreadExecutor::threadStateStr(const ThreadState& stat)
{
    switch (stat) {
    case ThreadState::INIT : return "INIT";
    case ThreadState::IDLE : return "IDLE";
    case ThreadState::BUSY : return "BUSY";
    case ThreadState::FINISH : return "FINISH";
    default : break;
    }
    return "?";
}

void
ThreadExecutor::threadMain()
{
    pinThreadToCpu();

    // First of all change threadState condition and do notify_one to caller
    {
        std::lock_guard<std::mutex> lock(mMutex);
        mThreadState = ThreadState::IDLE;
    }
    mCvBoot.notify_one(); // notify to ThreadExecutor constructor

    //------------------------------

#   ifdef DEBUG_MSG_THREAD
    std::ostringstream ostr;
    ostr << ">> ThreadExecutor::threadMain() ... threadId:" << mThreadId << '\n';
    std::cerr << ostr.str();
#   endif // end DEBUG_MSG_THREAD

    while (true) {
        // This call is blocked until the new task is ready.
        const ThreadPoolExecutor::TaskFunc& func = mPoolExecutor->taskDequeue();
#       ifdef DEBUG_MSG_THREAD
        ostr.str("");
        ostr << ">> ThreadExecutor::threadMain() ... threadId:" << mThreadId << " taskDequeue\n";
        std::cerr << ostr.str();
#       endif // end DEBUG_MSG_THREAD
        if (!func) break;

        if (mThreadShutdown) break; // before task shutdown check

        mThreadState = ThreadState::BUSY;
        {
            func();

            // After finishing the task, notify condition changing to the threadPoolExecutor.wait()
            mPoolExecutor->decrementActiveTaskCounter();
        }
        mThreadState = ThreadState::IDLE;

        if (mThreadShutdown) break; // after task shutdown check
    }

    mThreadState = ThreadState::FINISH;

#   ifdef DEBUG_MSG_THREAD
    ostr.str("");
    ostr << ">> ThreadExecutor::threadMain() ... threadId:" << mThreadId << " done\n";
    std::cerr << ostr.str();
#   endif // end DEBUG_MSG_THREAD
}

void
ThreadExecutor::pinThreadToCpu()
//
// might throw except::RuntimeError when it fails
//
{
#ifndef PLATFORM_APPLE
    if (mPinCpuId == ~static_cast<int>(0)) return; // no cpu affinity

    auto showError = [&](const std::string& funcName, const bool flag) {
        auto errorStr = [](int errorNo) {
            switch (errorNo) {
            case EFAULT : return "EFAULT";
            case EINVAL : return "EINVAL";
            case ESRCH : return "ESRCH";
            default : break;
            }
            return "?";
        };
        std::ostringstream ostr;
        ostr << "ERROR : " << funcName << " failed. errorNo:" << flag << " (" << errorStr(flag) << ")\n";
        return ostr.str();
    };

    CpuAffinityMask mask; // might throw exception
    mask.set(mPinCpuId);

    pthread_t thread = pthread_self();
    int flag = pthread_setaffinity_np(thread, mask.getMaskSize(), mask.getMaskPtr());
    if (flag != 0) {
        std::ostringstream ostr;
        ostr << showError("pthread_setaffinity_np()", flag);
        throw except::RuntimeError(ostr.str());
    }

#   ifdef DEBUG_MSG_THREAD_CPUAFFINITY
    flag = pthread_getaffinity_np(thread, mask.getMaskSize(), mask.getMaskPtr());
    if (flag != 0) {
        std::ostringstream ostr;
        ostr << showError("pthread_getaffinity_np()", flag);
        throw except::RuntimeError(ostr.str());
    } else {
        std::ostringstream ostr;
        ostr << "Set returned by pthread_getaffinity_np()"
             << " threadId:" << mThreadId
             << " requested:" << mPinCpuId
             << " contained:";
        for (size_t j = 0; j < mask.getNumCpu(); ++j) {
            if (mask.isSet(j)) ostr << "CPU" << j << ' ';
        }
        ostr << '\n';
        std::cerr << ostr.str();
    }
#   endif // end DEBUG_MSG_THREAD_CPUAFFINITY
#endif // end ifndef PLATFORM_APPLE
}

//------------------------------------------------------------------------------------------

ThreadPoolExecutor::ThreadPoolExecutor(size_t threadTotal, const CalcCpuIdFunc& cpuIdFunc)
    : mThreadTbl { (threadTotal == 0) ? std::thread::hardware_concurrency() : threadTotal }
//
// Might throw except::RuntimeError when it fails
//
{
    auto cpuId = [&](size_t id) {
        return (!cpuIdFunc) ? ~static_cast<int>(0) : static_cast<int>(cpuIdFunc(id));
    };

    // sequentially boot all threads here.
    for (size_t threadId = 0; threadId < mThreadTbl.size(); ++threadId) {
        mThreadTbl[threadId].boot(threadId, this, cpuId(threadId));
    }

#   ifdef DEBUG_MSG_THREAD_POOL
    std::ostringstream ostr;
    ostr << ">> ThreadPoolExecutor.cc ThreadPoolExecutor() threadTotal:" << threadTotal << " done\n";
    std::cerr << ostr.str();
#   endif // end DEBUG_MSG_THREAD_POOL
}

void
ThreadPoolExecutor::run(const TaskFunc& task)
{
    {
        std::lock_guard<std::mutex> lock(mTaskMutex);
#       ifdef DEBUG_MSG_THREAD_POOL
        std::cerr << ">> ThreadPoolExecutor.cc run()\n";
#       endif // end DEBUG_MSG_THREAD_POOL
        mTasks.push(task);
    }
    mCvTask.notify_one();
}

void
ThreadPoolExecutor::wait()
{
    std::unique_lock<std::mutex> uqLock(mWaitMutex);
#   ifdef DEBUG_MSG_THREAD_POOL
    std::cerr << ">> ThreadPoolExecutor.cc wait()\n";
#   endif // end DEBUG_MSG_THREAD_POOL
    mCvWait.wait(uqLock, [&] { return mTasks.empty() && mActiveTask == 0; });
}

void
ThreadPoolExecutor::shutdown()
{
#ifdef DEBUG_MSG_SHUTDOWN_TIME
    rec_time::RecTime recTime;
    recTime.start();
#endif // end DEBUG_MSG_SHUTDOWN_TIME

    //
    // In the endurance test of ThreadPoolExecutor, We got lots of shutdown hangups due to some threads
    // that could not wake up even if we sent notify_all().
    // To properly shutdown all threads, We do retry to execute notify_all().
    //
    // This is a busy loop with no wait.
    // This code was tested by unitTest (TestThreadPoolExecutor) w/ ENDURANCE_TEST mode and passed for
    // over 1,500,000 runs without hang-up.
    // The current average time of this shutdown() function on cobaltcard 128 HT-cores 
    // (AMD Ryzen Threadripper PRO 5995WX 64-Cores) of 10,000 runs is around 2 ~ 3 ms.
    //    
    while (true) {
        mShutdown = true;
        mCvTask.notify_all();

        if (isShutdownComplete()) break;
    }

#ifdef DEBUG_MSG_SHUTDOWN_TIME
    shutdownTime.push(recTime.end());
#endif // end DEBUG_MSG_SHUTDOWN_TIME
}

ThreadPoolExecutor::TaskFunc
ThreadPoolExecutor::taskDequeue()
{
    std::unique_lock<std::mutex> uqLock(mTaskMutex);
    mCvTask.wait(uqLock, [&] { return !mTasks.empty() || mShutdown; });
    if (mTasks.empty() && mShutdown) return nullptr;

    TaskFunc func = std::move(mTasks.front());
    mTasks.pop();
    ++mActiveTask;

    return func;
}

void
ThreadPoolExecutor::decrementActiveTaskCounter()
{
    {
        std::lock_guard<std::mutex> lock(mWaitMutex);
        --mActiveTask;
    }
    mCvWait.notify_one();
}

bool
ThreadPoolExecutor::testBootShutdown()
//
// boot and shutdown test
//
{
    size_t threadTotal = mThreadTbl.size();

    std::atomic<int> bootedThreadTotal {0};
    std::atomic<int> sum {0};
    for (size_t threadId = 0; threadId < threadTotal; ++threadId) {
        run([&bootedThreadTotal, &sum, threadId, threadTotal] {
                // This simulates MoonRay's MCRT thread boot logic
                ++bootedThreadTotal;
                while (bootedThreadTotal < threadTotal) {
                    struct timespec tm;
                    tm.tv_sec = 0;
                    tm.tv_nsec = 1000; // 0.001ms
                    nanosleep(&tm, NULL); // yield CPU resources
                }

                sum += static_cast<int>(threadId);
            });
    }

    wait();

    int target = 0;
    for (size_t threadId = 0; threadId < threadTotal; ++threadId) target += threadId;

    return (sum == target);
}

bool
ThreadPoolExecutor::isShutdownComplete()
{
    for (const auto& itr : mThreadTbl) {
        if (itr.getThreadState() != ThreadExecutor::ThreadState::FINISH) return false;
    }
    return true;
}

} // namespace scene_rdl2
