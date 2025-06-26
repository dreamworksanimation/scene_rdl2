// Copyright 2024-2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "TestCpuSocketUtil.h"
#include "TimeOutput.h"

#include <scene_rdl2/common/grid_util/CpuSocketUtil.h>
#include <scene_rdl2/render/util/StrUtil.h>

#include <cassert>
#include <thread>

//#define DEBUG_MSG_SETUP_CPU_INFO

namespace {

bool
testCpuIdDefMain(const std::string& defStr,
                 const bool targetResultFlag,
                 const std::vector<unsigned>& targetOut,
                 const std::string& targetErrMsg)
{
    std::vector<unsigned> out;
    std::string errMsg;
    const bool resultFlag = scene_rdl2::grid_util::CpuSocketUtil::cpuIdDefToCpuIdTbl(defStr, out, errMsg);

    auto showTbl = [](const std::vector<unsigned>& tbl) {
        std::ostringstream ostr;
        ostr << '(';
        for (size_t i = 0; i < tbl.size(); ++i) { ostr << ((i != 0) ? "," : "") << tbl[i]; }
        ostr << ')';
        return ostr.str();
    };

    std::ostringstream ostr;
    bool returnFlag = true;
    if (resultFlag) {
        if (resultFlag != targetResultFlag || out != targetOut) {
            ostr << "verify-ERROR {\n"
                 << "  defStr:" << defStr << '\n'
                 << "  targetOut:" << showTbl(targetOut) << '\n'
                 << "        out:" << showTbl(out) << '\n'
                 << "  targetResultFlag:" << scene_rdl2::str_util::boolStr(targetResultFlag) << '\n'
                 << "        resultFlag:" << scene_rdl2::str_util::boolStr(resultFlag) << '\n'
                 << "}";
            returnFlag = false;
        }
    } else {
        if (resultFlag != targetResultFlag || errMsg != targetErrMsg) {
            ostr << "verify-ERROR {\n"
                 << "  defStr:" << defStr << '\n'
                 << "  targetResultFlag:" << scene_rdl2::str_util::boolStr(targetResultFlag) << '\n'
                 << "        resultFlag:" << scene_rdl2::str_util::boolStr(resultFlag) << '\n'
                 << "- - - targetErrMsg - - -\n"
                 << targetErrMsg << '\n'
                 << "- - - errMsg - - -\n"
                 << errMsg << '\n'
                 << "}";
            returnFlag = false;
        }
    }

    if (returnFlag) {
        if (resultFlag) {
            ostr << "verify-OK def:" << defStr << " out:" << showTbl(out);
        } else {
            ostr << "verify-OK def:" << defStr << " result:false errorMsg:" << errMsg;
        }
    }
    std::cerr << ostr.str() << '\n';

    return returnFlag;
}

int
runCommand(const std::string& command)
{
    FILE* fp;
    if ((fp = popen(command.c_str(), "r")) == NULL) {
        return -1;              // error
    }
    std::string str;
    int c;
    while ((c = fgetc(fp)) != EOF) {
        if (isspace(c)) break;
        str.push_back(c);
    }

    pclose(fp);
    return std::stoi(str);
}

bool
testShowCpuIdTblMain(const scene_rdl2::grid_util::CpuSocketUtil::CpuIdTbl& cpuIdTbl,
                     const std::string& targetMsg)
{
    auto showCpuIdTbl = [&] {
        std::ostringstream ostr;
        ostr << "cpuIdTbl {";
        for (size_t i = 0; i < cpuIdTbl.size(); ++i) { ostr << ((i != 0) ? "," : "") << cpuIdTbl[i]; }
        ostr << "}";
        return ostr.str();
    };

    const std::string msg = scene_rdl2::grid_util::CpuSocketUtil::showCpuIdTbl("CpuIdTbl", cpuIdTbl);
    bool result = true;
    if (msg != targetMsg) {
        std::cerr << "Verify ERROR : " << showCpuIdTbl() << " => \"" << msg << "\" target:\"" << targetMsg << "\"\n";
        result = false;
    } else {
        std::cerr << "Verify OK : " << showCpuIdTbl() << " => \"" << msg << "\"\n";
    }

    return result;
}

} // namespace

