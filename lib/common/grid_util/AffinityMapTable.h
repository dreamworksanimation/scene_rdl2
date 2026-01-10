// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "Arg.h"
#include "Parser.h"
#include "ShmAffinityInfo.h"

#include <memory>
#include <string>

namespace scene_rdl2 {
namespace grid_util {

class AffinityMapTable
//
// This class maintains a CPU affinity mapping table by using shared memory. 
// This class provides 2 functionalities.
//  1) The process can save its CPU affinity information into the shared memory,
//     and this information would be shared with other processes by using this class.
//  2) This class smartly allocates the new CPU IDs which is not overlapped with
//     actively used CPUs. This is good for the decision of which CPUs are used for
//     affinity target on the newly booted process.
// We can run the new process without overlapping the CPU resources by using this
// class and maximize the performance easily.
//
// acquire() and release() function is multi-process safe function. You can execute
// them multi-process environment. This class properly initializes the shared memory
// data automatically if it has not been initialized yet. This means we don't need
// to explicitly initialization for the shared memory data itself.
// The multi-process safe access of the shared memory is executed by SYSTEM-V semaphore.
//
// This class only provides and maintains the CPU ID information and does not provide
// any affinity action itself.
//
{
public:
    using Arg = scene_rdl2::grid_util::Arg;
    using Parser = scene_rdl2::grid_util::Parser;

    AffinityMapTable(const bool testMode = false)
        : mTestMode {testMode}
    {
        parserConfigure();
    }

    // We have 2 runtime modes for this class. regular mode and test mode.
    // If test mode is on, all the actions of this class use test mode configuration,
    // which uses a different semaphore and shared memory from regular configurations.
    // This allows us to run a test program (and also unitTest) safely without making
    // any impact on the process that already uses the regular AffinityMapTable.
    void setTestMode(const bool mode) { mTestMode = mode; }
    bool getTestMode() const { return mTestMode; }

    // Multi-process safe function. Throw exception(std::string) if error
    std::string acquire(const int requestedThreadTotal, const float timeoutSec);

    // Multi-process safe function. Throw exception(std::string) if error
    void release(const float timeoutSec) const;

    std::string show() const;
    std::string showAffinityInfoManager() const;
    static std::string showSemaphoreInfo(const int testMode);

    // Output both tests on/off ShmAffinityInfo shared memory information without doing
    // a lock for debugging purposes.
    static std::string showShmAffinityInfoDump();

    // Dump both tests on/off SemaphoreInfo
    static std::string showSemaphoreInfoDump();

    // Dump both test on/off of SemaphoreInfo and ShmAffinityInfo
    static std::string showInfoDump();

    Parser& getParser() { return mParser; }

protected:
    using Msg = std::function<bool(const std::string& msg)>;

    enum class OpenCondition : int {
        UNDEFINED,
        INITIALIZED,
        ALREADY_EXISTED
    };

    void open(); // Throw exception(std::string) if error
    bool openMain(); // return true:OK false:needRetry : Throw exception(std::string) if error 

    static const char* getSemKeyStr(const bool testMode);    
    void setupFreshAffinityInfoManager(); // Throw exception(std::string) if error
    bool checkSemaphoreInitCompletion(); // Throw exception(std::string) if error
    Sha1Util::Hash genSemInitHash(const int semId) const;

    // Blocking wait until successfully lock semaphore with timeout control
    // Throw exception(std::string) if error
    // return true : lock succeesed
    //        false : lock timed out and lock failed
    bool lockSemaphoreBlockingWithTimeout(const float timeoutSec) const;

    void unlockSemaphore() const; // Throw exception(std::string) if error
    void removeSemaphore(const std::string& rmReason); // Throw exception(std::string) if error

    static void removeSemaphore(const int semId, const std::string& rmReason); // Throw exception(std::string) if error
    static int getSemaphoreId(const int testMode);

    void verifyAndCleanupAffinityInfo();

    static std::string openConditionStr(const OpenCondition& condition);

    void parserConfigure();
    bool testOpen(const Msg& msgFunc);
    bool emulateOpenCrash(const Msg& msgFunc);

    // An existing semaphore/shared-memory can be deleted only by its creator or by the root user.
    // If anyone other than the creator or root attempts to remove it, an error will occur.
    bool rmUnusedSemaphore(const bool testMode, const Msg& msgFunc); // try to clean up testMode semaphore if testMode shm is empty
    bool removeAllSemShm(const Msg& msgFunc);

    //------------------------------

    static constexpr const char* sSemaphoreKeyStr = "AffinityMapTable";
    static constexpr const char* sSemaphoreTestKeyStr = "AffinitMapTableTest";
    static constexpr const char* sSemaphoreInitCompleteHashStr = "AffinityMapTableSemaphoreInitialized";
    static constexpr float sOpenTimeoutSec {10.0}; // semaphore open timeout = 10sec
    static constexpr int sOpenRetry {3}; // semaphore open retry max

    bool mTestMode {false};

    OpenCondition mSemOpenCondition {OpenCondition::UNDEFINED};
    int mSemId {0};

    std::shared_ptr<ShmAffinityInfoManager> mAffinityInfoManager;

    std::string mCurrCpuIdDefStr;

    Parser mParser;
};

} // namespace grid_util
} // namespace scene_rdl2
