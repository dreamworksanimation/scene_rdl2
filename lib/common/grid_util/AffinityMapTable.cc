// Copyright 2025-2026 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "AffinityMapTable.h"
#include "Pipe.h"
#include "Process.h"
#include "Sha1Util.h"
#include "UserUtil.h"

#include <pwd.h> // getpwuid()
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include <scene_rdl2/common/platform/Platform.h> // MNRY_ASSERT
#include <scene_rdl2/common/rec_time/RecTime.h>

namespace scene_rdl2 {
namespace grid_util {

namespace affMapTbl_detail {

class Helper {
public:
    static bool crawlAllSemaphore(int permission, // like 0644, no permission check if negative value 
                                  const std::function<void(const int key,
                                                           const int semId,
                                                           const std::string& userName,
                                                           const int permission,
                                                           const unsigned nsems)>& callBack,
                                  std::string* errMsg);

    //------------------------------

    static ShmDataManager::KeyIdByStrVer
    convertToKeyIdByStrVer(AffinityMapTable::TestKeyStrFormat formatVersion);
    static ShmAffinityInfoManager::TestKeyStrFormat
    convertSemaphoreTestKeyStrFormatToShm(AffinityMapTable::TestKeyStrFormat formatVersion);

    static int getSemaphoreId(int testMode, AffinityMapTable::TestKeyStrFormat formatVersion);
    static int getTestSemaphoreId(const std::string& userName, AffinityMapTable::TestKeyStrFormat formatVersion);

    //------------------------------

    static void removeSemaphore(int semId, const std::string& rmReason); // Throw exception(std::string) if error

    static bool removeOrphanedSemaphore(bool testMode,
                                        AffinityMapTable::TestKeyStrFormat formatVersion,
                                        const AffinityMapTable::Msg& msgFunc); // try to clean up data-less semaphore
    static bool removeAllSemaphoreShm(AffinityMapTable::TestKeyStrFormat formatVersion,
                                      const AffinityMapTable::Msg& msgFunc);


    //------------------------------

    static std::string openConditionStr(const AffinityMapTable::OpenCondition& condition);

    //
    // Analyze whether the specified targetSemKey and targetSemId, within the environment of the specified
    // userName, were created for affinityMapTable operations. If the semaphore is related to the
    // affinityMapTable, return a string describing its specifications (keyFormatVersion and other details).
    // If it is not related, return "?".
    // The detailedMsg flag controls whether the returned string includes detailed information.
    //
    static std::string analyzeSemaphoreIdMsg(int targetSemKey,
                                             int targetSemId,
                                             const std::string& userName,
                                             bool detailedMsg);

private:
    static std::string getSemaphoreUserNameBySemaphoreId(int semId);

    //
    // A function that verifies whether the targetSemKey and targetSemId provided as arguments were created based
    // on the conditions specified by the other arguments. It returns true if the targetSemKey and targetSemId
    // were created according to the specified conditions; otherwise, it returns false. If the string pointers
    // shortMsg and detailedMsg are provided, the function saves messages describing the processing into those
    // buffers.
    //
    static bool analyzeSemaphoreId(int targetKey,
                                   int targetSemId,
                                   const std::string& userName,
                                   bool testMode,
                                   AffinityMapTable::TestKeyStrFormat keyStrVer,
                                   ShmDataManager::KeyIdByStrVer keyIdByStrVer,
                                   std::string* shortMsg,
                                   std::string* detailedMsg);
};
    
} // namespace affmapTbl_detail

//------------------------------------------------------------------------------------------

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
    using affMapTbl_detail::Helper;

