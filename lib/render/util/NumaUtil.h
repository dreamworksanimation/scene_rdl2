// Copyright 2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <string>
#include <vector>

namespace scene_rdl2 {

class NumaNode
//
// This class keeps NUMA-node information and provides node-dependent memory management APIs.
//
{
public:
    NumaNode(const unsigned nodeId,
             const unsigned totalNode,
             const size_t memSize,
             const std::vector<unsigned>& cpuIdList,
             const std::vector<int>& nodeDistance );

    unsigned getNodeId() const { return mNodeId; }
    size_t getMemSize() const { return mMemSize; } // Return NUMA-node memory size

    const std::vector<int>& getNodeDistance() const { return mNodeDistance; }

    //
    // alloc/free which is related to this NUMA-node memory.
    //
    void* alloc(const size_t size) const; // might throw except::RuntimeError(std::string) when error
    void free(void* const memory, const size_t size) const;
    /* Not used so far but might be needed in the near future.
    void* alignedAlloc(const size_t size, const size_t align) const;
    void alignedFree(void* const alignedMemory, const size_t size) const;
    */

    // Does the memory (from memory to memory + size - 1) belong to this NUMA-node?
    // rturn true if all the memory is belong to this NUMA-node.
    //       false if some of the memory page is not belong to this NUMA-node.
    bool isBelongMem(void* const memory, const size_t size) const;

    bool isBelongCpu(const unsigned cpuId) const;

    // All the memory is allocated by mmap and the address is always aligned by pagesize.
    // This function checks that the requested alignment is valid by pagesize alignment.
    // We can safely use the address allocated by NumaNode::alloc() as an aligned address
    // if ths aligned size is passed by this test.
    bool alignmentSizeCheck(const size_t alignment) const;

    std::string show() const; 

private:
    const unsigned mNodeId {~static_cast<unsigned>(0)}; // This NUMA-node id
    const unsigned mTotalNode {0}; // Total number of NUMA-node on this machine
    const size_t mMemSize {0}; // This NUMA-node memory size
    const size_t mPageSize {0}; // Pagesize of this machine
    std::vector<unsigned> mCpuIdList; // sorted
    std::vector<int> mNodeDistance; // Node distance information of this NUMA-node
};

class NumaUtil
//
// This class keeps all the NUMA-related information on the current hosts.
//
{
public:
    NumaUtil(); // Might throw scene_rdl2::except::RuntimeError() when error

    size_t getTotalNumaNode() const { return mNumaNodeTbl.size(); }
    const NumaNode* getNumaNode(const unsigned nodeId) const;

    const NumaNode* findNumaNodeByCpuId(const unsigned cpuId) const;
    std::vector<unsigned> genActiveNumaNodeIdTblByCpuIdTbl(const std::vector<unsigned>& cpuIdTbl) const;

    static unsigned findNumaNodeByMemAddr(void* addr); // for verify: Throw except::RuntimeError if error

    std::string show() const;

private:
    std::vector<NumaNode> mNumaNodeTbl;
};

} // namespace scene_rdl2    
