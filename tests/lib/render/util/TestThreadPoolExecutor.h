// Copyright 2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <scene_rdl2/render/util/ThreadPoolExecutor.h>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

namespace scene_rdl2 {
namespace threadPoolExecutor {
namespace unittest {

class TestThreadPoolExecutor : public CppUnit::TestFixture
{
public:
    void setUp() override {};
    void tearDown() override {};

    void testBootAndShutdown();

    CPPUNIT_TEST_SUITE(TestThreadPoolExecutor);
    CPPUNIT_TEST(testBootAndShutdown);
    CPPUNIT_TEST_SUITE_END();

private:
    enum class ThreadState : int {INIT, IDLE, BUSY};

    void bootWatcher(const float maxTestDurationSec);
    void watcherThreadMain(const float maxTestDurationSec);
    void shutdownWatcher();

    void bootAndShutdownLoop(const std::string& msg,
                             const int maxLoop,
                             const ThreadPoolExecutor::CalcCpuIdFunc& calcCpuIdFunc) const;

    //------------------------------

    std::atomic<ThreadState> mWatcherThreadState {ThreadState::INIT};
    bool mWatcherThreadShutdown {false};

    mutable std::mutex mWatcherMutex;
    std::thread mWatcherThread;
    std::condition_variable mCvWatcherBoot;
};

} // namespace unittest
} // namespace threadPoolExecutor
} // namespace scene_rdl2