    std::ostringstream ostr;
    ostr << "AffinityMapTable {\n"
         << "  sSemaphoreKeyStr:" << sSemaphoreKeyStr << '\n'
         << "  sSemaphoreTestKeyStr:" << sSemaphoreTestKeyStr << '\n'
         << "  sSemaphoreInitCompleteHashStr:" << sSemaphoreInitCompleteHashStr << '\n'
         << "  sOpenTimeoutSec:" << sOpenTimeoutSec << " sec\n"
         << "  sOpenRetry:" << sOpenRetry << '\n'
         << "  mTestMode:" << str_util::boolStr(mTestMode) << '\n'
         << "  mTestKeyStrFormatVersion:" << showTestKeyStrFormat(mTestKeyStrFormatVersion) << '\n'
         << "  mSemaphoreOpenCondition:" << Helper::openConditionStr(mSemaphoreOpenCondition) << '\n'
         << "  mSemaphoreId:" << mSemaphoreId << '\n'
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
AffinityMapTable::showSemaphoreInfo(const int testMode,
                                    const TestKeyStrFormat formatVersion)
{
    using affMapTbl_detail::Helper;

    const int semId = Helper::getSemaphoreId(testMode, formatVersion);
    const std::string semKeyStr = getSemaphoreKeyStr(testMode, formatVersion);
    const int semKey =
        scene_rdl2::grid_util::ShmDataManager::genKeyIdByStr(semKeyStr,
                                                             Helper::convertToKeyIdByStrVer(formatVersion));

    std::ostringstream ostr;
    ostr << "Semaphore info {\n"
         << "  mTestMode:" << str_util::boolStr(testMode) << '\n'
         << "  formatVersion:" << showTestKeyStrFormat(formatVersion) << '\n'
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
AffinityMapTable::showAllSemaphoreInfoList()
{
    using affMapTbl_detail::Helper;

    std::vector<unsigned> keyArray;
    std::vector<int> semIdArray;
    std::vector<std::string> userNameArray;
    std::vector<int> permissionArray;
    std::vector<std::string> analysisMsgArray;
    std::string errMsg;
    if (!Helper::crawlAllSemaphore(ShmDataManager::SHMAFFINFO_PERMISSION,
                                   [&](const int key,
                                       const int semId,
                                       const std::string& userName,
                                       const int permission,
                                       const unsigned nsems) {
                                       if (nsems != 1) return; // skipped if semaphore is not an affinityMapTable related

                                       std::string currMsg = Helper::analyzeSemaphoreIdMsg(key, semId, userName, false);
                                       if (currMsg != "?") {
                                           keyArray.emplace_back(key);
                                           semIdArray.emplace_back(semId);
                                           userNameArray.emplace_back(userName);
                                           permissionArray.emplace_back(permission);
                                           analysisMsgArray.emplace_back(currMsg);
                                       }
                                   },
                                   &errMsg)) {
        std::ostringstream ostr;
        ostr << "ERROR : crawlAllSemaphore() failed. err=>{\n"
             << str_util::addIndent(errMsg) << '\n'
             << "}";
        return ostr.str();
    }

    std::ostringstream ostr;
    ostr << "SemaphoreAffInfoList";
    if (keyArray.size()) {
        int wMaxKey = str_util::getHexNumberOfDigits(*std::max_element(keyArray.begin(), keyArray.end()));
        int wMaxSemId = str_util::getNumberOfDigits((unsigned)(*std::max_element(semIdArray.begin(), semIdArray.end())));
        size_t maxUserName = std::max_element(userNameArray.begin(), userNameArray.end(),
                                              [](auto& a, auto& b) { return a.size() < b.size(); })->size();
        ostr << " {\n";
        for (size_t id = 0; id < keyArray.size(); ++id) {
            ostr << "  key:0x" << std::hex << std::setw(wMaxKey) << keyArray[id] << std::dec
                 << " semId:" << std::setw(wMaxSemId) << semIdArray[id]
#ifdef PLATFORM_APPLE
                 << " perm:" << str_util::intToPermissionStrMacSysVSemaphore(permissionArray[id])
#else // !PLATFORM_APPLE
                 << " perm:" << str_util::intToPermissionStr(permissionArray[id])
#endif // end of !PLATFORM_APPLE
                 << " owner:" << std::setw(maxUserName) << std::left << userNameArray[id]
                 << ' ' << analysisMsgArray[id] << '\n';
        }
        ostr << "} (total:" << keyArray.size() << ")";
    } else {
        ostr << " is empty";
    }    
    return ostr.str();
}

// static function
std::string
AffinityMapTable::showTestSemaphoreInfo(const std::string& userName,
                                        const TestKeyStrFormat formatVersion)
{
    using affMapTbl_detail::Helper;

    const int semId = Helper::getTestSemaphoreId(userName, formatVersion);
    const std::string semKeyStr = getSemaphoreTestKeyStr(userName, formatVersion);
    const int semKey =
        scene_rdl2::grid_util::ShmDataManager::genKeyIdByStr(semKeyStr,
                                                             Helper::convertToKeyIdByStrVer(formatVersion));

    std::ostringstream ostr;
    ostr << "Test Semaphore info {\n"
         << "  userName:" << userName << '\n'
         << "  formatVersion:" << showTestKeyStrFormat(formatVersion) << '\n'
         << "  semKeyStr:" << semKeyStr << '\n'
         << "  semKey:0x" << std::hex << std::setw(8) << std::setfill('0') << semKey << std::dec << '\n';
    if (semId == -1) {
        ostr << "  semId: NON-EXISTENT\n";
    } else {
        ostr << "  semId:" << semId << '\n';
    }
    ostr << "}";
    return ostr.str();
}

// static function
std::string
AffinityMapTable::showShmAffinityInfoDump(const TestKeyStrFormat formatVersion)
//
// Output both tests on/off ShmAffinityInfo shared memory information without doing
// a lock for debugging purposes.
//
{
    using affMapTbl_detail::Helper;

    const ShmAffinityInfoManager::TestKeyStrFormat shmTestKeyStrFormat =
        Helper::convertSemaphoreTestKeyStrFormatToShm(formatVersion);

    std::ostringstream ostr;
    ostr << "ShmAffinityInfo {\n"
         << str_util::addIndent(ShmAffinityInfoManager::showShmDump(false, shmTestKeyStrFormat)) << '\n'
         << str_util::addIndent(ShmAffinityInfoManager::showShmDump(true, shmTestKeyStrFormat)) << '\n'
         << "}";
    return ostr.str();
}

// static function
std::string
AffinityMapTable::showSemaphoreInfoDump(const TestKeyStrFormat formatVersion)
{
    std::ostringstream ostr;
    ostr << "SemaphoreInfo {\n"
         << str_util::addIndent(showSemaphoreInfo(false, formatVersion)) << '\n'
         << str_util::addIndent(showSemaphoreInfo(true, formatVersion)) << '\n'
         << "}";
    return ostr.str();
}

// static function
std::string
AffinityMapTable::showInfoDump(const TestKeyStrFormat formatVersion)
{
    using affMapTbl_detail::Helper;

    ShmAffinityInfoManager::TestKeyStrFormat shmFormatVersion =
        Helper::convertSemaphoreTestKeyStrFormatToShm(formatVersion);    

    std::ostringstream ostr;
    ostr << "info {\n"
         << "  liveMode {\n"
         << str_util::addIndent(showSemaphoreInfo(false, formatVersion), 2) << '\n'
         << str_util::addIndent(ShmAffinityInfoManager::showShmDump(false, shmFormatVersion), 2) << '\n'
         << "  }\n"
         << "  testMode {\n"
         << str_util::addIndent(showSemaphoreInfo(true, formatVersion), 2) << '\n'
         << str_util::addIndent(ShmAffinityInfoManager::showShmDump(true, shmFormatVersion), 2) << '\n'
         << "  }\n"
         << "}";
    
    return ostr.str();
}

// static function
std::string
AffinityMapTable::showTestKeyStrFormat(const TestKeyStrFormat formatVersion)
{
    switch (formatVersion) {
    case TestKeyStrFormat::VER_0 : return "VER_0";
    case TestKeyStrFormat::VER_1 : return "VER_1";
    default : return "?";
    }
}

// static function
std::string
AffinityMapTable::showAllShmInfoList()
{
    return ShmDataManager::showAllShmAffInfoList([](const int shmId, const std::string& userName) {
        return ShmAffinityInfoManager::showShmIdDetailedInfo(shmId, userName);
    });
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
    using affMapTbl_detail::Helper;

    //
    // open semaphore
    //
    const std::string semKeyStr = getSemaphoreKeyStr(mTestMode, mTestKeyStrFormatVersion);
    const int semKey =
        scene_rdl2::grid_util::ShmDataManager::genKeyIdByStr(semKeyStr,
                                                             Helper::convertToKeyIdByStrVer(mTestKeyStrFormatVersion));

    mSemaphoreId = semget(semKey,
                          1, // number of semaphore inside semaphore-set
                          IPC_CREAT | IPC_EXCL | ShmDataManager::SHMAFFINFO_PERMISSION);
    if (mSemaphoreId != -1) {
        //
        // Freshly created semaphore and initialized here. This is done by single process.
        //
        semctl(mSemaphoreId,
               0, // target semaphore index. start from 0
               SETVAL, 1); // set semaphore value to 1

        setupFreshAffinityInfoManager();
        // After finishing the initialization, the semInitHash value is changed to the actual
        // hash value. This indicates that the related semaphore has been properly constructed
        // and initialization has been done explicitly atomically.

        mSemaphoreOpenCondition = OpenCondition::INITIALIZED;

    } else {
        //
        // Failed to create a fresh semaphore, so try to get an already existing semaphore here.
        //
        mSemaphoreId = semget(semKey,
                              1, // number of semaphore inside semaphore-set
                              ShmDataManager::SHMAFFINFO_PERMISSION);
        if (mSemaphoreId == -1) {
            std::ostringstream ostr;
            ostr << "AffinityMapTable::openMain() failed. Could not get already existed semId."
                 << " testMode:" << str_util::boolStr(mTestMode)
                 << " semaphoreKeyStr:" << semKeyStr
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
                          << " semaphoreId:" << mSemaphoreId << " -> start retry <<<<<\n";
                removeSemaphore("Timeout and retry of AffinityMapTable open"); // throw exception if error
                return false;
            }

            usleep(10000); // 10 millisec sleep to yield CPU resources.
        }

        mSemaphoreOpenCondition = OpenCondition::ALREADY_EXISTED;
    }

    return true;
}

// static function
std::string
AffinityMapTable::getSemaphoreKeyStr(const bool testMode,
                                     const TestKeyStrFormat formatVersion)
{
    return (!testMode) ? std::string(sSemaphoreKeyStr) : getSemaphoreTestKeyStr(std::string(""), formatVersion);
}

// static function
std::string
AffinityMapTable::getSemaphoreTestKeyStr(const std::string& userName,
                                         const TestKeyStrFormat formatVersion)
{
    auto getSemaphoreTestKeyStrVer1 = [&]() {
        return std::string(sSemaphoreTestKeyStr) + "_" + (userName.empty() ? UserUtil::getUserName() : userName);
    };

    switch(formatVersion) {
    case TestKeyStrFormat::VER_0 : return std::string(sSemaphoreTestKeyStr);
    case TestKeyStrFormat::VER_1 : return getSemaphoreTestKeyStrVer1();
    default : return "";
    }
}

void
AffinityMapTable::setupFreshAffinityInfoManager()
//
// Throw exception(std:string) if error
//
{
    using affMapTbl_detail::Helper;

    if (!mAffinityInfoManager) {
        try {
            //
            // Shared memory does not exist. We will construct fresh shared memory here.
            // This construction is executed in multi-process-safe manner and only a single process
            // constructs shared memory.
            //
            constexpr bool accessOnlyFlag = false;
            mAffinityInfoManager =
                std::make_shared<ShmAffinityInfoManager>(accessOnlyFlag,
                                                         mTestMode,
                                                         Helper::convertSemaphoreTestKeyStrFormatToShm(mTestKeyStrFormatVersion));
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
        const Sha1Util::Hash hash = genSemaphoreInitHash(mSemaphoreId);
        /* useful debug message
        std::cerr << ">> AffinityMapTable.cc setupFreshAffinityInfoManager() {\n"
                  << "  sSemaphoreInitCompleteHashStr:" << sSemaphoreInitCompleteHashStr << '\n'
                  << "  mSemaphoreId:" << mSemaphoreId << '\n'
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
    using affMapTbl_detail::Helper;

    if (!mAffinityInfoManager) {
        const ShmAffinityInfoManager::TestKeyStrFormat shmTestKeyStrFormat =
            Helper::convertSemaphoreTestKeyStrFormatToShm(mTestKeyStrFormatVersion);
        try {
            if (!ShmAffinityInfoManager::doesShmAlreadyExist(mTestMode, shmTestKeyStrFormat)) {
                setupFreshAffinityInfoManager();
                return false; // exec this function again
            }

            // access already existed affinityInfo
            constexpr bool accessOnlyFlag = true;
            mAffinityInfoManager = std::make_shared<ShmAffinityInfoManager>(accessOnlyFlag,
                                                                            mTestMode,
                                                                            shmTestKeyStrFormat);
        }
        catch (const std::string& err) {
            std::ostringstream ostr;
            ostr << "AffinityMapTable::checkSemaphoreInitCompletion() failed. error=>{\n"
                 << str_util::addIndent(err) << '\n'
                 << "}";
            throw ostr.str();
        }
    }
            
    if (mAffinityInfoManager->getAffinityInfo().getSemInitHash() != genSemaphoreInitHash(mSemaphoreId)) {
        return false; // semaphore is not initialized yet
    }
    return true; // semaphore has initialized properly
}

Sha1Util::Hash
AffinityMapTable::genSemaphoreInitHash(const int semId) const
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
        if (semop(mSemaphoreId, &op, 1) == 0) break;

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

    if (semtimedop(mSemaphoreId,
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
    if (semop(mSemaphoreId, &op, 1 /* num of operations */ ) == -1) {
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
    affMapTbl_detail::Helper::removeSemaphore(mSemaphoreId, rmReason);
    mSemaphoreId = 0;
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

void
AffinityMapTable::parserConfigure()
{
    using affMapTbl_detail::Helper;

    mParser.description("AffinityMapTable command");

    mParser.opt("show", "", "show all info",
                [&](Arg& arg) { return arg.msg(show() + '\n'); });
    mParser.opt("testMode", "<on|off|show>", "set testMode",
                [&](Arg& arg) {
                    if (arg() == "show") arg++;
                    else setTestMode((arg++).as<bool>(0));
                    return arg.fmtMsg("mTestMode %s\n", str_util::boolStr(mTestMode).c_str());
                });
    mParser.opt("testKeyFormat", "<ver0|ver1|show>", "set testKeyStr format version",
                [&](Arg& arg) {
                    if (arg() == "show") arg++;
                    else if (arg() == "ver0") { setTestKeyStrFormat(TestKeyStrFormat::VER_0); arg++; }
                    else if (arg() == "ver1") { setTestKeyStrFormat(TestKeyStrFormat::VER_1); arg++; }
                    else return arg.fmtMsg("unknown format version %s\n", (arg++)().c_str());
                    return arg.msg(showTestKeyStrFormat(mTestKeyStrFormatVersion) + '\n');
                });
    mParser.opt("open", "", "execute open procedures (respects testMode, testKeyFormat)",
                [&](Arg& arg) {
                    return testOpen([&](const std::string& msg) { return arg.msg(msg); });
                });
    mParser.opt("emulateOpenCrash", "",
                "emulate crash at open operation for testing purposes (respects testMode, testKeyFormat)",
                [&](Arg& arg) {
                    return emulateOpenCrash([&](const std::string& msg) { return arg.msg(msg); });
                });
    mParser.opt("acquire", "<numThread> <timeout>",
                "execute acquire action (respects testMode, testKeyFormat)",
                [&](Arg& arg) {
                    int reqThreadTotal = (arg++).as<int>(0);
                    float timeoutSec = (arg++).as<float>(0);
                    return arg.msg(acquire(reqThreadTotal, timeoutSec) + '\n');
                });
    mParser.opt("removeAllSemaphoreShm", "",
                "remove live+test semaphore and shared memory then create initial environment (respects testKeyFormat)",
                [&](Arg& arg) {
                    return Helper::removeAllSemaphoreShm(mTestKeyStrFormatVersion,
                                                         [&](const std::string& msg) { return arg.msg(msg); });
                });
    mParser.opt("removeOrphanedSemaphore", "",
                "remove unused testMode data-less semaphore if possible (respects testKeyFormat)",
                [&](Arg& arg) {
                    return Helper::removeOrphanedSemaphore(true, mTestKeyStrFormatVersion,
                                                           [&](const std::string& msg) { return arg.msg(msg); });
                });
    mParser.opt("removeShmIfAlreadyExist", "", "remove testMode ShmAffinityInfo if exist (respects testKeyFormat)",
                [&](Arg& arg) {
                    return ShmAffinityInfoManager::
                        removeShmIfAlreadyExistCmd(true,
                                                   Helper::convertSemaphoreTestKeyStrFormatToShm(mTestKeyStrFormatVersion),
                                                   [&](const std::string& msg) { return arg.msg(msg); });
                });
    mParser.opt("infoDump", "", "live+test semaphore and shmAffinityInfo dump (respects testKeyFormat)",
                [&](Arg& arg) { return arg.msg(showInfoDump(mTestKeyStrFormatVersion) + '\n'); });
    mParser.opt("shmInfoDump", "",
                "live+test shmAffinityInfo dump without semaphore lock (respects testKeyFormat) for debug",
                [&](Arg& arg) { return arg.msg(showShmAffinityInfoDump(mTestKeyStrFormatVersion) + '\n'); });
    mParser.opt("semInfoDump", "", "live+test semaphore related info dump (respects testKeyFormat)",
                [&](Arg& arg) { return arg.msg(showSemaphoreInfoDump(mTestKeyStrFormatVersion) + '\n'); });
    mParser.opt("testSemInfoDump", "<userName>",
                "semaphore info dump for testMode by userName (respects testKeyFormat)",
                [&](Arg& arg) { return arg.msg(showTestSemaphoreInfo((arg++)(), mTestKeyStrFormatVersion) + '\n'); });
    mParser.opt("allShmInfoList", "", "all affinityMapTable related shared memory list up (respects testKeyFormat)",
                [&](Arg& arg) { return arg.msg(showAllShmInfoList() + '\n'); });
    mParser.opt("allSemInfoList", "", "all affinityMapTable related semaphore list up (respects testKeyFormat)",
                [&](Arg& arg) { return arg.msg(showAllSemaphoreInfoList() + '\n'); });
    mParser.opt("affinityInfoManager", "...command...", "affinityInfoManager command",
                [&](Arg& arg) {
                    if (!mAffinityInfoManager) { arg.msg("mAffinityInfoManager is empty\n"); return false; }
                    return mAffinityInfoManager->getParser().main(arg.childArg());
                });
    mParser.opt("analyzeSemaphoreId", "<key-0x> <semId> <userName>",
                "analyze semaphore given key, semId, and userName in terms of affinityMapTable."
                " key should be hexadecimal start with 0x",
                [&](Arg& arg) {
                    const unsigned key = std::stoul((arg++)(), nullptr, 16);
                    const int semId = (arg++).as<int>(0);
                    const std::string userName = (arg++)();
                    return arg.msg(Helper::analyzeSemaphoreIdMsg(key, semId, userName, true) + '\n');
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

//--------------------------------------------------------------------------------------------------

namespace affMapTbl_detail {

#ifdef PLATFORM_APPLE

// static function
bool
Helper::crawlAllSemaphore(const int permission, // like 0644, no permission check if negative value 
                          const std::function<void(const int key,
                                                   const int semId,
                                                   const std::string& userName,
                                                   const int permission,
                                                   const unsigned nsems)>& callBack,
                          std::string* errMsg)
//
// Mac version
//
// If an error occurs and errMsg is set, the error message will be written to it.
//
{
    std::string permissionStr;
    if (permission >= 0) {
        permissionStr = std::string("--") + scene_rdl2::str_util::intToPermissionStrMacSysVSemaphore(permission);
    }
    auto permissionCheck = [&](const std::string& permsStr) {
        if (permission < 0) return true;
        return permsStr == permissionStr;
    };

    constexpr const char* ipcsCmdLine = "ipcs -sa"; // macOS

    std::vector<std::string> ipcsResultVecStr;
    if (!scene_rdl2::grid_util::execCommand(ipcsCmdLine, ipcsResultVecStr, errMsg)) {
        return false; // exec command failed.
    }

    //
    // The ipcs command first displays the following three lines, so we skip them.
    //
    //  IPC status from <running system> as of Mon Jan 26 07:06:28 PST 2026
    //  T     ID     KEY        MODE       OWNER    GROUP  CREATOR   CGROUP NSEMS   OTIME    CTIME
    //  Semaphores:
    //
    constexpr size_t skipLines = 3; // MacOS specific
    if (ipcsResultVecStr.size() < skipLines) return false; // format error
    if (ipcsResultVecStr[2] != "Semaphores:\n") return false; // format error

    for (size_t i = skipLines; i < ipcsResultVecStr.size(); ++i) {
        std::vector<std::string> tokens;
        {
            std::stringstream sstr(ipcsResultVecStr[i]);
            std::string token;
            while (sstr >> token) tokens.emplace_back(token);
        }
        if (tokens.size() < 11) continue; // format error -> skip this line

        // The expected information order would be
        // "T", "ID", "KEY", "MODE", "OWNER", "GROUP" "CREATOR", "CGROUP", "NSEMS", "OTIME", and "CTIME"
        //
        //       T : type (s:semaphore)
        //      ID : semaphore id
        //     KEY : semaphore key
        //    MODE : permission
        //   OWNER : owner
        //   GROUP : group of owner
        // CREATOR : creator username of this semaphore
        //  CGROUP : creator's group
        //   NSEMS : number of semaphore
        //   OTIME : last operation time
        //   CTIME : creted time
        //
        std::string& tStr = tokens[0];
        std::string& semIdStr = tokens[1];
        std::string& keyStr = tokens[2];
        std::string& modeStr = tokens[3];
        std::string& ownerStr = tokens[4];

        // Specify by the number counted from the end. This is to prevent the order from being shifted
        // when there are spaces in names such as the Group name, depending on the situation.
        std::string& nsemsStr = tokens[tokens.size() - 3];

        /* for debug
        std::cerr << "i:" << i
                  << " tStr:" << tStr << '\n'
                  << " semIdStr:" << semIdStr << '\n'
                  << " keyStr:" << keyStr << '\n'
                  << " modeStr:" << modeStr << '\n'
                  << " ownerStr:" << ownerStr << '\n'
                  << " nsemsStr:" << nsemsStr << '\n';
        */

        if (tStr != "s") continue; // skip regarding non semaphore field
        if (permissionCheck(modeStr)) {
            const int key = static_cast<int>(std::stoul(keyStr, nullptr, 16));
            const int semId = std::stoi(semIdStr);
            const unsigned perms = scene_rdl2::str_util::permissionStrToIntMacSysVSemaphore(modeStr);
            const unsigned nsems = std::stoi(nsemsStr);
            callBack(key, semId, ownerStr, perms, nsems);
        }
    }

    return true;
}
    
#else // !PLATFORM_APPLE

// static function
bool
Helper::crawlAllSemaphore(const int permission, // like 0644, no permission check if negative value 
                          const std::function<void(const int key,
                                                   const int semId,
                                                   const std::string& userName,
                                                   const int permission,
                                                   const unsigned nsems)>& callBack,
                          std::string* errMsg)
//
// Linux version
//
// If an error occurs and errMsg is set, the error message will be written to it.
//
{
    std::string permissionStr;
    if (permission >= 0) {
        permissionStr = scene_rdl2::str_util::intToOctal3DigitsStr(permission);
    }
    auto permissionCheck = [&](const std::string& permsStr) {
        if (permission < 0) return true;
        return permsStr == permissionStr;
    };

    constexpr const char* ipcsCmdLine = "ipcs -s"; // linux
    
    std::vector<std::string> ipcsResultVecStr;
    if (!scene_rdl2::grid_util::execCommand(ipcsCmdLine, ipcsResultVecStr, errMsg)) {
        return false;
    }

    //
    // The ipcs command first displays the following line, so we skip it.
    //
    //  ------ Semaphore Arrays --------
    //
    constexpr size_t skipLines = 2; // Linux specific
    if (ipcsResultVecStr.size() < skipLines) return false; // format error
    for (size_t i = skipLines; i < ipcsResultVecStr.size(); ++i) {
        std::stringstream sstr(ipcsResultVecStr[i]);

        // The expected information order would be "key" "semid" "owner" "perms" "nsems"
        std::string keyStr, semIdStr, ownerStr, permsStr, nsemsStr;
        sstr >> keyStr >> semIdStr >> ownerStr >> permsStr >> nsemsStr;

        /* for debug
        std::cerr << "i:" << i
                  << " keyStr:" << keyStr
                  << " semIdStr:" << semIdStr
                  << " ownerStr:" << ownerStr
                  << " permsStr:" << permsStr
                  << " nsemsStr:" << nsemsStr << '\n';
        */        

        if (permissionCheck(permsStr)) {
            const int key = static_cast<int>(std::stoul(keyStr, nullptr, 16));
            const int semId = std::stoi(semIdStr);
            const unsigned perms = scene_rdl2::str_util::octal3DigitsStrToInt(permsStr);
            const unsigned nsems = std::stoi(nsemsStr);
            callBack(key, semId, ownerStr, perms, nsems);
        }
    }

    return true;
}

#endif // end of !PLATFORM_APPLE

// static function
ShmDataManager::KeyIdByStrVer
Helper::convertToKeyIdByStrVer(const AffinityMapTable::TestKeyStrFormat formatVersion)
{
    switch (formatVersion) {
    case AffinityMapTable::TestKeyStrFormat::VER_0 : return ShmDataManager::KeyIdByStrVer::VER_0;
    case AffinityMapTable::TestKeyStrFormat::VER_1 : return ShmDataManager::KeyIdByStrVer::VER_1;
    default : return ShmDataManager::KeyIdByStrVer::VER_1;
    }
}

// static function
ShmAffinityInfoManager::TestKeyStrFormat
Helper::convertSemaphoreTestKeyStrFormatToShm(const AffinityMapTable::TestKeyStrFormat formatVersion)
{
    switch (formatVersion) {
    case AffinityMapTable::TestKeyStrFormat::VER_0 : return ShmAffinityInfoManager::TestKeyStrFormat::VER_0;
    case AffinityMapTable::TestKeyStrFormat::VER_1 : return ShmAffinityInfoManager::TestKeyStrFormat::VER_1;
    default : return ShmAffinityInfoManager::TestKeyStrFormat::VER_1;
    }
}

// static function
int
Helper::getSemaphoreId(const int testMode,
                       const AffinityMapTable::TestKeyStrFormat formatVersion)
//
// return -1 if it does not exist. This case new semaphore has not been created.
//
{
    const std::string semKeyStr = AffinityMapTable::getSemaphoreKeyStr(testMode, formatVersion);
    const int semKey = ShmDataManager::genKeyIdByStr(semKeyStr,
                                                     Helper::convertToKeyIdByStrVer(formatVersion));
    return semget(semKey, 0, 0);
}

// static function
int
Helper::getTestSemaphoreId(const std::string& userName,
                           const AffinityMapTable::TestKeyStrFormat formatVersion)
//
// return -1 if it does not exist. This case new semaphore has not been created.
//
{
    const std::string semKeyStr = AffinityMapTable::getSemaphoreTestKeyStr(userName, formatVersion);
    const int semKey =
        scene_rdl2::grid_util::ShmDataManager::genKeyIdByStr(semKeyStr,
                                                             Helper::convertToKeyIdByStrVer(formatVersion));

    return semget(semKey, 0, 0);
}

// static function
void
Helper::removeSemaphore(const int semId, const std::string& rmReason)
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
        ostr << "Helpers::removeSemaphore() failed. semId:" << semId << " error=>{\n"
             << str_util::addIndent(strerror(errno)) << '\n'
             << "}";
        throw ostr.str();
    }
}

// static function
bool
Helper::removeOrphanedSemaphore(const bool testMode,
                                const AffinityMapTable::TestKeyStrFormat formatVersion,
                                const AffinityMapTable::Msg& msgFunc)
{
    try {
        //
        // This function attempts to delete a semaphore that is used to access shared memory data
        // specified by the given testMode and formatVersion, in cases where the shared memory data
        // itself does not exist (or itself created by other owner) but only the semaphore remains.
        // Theoretically, such a semaphore is unnecessary and has no purpose, so deletion is
        // attempted. Whether the deletion actually succeeds depends on whether the owner of the
        // semaphore matches the owner of the current process; if they are different, an error
        // occurs and the semaphore cannot be deleted. (However, if the process is running as root,
        // any semaphore can be deleted regardless of its owner.)
        //
        ShmAffinityInfoManager::TestKeyStrFormat shmKeyStrFormat =
            convertSemaphoreTestKeyStrFormatToShm(formatVersion);
        if (!ShmAffinityInfoManager::doesShmAlreadyExist(testMode, shmKeyStrFormat)) {
            // no related shared memory
            int semId = getSemaphoreId(testMode, formatVersion);
            if (semId != -1) {
                // semaphore exists

                // throw exception(std::string) if error
                removeSemaphore(semId, "unused dataless semaphore clean up");

                std::ostringstream ostr;
                ostr << "Removed unused semaphore. semId:" << semId;
                msgFunc(ostr.str() + '\n');
            }
        } else {
            // shared memory exist
            int semId = getSemaphoreId(testMode, formatVersion);
            if (semId != -1) {
                // semaphore exists
                // We have to check semaphore's owner is match with the shared memory's owner.
                int shmId = ShmAffinityInfoManager::getShmIdIfAvailable(testMode, shmKeyStrFormat);
                std::string shmUserName = ShmDataManager::getShmUserNameByShmId(shmId);
                std::string semUserName = getSemaphoreUserNameBySemaphoreId(semId);
                if (shmUserName != semUserName) {
                    // mismatch of owner between shared memory and semaphore

                    // throw exception(std::string) if error
                    removeSemaphore(semId, "owner mismatch semaphore clean up");

                    std::ostringstream ostr;
                    ostr << "Removed owner mismatch semaphore. semId:" << semId;
                    msgFunc(ostr.str() + '\n');
                }
            }
        }
    }
    catch (const std::string& err) {
        std::ostringstream ostr;
        ostr << "ERROR : AffinityMapTable::removeOrphanedSemaphore() failed. err=>{\n"
             << str_util::addIndent(err) << '\n'
             << "}";
        msgFunc(ostr.str() + '\n');
        return false;
    }
    return true;
}

// static function
bool
Helper::removeAllSemaphoreShm(const AffinityMapTable::TestKeyStrFormat formatVersion,
                              const AffinityMapTable::Msg& msgFunc)
{
    ShmAffinityInfoManager::TestKeyStrFormat shmFormatVersion =
        convertSemaphoreTestKeyStrFormatToShm(formatVersion);

    bool result = true;
    
    //
    // TestMode
    //
    // The shared memory affinityMapTable in TestMode is basically only used during the execution of
    // unit tests, so it is not intended for multiple unit tests to be run in parallel. Therefore,
    // deletion is attempted in this context. Strictly speaking, the same issues as deleting the
    // affinityMapTable in liveMode can occur, but since unit tests are extremely localized processes
    // and it is assumed that individual unit tests will be executed independently, deletion is attempted
    // here on that basis.
    //
    if (!ShmAffinityInfoManager::removeShmIfAlreadyExistCmd(true, shmFormatVersion, msgFunc)) {
        result = false;
        msgFunc("remove TestMode ShmAffinityInfo failed\n");
    }
    if (!removeOrphanedSemaphore(true, formatVersion, msgFunc)) {
        result = false;
        msgFunc("remove TestMode semaphore failed\n");
    }

    //
    // LiveMode
    //
    // Deleting the shared memory affinityMapTable for LiveMode involves significant risk, so by design, this
    // operation is not allowed. This is because there is no guarantee that the MoonRay process does not exist
    // at the time the deletion is attempted. Therefore, deleting the affinityMapTable for LiveMode is
    // prohibited via the API. If you absolutely need to delete an existing affinityMapTable shared memory,
    // please use "ipcrm" as root or reboot the machine.
    //
    /* -- do not remove this comment --
    if (!ShmAffinityInfoManager::removeShmIfAlreadyExistCmd(false, shmFormatVersion, msgFunc)) {
        result = false;
        msgFunc("remove LiveMode ShmAffinityInfo failed\n");
    }
    */
    if (!removeOrphanedSemaphore(false, formatVersion, msgFunc)) {
        result = false;
        msgFunc("remove LiveMode semaphore failed\n");
    }

    return result;
}

// static function
std::string
Helper::openConditionStr(const AffinityMapTable::OpenCondition& condition)
{
    switch (condition) {
    case AffinityMapTable::OpenCondition::UNDEFINED : return "UNDEFINED";
    case AffinityMapTable::OpenCondition::INITIALIZED : return "INITIALIZED";
    case AffinityMapTable::OpenCondition::ALREADY_EXISTED : return "ALREADY_EXISTED";
    default : return "?";
    }
}

// static function
std::string
Helper::analyzeSemaphoreIdMsg(const int targetSemKey,
                              const int targetSemId,
                              const std::string& userName,
                              const bool detailedMsg)
//
// Analyze whether the specified targetSemKey and targetSemId, within the environment of the specified
// userName, were created for affinityMapTable operations. If the semaphore is related to the
// affinityMapTable, return a string describing its specifications (keyFormatVersion and other details).
// If it is not related, return "?".
// The detailedMsg flag controls whether the returned string includes detailed information.
//
{
    constexpr AffinityMapTable::TestKeyStrFormat strVer0 = AffinityMapTable::TestKeyStrFormat::VER_0;
    constexpr AffinityMapTable::TestKeyStrFormat strVer1 = AffinityMapTable::TestKeyStrFormat::VER_1;
    constexpr ShmDataManager::KeyIdByStrVer idVer0 = ShmDataManager::KeyIdByStrVer::VER_0;
    constexpr ShmDataManager::KeyIdByStrVer idVer1 = ShmDataManager::KeyIdByStrVer::VER_1;

    std::string shortMsg0, shortMsg1, longMsg0, longMsg1;
    bool flag0 = analyzeSemaphoreId(targetSemKey, targetSemId, userName, false, strVer1, idVer0, &shortMsg0, &longMsg0);
    bool flag1 = analyzeSemaphoreId(targetSemKey, targetSemId, userName, false, strVer1, idVer1, &shortMsg1, &longMsg1);

    std::string shortTestMsg00, shortTestMsg01, shortTestMsg10, shortTestMsg11;
    std::string longTestMsg00, longTestMsg01, longTestMsg10, longTestMsg11;
    bool testFlag00 = analyzeSemaphoreId(targetSemKey, targetSemId, userName, true, strVer0, idVer0, &shortTestMsg00, &longTestMsg00);
    bool testFlag01 = analyzeSemaphoreId(targetSemKey, targetSemId, userName, true, strVer0, idVer1, &shortTestMsg01, &longTestMsg01);
    bool testFlag10 = analyzeSemaphoreId(targetSemKey, targetSemId, userName, true, strVer1, idVer0, &shortTestMsg10, &longTestMsg10);
    bool testFlag11 = analyzeSemaphoreId(targetSemKey, targetSemId, userName, true, strVer1, idVer1, &shortTestMsg11, &longTestMsg11);

    if (!detailedMsg) {
        const unsigned total = (int)flag0 + (int)flag1 + (int)testFlag00 + (int)testFlag01 + (int)testFlag10 + (int)testFlag11;
        if (total == 0) return "?";

        std::ostringstream ostr;
        if (flag0) ostr << shortMsg0 << ' ';
        if (flag1) ostr << shortMsg1 << ' ';
        if (testFlag00) ostr << shortTestMsg00 << ' ';
        if (testFlag01) ostr << shortTestMsg01 << ' ';        
        if (testFlag10) ostr << shortTestMsg10 << ' ';
        if (testFlag11) ostr << shortTestMsg11 << ' ';

        return str_util::trimBlank(ostr.str());
    } else {
        std::ostringstream ostr;
        ostr << "====>>> analysis of"
             << " semKey:0x" << std::hex << targetSemKey << std::dec << '(' << targetSemKey << ')'
             << " semId:" << targetSemId << " userName:" << userName << " <<<==== {\n"
             << str_util::addIndent(longMsg0) << '\n'
             << str_util::addIndent(longMsg1) << '\n'
             << str_util::addIndent(longTestMsg00) << '\n'
             << str_util::addIndent(longTestMsg01) << '\n'            
             << str_util::addIndent(longTestMsg10) << '\n'
             << str_util::addIndent(longTestMsg11) << '\n'
             << "}";
        return ostr.str();
    }    
}

//------------------------------------------------------------------------------------------

// static function
std::string
Helper::getSemaphoreUserNameBySemaphoreId(const int semId)
{
    struct semid_ds semInfo;
    if (semctl(semId, 0, IPC_STAT, &semInfo) == -1) return "?";

    uid_t ownerUid = semInfo.sem_perm.uid;
    struct passwd* pw = getpwuid(ownerUid);
    if (!pw) return "?";
    return std::string(pw->pw_name);    
}

// static function
bool
Helper::analyzeSemaphoreId(const int targetSemKey,
                           const int targetSemId,
                           const std::string& userName,
                           const bool testMode,
                           const AffinityMapTable::TestKeyStrFormat keyStrVer,
                           const ShmDataManager::KeyIdByStrVer keyIdByStrVer,
                           std::string* shortMsg,
                           std::string* detailedMsg)
//
// A function that verifies whether the targetSemKey and targetSemId provided as arguments were created based
// on the conditions specified by the other arguments. It returns true if the targetSemKey and targetSemId
// were created according to the specified conditions; otherwise, it returns false. If the string pointers
// shortMsg and detailedMsg are provided, the function saves messages describing the processing into those
// buffers.
//
{
    const std::string semKeyStr = ((!testMode) ?
                                   AffinityMapTable::getSemaphoreKeyStr(false, keyStrVer) :
                                   AffinityMapTable::getSemaphoreTestKeyStr(userName, keyStrVer));
    const int semKey = ShmDataManager::genKeyIdByStr(semKeyStr, keyIdByStrVer);
    const int semId = (semKey == targetSemKey) ? semget(semKey, 0, 0) : -1;
    const bool flag = (semKey == targetSemKey) ? (semId == targetSemId) : false;

    if (shortMsg) {
        std::ostringstream ostr;
        if (flag) {
            ostr << (testMode ? "TEST" : "LIVE") << '-';
            if (keyStrVer == AffinityMapTable::TestKeyStrFormat::VER_0 &&
                keyIdByStrVer == ShmDataManager::KeyIdByStrVer::VER_0) {
                ostr << "VER_0";
            } else if (keyStrVer == AffinityMapTable::TestKeyStrFormat::VER_1 &&
                       keyIdByStrVer == ShmDataManager::KeyIdByStrVer::VER_1) {
                ostr << "VER_1";
            } else {
                ostr << "(str:" << AffinityMapTable::showTestKeyStrFormat(keyStrVer) << '+'
                     << "id:" << ShmDataManager::showKeyIdByStrVer(keyIdByStrVer) << ')';
            }
        } else {
            ostr << "?";
        }
        (*shortMsg) = ostr.str();
    }

    if (detailedMsg) {
        std::ostringstream ostr;
        ostr << "targetSemKey:0x" << std::hex << targetSemKey << std::dec << '(' << targetSemKey << ')'
             << " targetSemId:" << targetSemId
             << " userName:" << userName
             << " testMode:" << str_util::boolStr(testMode)
             << " keyStrVer:" << AffinityMapTable::showTestKeyStrFormat(keyStrVer)
             << " keyIdVer:" << ShmDataManager::showKeyIdByStrVer(keyIdByStrVer) << " {\n"
             << "  semKeyStr:" << semKeyStr << '\n'
             << "  semKey:0x" << std::hex << semKey << std::dec << '(' << (int)semKey << ")\n"
             << "  semId:" << semId << '\n'
             << "}";
        if (flag) {
            ostr << " semKey == targetSemKey && semId == targetSemId";
        } else {
            ostr << " skipped"; 
        }
        (*detailedMsg) = ostr.str();
    }

    return flag;
}

} // namespace affMapTbl_detail

} // namespace grid_util
} // namespace scene_rdl2
