// Copyright 2024-2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "TestProcCpuAffinity.h"
#include "TimeOutput.h"

#include <scene_rdl2/common/except/exceptions.h>

#include <thread>

namespace scene_rdl2 {
namespace affinity {
namespace unittest {

void
TestProcCpuAffinity::testPartialAffinity()
{
    TIME_START;

    testMain([&](const unsigned numCpu, ProcCpuAffinity& proc) {
            for (unsigned cpuId = 0; cpuId < numCpu; cpuId += 2) { proc.set(cpuId); }
        });

    TIME_END;
}

void
TestProcCpuAffinity::testFullAffinity()
{
    TIME_START;

    testMain([&](const unsigned numCpu, ProcCpuAffinity& proc) {
            proc.setFull();
        });

    TIME_END;
}

void
TestProcCpuAffinity::testMain(const SetCpuIdFunc& setCpuIdFunc)
{
    const unsigned numCpu = std::thread::hardware_concurrency();

    CpuAffinityMask setMask;
    try {
        // setup test cpuId test pattern
        ProcCpuAffinity proc;
        setCpuIdFunc(numCpu, proc);
        setMask = proc.copyMask(); // keep setup mask for verifying

        std::string msg;
        if (!proc.bindAffinity(msg)) {
            std::cerr << "ProcCpuAffinity::bindAffinity() failed. error:" << msg << '\n';
            CPPUNIT_ASSERT(0);
        }
        std::cerr << msg << '\n';
    }
    catch (except::RuntimeError& e) {
        std::cerr << "testPartialAffinity() bindAffinity failed. error:" << e.what() << '\n';
        CPPUNIT_ASSERT(0);
    }
        
    CpuAffinityMask currMask;
    try {
        ProcCpuAffinity proc;

        std::string errorMsg;
        if (!proc.getAffinity(errorMsg)) {
            std::cerr << "ProcCpuAffinity::getAffinity() failed. error:" << errorMsg << '\n';
            CPPUNIT_ASSERT(0);
        }
        currMask = proc.getMask();
    }
    catch (except::RuntimeError& e) {
        std::cerr << "testPartialAffinity() getAffinity failed. error:" << e.what() << '\n';
        CPPUNIT_ASSERT(0);
    }

    CPPUNIT_ASSERT(setMask.isSame(currMask));
}

} // namespace unittest
} // namespace affinity
} // namespace scene_rdl2
