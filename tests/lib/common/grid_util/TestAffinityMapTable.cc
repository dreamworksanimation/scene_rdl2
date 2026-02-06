// Copyright 2025-2026 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "TestAffinityMapTable.h"
#include "TimeOutput.h"

#include <scene_rdl2/common/grid_util/AffinityMapTable.h>

namespace {

bool
execDebugConsoleCmd(const std::string& funcName,
                    scene_rdl2::grid_util::AffinityMapTable& affMapTbl,
                    const std::string& command)
{
    std::string outMsg;

    auto cmdErrorMsg = [&]() {
        std::ostringstream ostr;
        ostr << "ERROR : " << funcName << " debugConsoleCommand:\"" << command << "\" failed. outMsg={\n"
             << scene_rdl2::str_util::addIndent(outMsg) << '\n'
             << "}";
        return ostr.str();        
    };

    bool result = true;
    if (!affMapTbl.getParser().main(command, outMsg)) {
        result = false;
        std::cerr << cmdErrorMsg() << '\n';
    }

    return result;
}

} // namespace

namespace scene_rdl2 {
namespace grid_util {
namespace unittest {

void
TestAffinityMapTable::testAffMapTblOpen()
{
    TIME_START;

    CPPUNIT_ASSERT_MESSAGE("testAffmapTblOpen initial cleanup", rmOldSemShm());
    CPPUNIT_ASSERT_MESSAGE("openAffmapTbl", openAffMapTbl());
    CPPUNIT_ASSERT_MESSAGE("testAffmapTblOpen post cleanup", rmOldSemShm());

    TIME_END;
}

void
TestAffinityMapTable::testAffMapTblOpenTimeout()
{
    TIME_START;

    CPPUNIT_ASSERT_MESSAGE("testAffmapTblOpen initial cleanup", rmOldSemShm());

    // This test needs around 10 sec due to the involved internal semaphore initialization timeout
    CPPUNIT_ASSERT_MESSAGE("openAffmapTblTimeout", openAffMapTblTimeout());

    CPPUNIT_ASSERT_MESSAGE("testAffmapTblOpen post cleanup", rmOldSemShm());

    TIME_END;
}

//------------------------------------------------------------------------------------------

bool
TestAffinityMapTable::rmOldSemShm() const
//
// Remove testMode ShmAffinityInfo and its semaphore if they exist.
//
{
    AffinityMapTable affMapTbl(true);

    bool result = true;
    if (!execDebugConsoleCmd("rmOldSemShm()", affMapTbl, "removeShmIfAlreadyExist")) result = false;
    if (!execDebugConsoleCmd("rmOldSemShm()", affMapTbl, "removeOrphanedSemaphore")) result = false;

    return result;
}

bool
TestAffinityMapTable::openAffMapTbl() const
{
    AffinityMapTable affMapTbl(true);

    bool result = true;
    if (!execDebugConsoleCmd("openAffMapTbl()", affMapTbl, "testMode on open")) result = false;

    return result;
}

bool
TestAffinityMapTable::openAffMapTblTimeout() const
//
// This test consists of 2 stages.
// 1) Create an emulated environment for a crashed middle of an open operation
// 2) Open in this condition and make sure the retry logic works properly.
//
{
    AffinityMapTable affMapTbl(true);
    bool result = true;

    //
    // If the crash happens in the middle of the open operation, the internal hash code
    // was not properly setup up in the shred memory. In this case, the hash value is
    // all 0x0. The following operation mimics this condition.
    //
    if (!execDebugConsoleCmd("openAffMapTblTimeout()", affMapTbl, "testMode on emulateOpenCrash")) result = false;

    //
    // The initial try of open operation wait for the internal hash value to be updated,
    // but times out and retris the open sequence. This timeout is set up for 10 seconds
    // at this moment. If the retry succeeds, return true.
    //
    if (!execDebugConsoleCmd("openAffMapTblTimeout()", affMapTbl, "testMode on open")) result = false;

    return result;
}

} // namespace unittest
} // namespace grid_util
} // namespace scene_rdl2