namespace scene_rdl2 {
namespace grid_util {
namespace unittest {

void
TestCpuSocketUtil::testCpuIdDef()
{
    TIME_START;

    // We have to run this unitTest 8 cores or more environment.
    assert(std::thread::hardware_concurrency() >= 8);

    //
    // result=true test
    //
    CPPUNIT_ASSERT(testCpuIdDefMain("0,1,2,3,4",
                                    true,
                                    std::vector<unsigned> {0,1,2,3,4},
                                    ""));

    CPPUNIT_ASSERT(testCpuIdDefMain("0-2,4,6-7",
                                    true,
                                    std::vector<unsigned> {0,1,2,4,6,7},
                                    ""));

    CPPUNIT_ASSERT(testCpuIdDefMain("6-7,0-2,4",
                                    true,
                                    std::vector<unsigned> {0,1,2,4,6,7},
                                    ""));

    //
    // result=false test
    //
    CPPUNIT_ASSERT(testCpuIdDefMain("x",
                                    false,
                                    std::vector<unsigned> {},
                                    { "Wrong Format : {\n"
                                      "  x\n"
                                      "  ^\n"
                                      "}" }));

    CPPUNIT_ASSERT(testCpuIdDefMain("0-2,a,9-11",
                                    false,
                                    std::vector<unsigned> {},
                                    { "Wrong Format : {\n"
                                      "  0-2,a,9-11\n"
                                      "      ^\n"
                                      "}" }));

    CPPUNIT_ASSERT(testCpuIdDefMain("0-2,5-b,9-11",
                                    false,
                                    std::vector<unsigned> {},
                                    { "Wrong Format : {\n"
                                      "  0-2,5-b,9-11\n"
                                      "      ^^^\n"
                                      "}" }));

    CPPUNIT_ASSERT(testCpuIdDefMain("0-2,a-5,9-11",
                                    false,
                                    std::vector<unsigned> {},
                                    { "Wrong Format : {\n"
                                      "  0-2,a-5,9-11\n"
                                      "      ^^^\n"
                                      "}" }));
    
    CPPUNIT_ASSERT(testCpuIdDefMain("0-2,-5,9-11",
                                    false,
                                    std::vector<unsigned> {},
                                    { "Wrong Format : {\n"
                                      "  0-2,-5,9-11\n"
                                      "      ^^\n"
                                      "}" }));

    CPPUNIT_ASSERT(testCpuIdDefMain("0-2,4-,9-11",
                                    false,
                                    std::vector<unsigned> {},
                                    { "Wrong Format : {\n"
                                      "  0-2,4-,9-11\n"
                                      "      ^^\n"
                                      "}" }));

    CPPUNIT_ASSERT(testCpuIdDefMain("0-2,-,9-11",
                                    false,
                                    std::vector<unsigned> {},
                                    { "Wrong Format : {\n"
                                      "  0-2,-,9-11\n"
                                      "      ^\n"
                                      "}" }));

    CPPUNIT_ASSERT(testCpuIdDefMain("0-2,11-9,5",
                                    false,
                                    std::vector<unsigned> {},
                                    { "Wrong Format : {\n"
                                      "  0-2,11-9,5\n"
                                      "      ^^^^\n"
                                      "}" }));

    TIME_END;
}

void
TestCpuSocketUtil::testShowCpuIdTbl()
{
    TIME_START;

    CPPUNIT_ASSERT(testShowCpuIdTblMain(CpuSocketUtil::CpuIdTbl {0,2,4,6}, "CpuIdTbl (total:4) {0,2,4,6}"));
    CPPUNIT_ASSERT(testShowCpuIdTblMain(CpuSocketUtil::CpuIdTbl {0,1,2,3,4,5}, "CpuIdTbl (total:6) {0-5}"));
    CPPUNIT_ASSERT(testShowCpuIdTblMain(CpuSocketUtil::CpuIdTbl {0,1,3,4,6}, "CpuIdTbl (total:5) {0-1,3-4,6}"));
    CPPUNIT_ASSERT(testShowCpuIdTblMain(CpuSocketUtil::CpuIdTbl {0,1,2,4,5}, "CpuIdTbl (total:5) {0-2,4-5}"));

    TIME_END;
}

void
TestCpuSocketUtil::testSetupCpuInfo()
{
    TIME_START;

#ifdef PLATFORM_APPLE
    const int totalSockets = 1; // All the Apple Silicon Macs only have a single socket 
    const int totalCores = std::thread::hardware_concurrency();
#else // !PLATFORM_APPLE
    const int totalSockets = runCommand("grep physical.id /proc/cpuinfo | sort -u | wc -l");
    const int totalCores = runCommand("grep processor /proc/cpuinfo | wc -l");
#endif // end of !PLATFORM_APPLE

    std::vector<int> totalCoresOnEachSocket(totalSockets);
#ifdef PLATFORM_APPLE
    totalCoresOnEachSocket[0] = totalCores;
#else // !PLATFORM_APPLE
    for (int socketId = 0; socketId < totalSockets; ++socketId) {
        std::ostringstream ostr;
        ostr << "grep physical.id /proc/cpuinfo | grep \": " << socketId << "\" | wc -l";
        totalCoresOnEachSocket[socketId] = runCommand(ostr.str());
    }
#endif // end of !PLATFORM_APPLE

#   ifdef DEBUG_MSG_SETUP_CPU_INFO
    std::ostringstream ostr;
    ostr << "totalSockets:" << totalSockets << '\n'
         << "totalCores:" << totalCores << '\n'
         << "totalCoresOnEachSocket (size:" << totalCoresOnEachSocket.size() << ") {\n";
    for (size_t i = 0; i < totalCoresOnEachSocket.size(); ++i) {
        ostr << "  socketId:" << i << " totalCores:" << totalCoresOnEachSocket[i] << '\n';
    }
    ostr << "}";
    std::cerr << ostr.str() << '\n';
#   endif // end DEBUG_MSG_SETUP_CPU_INFO

    CpuSocketUtil cpuSocketUtil;

    bool resultFlag = true;
    if (totalSockets != cpuSocketUtil.getTotalSockets()) {
        std::ostringstream ostr;
        ostr << "ERROR testSetupCpuInfo failed."
             << " totalSockets:" << totalSockets << " !="
             << " cpuSocketUtil.getTotalSockets():" << cpuSocketUtil.getTotalSockets();
        std::cerr << ostr.str();
        resultFlag = false;
    }
    if (totalCores != cpuSocketUtil.getTotalCores()) {
        std::ostringstream ostr;
        ostr << "ERROR testSetupCpuInfo failed."
             << " totalCores:" << totalCores << " !="
             << " cpuSocketUtil.getTotalCores():" << cpuSocketUtil.getTotalCores();
        std::cerr << ostr.str();
        resultFlag = false;
    }

    for (int socketId = 0; socketId < totalSockets; ++socketId) {
        const int valA = totalCoresOnEachSocket[socketId];
        const int valB = cpuSocketUtil.getTotalCoresOnSocket(socketId);
        if (valA != valB) {
            std::ostringstream ostr;
            ostr << "ERROR coresTotal on each socket failed."
                 << " totalCoresOnEachSocket[socketId:" << socketId << "]:" << valA << " !="
                 << " cpuSocketUtil.getTotalCoresOnSocket(socketId:" << socketId << "):" << valB;
            std::cerr << ostr.str();
            resultFlag = false;
        }
    }

    if (resultFlag) {
        std::cerr << "testSetuCpuInfo() OK\n";
    }

    CPPUNIT_ASSERT("testSetupCpuInfo" && resultFlag);

    TIME_END;
}

void
TestCpuSocketUtil::testIdTblToDefStr()
{
    TIME_START;

    CPPUNIT_ASSERT("testIdTblToDefStr A" && testIdTblToDefStrMain("0,1,2,3,4,5"));
    CPPUNIT_ASSERT("testIdTblToDefStr B" && testIdTblToDefStrMain("0-9"));
    CPPUNIT_ASSERT("testIdTblToDefStr C" && testIdTblToDefStrMain("0-2,4,6-7"));
    CPPUNIT_ASSERT("testIdTblToDefStr D" && testIdTblToDefStrMain("0,1-3,5,7-9"));
    CPPUNIT_ASSERT("testIdTblToDefStr E" && testIdTblToDefStrMain("6-7,0-2,9"));

    TIME_END;
}

bool
TestCpuSocketUtil::testIdTblToDefStrMain(const std::string& idTblDefStr) const
{
    std::string errMsg;
    CpuSocketUtil::IdTbl workIdTbl;
    if (!CpuSocketUtil::parseIdDef(idTblDefStr, workIdTbl, errMsg)) {
        std::ostringstream ostr;
        ostr << "TestCpuSocketUtil::testIdTblToDefStrMain() parseIdDef() failed."
             << " idTblDefStr:" << idTblDefStr << " err=>{\n"
             << str_util::addIndent(errMsg) << '\n'
             << "}";
        std::cerr << ostr.str() << '\n';
        return false;
    }

    std::string workIdDefStr = CpuSocketUtil::idTblToDefStr(workIdTbl);

    CpuSocketUtil::IdTbl workIdTbl2;
    if (!CpuSocketUtil::parseIdDef(workIdDefStr, workIdTbl2, errMsg)) {
        std::ostringstream ostr;
        ostr << "TestCpuSocketUtil::testIdTblToDefStrMain() parseIdDef() failed-B."
             << " workIdDefStr:" << workIdDefStr << " err=>{\n"
             << str_util::addIndent(errMsg) << '\n'
             << "}";
        std::cerr << ostr.str() << '\n';
        return false;
    }
                                     
    auto showIdTbl = [](const CpuSocketUtil::IdTbl & tbl) {
        std::ostringstream ostr;
        ostr << "{ ";
        for (const auto& v : tbl) ostr << v << ' ';
        ostr << "}";
        return ostr.str();
    };
    auto showProgress = [&]() {
        std::ostringstream ostr;
        ostr << "TestCpuSocketUtil::testIdTblToDefStrMain(idTblDefStr:" << idTblDefStr << ") {\n"
             << "  => " << showIdTbl(workIdTbl) << '\n'
             << "  => " << workIdDefStr << '\n'
             << "  => " << showIdTbl(workIdTbl2) << '\n'
             << "}";
        return ostr.str();
    };

    // std::cerr << showProgress() << '\n'; // for debug
    if (workIdTbl != workIdTbl2) {
        std::cerr << "ERROR: " << showProgress() << '\n';
        return false;
    }
    return true;
}

} // namespace unittest
} // namespace grid_util
} // namespace scene_rdl2
