// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "TestAffinityMapTable.h"
#include "TimeOutput.h"

#include <scene_rdl2/common/grid_util/AffinityMapTable.h>

namespace scene_rdl2 {
namespace grid_util {
namespace unittest {

void
TestAffinityMapTable::testAffMapTblOpen()
{
    TIME_START;

    CPPUNIT_ASSERT("testAffmapTblOpen initial cleanup" && rmOldSemShm());
    CPPUNIT_ASSERT("openAffmapTbl" && openAffMapTbl());
    CPPUNIT_ASSERT("testAffmapTblOpen post cleanup" && rmOldSemShm());

    TIME_END;
}

void
TestAffinityMapTable::testAffMapTblOpenTimeout()
{
    TIME_START;

    CPPUNIT_ASSERT("testAffmapTblOpen initial cleanup" && rmOldSemShm());

    // This test needs around 10 sec due to the involved internal semaphore initialization timeout
    CPPUNIT_ASSERT("openAffmapTblTimeout" && openAffMapTblTimeout());

    CPPUNIT_ASSERT("testAffmapTblOpen post cleanup" && rmOldSemShm());

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
    std::string outMsg;

    bool result = true;
    if (!affMapTbl.getParser().main("rmShmIfAlreadyExist", outMsg)) {
        result = false;
        std::cerr << "ERROR : parser command rmShmIfAlreadyExit failed.\n";
    }
    if (!affMapTbl.getParser().main("rmUnusedSemaphore", outMsg)) {
        result = false;
        std::cerr << "ERROR : parser command rmUnusedSemaphore failed.\n";
    }
    return result;
}

bool
TestAffinityMapTable::openAffMapTbl() const
{
    AffinityMapTable affMapTbl(true);
    std::string outMsg;

    bool result = true;
    if (!affMapTbl.getParser().main("testMode on open", outMsg)) {
        result = false;
        std::cerr << "ERROR : construct AffinityMapTable failed.\n";
    }
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
    std::string outMsg;

    bool result = true;

    //
    // If the crash happens in the middle of the open operation, the internal hash code
    // was not properly setup up in the shred memory. In this case, the hash value is
    // all 0x0. The following operation mimics this condition.
    //
    if (!affMapTbl.getParser().main("testMode on emulateOpenCrash", outMsg)) {
        result = false;
        std::cerr << "ERROR : AffinityMapTable emulateOpenCrash failed.\n";
    }

    //
    // The initial try of open operation wait for the internal hash value to be updated,
    // but times out and retris the open sequence. This timeout is set up for 10 seconds
    // at this moment. If the retry succeeds, return true.
    //
    if (!affMapTbl.getParser().main("testMode on open", outMsg)) {
        result = false;
        std::cerr << "ERROR : retry open AffinityMapTable failed.\n";
    }
    return result;
}

} // namespace unittest
} // namespace grid_util
} // namespace scene_rdl2
