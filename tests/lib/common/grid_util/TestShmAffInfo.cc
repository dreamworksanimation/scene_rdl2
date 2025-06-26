// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "TestShmAffInfo.h"
#include "TestShmUtil.h"
#include "TimeOutput.h"

#include <scene_rdl2/common/grid_util/CpuSocketUtil.h>
#include <scene_rdl2/common/grid_util/ShmAffinityInfo.h>

#include <thread>

namespace scene_rdl2 {
namespace grid_util {
namespace unittest {

void
TestShmAffInfo::testAffInfoDataSize()
{
    TIME_START;

    const ShmAffinityInfo::Hash initHash = Sha1Util::init();

    DataSizeTestConstructionFunc func = [&](void* mem, size_t memSize) {
        ShmAffinityInfo shmAffinityInfo(initHash, mem, memSize, true);
    };

    bool flag = true;
    if (!dataSizeTest(0, false, func) ||
        !dataSizeTest2(ShmAffinityInfo::calcDataSize(), false, true, false, func)) {
        flag = false;
    }
    CPPUNIT_ASSERT("testAffInfoDataSize" && flag);

    TIME_END;
}

void
TestShmAffInfo::testAffInfo()
{
    TIME_START;

    CPPUNIT_ASSERT("testAffInfo" && testAffInfoMain());

    TIME_END;
}

void
TestShmAffInfo::testAffInfoManager()
{
    TIME_START;

    testAffInfoManagerMain(0);
    testAffInfoManagerMain(1);    

    TIME_END;
}

void
TestShmAffInfo::testCoreAllocation()
{
    TIME_START;

    constexpr int randMaxLoopCount = 10;
    int total_ag = testCoreAllocationLoop("ag", randMaxLoopCount);
    int total_tin = testCoreAllocationLoop("tin", randMaxLoopCount);
    int total_cobalt = testCoreAllocationLoop("cobalt", randMaxLoopCount);
    int total = total_ag + total_tin + total_cobalt;

    std::ostringstream ostr;
    ostr << "testCoreAllocation completed summary {\n"
         << "  ag     : total:" << total_ag << '\n'
         << "  tin    : total:" << total_tin << '\n'
         << "  cobalt : total:" << total_cobalt << '\n'
         << "} total:" << total;
    std::cerr << ostr.str() << '\n';

    TIME_END;
}

//------------------------------------------------------------------------------------------

bool
TestShmAffInfo::testAffInfoMain() const
{
    const size_t memSize = ShmAffinityInfo::calcDataSize();
    void* mem = malloc(memSize);

    const ShmAffinityInfo::Hash initHash = Sha1Util::init();

    bool flag = true;
    try {
        ShmAffinityInfo affInfo(initHash, mem, memSize, true);

        constexpr int dataTypeId = 0;
        if (!affInfo.verifySetGet(dataTypeId)) {
            std::cerr << "ERROR : ShmAffinityInfo verifySetGet failed\n";
            flag = false;
        }
    }        
    catch (const std::string& err) {
        std::cerr << "ERROR : ShmAffinityInfo construction failed (testAffInfoMain)\n"
                  << "  hash:" << Sha1Util::show(initHash) << '\n'
                  << "  error=>{\n"
                  << str_util::addIndent(err, 2) << '\n'
                  << "  }\n";
        flag = false;
    }

    free(mem);

    return flag;
}

void
TestShmAffInfo::testAffInfoManagerMain(const int dataTypeId) const
{
    CPPUNIT_ASSERT("testAffinityManager initial cleanup" && rmOldShmAffInfo("testAffInfoManagerMain() before"));

    constexpr bool testMode = true;
    std::string dataTypeIdStr = std::to_string(dataTypeId);
    std::string outMessage;

    //------------------------------
    //
    // Setup data
    //
    std::shared_ptr<ShmAffinityInfoManager> infoManager = std::make_shared<ShmAffinityInfoManager>(false, testMode);

    bool flag = infoManager->getParser().main("storeTestData " + dataTypeIdStr, outMessage);
    if (!flag) {
        std::cerr << "RUNTIME-ERROR: TestShmAffInfo.cc parser.main() failed storeTestData command."
                  << " dataTypeId:" << dataTypeId << " outMessage:{\n"
                  << str_util::addIndent(outMessage) << '\n'
                  << "}\n";
    }
    CPPUNIT_ASSERT("testAffinityManager storeTestData" && flag);

    infoManager.reset();

    //------------------------------
    //
    // Verify data
    //
    infoManager = std::make_shared<ShmAffinityInfoManager>(true, testMode); // recreate new infoManager
    flag = infoManager->getParser().main("verifyTestData " + dataTypeIdStr, outMessage);
    if (!flag) {
        std::cerr << "RUNTIME-ERROR: TestShmAffInfo.cc parser.main() failed. verifyTestData."
                  << " dataTypeId:" << dataTypeId << " outMessage:{\n"
                  << str_util::addIndent(outMessage) << '\n'
                  << "}\n";
    }
    CPPUNIT_ASSERT("testAffinityManager verifyTestData" && flag);

    infoManager.reset(); // release shared memory access.

    //------------------------------

    CPPUNIT_ASSERT("testAffinityManager post cleanup" && rmOldShmAffInfo("testAffInfoManagerMain() after"));
}

int
TestShmAffInfo::testCoreAllocationLoop(const std::string& modeStr,
                                       const int randMaxLoopCount) const
{
    CpuSocketUtil cpuSocketUtil;
    cpuSocketUtil.reset(modeStr);
    int maxCores = static_cast<int>(cpuSocketUtil.getTotalCores());

    int totalTest = 0;
    for (int i = 0; i < randMaxLoopCount; ++i) {
        int randMaxSize = maxCores / (i + 1);
        int stepJ = (randMaxSize + 3 - 1) / 3;
        for (int j = 0; j < randMaxSize; j += stepJ) {
            int myPidUpdateInterval = j + 1;
            totalTest += testCoreAllocationMain(modeStr, randMaxSize, myPidUpdateInterval);
        }
    }
    return totalTest;
}

int
TestShmAffInfo::testCoreAllocationMain(const std::string& modeStr,
                                       const int randMaxSize, const int myPidUpdateInterval) const
{
    auto analyzeLogAndGetTotalTestCount = [](const std::string& log) -> int {
        constexpr const char* key = "verityCoreAllocationTestTotal=";
        size_t keySize = strlen(key); 
        std::istringstream istr(log);
        std::string line;
        while (std::getline(istr, line)) {
            if (line.size() <= keySize) continue;
            if (line.compare(0, keySize, key, 0, keySize) == 0) {
                try {
                    return std::stoi(line.substr(keySize));
                }
                catch (...) {
                    return -1; // error
                }
            }
        }
        return 0;
    };

    CPPUNIT_ASSERT("testCoreAllocation initial cleanup" && rmOldShmAffInfo("testCoreAllocationMain() before"));

    constexpr bool testMode = true;
    std::shared_ptr<ShmAffinityInfoManager> infoManager = std::make_shared<ShmAffinityInfoManager>(false, testMode);

    std::string outMessage;
    std::ostringstream ostr;
    ostr << "verifyCoreAllocation " << modeStr << ' ' << randMaxSize << ' ' << myPidUpdateInterval;
    bool flag = infoManager->getParser().main(ostr.str(), outMessage);
    int totalTest = 0;
    if (!flag) {
        std::cerr << "RUNTIME-ERROR : TestShmAffInfo.cc parser.main() failed verifyCoreAllocation command."
                  << " modeStr:" << modeStr
                  << " randMaxSize:" << randMaxSize
                  << " myPidUpdateInterval:" << myPidUpdateInterval << " {\n"
                  << str_util::addIndent(outMessage) << '\n'
                  << "}\n";
    } else {
        totalTest = analyzeLogAndGetTotalTestCount(outMessage);
        CPPUNIT_ASSERT("testCoreAllocation verify log analyze" && totalTest >= 0);
    }
    CPPUNIT_ASSERT("testCoreAllocation verifyCoreAllocation" && flag);

    CPPUNIT_ASSERT("testCoreAllocation post cleanup" && rmOldShmAffInfo("testCoreAllocationMain() after"));

    return totalTest;
}

bool
TestShmAffInfo::rmOldShmAffInfo(const std::string& headMsg) const
{
    constexpr bool testModeEnabled = true;
    return ShmAffinityInfoManager::rmShmIfAlreadyExistCmd(testModeEnabled,
                                                         [&](const std::string& msg) {
                                                             std::cerr << headMsg << ' ' << msg;
                                                             return true;
                                                         });
}

} // namespace unittest
} // namespace grid_util
} // namespace scene_rdl2
