// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "AffinityResourceControl.h"
#include "CpuSocketUtil.h"
#include "NumaUtil.h"

#include "Arg.h"
#include "Parser.h"
#include "Sha1Util.h"
#include "ShmData.h"

#include <memory>

namespace scene_rdl2 {
namespace grid_util {

#define SHM_AFFINITY_INFO_HEADKEY "affinityInfo"

class CpuSocketUtil;
class NumaUtil;

class ShmAffinityInfo : public ShmDataIO
//
// This is an affinity information of this host, which includes all the cores' affinity conditions.
//
{
public:
    using Hash = Sha1Util::Hash;

    ShmAffinityInfo(const Hash& hash,
                    void* const dataStartAddr, const size_t dataSize, const bool doInit)
        : ShmDataIO {dataStartAddr, dataSize}
    {
        if (!verifyMemBoundary()) {
            throw(errMsg("ShmAffinityInfo constructor", "verify memory size/boundary failed"));
        }
        if (doInit) {
            setHeadMessage(SHM_AFFINITY_INFO_HEADKEY);
            setShmDataSize(dataSize);
            //------------------------------
            setSemInitHash(hash);
            setNumCores(getTotalNumCores());
            initCoreInfoTable();
        }
    }

    static size_t calcDataSize()
    {
        return offset_coreInfoStart + size_singleCoreInfo * getTotalNumCores();
    }

    static std::string retrieveHeadMessage(void* const topAddr)
    {
        return retrieveMessage(topAddr, offset_headMessage, size_headMessage); 
    }
    static size_t retrieveShmDataSize(void* const topAddr) { return retrieveSizeT(topAddr, offset_shmDataSize); }
    static Hash retrieveSemInitHash(void* const topAddr) { return retrieveHash(topAddr, offset_semInitHash); }
    static unsigned retrieveNumCores(void* const topAddr) { return retrieveUnsigned(topAddr, offset_numCores); }
    static bool retrieveCoreInfo(void* const topAddr, const unsigned coreId, bool& occupancy, size_t& pid)
    {
        if (!checkCoreId(coreId)) return false; // error
        occupancy = retrieveBool(topAddr, calcCoreInfoOffset(coreId) + localOffset_coreInfoOccupancy);
        pid = retrieveSizeT(topAddr, calcCoreInfoOffset(coreId) + localOffset_coreInfoPID);
        return true;
    }

    std::string getHeadMessage() const { return getMessage(offset_headMessage); }
    size_t getShmDataSize() const { return getSizeT(offset_shmDataSize); }
    Hash getSemInitHash() const { return getHash(offset_semInitHash); }
    unsigned getNumCores() const { return getUnsigned(offset_numCores); }
    bool getCoreInfo(const unsigned coreId, bool& occupancy, size_t& pid) const
    {
        if (!checkCoreId(coreId)) return false; // error
        occupancy = getBool(calcCoreInfoOffset(coreId) + localOffset_coreInfoOccupancy);
        pid = getSizeT(calcCoreInfoOffset(coreId) + localOffset_coreInfoPID);
        return true;
    }

    void setSemInitHash(const Hash& hash) { setHash(offset_semInitHash, hash); }
    bool setCoreInfo(const unsigned coreId, const bool occupancy, const size_t pid)
    {
        if (!checkCoreId(coreId)) return false; // error
        setBool(calcCoreInfoOffset(coreId) + localOffset_coreInfoOccupancy, occupancy);
        setSizeT(calcCoreInfoOffset(coreId) + localOffset_coreInfoPID, pid);
        return true;
    }
    void initCoreInfo(const unsigned coreId) { setCoreInfo(coreId, false, 0); }

    static std::string showOffset();
    static std::string showSizeInfo();
    std::string show(const NumaUtil* numaUtilObsPtr = nullptr,
                     const CpuSocketUtil* cpuSocketUtilObsPtr = nullptr) const;
    std::string showCoreInfoTable(const NumaUtil* numaUtilObsPtr = nullptr,
                                  const CpuSocketUtil* cpuSocketUtilObsPtr = nullptr) const;
    std::string showCoreInfoTable2(const NumaUtil* numaUtilObsPtr = nullptr,
                                   const CpuSocketUtil* cpuSocketUtilObsPtr = nullptr) const;

    bool verifySetGet(const int dataTypeId); // so far only available 0 for dataTypeId at this moment

private:
    // We should not remove or change the order of following items. We are only available to add new items at
    // the end of shared memory data. This is mandatory to keep backward compatibility to safely access
    // shared memory fb via old binary.
    //
    // (A): Semaphore initialization completion hash
    //      We have to consider carefully the semaphore initialization under multi-process safe conditions.
    //      This hash value is used to make sure the semaphore is properly initialized by the process that
    //      constructed The initial condition of this field is 0x0.
    // (B): All the core information is stored from this point. Each core information consists of (C) and (D)
    //      See calcCoreInfoOffset() for the data layout of each core.
    // (C): Indicate occupancy condition: true or false
    // (D): Process ID that uses this core by CPU affinity control. Only valid if occupancy is true.
    //
    static constexpr size_t offset_headMessage = 0;
    static constexpr size_t size_headMessage = ShmDataIO::headerSize;
    static constexpr size_t offset_shmDataSize = offset_headMessage + size_headMessage;
    static constexpr size_t offset_semInitHash = offset_shmDataSize + sizeof(size_t); // ...(A)
    static constexpr size_t size_semInitHash = Sha1Util::HASH_SIZE;
    static constexpr size_t offset_numCores = offset_semInitHash + size_semInitHash;
    static constexpr size_t offset_coreInfoStart = offset_numCores + sizeof(unsigned); // ...(B)
    static constexpr size_t size_singleCoreInfo = sizeof(size_t) * 2;
    static constexpr size_t localOffset_coreInfoOccupancy = 0; // ...(C)
    static constexpr size_t localOffset_coreInfoPID = sizeof(size_t); // ...(D)

    bool verifyMemBoundary() const { return calcDataSize() == mDataSize; } 

    void initCoreInfoTable();

    void setHeadMessage(const std::string& msg) { setMessage(offset_headMessage, size_headMessage, msg); }
    void setShmDataSize(const size_t size) { setSizeT(offset_shmDataSize, size); } 
    void setNumCores(const unsigned ui) { setUnsigned(offset_numCores, ui); }

    size_t getMaxPid() const;

    template <typename F>
    bool
    crawlAllCores(F coreFunc) const
    {
        const unsigned numCores = getNumCores();
        for (unsigned coreId = 0; coreId < numCores; ++coreId) {
            bool currOccupancy;
            size_t currPID;
            getCoreInfo(coreId, currOccupancy, currPID);
            if (!coreFunc(coreId, currOccupancy, currPID)) return false;
        }
        return true;
    }

    static bool checkCoreId(const unsigned coreId) { return (coreId < getTotalNumCores()); }
    static size_t calcCoreInfoOffset(const unsigned coreId)
    {
        return size_singleCoreInfo * coreId + offset_coreInfoStart;
    }
    static size_t calcCoreInfoTableSize() { return size_singleCoreInfo * getTotalNumCores(); }
    static size_t calcTotalShmSize() { return offset_coreInfoStart + calcCoreInfoTableSize(); }
    static unsigned getTotalNumCores();

    bool verifySetGetMain(const int dataTypeId, const bool setup);
    bool verifySetGetMain_type0(const bool setup);
};

class ShmAffinityInfoManager : public ShmDataManager
//
// This class constructs CPU affinity information of this host on the shared memory or
// accesses a CPU affinity information that is already stored on the shared memory.
//
{
public:
    using Hash = Sha1Util::Hash;
    using MsgFunc = std::function<bool(const std::string& msg)>;

    // Construct a fresh ShmAffinityInfoManager from scratch and generate a new shmId
    // Might throw exception(std::string) if error
    ShmAffinityInfoManager(const bool accessOnly, const bool testMode = false);

    static bool doesShmAlreadyExist(const bool testMode); // might throw exception(std::string) if error

    // An existing shared memory segment can be deleted only by its creator or by the root user.
    // If anyone other than the creator or root attempts to remove it, an error will occur.
    static bool rmShmIfAlreadyExist(const bool testMode, const MsgFunc& msgCallBack); // might throw exception(std::string) if error
    static bool rmShmIfAlreadyExistCmd(const bool testMode, const MsgFunc& msgCallBack);

    std::string acquireAffinityCores(const int requestedCoreTotal,
                                    const bool verifyMode = false); // Might throw exception(std::string) if error
    void releaseAffinityCores(const std::string& coreIdDefStr); // Might throw exception(std::string) if error

    ShmAffinityInfo& getAffinityInfo() const { return *(mAffinityInfo.get()); };

    std::string show() const;
    std::string showAffinityInfo() const;
    std::string showCoreInfoTable() const;
    std::string showNumaUtil() const;
    std::string showCpuSocketUtil() const;

    static std::string showShmDump(const bool testMode = false);

    Parser& getParser() { return mParser; }

private:
    enum class SetupCondition : int {
        UNDEFINED,
        INITIALIZED,
        ALREADY_EXISTED
    };

    void setupFreshAffinityInfo(); // might throw exception(std::string) if error
    void accessAffinityInfo(); // might throw exception(std::string) if error
    static const std::string getShmKeyStr(const bool testMode);

    bool setCore(const unsigned coreId, const bool occupancy, const size_t pid);

    static std::string setupConditionStr(const SetupCondition& condition);

    void parserConfigure();
    bool updateCoreInfo(const std::string& coreIdDefStr, const bool occupancy, const size_t pid,
                        const MsgFunc& msgCallBack);
    bool updateCoreInfo(const std::vector<unsigned>& coreIdTbl, const bool occupancy, const size_t pid,
                        const MsgFunc& msgCallBack);
    bool updateAllCoreInfo(const bool occupancy, const size_t pid,
                           const MsgFunc& msgCallBack);
    bool storeTestData(const int testDataTypeId, const MsgFunc& msgCallBack);
    bool verifyTestData(const int testDataTypeId, const MsgFunc& msgCallBack);
    bool setGetTestData(const int testDataTypeId, const bool storeFlag);

    bool verifyCoreAllocation(const std::string& modeStr,
                              const int randMaxSize,
                              const int myPidUpdateInterval,
                              const MsgFunc& msgCallBack);
    void resetMode(const std::string& modeStr, const MsgFunc& msgCallBack);
    bool verifyCoreAllocationMain(const int requestCoresTotal, const MsgFunc& msgCallBack);
    std::string msgVerifyStrInit(const int randMaxSize, const int myPidUpdateInterval) const;
    std::string msgVerifyStrTestLoopHeader(const int testId, const size_t testMyPid) const;
    std::string msgVerifyErrorStrCoreSizeMismatch(const int availableTotal, const int currAvailableTotal) const;
    std::string msgVerifyStrTestLoopVerifyOK(const int testId, const int changePidTotal) const;
    std::string msgVerifyStrFinalOK(const std::string& modeStr,
                                    const int initialAvailableTotal,
                                    const int randMaxSize,
                                    const int myPidUpdateInterval,
                                    const int totalTest,
                                    const int changePidTotal) const;

    //------------------------------

    static constexpr const char* sShmKeyStr = "AffinityInfoSharedMemoryKey";
    static constexpr const char* sShmTestKeyStr = "AffinityInfoSharedMemoryTestKey";

    //
    // mTestMode = true is used only in UnitTests.
    // The UnitTests are designed to verify the behavior of ShmAffinityInfo, but runnning these tests using
    // the same shared memory as the one used in the production environment poses a significant risk.
    // This is because processes in the production environment may already be using that shared memory,
    // and they cannot be stopped just to run the tests. To avoid this, the UnitTest internally switches
    // to use a different shared memory key than the one used in production.
    // This switching behavior is triggered by the mTestMode flag. Therefore, mTestMode = true must be used
    // only in the UnitTest environment, and in all release environment, it must always remain set to false.
    //
    const bool mTestMode {false};

    std::unique_ptr<ShmAffinityInfo> mAffinityInfo;
    SetupCondition mShmSetupCondition { SetupCondition::UNDEFINED };

    std::unique_ptr<NumaUtil> mNumaUtil;
    std::unique_ptr<CpuSocketUtil> mCpuSocketUtil;
    std::unique_ptr<AffinityResourceControl> mAffinityResourceControl;

    Parser mParser;
};

} // namespace grid_util
} // namespace scene_rdl2
