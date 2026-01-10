// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "AffinityMapTable.h"
#include "Process.h"
#include "Sha1Util.h"

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include <scene_rdl2/common/platform/Platform.h> // MNRY_ASSERT
#include <scene_rdl2/common/rec_time/RecTime.h>

namespace {

static int
generateSemaphoreKey(const std::string& keyStr)
{
    const int workKey = scene_rdl2::grid_util::ShmDataManager::genInt32KeyBySHA1(keyStr);
    
    // We need positive int value to return.
    // Following code always generate positive int value with same distribution of input workKey.
    // We use 10^9 + 7 for this computation (pretty big prime number and can save into 32bit int
    // 2^29 < (10^9 + 7) < 2^30
    // 10^9 for modVal is better than INT_MAX actually to keep better distribution.
    constexpr int modVal = 1000000007;
    return ((workKey % modVal) + modVal) % modVal;
}

} // namespace

namespace scene_rdl2 {
namespace grid_util {

std::string
AffinityMapTable::acquire(const int requestedThreadTotal, const float timeoutSec)
//
// Multi-Process safe function
// Throw exception(std::string) if error.
// Return acquired CPUs as cpuId def string.
//
{
    try {
        open();

        if (!lockSemaphoreBlockingWithTimeout(timeoutSec)) {
            // timeout
            std::ostringstream ostr;
            ostr << "AffinityMapTable::acquire() timed out. (timeoutSec:" << timeoutSec << ")";
            throw ostr.str();
        }
        {
            verifyAndCleanupAffinityInfo();
            mCurrCpuIdDefStr = mAffinityInfoManager->acquireAffinityCores(requestedThreadTotal);
        }
        unlockSemaphore();
    }
    catch (const std::string& err) {
        std::ostringstream ostr;
        ostr << "AffinityMapTable::acquire() failed. error=>{\n"
             << str_util::addIndent(err) << '\n'
             << "}";
        throw ostr.str();
    }

    return mCurrCpuIdDefStr;
}

void
AffinityMapTable::release(const float timeoutSec) const
//
// Multi-Process safe function
//
{
    if (mCurrCpuIdDefStr.empty()) return;

    MNRY_ASSERT(mAffinityInfoManager);

    try {
        if (!lockSemaphoreBlockingWithTimeout(timeoutSec)) {
            // timeout
            std::ostringstream ostr;
            ostr << "AffinityMapTable::acquire() timed out. (timeoutSec:" << timeoutSec << ")";
            throw ostr.str();
                   
        }
        {
            mAffinityInfoManager->releaseAffinityCores(mCurrCpuIdDefStr);
        }
        unlockSemaphore();
    }
    catch (const std::string& err) {
        std::ostringstream ostr;
        ostr << "AffinityMapTable::release() failed. error=>{\n"
             << str_util::addIndent(err) << '\n'
             << "}";
        throw ostr.str();
    }
}

std::string
AffinityMapTable::show() const
{
    std::ostringstream ostr;
    ostr << "AffinityMapTable {\n"
         << "  sSemaphoreKeyStr:" << sSemaphoreKeyStr << '\n'
         << "  sSemaphoreTestKeyStr:" << sSemaphoreTestKeyStr << '\n'
         << "  sSemaphoreInitCompleteHashStr:" << sSemaphoreInitCompleteHashStr << '\n'
         << "  sOpenTimeoutSec:" << sOpenTimeoutSec << " sec\n"
         << "  sOpenRetry:" << sOpenRetry << '\n'
         << "  mTestMode:" << str_util::boolStr(mTestMode) << '\n'
         << "  mSemOpenCondition:" << openConditionStr(mSemOpenCondition) << '\n'
         << "  mSemId:" << mSemId << '\n'
         << str_util::addIndent(showAffinityInfoManager()) << '\n'
         << "  mCurrCpuIdDefStr:" << mCurrCpuIdDefStr << '\n'
         << "}";
    return ostr.str();
}

std::string
AffinityMapTable::showAffinityInfoManager() const
{
    if (!mAffinityInfoManager) return "mAffinityInfoManager is empty";
    return mAffinityInfoManager->show();
}

// static function
std::string
AffinityMapTable::showSemaphoreInfo(const int testMode)
{
    const int semId = getSemaphoreId(testMode);
    const char* const semKeyStr = getSemKeyStr(testMode);
    const int semKey = generateSemaphoreKey(semKeyStr);

    std::ostringstream ostr;
    ostr << "Semaphore info {\n"
         << "  mTestMode:" << str_util::boolStr(testMode) << '\n'
         << "  semKeyStr:" << semKeyStr << '\n'
         << "  semKey:0x" << std::hex << std::setw(8) << std::setfill('0') << semKey << std::dec << '\n';
    if (semId == -1) {
        ostr << "  semId: NOT_EXISTED\n";
    } else {
        ostr << "  semId:" << semId << '\n';
    }
    ostr << "}";
    return ostr.str();
}

// static function
std::string
AffinityMapTable::showShmAffinityInfoDump()
//
// Output both tests on/off ShmAffinityInfo shared memory information without doing
// a lock for debugging purposes.
//
{
    std::ostringstream ostr;
    ostr << "ShmAffinityInfo {\n"
         << str_util::addIndent(ShmAffinityInfoManager::showShmDump(false)) << '\n'
         << str_util::addIndent(ShmAffinityInfoManager::showShmDump(true)) << '\n'
         << "}";
    return ostr.str();
}

// static function
std::string
AffinityMapTable::showSemaphoreInfoDump()
{
    std::ostringstream ostr;
    ostr << "SemaphoreInfo {\n"
         << str_util::addIndent(showSemaphoreInfo(false)) << '\n'
         << str_util::addIndent(showSemaphoreInfo(true)) << '\n'
         << "}";
    return ostr.str();
}

// static function
std::string
AffinityMapTable::showInfoDump()
{
    std::ostringstream ostr;
    ostr << "info {\n"
         << "  testMode:false {\n"
         << str_util::addIndent(showSemaphoreInfo(false), 2) << '\n'
         << str_util::addIndent(ShmAffinityInfoManager::showShmDump(false), 2) << '\n'
         << "  }\n"
         << "  testMode:true {\n"
         << str_util::addIndent(showSemaphoreInfo(true), 2) << '\n'
         << str_util::addIndent(ShmAffinityInfoManager::showShmDump(true), 2) << '\n'
         << "  }\n"
         << "}";
    return ostr.str();
}

//------------------------------------------------------------------------------------------

void
AffinityMapTable::open()
//
// Throw exception(std::string) if error.
// return true  : open completed OK
//        false : open failed
//
{
    for (int i = 0; i < sOpenRetry; ++i) {
        if (openMain()) {
            return; // open completed without error.
        }
    }

    // we tried to open it multiple times but had no luck
    std::ostringstream ostr;
    ostr << "AffinityMapTable::open() retry " << sOpenRetry << " times failed.";
    throw ostr.str();
}

bool
AffinityMapTable::openMain()
//
// Throw exception(std::string) if error.
// return true : open completed OK
//        false : retry required with a fresh semaphore
//
{
    //
    // open semaphore
    //
    const int semKey = generateSemaphoreKey(getSemKeyStr(mTestMode)); 
    mSemId = semget(semKey,
                    1, // number of semaphore inside semaphore-set
                    IPC_CREAT | IPC_EXCL | 0666);
    if (mSemId != -1) {
        //
        // Freshly created semaphore and initialized here. This is done by single process.
        //
        semctl(mSemId,
               0, // target semaphore index. start from 0
               SETVAL, 1); // set semaphore value to 1

        setupFreshAffinityInfoManager();
        // After finishing the initialization, the semInitHash value is changed to the actual
        // hash value. This indicates that the related semaphore has been properly constructed
        // and initialization has been done explicitly atomically.

        mSemOpenCondition = OpenCondition::INITIALIZED;

    } else {
        //
        // Failed to create a fresh semaphore, so try to get an already existing semaphore here.
        //
        mSemId = semget(semKey,
                        1, // number of semaphore inside semaphore-set
                        0666);
        if (mSemId == -1) {
            std::ostringstream ostr;
            ostr << "AffinityMapTable::openMain() failed. Could not get already existed semId."
                 << " testMode:" << str_util::boolStr(mTestMode)
                 << " semaphoreKeyStr:" << getSemKeyStr(mTestMode)
                 << " semKey:0x" << std::hex << semKey << std::dec;
            throw ostr.str();
        }

        rec_time::RecTime recTime;
        recTime.start();

        //
        // We have to wait until the semaphore has been properly initialized. To do this,
        // we have to check semInitHash value is changed to the actual hash that indicates
        // initialization completion. We also considered avoiding deadlock by timeout.
        //
        while (!checkSemaphoreInitCompletion()) {
            if (recTime.end() > sOpenTimeoutSec) {
                //
                // timeout!
                //
                // Something is going wrong. This looks like previously, the semaphore was created
                // but that process was crashed before setup semInitHash value into the shared memory.
                // We will retry with a fresh semaphore in this case.
                // This situation might happen in a multi-process environment. i.e. multiple processes
                // are trying to remove the same semaphore simultaneously. This is as expected and OK.
                // We can safely clean up the target semaphore. Retry the open procedure and try to
                // create a semaphore with the same key and will create a different semId safely.
                // 
                std::cerr << ">>>>> TIMEOUT : AffinityMapTable open semaphore timeout"
                          << " semId:" << mSemId << " -> start retry <<<<<\n";
                removeSemaphore("Timeout and retry of AffinityMapTable open"); // throw exception if error
                return false;
            }

            usleep(10000); // 10 millisec sleep to yield CPU resources.
        }

        mSemOpenCondition = OpenCondition::ALREADY_EXISTED;
    }

    return true;
}

// static function
const char*
AffinityMapTable::getSemKeyStr(const bool testMode)
{
    return (testMode) ? sSemaphoreTestKeyStr : sSemaphoreKeyStr;
}

void
AffinityMapTable::setupFreshAffinityInfoManager()
//
// Throw exception(std:string) if error
//
{
    if (!mAffinityInfoManager) {
        try {
            //
            // Shared memory does not exist. We will construct fresh shared memory here.
            // This construction is executed in multi-process-safe manner and only a single process
            // constructs shared memory.
            //
            constexpr bool accessOnlyFlag = false;
            mAffinityInfoManager = std::make_shared<ShmAffinityInfoManager>(accessOnlyFlag, mTestMode);
        }
        catch (const std::string& err) {
            std::ostringstream ostr;
            ostr << "AffinityMapTable::setupFreshAffinityInfoManager() failed. error=>{\n"
                 << str_util::addIndent(err) << '\n'
                 << "}";
            throw ostr.str();
        }
    }

    {
        const Sha1Util::Hash hash = genSemInitHash(mSemId);
        /* useful debug message
        std::cerr << ">> AffinityMapTable.cc setupFreshAffinityInfoManager() {\n"
                  << "  sSemaphoreInitCompleteHashStr:" << sSemaphoreInitCompleteHashStr << '\n'
                  << "  mSemId:" << mSemId << '\n'
                  << "  hash:" << Sha1Util::show(hash) << '\n'
                  << "}\n";
        */
        
        // Setup semaphore initialize complete hash to the shared memory
        mAffinityInfoManager->getAffinityInfo().setSemInitHash(hash);
    }
}

bool
AffinityMapTable::checkSemaphoreInitCompletion()
//
// Throw exception(std::string) if error
// return true  : semaphore has initialized properly
//        false : needs retry 
//
{
    if (!mAffinityInfoManager) {
        try {
            if (!ShmAffinityInfoManager::doesShmAlreadyExist(mTestMode)) {
                setupFreshAffinityInfoManager();
                return false; // exec this function again
            }

            // access already existed affinityInfo
            constexpr bool accessOnlyFlag = true;
            mAffinityInfoManager = std::make_shared<ShmAffinityInfoManager>(accessOnlyFlag, mTestMode);
        }
        catch (const std::string& err) {
            std::ostringstream ostr;
            ostr << "AffinityMapTable::checkSemaphoreInitCompletion() failed. error=>{\n"
                 << str_util::addIndent(err) << '\n'
                 << "}";
            throw ostr.str();
        }
    }
            
    if (mAffinityInfoManager->getAffinityInfo().getSemInitHash() != genSemInitHash(mSemId)) {
        return false; // semaphore is not initialized yet
    }
    return true; // semaphore has initialized properly
}

Sha1Util::Hash
AffinityMapTable::genSemInitHash(const int semId) const
{
    std::ostringstream ostr;
    ostr << sSemaphoreInitCompleteHashStr << " semId:" << semId;
    return Sha1Util::hash(ostr.str());
}

#ifdef PLATFORM_APPLE
bool
AffinityMapTable::lockSemaphoreBlockingWithTimeout(const float timeoutSec) const
//
// Blocking wait until successfully lock sempahore with timeout control.
// return true : lock succeesed
//        false : lock timed out and lock failed
// Throw exception(std::string) if error
//
{
    //
    // Unfortunately, there is no timeout control option for SysV semaphore on the Mac.
    // We will do timeout emulation by a loop with a short sleep
    //
    struct sembuf op;
    op.sem_num = 0; // semaphore index
    op.sem_op = -1; // P operation (decrement)
    op.sem_flg = SEM_UNDO | IPC_NOWAIT; // release lock when process hangs up with no wait return if val is 0

    rec_time::RecTime recTime;
    recTime.start();

    while (true) {
        if (recTime.end() >= timeoutSec) return false; // timed out
        if (semop(mSemId, &op, 1) == 0) break;

        if (errno == EAGAIN) {
            usleep(1000); // 1 millisec sleep to yield CPU resources.
        } else {
            // This is error
            std::ostringstream ostr;
            ostr << "AffinityMapTable::lockSemaphoreBlockingWithTimeout() failed. error=>{\n"
                 << str_util::addIndent(strerror(errno)) << '\n'
                 << "}";
            throw ostr.str();
        }
    }
    return true; // lock succeeded
}
#else // !PLATFORM_APPLE
bool
AffinityMapTable::lockSemaphoreBlockingWithTimeout(const float timeoutSec) const
//
// Blocking wait until successfully lock sempahore with timeout control.
// return true : lock succeesed
//        false : lock timed out and lock failed
// Throw exception(std::string) if error
//
{
    struct sembuf op;
    op.sem_num = 0; // semaphore index
    op.sem_op = -1; // P operation (decrement)
    op.sem_flg = SEM_UNDO; // release lock when process hangs up

    struct timespec timeout;
    double intPart;
    timeout.tv_nsec = static_cast<long>(modf(timeoutSec, &intPart) * 1e9);
    timeout.tv_sec = static_cast<time_t>(intPart);

    if (semtimedop(mSemId,
                   &op,
                   1, // num of operations
                   &timeout) == -1) {
        if (errno == EAGAIN) {
            // timedout
            return false;
        } else {
            // error
            std::ostringstream ostr;
            ostr << "AffinityMapTable::lockSemaphoreBlockingWithTimeout() failed. error=>{\n"
                 << str_util::addIndent(strerror(errno)) << '\n'
                 << "}";
            throw ostr.str();
        }
    }
    return true;
}
#endif // end of !PLATFORM_APPLE

void
AffinityMapTable::unlockSemaphore() const
//
// Throw exception(std::string) if error
//
{
    struct sembuf op;
    op.sem_num = 0; // semaphore index
    op.sem_op = 1; // V operation (increment)
    op.sem_flg = SEM_UNDO; // releasel lock when process hangs up
    if (semop(mSemId, &op, 1 /* num of operations */ ) == -1) {
        std::ostringstream ostr;
        ostr << "AffinityMapTable::unlockSemaphore() failed. error=>{\n"
             << str_util::addIndent(strerror(errno)) << '\n'
             << "}";
        throw ostr.str();
    }
}

void
AffinityMapTable::removeSemaphore(const std::string& rmReason)
//
// Throw exception(std::string) if error
//
// An existing semaphore can be deleted only by its creator or by the root user.
// If anyone other than the creator or root attempts to remove it, an error will occur.
//
{
    removeSemaphore(mSemId, rmReason);
    mSemId = 0;
}

// static function
void
AffinityMapTable::removeSemaphore(const int semId, const std::string& rmReason)
//
// Throw exception(std::string) if error
//
// An existing semaphore can be deleted only by its creator or by the root user.
// If anyone other than the creator or root attempts to remove it, an error will occur.
//
{
    if (semctl(semId,
               0, // semaphore index
               IPC_RMID) == -1) {
        std::ostringstream ostr;
        if (!rmReason.empty()) {
            ostr << "Tried to remove semaphore as the reason of: " << rmReason << '\n';
            if (rmReason.find("Timeout") != std::string::npos) {
                // Special message for "timeout and retry"
                ostr << "If a user attempts to delete a semaphore created by another user, an error will occur.\n"
                     << "This is because a semaphore can be deleted only by its creator or by the root user.\n"
                     << "If this semaphore deletion issue is related to the AffinityMapTable open timeout and \n"
                     << "occurs during a retry process, please try manually deleting the semaphore using the \n"
                     << "account that created it, or as root.\n";
            }
        }
        ostr << "AffinityMapTable::removeSemaphore() failed. semId:" << semId << " error=>{\n"
             << str_util::addIndent(strerror(errno)) << '\n'
             << "}";
        throw ostr.str();
    }
}

// static function
int
AffinityMapTable::getSemaphoreId(const int testMode)
//
// return -1 if it does not exist
//
{
    const char* const semKeyStr = getSemKeyStr(testMode);
    const int semKey = generateSemaphoreKey(semKeyStr);
    return semget(semKey, 0, 0);
}

void
AffinityMapTable::verifyAndCleanupAffinityInfo()
{
    MNRY_ASSERT(mAffinityInfoManager);

    ShmAffinityInfo& affinityInfo = mAffinityInfoManager->getAffinityInfo();

    unsigned coreTotal = affinityInfo.getNumCores();
    for (unsigned coreId = 0; coreId < coreTotal; ++coreId) {
        bool occupancy;
        size_t pid;
        if (!affinityInfo.getCoreInfo(coreId, occupancy, pid)) continue;

        if (occupancy) {
            if (!processExists(static_cast<pid_t>(pid))) {
                // Can not find the target process -> try to disable this affinity info
                affinityInfo.setCoreInfo(coreId, false, 0);
            }
        } else {
            affinityInfo.initCoreInfo(coreId); // Just in case
        }
    }
}

// static function
std::string
AffinityMapTable::openConditionStr(const OpenCondition& condition)
{
    switch (condition) {
    case OpenCondition::UNDEFINED : return "UNDEFINED";
    case OpenCondition::INITIALIZED : return "INITIALIZED";
    case OpenCondition::ALREADY_EXISTED : return "ALREADY_EXISTED";
    default : return "?";
    }
}

void
AffinityMapTable::parserConfigure()
{
    mParser.description("AffinityMapTable command");

    mParser.opt("show", "", "show all info",
                [&](Arg& arg) { return arg.msg(show() + '\n'); });
    mParser.opt("testMode", "<on|off|show>", "set testMode",
                [&](Arg& arg) {
                    if (arg() == "show") arg++;
                    else mTestMode = (arg++).as<bool>(0);
                    return arg.fmtMsg("mTestMode %s\n", str_util::boolStr(mTestMode).c_str());
                });
    mParser.opt("open", "", "execute open procedures",
                [&](Arg& arg) {
                    return testOpen([&](const std::string& msg) { return arg.msg(msg); });
                });
    mParser.opt("emulateOpenCrash", "", "emulate crash at open operation for testing purposes",
                [&](Arg& arg) {
                    return emulateOpenCrash([&](const std::string& msg) { return arg.msg(msg); });
                });
    mParser.opt("removeAllSemShm", "", "rm all semaphore and shared memory then create initial environment",
                [&](Arg& arg) {
                    return removeAllSemShm([&](const std::string& msg) { return arg.msg(msg); });
                });
    mParser.opt("rmUnusedSemaphore", "", "rm unused testMode semaphore if possible",
                [&](Arg& arg) {
                    return rmUnusedSemaphore(true, [&](const std::string& msg) { return arg.msg(msg); });
                });
    mParser.opt("rmShmIfAlreadyExist", "", "remove testMode ShmAffinityInfo if exist",
                [&](Arg& arg) {
                    return ShmAffinityInfoManager::
                        rmShmIfAlreadyExistCmd(true,
                                               [&](const std::string& msg) { return arg.msg(msg); });
                });
    mParser.opt("infoDump", "", "semaphore and shmAffinityInfo info dump",
                [&](Arg& arg) { return arg.msg(showInfoDump() + '\n'); });
    mParser.opt("shmInfoDump", "", "shmAffinityInfo related info all dump without semaphore lock",
                [&](Arg& arg) { return arg.msg(showShmAffinityInfoDump() + '\n'); });
    mParser.opt("semInfoDump", "", "semaphore related info dump",
                [&](Arg& arg) { return arg.msg(showSemaphoreInfoDump() + '\n'); });
    mParser.opt("affinityInfoManager", "...command...", "affinityInfoManager command",
                [&](Arg& arg) {
                    if (!mAffinityInfoManager) {
                        arg.msg("mAffinityInfoManager is empty\n");
                        return false;
                    }
                    return mAffinityInfoManager->getParser().main(arg.childArg());
                });
}

bool
AffinityMapTable::testOpen(const Msg& msgFunc)
{
    try {
        open();
    }
    catch (const std::string& err) {
        std::ostringstream ostr;
        ostr << "open() failed. error=>{\n"
             << str_util::addIndent(err) << '\n'
             << "}";
        msgFunc(ostr.str() + '\n');
        return false;
    }

    msgFunc("===>>> open() OK <<<===\n");

    return true;
}

bool
AffinityMapTable::emulateOpenCrash(const Msg& msgFunc)
{
    if (!testOpen(msgFunc)) {
        return false;
    }

    // intentionally overwrite sem-init-hash by all empty values to emulate crash at open() timing.
    mAffinityInfoManager->getAffinityInfo().setSemInitHash(Sha1Util::init());

    std::ostringstream ostr;
    ostr << "===>>> emulateOpenCrash() OK <<<===\n"
         << show();
    msgFunc(ostr.str() + '\n');

    return true;
}

bool
AffinityMapTable::rmUnusedSemaphore(const bool testMode, const Msg& msgFunc)
//
// An existing semaphore can be deleted only by its creator or by the root user.
// If anyone other than the creator or root attempts to remove it, an error will occur.
//
{
    try {
        //
        // We clean up the testMode semaphore only. Basically, this semaphore is used only by unitTest,
        // and we can safely remove it. However, regular semaphore is not. It is used by MoonRay itself
        // and is very risky to remove without checking whether the MoonRay process is running or not.
        // At this moment, we don’t provide any remove semaphore APIs for regular semaphores.
        //
        if (!ShmAffinityInfoManager::doesShmAlreadyExist(testMode)) {
            //
            // We cannot find ShmAffinityInfo for testMode = on. So we remove the semaphore that
            // is related to the testMode shared memory.
            //
            int semId = getSemaphoreId(testMode);
            if (semId != -1) {
                removeSemaphore(semId, "unused semaphore clean up");

                std::ostringstream ostr;
                ostr << "Removed unused semaphore. semId:" << semId;
                msgFunc(ostr.str() + '\n');
            }
        }
    }
    catch (const std::string& err) {
        std::ostringstream ostr;
        ostr << "ERROR : AffinityMapTable::rmUnusedSemaphore() failed. err=>{\n"
             << str_util::addIndent(err) << '\n'
             << "}";
        msgFunc(ostr.str() + '\n');
        return false;
    }
    return true;
}

bool
AffinityMapTable::removeAllSemShm(const Msg& msgFunc)
//
// An existing semaphore/shared-memory can be deleted only by its creator or by the root user.
// If anyone other than the creator or root attempts to remove it, an error will occur.
//
{
    bool result = true;
    if (!ShmAffinityInfoManager::rmShmIfAlreadyExistCmd(true, msgFunc)) {
        result = false;
        msgFunc("remove testMode ShmAffinityInfo failed\n");
    }
    if (!rmUnusedSemaphore(true, msgFunc)) {
        result = false;
        msgFunc("remove testMode semaphore failed\n");
    }

    if (!ShmAffinityInfoManager::rmShmIfAlreadyExistCmd(false, msgFunc)) {
        result = false;
        msgFunc("remove regular ShmAffinityInfo failed\n");
    }
    if (!rmUnusedSemaphore(false, msgFunc)) {
        result = false;
        msgFunc("remove regular semaphore failed\n");
    }

    return result;
}

} // namespace grid_util
} // namespace scene_rdl2
