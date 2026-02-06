// Copyright 2025-2026 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "ShmAffinityInfo.h"
#include "UserUtil.h"

#include <scene_rdl2/common/except/exceptions.h>
#include <scene_rdl2/render/util/StrUtil.h>

#include <random>
#include <sstream>
#include <sys/shm.h>
#include <thread>

namespace scene_rdl2 {
namespace grid_util {

namespace shmAffinityInfo_detail {

class HelperManager {
public:
    static ShmDataManager::KeyIdByStrVer
    convertToKeyIdByStrVer(ShmAffinityInfoManager::TestKeyStrFormat formatVersion);    

    static std::string setupConditionStr(ShmAffinityInfoManager::SetupCondition condition);    

    static std::string analyzeShmIdMsg(int shmId, const std::string& userName, bool detailedMsg);

#ifdef RM_CANDIDATE
    // returns true if targetShmId is existed and it is associated with given argument parameters.
    // otherwise returns falase.
    static bool isExpectedExistedShmId(int shmId,
                                       bool testMode,
                                       ShmAffinityInfoManager::TestKeyStrFormat formatVersion,
                                       const std::string& userName);
#endif // end of RM_CANDIDATE
    
private:
    static bool analyzeShmId(int targetShmId,
                             const std::string& userName,
                             bool testMode,
                             ShmAffinityInfoManager::TestKeyStrFormat keyStrVer,
                             ShmDataManager::KeyIdByStrVer keyIdByStrVer,
                             std::string* shortMsg,
                             std::string* detailedMsg);    
};

} // namespace shmAffinityInfo_detail

// static function
std::string
ShmAffinityInfo::showOffset()
{
    std::ostringstream ostr;
    ostr << "ShmAffinityInfo offset {\n"
         << "  offset_headMessage:" << offset_headMessage
         << " (size_headMessage:" << size_headMessage << ")\n"
         << "  offset_shmDataSize:" << offset_shmDataSize << '\n'
         << "  offset_semInitHash:" << offset_semInitHash
         << " (size_semInitHash:" << size_semInitHash << ")\n"
         << "  offset_numCores:" << offset_numCores << '\n'
         << "  offset_coreInfoStart:" << offset_coreInfoStart
         << " (size_singleCoreInfo:" << size_singleCoreInfo << ")\n"
         << "  localOffset_coreInfoOccupancy:" << localOffset_coreInfoOccupancy << '\n'
         << "  localOffset_coreInfoPID:" << localOffset_coreInfoPID << '\n'
         << "} " << showSizeInfo();
    return ostr.str();
}

// static function
std::string
ShmAffinityInfo::showSizeInfo()
{
    const size_t totalShmSize = calcTotalShmSize();
    std::ostringstream ostr;
    ostr << "shmTotalSize:" << totalShmSize << " byte (" << str_util::byteStr(totalShmSize) << ")"
         << " coreTotal:" << getTotalNumCores();
    return ostr.str();
}

std::string
ShmAffinityInfo::show(const NumaUtil* numaUtilObsPtr,
                      const CpuSocketUtil* cpuSocketUtilObsPtr) const
{
    std::ostringstream ostr;
    ostr << "ShmAffinityInfo {\n"
         << "  getHeadMessage():" << getHeadMessage() << '\n'
         << "  getShmDataSize():" << getShmDataSize() << '\n'
         << "  getSemInitHash():" << Sha1Util::show(getSemInitHash()) << '\n'
         << "  getNumCores():" << getNumCores() << '\n'
         << str_util::addIndent(showCoreInfoTable2(numaUtilObsPtr, cpuSocketUtilObsPtr)) << '\n'
         << "} " << showSizeInfo();
    return ostr.str();
}

std::string
ShmAffinityInfo::showCoreInfoTable(const NumaUtil* numaUtilObsPtr,
                                   const CpuSocketUtil* cpuSocketUtilObsPtr) const
{
    const int wCoreId = str_util::getNumberOfDigits(getNumCores());
    const int wNumaId = (numaUtilObsPtr) ? str_util::getNumberOfDigits(numaUtilObsPtr->getTotalNumaNode()) : 0;    
    const int wSocketId = (cpuSocketUtilObsPtr) ? str_util::getNumberOfDigits(cpuSocketUtilObsPtr->getTotalSockets()) : 0;
    const int numCores = getNumCores();

    std::ostringstream ostr;
    ostr << "coreInfoTable (size:" << numCores << ") {\n";
    crawlAllCores([&](const unsigned coreId, const bool occupancy, const size_t pid) {
        ostr << "  coreId:" << std::setw(wCoreId) << coreId;
        if (numaUtilObsPtr) {
            ostr << "  numaNodeId:" << std::setw(wNumaId) << numaUtilObsPtr->getNumaNode(coreId)->getNodeId();
        }
        if (cpuSocketUtilObsPtr) {
            ostr << "  socketId:"
                 << std::setw(wSocketId) << cpuSocketUtilObsPtr->findSocketByCpuId(coreId)->getSocketId();
        }
        ostr << " occupancy:" << std::setw(5) << str_util::boolStr(occupancy);
        if (occupancy) ostr << " PID:" << pid << '\n';
        else ostr << '\n';
        return true;
    });
    ostr << "}";
    return ostr.str();
}

std::string
ShmAffinityInfo::showCoreInfoTable2(const NumaUtil* numaUtilObsPtr,
                                    const CpuSocketUtil* cpuSocketUtilObsPtr) const
{
    const unsigned numCores = getNumCores();
    const size_t numNumaNodes = (numaUtilObsPtr) ? numaUtilObsPtr->getTotalNumaNode() : 0;
    const size_t numSockets = (cpuSocketUtilObsPtr) ? cpuSocketUtilObsPtr->getTotalSockets() : 0;
    const int wCoreId = str_util::getNumberOfDigits(numCores);
    const int wPID = str_util::getNumberOfDigits(getMaxPid());
    const int wNumaId = str_util::getNumberOfDigits(numNumaNodes);
    const int wSocketId = str_util::getNumberOfDigits(numSockets);

    auto showCoreInfo = [&](const unsigned coreId, const bool occupancy, const size_t pid) {
        std::ostringstream ostr;
        ostr << std::setw(wCoreId) << coreId;
        if (numaUtilObsPtr) {
            const NumaNode* currNumaNode = numaUtilObsPtr->findNumaNodeByCpuId(coreId);
            if (currNumaNode) {
                ostr << '/' << std::setw(wNumaId) << currNumaNode->getNodeId();
            }
        }
        if (cpuSocketUtilObsPtr) {
            const CpuSocketInfo* currCpuSocketInfo = cpuSocketUtilObsPtr->findSocketByCpuId(coreId);
            if (currCpuSocketInfo) {
                ostr << '/' << std::setw(wSocketId) << currCpuSocketInfo->getSocketId();
            }
        }
        if (occupancy) ostr << ":" << std::setw(wPID) << pid;
        else           ostr << ":" << std::setw(wPID) << ' ';
        return ostr.str();
    };

    std::string formatStr = "coreId";

    constexpr unsigned maxItemsOneLine = 8;
    std::ostringstream ostr;
    ostr << "coreInfo table (numCores:" << numCores << ")";
    if (numaUtilObsPtr) {
        ostr << " (numNumaNodes:" << numNumaNodes << ')';
        formatStr += "/NumaNode";
    }
    if (cpuSocketUtilObsPtr) {
        ostr << " (numSockets:" << numSockets << ')';
        formatStr += "/Socket";
    }
    formatStr += ":PID";
    ostr << " [" << formatStr << "] {\n";

    crawlAllCores([&](const unsigned coreId, const bool occupancy, const size_t pid) {
        if (coreId != 0 && coreId % maxItemsOneLine == 0) ostr << '\n';
        ostr << "  " << showCoreInfo(coreId, occupancy, pid);
        return true;
    });
    ostr << "\n}";

    return ostr.str();
}

bool
ShmAffinityInfo::verifySetGet(const int dataTypeId)
{
    if (!verifySetGetMain(dataTypeId, true)) return false;
    if (!verifySetGetMain(dataTypeId, false)) return false;
    return true;
}

void
ShmAffinityInfo::initCoreInfoTable()
{
    for (unsigned coreId = 0; coreId < getNumCores(); ++coreId) {
        initCoreInfo(coreId);
    }
}

size_t
ShmAffinityInfo::getMaxPid() const
{
    size_t maxPid = 0;
    crawlAllCores([&](const unsigned coreId, const bool occupancy, const size_t pid) {
        if (occupancy) { if (maxPid < pid) maxPid = pid; }
        return true;
    });
    return maxPid;
}

// static function
unsigned
ShmAffinityInfo::getTotalNumCores()
{
    return std::thread::hardware_concurrency();
}

bool
ShmAffinityInfo::verifySetGetMain(const int dataTypeId, const bool setup)
//
// So far we only support dataTypeId == 0 at this moment
//
{
    if (dataTypeId == 0) return verifySetGetMain_type0(setup);
    return false;
}

bool
ShmAffinityInfo::verifySetGetMain_type0(const bool setup)
{
    const Hash hash = Sha1Util::hash("The input string for the test pattern Hash data");

    if (setup) {
        setSemInitHash(hash);
    } else {
        if (getSemInitHash() != hash) return false;
    }

    const unsigned coreTotal = getNumCores();
    for (unsigned coreId = 0; coreId < coreTotal; ++coreId) {
        const bool occupancy = (coreId % 2 == 0) ? true : false;
        const size_t pid = static_cast<size_t>(coreId) + 123; // dummy PID

        if (setup) {
            if (!setCoreInfo(coreId, occupancy, pid)) return false;
        } else {
            bool currOccupancy {false};
            size_t currPid {0};
            if (!getCoreInfo(coreId, currOccupancy, currPid)) return false;
            if (currOccupancy != occupancy || currPid != pid) return false;
        }
    }

    return true;
}

//------------------------------------------------------------------------------------------

ShmAffinityInfoManager::ShmAffinityInfoManager(const bool accessOnly,
                                               const bool testMode,
                                               const TestKeyStrFormat formatVersion)
    : mTestMode {testMode}
    , mTestKeyStrFormatVersion {formatVersion}
//
// Might throw exception(std::string) if error
//
{
    if (accessOnly) {
        // Access already existed affinityInfo only
        accessAffinityInfo(); 
    } else {
        // Access affinityInfo if it already exists otherwise construct the new one.
        setupFreshAffinityInfo();
    }

    try {
        mNumaUtil = std::make_unique<NumaUtil>();
    }
    catch (const except::RuntimeError& e) {
        std::ostringstream ostr;
        ostr << "ShmAffinityInfoManager construction failed. construct NumaUtil failed. error=>{\n"
             << str_util::addIndent(e.what()) << '\n'
             << "}";
        throw ostr.str();
    }
    
    try {
        mCpuSocketUtil = std::make_unique<CpuSocketUtil>();
    }
    catch (const except::RuntimeError& e) {
        std::ostringstream ostr;
        ostr << "ShmAffinityInfoManager construction failed. construct CpuSocketUtil failed. error=>{\n"
             << str_util::addIndent(e.what()) << '\n'
             << "}";
        throw ostr.str();
    }

    if ((mAffinityResourceControl =
         std::make_unique<AffinityResourceControl>(*(mCpuSocketUtil.get()),
                                                   *(mNumaUtil.get()),
                                                   *(mAffinityInfo.get()))) == nullptr) {
        throw("ShmAffinityInfoManager construction failed."
              " construct AffinityResourceControl construction failed."); 
    }

    parserConfigure();
}

// static function
bool
ShmAffinityInfoManager::doesShmAlreadyExist(const bool testMode,
                                            const TestKeyStrFormat formatVersion)
//
// Might throw exception(std::string) if error
//
{
    using shmAffinityInfo_detail::HelperManager;

    return isShmAvailableByKey(getShmKeyStr(testMode, formatVersion),
                               HelperManager::convertToKeyIdByStrVer(formatVersion));
}

// static function
int
ShmAffinityInfoManager::getShmIdIfAvailable(const bool testMode,
                                            const TestKeyStrFormat formatVersion)
// return -1 if there is no shared memory
// return positiveNumber : return existed shared memory's id related keyStr    
// never returns 0.
// throw exception(std::string) if error
{
    using shmAffinityInfo_detail::HelperManager;
    
    return getShmIdIfAvailableByKey(getShmKeyStr(testMode, formatVersion),
                                    HelperManager::convertToKeyIdByStrVer(formatVersion));
}

// static function
bool
ShmAffinityInfoManager::removeShmIfAlreadyExist(const bool testMode,
                                                const TestKeyStrFormat formatVersion,
                                                const MsgFunc& msgCallBack)
//
// Might throw exception(std::string) if error
//    
// return true  : Successfully removed, or there is no target shared memory 
//        false : Failed to remove the shared memory
//
// An existing shared memory segment can be deleted only by its creator or by the root user.
// If anyone other than the creator or root attempts to remove it, an error will occur.
//
{
    using shmAffinityInfo_detail::HelperManager;

    if (!doesShmAlreadyExist(testMode, formatVersion)) {
        return true; // not exist -> skip
    }
    return rmUnusedShmByKey(getShmKeyStr(testMode, formatVersion),
                            HelperManager::convertToKeyIdByStrVer(formatVersion),
                            std::string(ShmDataIO::headerKeyShmAffInfo), msgCallBack);
}

// static function
bool
ShmAffinityInfoManager::removeShmIfAlreadyExistCmd(const bool testMode,
                                                   const TestKeyStrFormat formatVersion,
                                                   const MsgFunc& msgCallBack)
//
// An existing shared memory segment can be deleted only by its creator or by the root user.
// If anyone other than the creator or root attempts to remove it, an error will occur.
//
{
    try {
        if (!removeShmIfAlreadyExist(testMode, formatVersion, msgCallBack)) {
            msgCallBack("ERROR : Could not remove already existed shared memory. (ShmAffinityInfoManager)\n");
            return false;
        }
    }
    catch (const std::string& err) {
        std::ostringstream ostr;
        ostr << "ERROR : ShmAffinityInfoManager::removeShmIfAlreadyExist() failed. err=>{\n"
             << str_util::addIndent(err) << '\n'
             << "}";
        msgCallBack(ostr.str() + '\n');
        return false;
    }
    return true;
}

std::string
ShmAffinityInfoManager::acquireAffinityCores(const int requestedCoreTotal,
                                             const bool verifyMode)
//
// Throw exception(std::string err) if error
//
{
    const int maxAvailableCores = mAffinityResourceControl->calcAvailableCoreTotal();
    const int threadTotal = (maxAvailableCores < requestedCoreTotal) ? maxAvailableCores : requestedCoreTotal;

    std::vector<unsigned> coreIdTbl;
    try {
        coreIdTbl = mAffinityResourceControl->coreAllocation(threadTotal, verifyMode);
    }
    catch (const std::string& err) {
        std::ostringstream ostr;
        ostr << "AffinityResourceControl coreAllocation failed."
             << " requestedCoreTotal:" << requestedCoreTotal
             << " actualThreadTotalForAcquire:" << threadTotal << " err=>{\n"
             << str_util::addIndent(err) << '\n'
             << "}";
        throw ostr.str();
    }

    std::string coreIdDefStr = CpuSocketUtil::idTblToDefStr(coreIdTbl);

    std::string msgBuff;
    if (!updateCoreInfo(coreIdTbl, true, mAffinityResourceControl->getMyPid(),
                        [&](const std::string& msg) {
                            msgBuff += msg;
                            return true;
                        })) {
        std::ostringstream ostr;
        ostr << "Update newly aquired core info to shared memory failed."
             << " coreIdDefStr:" << coreIdDefStr << " msg=>{\n"
             << str_util::addIndent(msgBuff) << '\n'
             << "}";
        throw ostr.str();
    }

    return coreIdDefStr;
}

void
ShmAffinityInfoManager::releaseAffinityCores(const std::string& coreIdDefStr)
//
// Might throw exception(std::string& err) if error
//
{
    std::string logMessage;
    if (!updateCoreInfo(coreIdDefStr, false, 0x0,
                        [&](const std::string& msg) {
                            logMessage += msg;
                            return true;
                        })) {
        std::ostringstream ostr;
        ostr << "Release core info to shared memory failed."
             << "  coreIdDefStr:" << coreIdDefStr << " msg=>{\n"
             << str_util::addIndent(logMessage) << '\n'
             << "}";
        throw ostr.str();
    }
}

std::string
ShmAffinityInfoManager::show() const
{
    using shmAffinityInfo_detail::HelperManager;

    std::ostringstream ostr;
    ostr << "ShmAffinityInfoManager {\n"
         << str_util::addIndent(ShmDataManager::show()) << '\n'
         << "  sShmKeyStr:" << sShmKeyStr << '\n'
         << "  sShmTestKeyStr:" << sShmTestKeyStr << '\n'
         << "  mTestMode:" << str_util::boolStr(mTestMode) << '\n'
         << "  mTestKeyStrFormatVersion:" << showTestKeyStrFormat(mTestKeyStrFormatVersion) << '\n'
         << str_util::addIndent(showAffinityInfo()) << '\n'
         << "  mShmSetupCondition:" << HelperManager::setupConditionStr(mShmSetupCondition) << '\n'
         << str_util::addIndent(showNumaUtil()) << '\n'
         << str_util::addIndent(showCpuSocketUtil()) << '\n'
         << "}";
    return ostr.str();
}

std::string    
ShmAffinityInfoManager::showAffinityInfo() const
{
    if (!mAffinityInfo) return "mAffinityInfo is empty";
    return mAffinityInfo->show(mNumaUtil.get(), mCpuSocketUtil.get());
}

std::string
ShmAffinityInfoManager::showCoreInfoTable() const
{
    if (!mAffinityInfo) return "mAffinityInfo is empty";
    return mAffinityInfo->showCoreInfoTable2(mNumaUtil.get(), mCpuSocketUtil.get());
}

std::string
ShmAffinityInfoManager::showNumaUtil() const
{
    if (!mNumaUtil) return "mNumaUtil is empty";
    return mNumaUtil->show();
}

std::string
ShmAffinityInfoManager::showCpuSocketUtil() const
{
    if (!mCpuSocketUtil) return "mCpuSocketUtil is empty";
    return mCpuSocketUtil->show();
}

// static function
std::string
ShmAffinityInfoManager::showShmDump(const bool testMode,
                                    const TestKeyStrFormat formatVersion)
{
    std::ostringstream ostr;
    ostr << "testMode:" << str_util::boolStr(testMode) << ' '
         << "formatVersion:" << showTestKeyStrFormat(formatVersion) << ' '
         << "ShmKey:\"" << getShmKeyStr(testMode, formatVersion) << "\" {\n";
    if (!doesShmAlreadyExist(testMode, formatVersion)) {
        ostr << "  does not exist\n";
    } else {
        ShmAffinityInfoManager tmpInfoMgr(true, testMode, formatVersion);
        ostr << str_util::addIndent(tmpInfoMgr.ShmDataManager::show()) << '\n'
             << str_util::addIndent(tmpInfoMgr.showAffinityInfo()) << '\n';
    }
    ostr << "}";
    return ostr.str();
}

// static function
std::string
ShmAffinityInfoManager::showTestKeyStrFormat(const TestKeyStrFormat formatVersion)
{
    switch (formatVersion) {
    case TestKeyStrFormat::VER_0 : return "VER_0";
    case TestKeyStrFormat::VER_1 : return "VER_1";
    default : return "?";
    }
}

// static function
std::string
ShmAffinityInfoManager::showShmIdDetailedInfo(const int shmId, const std::string& userName)
{
    return shmAffinityInfo_detail::HelperManager::analyzeShmIdMsg(shmId, userName, false);
}

void
ShmAffinityInfoManager::setupFreshAffinityInfo()
//
// Might throw exception(std::string) if error
//
{
    using shmAffinityInfo_detail::HelperManager;

    const bool existFlag =
        constructNewShmByKey(getShmKeyStr(mTestMode, mTestKeyStrFormatVersion),
                             HelperManager::convertToKeyIdByStrVer(mTestKeyStrFormatVersion),
                             ShmAffinityInfo::calcDataSize(),
                             ShmDataManager::SHMAFFINFO_PERMISSION);
    mShmSetupCondition = (existFlag) ? SetupCondition::ALREADY_EXISTED : SetupCondition::INITIALIZED;

    try {
        const bool initFlag = !existFlag;
        const Hash initHash = Sha1Util::init();

        mAffinityInfo = std::make_unique<ShmAffinityInfo>(initHash, mShmAddr, mShmSize, initFlag);
    }
    catch (const std::string& err) {
        std::ostringstream ostr;
        ostr << "ShmAffinityInfoManager construct ShmAffinityInfo failed. error={\n"
             << str_util::addIndent(err) << '\n'
             << "}";
        throw ostr.str();
    }
}

void
ShmAffinityInfoManager::accessAffinityInfo()
//
// Might throw exception(std::string) if error
//
{
    using shmAffinityInfo_detail::HelperManager;

    accessSetupShmByKey(getShmKeyStr(mTestMode, mTestKeyStrFormatVersion),
                        HelperManager::convertToKeyIdByStrVer(mTestKeyStrFormatVersion),
                        ShmAffinityInfo::calcDataSize());
    mShmSetupCondition = SetupCondition::ALREADY_EXISTED;

    //------------------------------

    const size_t shmSize = ShmAffinityInfo::retrieveShmDataSize(mShmAddr);
    if (mShmSize != shmSize) {
        std::ostringstream ostr;
        ostr << "ShmAffinityInfoManager::ShmAffinityInfoManager() shared memory size mismatch"
             << " storedSize:" << shmSize << " != currSize:" << mShmSize;
        throw ostr.str();
    }

    try {
        const Hash dummyHash = Sha1Util::init();
        mAffinityInfo = std::make_unique<ShmAffinityInfo>(dummyHash, mShmAddr, mShmSize, false);
    }
    catch (const std::string& err) {
        std::ostringstream ostr;
        ostr << "ShmAffinityInfoManager::ShmAffinityInfoManager() construct failed. error={\n"
             << str_util::addIndent(err) << '\n'
             << "}";
        throw ostr.str();
    }
}

// static function
std::string
ShmAffinityInfoManager::getShmKeyStr(const bool testMode,
                                     const TestKeyStrFormat formatVersion)
{
    return (!testMode) ? std::string(sShmKeyStr) : getShmTestKeyStr(std::string(""), formatVersion);
}

// static function
std::string
ShmAffinityInfoManager::getShmTestKeyStr(const std::string& userName,
                                         const TestKeyStrFormat formatVersion)
{
    auto getShmTestKeyStrVer1 = [&]() {
        return std::string(sShmTestKeyStr) + "_" + (userName.empty() ? UserUtil::getUserName() : userName);
    };

    switch(formatVersion) {
    case TestKeyStrFormat::VER_0 : return std::string(sShmTestKeyStr);
    case TestKeyStrFormat::VER_1 : return getShmTestKeyStrVer1();
    default : return "";
    }
}

bool
ShmAffinityInfoManager::setCore(const unsigned coreId,
                                const bool occupancy,
                                const size_t pid)
{
    if (!mAffinityInfo) return false;
    return mAffinityInfo->setCoreInfo(coreId, occupancy, pid);
}

void
ShmAffinityInfoManager::parserConfigure()
{
    mParser.description("ShmAffinityInfoManager command");

    mParser.opt("shmDataManager", "...command...", "shmDataManager command",
                [&](Arg& arg) { return ShmDataManager::getParser().main(arg.childArg()); });
    mParser.opt("show", "", "show all info",
                [&](Arg& arg) { return arg.msg(show() + '\n'); });
    mParser.opt("showTable", "", "show coreInfo as table",
                [&](Arg& arg) { return arg.msg(showCoreInfoTable() + '\n'); });
    mParser.opt("updateCore", "<coreIdDefStr> <occupancyBool> <PID>", "update coreInfo",
                [&](Arg& arg) {
                    const std::string coreIdDefStr = (arg++)();
                    const bool occupancy = (arg++).as<bool>(0);
                    const size_t pid = (arg++).as<size_t>(0);
                    return updateCoreInfo(coreIdDefStr, occupancy, pid,
                                          [&](const std::string& msg) { return arg.msg(msg); });
                });
    mParser.opt("updateAllCores", "<occupancyBool> <PID>", "update all coreInfo",
                [&](Arg& arg) {
                    const bool occupancy = (arg++).as<bool>(0);
                    const size_t pid = (arg++).as<size_t>(0);
                    return updateAllCoreInfo(occupancy, pid,
                                             [&](const std::string& msg) { return arg.msg(msg); });
                });
    mParser.opt("clearCore", "<coreIdDefStr>", "clear coreInfo",
                [&](Arg& arg) {
                    const std::string coreIdDefStr = (arg++)();
                    return updateCoreInfo(coreIdDefStr, false, 0, [&](const std::string& msg) { return arg.msg(msg); });
                });
    mParser.opt("clearAllCores", "", "clear all coreInfo",
                [&](Arg& arg) {
                    return updateAllCoreInfo(false, 0, [&](const std::string& msg) { return arg.msg(msg); });
                });
    mParser.opt("affinityResourceControl", "...command...", "affinity resource control command",
                [&](Arg& arg) { return mAffinityResourceControl->getParser().main(arg.childArg()); });
    mParser.opt("storeTestData", "<0|1>", "store testData into shared memory. Argument is dataTypeId (0 or 1)",
                [&](Arg& arg) {
                    return storeTestData((arg++).as<int>(0), [&](const std::string& msg) { return arg.msg(msg); });
                });
    mParser.opt("verifyTestData", "<0|1>", "verify testData of shared memory. Argument is dataTypeId (0 or 1)",
                [&](Arg& arg) {
                    return verifyTestData((arg++).as<int>(0), [&](const std::string& msg) { return arg.msg(msg); });
                });
    mParser.opt("verifyCoreAllocation", "<mode> <max> <update>",
                "verify coreAllocation logic. mode=localhost,ag,tin,cobalt max=CoreAllocMax update=PidUpdateInterval",
                [&](Arg& arg) {
                    const std::string modeStr = (arg++)();
                    const int randMaxSize = (arg++).as<int>(0);
                    const int myPidUpdateInterval = (arg++).as<int>(0);
                    return verifyCoreAllocation(modeStr,
                                                randMaxSize,
                                                myPidUpdateInterval,
                                                [&](const std::string& msg) { return arg.msg(msg); });
                });
    mParser.opt("cpuSocketUtil", "...command...", "mCpuSocketUtil command",
                [&](Arg& arg) { return mCpuSocketUtil->getParser().main(arg.childArg()); });
    mParser.opt("numaUtil", "...command...", "mNumaUtil command",
                [&](Arg& arg) { return mNumaUtil->getParser().main(arg.childArg()); });
    mParser.opt("analyzeShmId", "<shmId> <userName>", "analyze given shmId in terms of affinityMapTable",
                [&](Arg& arg) {
                    const int shmId = (arg++).as<int>(0);
                    const std::string userName = (arg++)();
                    return arg.msg(shmAffinityInfo_detail::HelperManager::analyzeShmIdMsg(shmId, userName, true) + '\n');
                });
}

bool
ShmAffinityInfoManager::updateCoreInfo(const std::string& coreIdDefStr,
                                       const bool occupancy,
                                       const size_t pid,
                                       const MsgFunc& msgCallBack)
{
    std::vector<unsigned> coreIdTbl;
    std::string errMsg;
    if (!CpuSocketUtil::parseIdDef(coreIdDefStr, coreIdTbl, errMsg)) {
        msgCallBack("Parse coreIdDefStr failed error:" + errMsg + '\n');
        return false;
    }

    return updateCoreInfo(coreIdTbl, occupancy, pid, msgCallBack);
}

bool
ShmAffinityInfoManager::updateCoreInfo(const std::vector<unsigned>& coreIdTbl,
                                       const bool occupancy,
                                       const size_t pid,
                                       const MsgFunc& msgCallBack)
{
    if (!mAffinityInfo) {
        msgCallBack("mAffinityInfo is empty\n");
        return false;
    }
    if (coreIdTbl.empty()) {
        msgCallBack("coreIdDefStr is empty\n");
        return false;
    }

    for (size_t id = 0; id < coreIdTbl.size(); ++id) {
        const unsigned coreId = coreIdTbl[id];
        if (!setCore(coreId, occupancy, pid)) {
            std::ostringstream ostr;
            ostr << "setCore() failed."
                 << " coreId:" << coreId
                 << " occupancy:" << str_util::boolStr(occupancy)
                 << " pid:" << pid;
            msgCallBack(ostr.str() + '\n');
            return false;
        }

        std::ostringstream ostr;
        ostr << "updateCoreInfo() OK."
             << " coreId:" << coreId
             << " occupancy:" << str_util::boolStr(occupancy)
             << " pid:" << pid;
        msgCallBack(ostr.str() + '\n');
    }

    return true;
}

bool
ShmAffinityInfoManager::updateAllCoreInfo(const bool occupancy,
                                          const size_t pid,
                                          const MsgFunc& msgCallBack)
{
    if (!mAffinityInfo) {
        msgCallBack("mAffinityInfo is empty\n");
        return false;
    }

    const unsigned numCores = mAffinityInfo->getNumCores();
    for (unsigned coreId = 0; coreId < numCores; ++coreId) {
        if (!setCore(coreId, occupancy, pid)) {
            std::ostringstream ostr;
            ostr << "setCore() failed."
                 << " coreId:" << coreId
                 << " occupancy:" << str_util::boolStr(occupancy)
                 << " pid:" << pid;
            msgCallBack(ostr.str() + '\n');
            return false;
        }
    }

    std::ostringstream ostr;
    ostr << "updateAllCoreInfo() OK."
         << " occupancy:" << str_util::boolStr(occupancy)
         << " pid:" << pid;
    msgCallBack(ostr.str() + '\n');

    return true;
}                                          

bool
ShmAffinityInfoManager::storeTestData(const int testDataTypeId, const MsgFunc& msgCallBack)
{
    if (!mAffinityInfo) return false;

    updateAllCoreInfo(false, 0, msgCallBack); // clear all coreInfo first.

    const bool flag = setGetTestData(testDataTypeId, true); // setup test data into shared memory

    std::ostringstream ostr;
    ostr << "ShmAffinityInfoManager::storeTestData() testDataTypeId:" << testDataTypeId << " {\n";
    if (flag) {
        ostr << str_util::addIndent(mAffinityInfo->show()) << '\n';
    } else {
        ostr << "  Failed.\n";
    }
    ostr << "}";
    msgCallBack(ostr.str() + '\n');

    return flag;
}

bool
ShmAffinityInfoManager::verifyTestData(const int testDataTypeId, const MsgFunc& msgCallBack)
{
    if (!mAffinityInfo) return false;

    const bool flag = setGetTestData(testDataTypeId, false); // verify test data
    std::ostringstream ostr;
    ostr << "ShmAffinityInfoManager::verifyTestData() testDataTypeId:" << testDataTypeId << " {\n";
    if (flag) {
        ostr << "  OK\n";
    } else {
        ostr << "  Failed\n";
    }
    ostr << "}";
    msgCallBack(ostr.str() + '\n');

    return flag;
}

bool
ShmAffinityInfoManager::setGetTestData(const int testDataTypeId, const bool storeFlag)
{
    if (!mAffinityInfo) return false;

    const unsigned totalCores = mAffinityInfo->getNumCores();
    for (unsigned coreId = 0; coreId < totalCores; ++coreId) {
        bool occupancy = false;
        size_t pid = 0;

        switch (testDataTypeId) {
        case 0 :
            if (coreId % 2 == 1) {
                occupancy = true;
                pid = 1000 + coreId; // dummy pid
            }
            break;
        case 1 :
            if (coreId > totalCores / 2) {
                occupancy = true;
                pid = 2000 + coreId; // dummy pid
            }
            break;
        default :
            break;
        }

        if (storeFlag) {
            if (!mAffinityInfo->setCoreInfo(coreId, occupancy, pid)) return false;
        } else {
            bool currOccupancy;
            size_t currPid;
            if (!mAffinityInfo->getCoreInfo(coreId, currOccupancy, currPid)) return false;
            if (currOccupancy != occupancy || currPid != pid) return false; // compare failed.
        }
    }
    return true;
}

bool
ShmAffinityInfoManager::verifyCoreAllocation(const std::string& modeStr,
                                             const int randMaxSize,
                                             const int myPidUpdateInterval,
                                             const MsgFunc& msgCallBack)
{
    if (!mNumaUtil || !mCpuSocketUtil || !mAffinityResourceControl) return false;
    resetMode(modeStr, msgCallBack); // reset environment
    updateAllCoreInfo(false, 0, msgCallBack); // all empty

    std::random_device rnd;
    std::mt19937 mt(rnd());
    std::uniform_int_distribution<> randGen(1, randMaxSize - 1); // range is [1, rnadMaxSize)

    const int initialAvailableTotal = mAffinityResourceControl->calcAvailableCoreTotal();
    int availableTotal = initialAvailableTotal;
    msgCallBack(msgVerifyStrInit(randMaxSize, myPidUpdateInterval) + '\n');

    size_t testMyPid = 1000;
    mAffinityResourceControl->updateMyPidForUnitTest(testMyPid);

    int testTotal = 0;
    int changePidTotal = 0;
    for (int testId = 0; ; ++testId) {
        msgCallBack(msgVerifyStrTestLoopHeader(testId, testMyPid) + '\n');

        const int currAvailableTotal = mAffinityResourceControl->calcAvailableCoreTotal();
        if (availableTotal != currAvailableTotal) {
            msgCallBack(msgVerifyErrorStrCoreSizeMismatch(availableTotal, currAvailableTotal) + '\n');
            return false;
        }
        if (!currAvailableTotal) {
            msgCallBack("currAvailableTotal == 0. end of test\n");
            testTotal = testId;
            break; // end of test loop
        }

        const int requestCoresTotal = std::min(randGen(mt), currAvailableTotal);
        msgCallBack("requestCoresTotal:" + std::to_string(requestCoresTotal) + '\n');

        if (!verifyCoreAllocationMain(requestCoresTotal, msgCallBack)) {
            msgCallBack("VERIFY-ERROR: verifyCoreAllocationMain() failed.\n");
            return false;
        }
        msgCallBack(msgVerifyStrTestLoopVerifyOK(testId, changePidTotal) + '\n');
        
        if ((testId + 1) % myPidUpdateInterval == 0) {
            mAffinityResourceControl->updateMyPidForUnitTest(++testMyPid);
            changePidTotal++;
            msgCallBack("testMyPid incremented to:" + std::to_string(testMyPid) + '\n');
        }
        availableTotal -= requestCoresTotal;
    }

    /* useful debug message
    std::cerr << ">> ShmAffinityInfo.cc verifyCoreAllocation()"
              << msgVerifyStrFinalOK(modeStr,
                                     initialAvailableTotal, randMaxSize, myPidUpdateInterval, testTotal, changePidTotal)
              << '\n';
    */
    msgCallBack(msgVerifyStrFinalOK(modeStr, initialAvailableTotal, randMaxSize, myPidUpdateInterval,
                                    testTotal, changePidTotal) + '\n');
    msgCallBack("verifyCoreAllocationTestTotal=" + std::to_string(testTotal) + '\n');
    return true; // verify OK
}

void
ShmAffinityInfoManager::resetMode(const std::string& modeStr,
                                  const MsgFunc& msgCallBack)
{
    if (!mNumaUtil || !mCpuSocketUtil) return;

    try {
        mNumaUtil->reset(modeStr);
        mCpuSocketUtil->reset(modeStr);
    }
    catch (const except::RuntimeError& e) {
        std::ostringstream ostr;
        ostr << "mNumaUtil/mCpuSocketUtil reset(modeStr:" << modeStr << ") failed. err=>{\n"
             << str_util::addIndent(e.what()) << '\n'
             << "}";
        msgCallBack(ostr.str() + '\n');
    }
}

bool
ShmAffinityInfoManager::verifyCoreAllocationMain(const int requestCoresTotal, const MsgFunc& msgCallBack)
{
    try {
        acquireAffinityCores(requestCoresTotal, true);
    }
    catch (const std::string& err) {
        std::ostringstream ostr;
        ostr << "VERIFY-ERROR : acquireAffinityCores() failed. {\n"
             << str_util::addIndent(err) + '\n'
             << "}";
        msgCallBack(ostr.str() + '\n');
        return false;
    }
    return true;
}

std::string
ShmAffinityInfoManager::msgVerifyStrInit(const int randMaxSize, const int myPidUpdateInterval) const
{
    std::ostringstream ostr;
    ostr << "======>>> ShmAffinityInfoManager::verifyCoreAllocation("
         << "randMaxSize:" << randMaxSize
         << " myPidUpdateInterval:" << myPidUpdateInterval
         << ") <<<====== initial condition {\n"
         << str_util::addIndent(showCoreInfoTable()) << '\n'
         << "}";
    return ostr.str();
}

std::string
ShmAffinityInfoManager::msgVerifyStrTestLoopHeader(const int testId, const size_t testMyPid) const
{
    std::ostringstream ostr;
    ostr << "======>>> testId:" << testId << " testMyPid:" << testMyPid << " <<<======";
    return ostr.str();
}

std::string
ShmAffinityInfoManager::msgVerifyErrorStrCoreSizeMismatch(const int availableTotal,
                                                          const int currAvailableTotal) const
{
    std::ostringstream ostr;
    ostr << "VERIFY-ERROR : Remaining available core size mismatch. {\n"
         << "  targetAvailableCoreTotal:" << availableTotal << '\n'
         << "  currAvailableCoreTotal:" <<  currAvailableTotal << '\n'
         << "}";
    return ostr.str();
}

std::string
ShmAffinityInfoManager::msgVerifyStrTestLoopVerifyOK(const int testId, const int changePidTotal) const
{
    std::ostringstream ostr;
    ostr << "RUNTIME-VERIFY: OK\n"
         << "TestId:" << testId << " (changePidTotal:" << changePidTotal << ") core condition result {\n"
         << str_util::addIndent(showCoreInfoTable()) << '\n'
         << "}";
    return ostr.str();
}

std::string
ShmAffinityInfoManager::msgVerifyStrFinalOK(const std::string& modeStr,
                                            const int initialAvailableTotal,
                                            const int randMaxSize,
                                            const int myPidUpdateInterval,
                                            const int totalTest,
                                            const int changePidTotal) const
{
    std::ostringstream ostr;
    ostr << "VerifyCoreAllocation() OK. {\n"
         << "  modeStr:" << modeStr << '\n'
         << "  initialAvailableTotal:" << initialAvailableTotal << '\n'
         << "  randMaxSize:" << randMaxSize << '\n'
         << "  myPidUpdateInterval:" << myPidUpdateInterval << '\n'
         << "  totalTest:" << totalTest << '\n'
         << "  changePidTotal:" << changePidTotal << '\n'
         << "}";
    return ostr.str();
}

namespace shmAffinityInfo_detail {

// static function
ShmDataManager::KeyIdByStrVer
HelperManager::convertToKeyIdByStrVer(const ShmAffinityInfoManager::TestKeyStrFormat formatVersion)
{
    switch (formatVersion) {
    case ShmAffinityInfoManager::TestKeyStrFormat::VER_0 : return ShmDataManager::KeyIdByStrVer::VER_0;
    case ShmAffinityInfoManager::TestKeyStrFormat::VER_1 : return ShmDataManager::KeyIdByStrVer::VER_1;
    default : return ShmDataManager::KeyIdByStrVer::VER_1;
    }
}

// static function
std::string
HelperManager::setupConditionStr(const ShmAffinityInfoManager::SetupCondition condition)
{
    switch (condition) {
    case ShmAffinityInfoManager::SetupCondition::UNDEFINED : return "UNDEFINED";
    case ShmAffinityInfoManager::SetupCondition::INITIALIZED : return "INITIALIZED";
    case ShmAffinityInfoManager::SetupCondition::ALREADY_EXISTED : return "ALREADY_EXISTED";
    default : return "?";
    }
}

// static function
std::string
HelperManager::analyzeShmIdMsg(const int shmId,
                               const std::string& userName,
                               const bool detailedMsg)
{
    constexpr ShmAffinityInfoManager::TestKeyStrFormat strVer0 =
        ShmAffinityInfoManager::TestKeyStrFormat::VER_0;
    constexpr ShmAffinityInfoManager::TestKeyStrFormat strVer1 =
        ShmAffinityInfoManager::TestKeyStrFormat::VER_1;
    constexpr ShmDataManager::KeyIdByStrVer idVer0 = ShmDataManager::KeyIdByStrVer::VER_0;
    constexpr ShmDataManager::KeyIdByStrVer idVer1 = ShmDataManager::KeyIdByStrVer::VER_1;

    std::string shortMsg0, shortMsg1, longMsg0, longMsg1;
    bool flag0 = analyzeShmId(shmId, userName, false, strVer1, idVer0, &shortMsg0, &longMsg0);
    bool flag1 = analyzeShmId(shmId, userName, false, strVer1, idVer1, &shortMsg1, &longMsg1);

    std::string shortTestMsg00, shortTestMsg01, shortTestMsg10, shortTestMsg11;
    std::string longTestMsg00, longTestMsg01, longTestMsg10, longTestMsg11;
    bool testFlag00 = analyzeShmId(shmId, userName, true, strVer0, idVer0, &shortTestMsg00, &longTestMsg00);
    bool testFlag01 = analyzeShmId(shmId, userName, true, strVer0, idVer1, &shortTestMsg01, &longTestMsg01);
    bool testFlag10 = analyzeShmId(shmId, userName, true, strVer1, idVer0, &shortTestMsg10, &longTestMsg10);
    bool testFlag11 = analyzeShmId(shmId, userName, true, strVer1, idVer1, &shortTestMsg11, &longTestMsg11);
    
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
        ostr << "====>>> analysis of shmId:" << shmId << " userName:" << userName << " <<<==== {\n"
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

#ifdef RM_CANDIDATE
// static function
bool
HelperManager::isExpectedExistedShmId(const int targetShmId,
                                      const bool testMode,
                                      const ShmAffinityInfoManager::TestKeyStrFormat formatVersion,
                                      const std::string& userName)
//
// returns true if targetShmId is existed and it is associated with given argument parameters.
// otherwise returns falase.
// using my process's user name if userName is empty
//
{
    std::string keyStr;
    if (!testMode) {
        keyStr = ShmAffinityInfoManager::getShmKeyStr(testMode, formatVersion);
    } else {
        keyStr = ShmAffinityInfoManager::getShmTestKeyStr(userName, formatVersion);
    }
    int shmId =
        ShmDataManager::getShmIdIfAvailableByKey(keyStr,
                                                 convertToKeyIdByStrVer(formatVersion));
    if (shmId < 0) return false; // There is no shared memory associated with his keyStr
    return (targetShmId == shmId);
}
#endif // end of RM_CANDIDATE

// static function
bool
HelperManager::analyzeShmId(const int targetShmId,
                            const std::string& userName,
                            const bool testMode,
                            const ShmAffinityInfoManager::TestKeyStrFormat keyStrVer,
                            const ShmDataManager::KeyIdByStrVer keyIdByStrVer,
                            std::string* shortMsg,
                            std::string* detailedMsg)
{
    const std::string keyStr =
        (!testMode) ?
        ShmAffinityInfoManager::getShmKeyStr(false, keyStrVer) :
        ShmAffinityInfoManager::getShmTestKeyStr(userName, keyStrVer);
    const int keyId = ShmAffinityInfoManager::genKeyIdByStr(keyStr, keyIdByStrVer);
    const int shmId = shmget(keyId, 0, ShmDataManager::SHMAFFINFO_PERMISSION);
    const bool flag = (shmId == targetShmId);    

    if (shortMsg) {
        std::ostringstream ostr;
        if (flag) {
            ostr << (testMode ? "TEST" : "LIVE") << '-';
            if (keyStrVer == ShmAffinityInfoManager::TestKeyStrFormat::VER_0 &&
                keyIdByStrVer == ShmDataManager::KeyIdByStrVer::VER_0) {
                ostr << "VER_0";
            } else if (keyStrVer == ShmAffinityInfoManager::TestKeyStrFormat::VER_1 &&
                       keyIdByStrVer == ShmDataManager::KeyIdByStrVer::VER_1) {
                ostr << "VER_1";
            } else {
                ostr << "(str:" << ShmAffinityInfoManager::showTestKeyStrFormat(keyStrVer) << '+'
                     << "id:" << ShmDataManager::showKeyIdByStrVer(keyIdByStrVer) << ')';
            }
        } else {
            ostr << "?";
        }
        (*shortMsg) = ostr.str();
    }

    if (detailedMsg) {
        std::ostringstream ostr;
        ostr << "targetShmId:" << targetShmId
             << " userName:" << userName
             << " testMode:" << str_util::boolStr(testMode)
             << " keyStrVer:" << ShmAffinityInfoManager::showTestKeyStrFormat(keyStrVer)
             << " keyIdVer:" << ShmDataManager::showKeyIdByStrVer(keyIdByStrVer) << " {\n"
             << "  keyStr:" << keyStr << '\n'
             << "  keyId:" << keyId << '\n'
             << "  shmId:" << shmId << '\n'
             << "}";
        if (flag) {
            ostr << " shmId == targetShmId";
        } else {
            ostr << " skipped"; 
        }
        (*detailedMsg) = ostr.str();
    }
    
    return flag;
}
    
} // namespace shmAffinityInfo_detail

} // namespace grid_util
} // namespace scene_rdl2
