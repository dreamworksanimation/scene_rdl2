// Copyright 2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "TestThreadPoolExecutor.h"

#include <scene_rdl2/common/rec_time/RecTime.h>

#include <unistd.h>

namespace scene_rdl2 {
namespace threadPoolExecutor {
namespace unittest {

CPPUNIT_TEST_SUITE_REGISTRATION(TestThreadPoolExecutor);

void
TestThreadPoolExecutor::testBootAndShutdown()
{
    std::cerr << "TestThreadPoolExecutor.cc testBootAndShutdown() start\n";

    constexpr float maxTestDurationSec = 4.0f;
    bootWatcher(maxTestDurationSec);

    constexpr int maxLoop = 10;
    bootAndShutdownLoop("no-CPU-Affinity", maxLoop, nullptr);
    bootAndShutdownLoop("CPU-Affinity", maxLoop, [](size_t id) -> size_t { return id; });

    shutdownWatcher();

    std::cerr << "TestThreadPoolExecutor.cc testBootAndShutdown() finish\n";    
}

void
TestThreadPoolExecutor::bootAndShutdownLoop(const std::string& msg,
                                            const int maxLoop,
                                            const ThreadPoolExecutor::CalcCpuIdFunc& calcCpuIdFunc) const
{
    unsigned threadTotal = std::thread::hardware_concurrency();

    std::cerr << msg << " {\n";
    for (int loopId = 0; loopId < maxLoop; ++loopId) {
        std::cerr << "  loopId:" << loopId << '/' << maxLoop - 1 << " threadTotal:" << threadTotal << '\n';

        ThreadPoolExecutor pool(threadTotal, calcCpuIdFunc);
        CPPUNIT_ASSERT(pool.testBootShutdown());
    }
    std::cerr << "}\n";
}

//------------------------------------------------------------------------------------------
    
void
TestThreadPoolExecutor::bootWatcher(const float maxTestDurationSec)
{
    mWatcherThreadState = ThreadState::INIT; // just in case
    mWatcherThread = std::move(std::thread([&] { watcherThreadMain(maxTestDurationSec); }));

    { // Wait until thread is booted
        std::unique_lock<std::mutex> uqLock(mWatcherMutex);
        mCvWatcherBoot.wait(uqLock, [&]() {
                return (mWatcherThreadState != ThreadState::INIT); // Not wait if already non INIT condition
            });
    }
}

void
TestThreadPoolExecutor::watcherThreadMain(const float maxTestDurationSec)
//
// This thread is watching the test thread is properly finished under the expected time duration.
// If the time exceeds the limit, the process exits itself. This is important to keep unitTest
// finished under a constant period.
//    
{
    // First of all change threadState condition and do notify_one to caller
    {
        std::lock_guard<std::mutex> lock(mWatcherMutex);
        mWatcherThreadState = ThreadState::IDLE;
    }
    mCvWatcherBoot.notify_one(); // notify to ThreadExecutor constructor
    
    rec_time::RecTime recTime;
    recTime.start();

    while (true) {
        if (mWatcherThreadShutdown) break;
        if (recTime.end() >= maxTestDurationSec) {
            std::cerr << "ERROR : watcherThread detected too long test execution."
                      << " duration:" << maxTestDurationSec << " sec\n";
            exit(1);
        }
        usleep(10000);
    }

    std::cerr << ">> Watcher thread shutdown <<\n";
}

void
TestThreadPoolExecutor::shutdownWatcher()
{
    mWatcherThreadShutdown = true;
    mWatcherThread.join();
}

} // namespace unittest
} // namespace threadPoolExecutor
} // namespace scene_rdl2


