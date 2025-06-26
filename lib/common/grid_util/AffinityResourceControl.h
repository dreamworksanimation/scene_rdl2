// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "Arg.h"
#include "Parser.h"

#include <memory>

namespace scene_rdl2 {
namespace grid_util {

class CpuSocketUtil;
class NumaUtil;
class ShmAffinityInfo;

class AffinityResourceCore
//
// This class stores single-core information for the affinity resource control.
//
{
public:
    AffinityResourceCore(const unsigned coreId)
        : mCoreId {coreId}
    {}

    void reset() { mUsedFlag = false; mPid = 0; }

    unsigned getCoreId() const { return mCoreId; }

    bool getUsedFlag() const { return mUsedFlag; }
    void setUsedFlag(const bool flag) { mUsedFlag = flag; }

    size_t getPid() const { return mPid; }
    void setPid(const size_t pid) { mPid = pid; }

    std::string show() const;

private:
    unsigned mCoreId {0};

    bool mUsedFlag {false};
    size_t mPid {0}; // process id
}; // class AffinityResourceCore

class AffinityResourceNumaNode
//
// This class stores single-NUMA-node information for the affinity resource control
//
{
public:
    AffinityResourceNumaNode(const unsigned numaNodeId, const NumaUtil& numaUtil);

    template <typename F> bool crawlAllCores(F func) const
    {
        for (const AffinityResourceCore& itr : mCoreTbl) { if (!func(itr)) return false; }
        return true;
    }

    template <typename F> bool crawlAllCores(F func)
    {
        for (AffinityResourceCore& itr : mCoreTbl) { if (!func(itr)) return false; }
        return true;
    }

    template <typename F> bool crawlAllActiveCores(const bool usedFlag, F func) const
    {
        return crawlAllCores([&](const AffinityResourceCore& core) {
            if (core.getUsedFlag() == usedFlag) { if (!func(core)) return false; }
            return true;
        });
    }

    template <typename F> bool crawlAllActiveCores(const bool usedFlag, F func)
    {
        return crawlAllCores([&](AffinityResourceCore& core) {
            if (core.getUsedFlag() == usedFlag) { if (!func(core)) return false; }
            return true;
        });
    }

    unsigned getNumaNodeId() const { return mNumaNodeId; }

    void resetWeight() { mWeight = -1; }
    int getWeight() const { return mWeight; }

    void calcSelectionWeight(const size_t pidOfMyProc, const int otherProcTotalOfThisSocket);
    int singleCoreAllocation(const size_t pidOfMyProc, std::string& errMsg);

    // return total other processes count in this NumaNode
    int calcTotalOtherProcesses(const size_t pidOfMyProc, bool& doesExistMyProc) const;

    bool isBelongCoreId(const unsigned coreId) const;

    std::string show() const;
    std::string showCoreTbl() const;

private:

    int calcAvailableCoreTotal() const;

    unsigned getMaxCoreId() const;
    size_t getMaxPid() const;

    //------------------------------

    unsigned mNumaNodeId {0};

    std::vector<AffinityResourceCore> mCoreTbl;

    int mWeight {0};
}; // class AffinityResourceNumaNode

class AffinityResourceSocket
//
// This class stores information for one socket for the affinity resource control
//
{
public:
    AffinityResourceSocket(const unsigned socketId,
                           const CpuSocketUtil& cpuSocketUtil,
                           const NumaUtil& numaUtil);

    template <typename F> bool crawlAllNumaNodes(F func)
    {
        for (AffinityResourceNumaNode& itr : mNumaNodeTbl) { if (!func(itr)) return false; }
        return true;
    }

    template <typename F> bool crawlAllNumaNodes(F func) const
    {
        for (const AffinityResourceNumaNode& itr : mNumaNodeTbl) { if (!func(itr)) return false; }
        return true;
    }

    template <typename F> bool crawlAllActiveCores(const bool usedFlag, F func) const
    {
        crawlAllNumaNodes([&](const AffinityResourceNumaNode& numaNode) {
            return numaNode.crawlAllActiveCores(usedFlag, [&](const AffinityResourceCore& core) {
                if (!func(core)) return false;
                return true;
            });
        });
        return true;
    }

    unsigned getSocketId() const { return mSocketId; }

    unsigned getNumaNodeTblSize() const { return static_cast<unsigned>(mNumaNodeTbl.size()); }
    AffinityResourceNumaNode& getNumaNode(const unsigned tblId) { return mNumaNodeTbl[tblId]; }

    void resetWeight(const bool onlySocket);
    int getWeight() const { return mWeight; }

    void calcSelectionWeight(const size_t pidOfMyProc,
                             const int otherProcTotalOfThisHost,
                             const bool onlySocket);
    int calcAvailableCoreTotal() const;
    int singleCoreAllocation(const size_t pidOfMyProc, std::string& errMsg);

    // return total other processes count in this Socket
    int calcTotalOtherProcesses(const size_t pidOfMyProc, bool& doesExistMyProc) const;

    bool isBelongCoreId(const unsigned coreId) const;

    std::string show() const;
    std::string showNumaNodeTbl() const;
    std::string showAllNumaNodeWeight() const;

private:

    void resetWeightNumaNode();

    AffinityResourceNumaNode* pickNumaNodeCandidate();

    //------------------------------

    unsigned mSocketId {0};

    std::vector<AffinityResourceNumaNode> mNumaNodeTbl;

    int mWeight {0};
}; // class AffinityResourceSocket

class AffinityResourceControl
//
// The main purpose of this class is to calculate a new CPU ID table which
// is not overlap with the current actively used CPUs.
//
{
public:
    using Arg = scene_rdl2::grid_util::Arg;
    using Parser = scene_rdl2::grid_util::Parser;

    // Might throw exception(std::string) if error
    AffinityResourceControl(const CpuSocketUtil& cpuSocketUtil,
                            const NumaUtil& numaUtil,
                            const ShmAffinityInfo& affinityInfo);

    int calcAvailableCoreTotal() const;
    size_t getMyPid() const { return mMyPid; } 
    void updateMyPidForUnitTest(const size_t pid) { mMyPid = pid; } // for unitTest purposes only

    // Calculate a new coreId table where the CPU does not overlap with the current active CPUs.
    // The newly allocated CPU information is stored in the affinity shared memory table and
    // can be shared with other processes. This API only provides the allocated core info
    // and does not provide any affinity action itself.
    // Throw exception(std::string err) if error
    std::vector<unsigned> coreAllocation(const int numCores, const bool verifyMode=false);

    std::string show() const;
    std::string showSocketTbl() const;
    std::string showCoreTbl() const;
    std::string show2CoreTbl() const;

    Parser& getParser() { return mParser; }

private:

    using MsgFunc = std::function<bool(const std::string& msg)>;

    template <typename F> bool crawlAllSockets(F func)
    {
        for (AffinityResourceSocket& itr : mSocketTbl) { if (!func(itr)) return false; }
        return true;
    }
    template <typename F> bool crawlAllSockets(F func) const
    {
        for (const AffinityResourceSocket& itr : mSocketTbl) { if (!func(itr)) return false; }
        return true;
    }

    template <typename F> bool crawlAllNumaNodes(F func) const
    {
        return crawlAllSockets([&](const AffinityResourceSocket& socket) {
            return socket.crawlAllNumaNodes([&](const AffinityResourceNumaNode& numaNode) {
                if (!func(numaNode)) return false;
                return true;
            });
        });
    }

    template <typename F> bool crawlAllCores(F func) const
    {
        for (const AffinityResourceCore* itr : mCoreTbl) { if (!func(itr)) return false; }
        return true;
    }

    template <typename F> bool crawlAllActiveCores(const bool usedFlag, F func) const
    {
        return crawlAllCores([&](const AffinityResourceCore* core) {
            if (core->getUsedFlag() == usedFlag) { if (!func(core)) return false; }
            return true;
        });
    }

    unsigned getMaxCoreId() const;
    unsigned getMaxNumaNodeId() const;
    unsigned getMaxSocketId() const;
    size_t getMaxPid() const;
    int getMaxSocketWeight() const;
    int getMaxNumaNodeWeight() const;
    const AffinityResourceSocket* getSocketByCoreId(const unsigned coreId) const;
    const AffinityResourceNumaNode* getNumaNodeByCoreId(const unsigned coreId) const;

    int singleCoreAllocation(std::string& errMsg);
    AffinityResourceSocket* pickSocketCandidate();

    void resetPid();
    void resetWeight(const bool onlySocket);
    void calcSelectionWeight(const bool onlySocket);
    int calcTotalOtherProcesses() const;

    bool verifyAllocation(const int targetCoreId, std::string& err) const; 

    struct CoreCondition { // for testCoreAllocation()
        int mCoreId;
        bool mSockMyProc;
        int mSockOtherProcTotal;
        bool mNodeMyProc;
        int mNodeOtherProcTotal;
    };
    CoreCondition computeCoreCondition(const unsigned coreId) const;
    bool shouldPickUpTrialRatherThanTarget(const CoreCondition& selectedCoreCondition,
                                           const CoreCondition& trialCoreCondition) const;

    void parserConfigure();    
    bool testWeight(const MsgFunc& msgFunc);
    bool testCoreAllocation(const int numCores, const bool verify, const MsgFunc& msgFunc);

    //------------------------------

    size_t mMyPid {0};

    const ShmAffinityInfo& mAffinityInfo;
    std::vector<AffinityResourceSocket> mSocketTbl;
    std::vector<AffinityResourceCore*> mCoreTbl;

    Parser mParser;
}; // class AffinityResourceControl

} // namespace grid_util
} // namespace scene_rdl2
