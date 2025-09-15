// Copyright 2024-2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "NumaInfo.h"

#ifdef PLATFORM_APPLE
#include <iostream>
#endif // end of PLATFORM_APPLE 

#include <cstdint>

namespace scene_rdl2 {
namespace grid_util {

bool
NumaInfo::allocFreeTest(const unsigned numaNodeId, const size_t size, const MsgFunc& msgFunc) const
{
#ifdef PLATFORM_APPLE
    return false;
#else // else PLATFORM_APPLE

    if (numaNodeId >= mNumaUtil.getTotalNumaNode()) {
        std::ostringstream ostr;
        ostr << "ERROR : numaNodeId:" << numaNodeId << " is out of range";
        msgFunc(ostr.str() + '\n');
        return false;
    }

    const NumaNode* const numaNode = mNumaUtil.getNumaNode(numaNodeId);

    void* const mem = numaNode->alloc(size);
    if (!mem) {
        std::ostringstream ostr;
        ostr << "ERROR : Could not alloc memory from NumaNodeId:" << numaNodeId << " size:" << size;
        msgFunc(ostr.str() + '\n');
        return false;
    }

    memset(mem, 0, size); // Without access memory, memory itself is not allocated yet.

    bool result = true;
    //------------------------------
    //
    // Verify #1
    //
    std::ostringstream ostr;
    ostr << "Alloced memory:0x" << std::hex << reinterpret_cast<uintptr_t>(mem) << std::dec
         << " size:" << size << " @ NUMA-nodeId:" << numaNode->getNodeId();
    msgFunc(ostr.str() + '\n');

    if (numaNode->isBelongMem(mem, size)) msgFunc("Verify#1 : OK\n");
    else { msgFunc("Verify#1 : Failed\n"); result = false; }

    //
    // Verify #2
    //
    if (mNumaUtil.findNumaNodeByMemAddr(mem) == numaNode->getNodeId()) msgFunc("Verify#2 : OK\n");
    else { msgFunc("Verify#2 : Failed\n"); result = false; }

    //------------------------------

    numaNode->free(mem, size);
    return result;
#endif // end of Non PLATFORM_APPLE
}

void
NumaInfo::parserConfigure()
{
#ifndef PLATFORM_APPLE
    mParser.description("NumaInfo command");

    mParser.opt("-showAll", "", "dump all NUMA-node info",
                [&](Arg& arg) { return arg.msg(mNumaUtil.show() + '\n'); });
    mParser.opt("-show", "<NUMA-nodeId>", "dump a particular NUMA-node info only",
                [&](Arg& arg) {
                    const unsigned numaNodeId = (arg++).as<unsigned>(0);
                    const NumaNode* numaNode = mNumaUtil.getNumaNode(numaNodeId);
                    if (!numaNode) {
                        return arg.msg("Cannot get NumaNode. nodeId:" + std::to_string(numaNodeId) + '\n');
                    }
                    return arg.msg(numaNode->show() + '\n');
                });
    mParser.opt("-allocFreeTest", "<NUMA-nodeId> <size>", "do memory alloc/free test",
                [&](Arg& arg) {
                    const unsigned numaNodeId = (arg++).as<unsigned>(0);
                    const size_t size = (arg++).as<size_t>(0);
                    return allocFreeTest(numaNodeId,
                                         size,
                                         [&](const std::string& msg) { return arg.msg(msg); });
                });
#endif // end of Non PLATFORM_APPLE
}

} // namespace grid_util
} // namespace scene_rdl2
