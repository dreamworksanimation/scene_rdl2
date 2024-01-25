// Copyright 2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <queue>
#include <thread>
#include <vector>

namespace scene_rdl2 {

class ThreadPoolExecutor;

class ThreadExecutor
//
// This class is in charge of single thread boot, exec, and shutdown for thread pool.
// The booted thread will get the execution task from the task queue of ThreadPoolExecutor.
// If the task queue is empty, this thread is waited by condition_wait until the new task is
// enqueued or shutdown.
//
{
public:
    enum class ThreadState : int {INIT, IDLE, BUSY, FINISH};

    ThreadExecutor() = default;
    ~ThreadExecutor();

    // Might throw except::RuntimeError
    void boot(size_t threadId, ThreadPoolExecutor* poolExecutor, int pinCpuId = ~static_cast<int>(0));

    ThreadState getThreadState() const { return mThreadState; }

    static std::string threadStateStr(const ThreadState& stat);

private:

    void threadMain();
    void pinThreadToCpu(); // might throw except::RuntimeError

    //------------------------------

    size_t mThreadId {0};
    int mPinCpuId {~static_cast<int>(0)};
    ThreadPoolExecutor* mPoolExecutor {nullptr};

    std::atomic<ThreadState> mThreadState {ThreadState::INIT};
    bool mThreadShutdown {false};

    mutable std::mutex mMutex;
    std::thread mThread;
    std::condition_variable mCvBoot;
};

class ThreadPoolExecutor
//
// This class provides a thread pool with CPU-affinity control.
// You can enqueue the task by run() multiple times and the enqueued task is executed in one of
// the available pooled threads in parallel. When you set CPU_affinity control functions
// (i.e. cpuIdFunc of argument), internal pool threads are attached to the particular CPUid which
// is calculated by cpuIdFunc.
//
// == How to use ==
// Following is a pseudo code example
//
// try {
//     ThreadPoolExecutor pool(32); // ... (A)
//
//     for (int taskId = 0; taskId < taskMax; ++taskId) {
//         pool.run([] { // ... (B)
//              ... some task ...
//         });
//     }
//     pool.wait(); // ... (C)
// } // ... (D)
// catch (scene_rdl2::except::RuntimeError& e) { // ... (E)
//     std::cerr << e.what() << '\n';
//     return; // error exit
// }
//
// A) Need to construct ThreadPoolExecutor.
//    In this case, the pool size is 32, and no CPU-affinity control
//    After the construction of ThreadPoolExecutor, all pool threads are booted internally but
//    it is a waiting condition and waiting for a new task.
// B) Enqueueing the task.
//    A task is defined as a function object. This task is processed by one of the available pool
//    threads in parallel. In this case, total taskMax tasks are processed in parallel by the max of
//    32 threads.
// C) Waiting for all the tasks finished
//    The caller thread is waiting until all enqueued tasks (B) are finished.
// D) Destruction of ThreadPoolExecutor
//    All internally generated pool threads are shut down inside the destructor of ThreadpoolExecutor.
//    You can also call shutdown() explicitly.
// E) Error handling
//    ThreadPoolExecutor constructor throws an exception when it fails. You should catch the exception.
//
// If you want to add CPU-affinity control, you have to set a function object which calculates cpuId
// from threadId. ThreadId is from 0 to 31 in this case because this example uses 32 pool size at (A).
//
// This is an example of CPU-affinity control that assigns the same cpuId of threadId :
//
//    ThreadPoolExecutor pool(32,
//                            [](size_t threadId) -> size_t {
//                                return threadId;
//                            }); // ... (A)'
//
// Using (A)' instead of (A) does CPU-affinity control. ThreadId=0 is running on CPUid=0, threadId=1
// is running on CPUid=1, and so on.
//
{
public:
    using TaskFunc = std::function<void()>;
    using CalcCpuIdFunc = std::function<size_t(size_t threadId)>;

    // threadTotal = 0 means set same number of all cpus
    ThreadPoolExecutor(size_t threadTotal = 0, const CalcCpuIdFunc& cpuIdFunc = nullptr);
    ~ThreadPoolExecutor() { shutdown(); }

    void run(const TaskFunc& task); // MTsafe
    void wait(); // wait until all queued tasks are processed

    void shutdown();

    //------------------------------
    //
    // internally used APIs
    //
    TaskFunc taskDequeue(); // blocking MTsafe
    void decrementActiveTaskCounter(); // MTsafe

    //------------------------------
    //
    // testing function
    //
    bool testBootShutdown(); // only used for testing purposes

private:

    bool isShutdownComplete();

    //------------------------------

    std::vector<ThreadExecutor> mThreadTbl;

    bool mShutdown {false};
    std::mutex mTaskMutex;
    std::mutex mWaitMutex;
    std::queue<TaskFunc> mTasks;
    std::condition_variable mCvTask;
    std::condition_variable mCvWait;
    std::atomic<int> mActiveTask {0};
};

} // namespace scene_rdl2
