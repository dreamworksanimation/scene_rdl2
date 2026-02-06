// Copyright 2025-2026 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "Arg.h"
#include "Parser.h"
#include "ShmAffinityInfo.h"

#include <memory>
#include <string>

namespace scene_rdl2 {
namespace grid_util {

namespace affMapTbl_detail {
    class Helper;
}

class AffinityMapTable
//
// This class maintains a CPU affinity mapping table via shared memory.
// It provides two main functionalities:
//  1) Processes can save their CPU affinity information into shared memory,
//     allowing this information to be shared with other processes using this class.
//  2) The class intelligently allocates new CPU IDs that do not overlap with
//     actively used CPUs. This facilitates optimal CPU selection for affinity targets
//     when launching new processes.
// By using this class, new processes can run without overlapping CPU resources, thereby
// maximizing performance.
//
// The acquire() and release() functions are multi-process-safe and can be executed in a
// multi-process environment. This class automatically initializes the shared memory data
// if it has not been initialized yet, eliminating the need for explicit initialization.
// Multi-process-safe access to shared memory is implemented using SYSTEM-V semaphores.
//
// This class provides and maintains CPU ID information only; it does not perform any
// affinity actions itself.
//
// An important aspect of this design is that although any process can create the shared
// memory and semaphore, the fundamental policy is that once created, they must never be
// deleted. This is because deleting shared memory and semaphores requires either the owner
// that created them or root privileges; shared memory and semaphores created by others cannot
// be deleted. Therefore, for AffinityMapTable operations, if resources do not already exist,
// they will be automatically created; if they already exist, they will be reused. In normal
// operation, existing AffinityMapTable resources should never be deleted.
//
// If, for some reason such as a system malfunction, you need to completely delete the shared
// memory and semaphore, they must be deleted either by the owner who created them, by root,
// or by rebooting the system itself. In normal operation, existing shared memory and semaphores
// are never deleted. However, in unit tests, it may be necessary to delete these resources to
// test initialization and other procedures. To enable this, shared memory and semaphores for
// unit tests are created independently for each user ID running the tests.
// This AffinityMapTable operation mode for unit testing is referred to as "TestMode", and several
// TestMode flags exist in the related source code. In contrast, the production AffinityMapTable
// operation mode is called "LiveMode", which is also mentioned in comments throughout the code.
//
{
public:
    using Arg = scene_rdl2::grid_util::Arg;
    using Parser = scene_rdl2::grid_util::Parser;

    enum class TestKeyStrFormat : unsigned int {
        VER_0 = 0, // Original version: uses sSemaphoreTestKeyStr only
        VER_1      // Extended version: sSemaphoreTestKeyStr + '_' + userName
    };

    // In production releases, always use TestKeyStrFormat::VER_1 for formatVersion.
    // With VER_1, when testMode is enabled, shared memory and semaphores are created independently
    // for each user running the process, fundamentally preventing data collisions with other users.
    // With VER_0, shared memory and semaphores created in testMode were shared by all users, which
    // could cause unit tests to fail depending on the environment.
    AffinityMapTable(bool testMode = false,
                     TestKeyStrFormat formatVersion = TestKeyStrFormat::VER_1)
        : mTestMode {testMode}
        , mTestKeyStrFormatVersion {formatVersion}
    {
        parserConfigure();
    }

    // This class has two runtime modes: live mode (production) and test mode.
    // When test mode is enabled, all actions use test mode configuration,
    // which employs different semaphores and shared memory from live mode configurations.
    // This allows test programs (including unit tests) to run safely without impacting
    // processes that are already using the live mode AffinityMapTable.
    void setTestMode(bool mode) { mTestMode = mode; }
    bool getTestMode() const { return mTestMode; }

    void setTestKeyStrFormat(TestKeyStrFormat formatVersion) { mTestKeyStrFormatVersion = formatVersion; }
    TestKeyStrFormat getTestKeyStrFormat() const { return mTestKeyStrFormatVersion; }

    // Multi-process-safe function. Throws exception (std::string) on error.
    std::string acquire(int requestedThreadTotal,
                        float timeoutSec); // Semaphore access timeout in seconds

    // Multi-process-safe function. Throws exception (std::string) on error.
    void release(float timeoutSec) const;

    std::string show() const;
    std::string showAffinityInfoManager() const;
    static std::string showSemaphoreInfo(int testMode,
                                         TestKeyStrFormat formatVersion);
    static std::string showTestSemaphoreInfo(const std::string& userName,
                                             TestKeyStrFormat formatVersion);

    // Output ShmAffinityInfo shared memory information for both test and live modes
    // without acquiring a lock, for debugging purposes.
    static std::string showShmAffinityInfoDump(TestKeyStrFormat formatVersion);

    static std::string showSemaphoreInfoDump(TestKeyStrFormat formatVersion); // Dump semaphore info for both test and live modes
    static std::string showAllSemaphoreInfoList(); // List all AffinityMapTable-related semaphores

    // Dump both SemaphoreInfo and ShmAffinityInfo for test and live modes
    static std::string showInfoDump(TestKeyStrFormat formatVersion);
    static std::string showTestKeyStrFormat(TestKeyStrFormat formatVersion);

    static std::string showAllShmInfoList(); // List all AffinityMapTable-related shared memory

    Parser& getParser() { return mParser; }

protected:
    friend class affMapTbl_detail::Helper; // Helper class requires access to protected members

    using Msg = std::function<bool(const std::string& msg)>;

    enum class OpenCondition : int {
        UNDEFINED,
        INITIALIZED,
        ALREADY_EXISTED
    };

    void open(); // Throws exception (std::string) on error
    bool openMain(); // Returns true if successful, false if retry needed. Throws exception (std::string) on error. 

    static std::string getSemaphoreKeyStr(bool testMode, TestKeyStrFormat formatVersion);
    static std::string getSemaphoreTestKeyStr(const std::string& userName, // Uses current process's user name if userName is empty
                                              TestKeyStrFormat formatVersion);
    
    void setupFreshAffinityInfoManager(); // Throws exception (std::string) on error
    bool checkSemaphoreInitCompletion(); // Throws exception (std::string) on error
    Sha1Util::Hash genSemaphoreInitHash(const int semId) const;

    // Blocks until semaphore is successfully locked, with timeout control.
    // Throws exception (std::string) on error.
    // Returns true if lock succeeded, false if lock timed out and failed.
    bool lockSemaphoreBlockingWithTimeout(float timeoutSec) const;

    void unlockSemaphore() const; // Throws exception (std::string) on error
    void removeSemaphore(const std::string& rmReason); // Throws exception (std::string) on error

    void verifyAndCleanupAffinityInfo();

    void parserConfigure();
    bool testOpen(const Msg& msgFunc);
    bool emulateOpenCrash(const Msg& msgFunc);

    //------------------------------

    static constexpr const char* sSemaphoreKeyStr = "AffinityMapTable";
    static constexpr const char* sSemaphoreTestKeyStr = "AffinitMapTableTest";
    static constexpr const char* sSemaphoreInitCompleteHashStr = "AffinityMapTableSemaphoreInitialized";
    static constexpr float sOpenTimeoutSec {10.0}; // Semaphore open timeout: 10 seconds
    static constexpr int sOpenRetry {3}; // Maximum semaphore open retry attempts

    bool mTestMode {false};
    TestKeyStrFormat mTestKeyStrFormatVersion {TestKeyStrFormat::VER_1};

    OpenCondition mSemaphoreOpenCondition {OpenCondition::UNDEFINED};
    int mSemaphoreId {0};

    std::shared_ptr<ShmAffinityInfoManager> mAffinityInfoManager;

    std::string mCurrCpuIdDefStr;

    Parser mParser;
};

} // namespace grid_util
} // namespace scene_rdl2
