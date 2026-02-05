// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "AffinityResourceControl.h"
#include "CpuSocketUtil.h"
#include "NumaUtil.h"
#include "ShmAffinityInfo.h"

#include <algorithm> // sort
#include <iostream> // debug
#include <unistd.h> // getpid()
#include <unordered_set>

namespace {

std::vector<unsigned>
calcNumaNodeIdTbl(const unsigned socketId,
                  const scene_rdl2::grid_util::CpuSocketUtil& cpuSocketUtil,
                  const scene_rdl2::grid_util::NumaUtil& numaUtil)
//
// Return sorted numaNodeId table which belonged to a particular socket that specified as socketId
//
{
    std::vector<unsigned> numaNodeTbl;

    const scene_rdl2::grid_util::CpuSocketInfo* const currCpuSocketInfo = cpuSocketUtil.getCpuSocketInfo(socketId);
    if (!currCpuSocketInfo) return numaNodeTbl; // return empty table

    std::unordered_set<unsigned> numaNodeTblSet; // Using unordered_set in order to reduce search cost to O(1)

    for (size_t i = 0; i < currCpuSocketInfo->getTotalCores(); ++i) {
        const int coreId = currCpuSocketInfo->getCpuIdTbl()[i];
        const scene_rdl2::grid_util::NumaNode* const currNumaNode = numaUtil.findNumaNodeByCpuId(coreId);
        if (!currNumaNode) continue; // skip
        const unsigned currNumaNodeId = currNumaNode->getNodeId();
        if (numaNodeTblSet.find(currNumaNodeId) == numaNodeTblSet.end()) {
            numaNodeTbl.push_back(currNumaNodeId);
            numaNodeTblSet.insert(currNumaNodeId);
        }
    }
    
    std::sort(numaNodeTbl.begin(), numaNodeTbl.end());
    return numaNodeTbl;
}

} // namespace

//------------------------------------------------------------------------------------------

namespace scene_rdl2 {
namespace grid_util {

std::string
AffinityResourceCore::show() const
{
    std::ostringstream ostr;
    ostr << "AffinityResourceCore {\n"
         << "  mCoreId:" << mCoreId << '\n'
         << "  mUsedFlag:" << str_util::boolStr(mUsedFlag) << '\n'
         << "  mPid:" << mPid << '\n'
         << "}";
    return ostr.str();
}

//------------------------------------------------------------------------------------------

AffinityResourceNumaNode::AffinityResourceNumaNode(const unsigned numaNodeId,
                                                   const NumaUtil& numaUtil)
    : mNumaNodeId {numaNodeId}
{
    const NumaNode* const currNumaNode = numaUtil.getNumaNode(mNumaNodeId);
    const std::vector<unsigned>& coreIdTbl = currNumaNode->getCpuIdList();
    const size_t coreIdTotal = coreIdTbl.size();
    mCoreTbl.reserve(coreIdTotal);
    for (size_t tblId = 0; tblId < coreIdTotal; ++tblId) {
        mCoreTbl.emplace_back(coreIdTbl[tblId]);
    }
}

void
AffinityResourceNumaNode::calcSelectionWeight(const size_t pidOfMyProc,
                                              const int otherProcTotalOfThisSocket)
{
    if (calcAvailableCoreTotal() == 0) {
        mWeight = -1;
        return;
    }

    //
    // Select the NUMA-node on which as few other processes as possible are running.
    // However, if a process with the same PID as myself already exists, give priority
    // to that NUMA-node
    //
    bool doesExistMyProc;
    const int currOtherProcTotal = calcTotalOtherProcesses(pidOfMyProc, doesExistMyProc);
    mWeight = otherProcTotalOfThisSocket - currOtherProcTotal; // Weight range : 0 ~ otherProcTotalOfThisSocket
    if (doesExistMyProc) {
        // We want to pick numaNode that has my process. Boost weight here.
        // Weight range : otherProcTotalOfThisSocket + 1 ~ 2*otherProcTotalOfThisSocket + 1
        mWeight += otherProcTotalOfThisSocket + 1;
    }

    /* for debug
    std::cerr << ">> AffinityResourceControl.cc NUMA-NODE calcSelectionWeight() {\n"
              << "  currOtherProcTotal:" << currOtherProcTotal << '\n'
              << "  otherProcTotalOfThisSocket:" << otherProcTotalOfThisSocket << '\n'
              << "  mWeight:" << mWeight << '\n'
              << "}\n";
    */
}

int
AffinityResourceNumaNode::singleCoreAllocation(const size_t pidOfMyProc, std::string& errMsg)
{
    //
    // All the empty core has an equal chance to be selected
    //
    int coreId = -1;
    crawlAllActiveCores(false, [&](AffinityResourceCore& core) {
        coreId = core.getCoreId();

        core.setUsedFlag(true);
        core.setPid(pidOfMyProc);
        return false; // Return value false makes early exit of crawlAllActiveCores
    });

    if (coreId < 0) {
        std::ostringstream ostr;
        ostr << "New core allocation failed. Cannot find available core inside numaNode. {\n"
             << str_util::addIndent(show()) << '\n'
             << "}";
        errMsg = ostr.str();
    }
    return coreId;
}

int
AffinityResourceNumaNode::calcTotalOtherProcesses(const size_t pidOfMyProc,
                                                  bool& doesExistMyProc) const
{
    doesExistMyProc = false;
    int total = 0;
    std::unordered_set<size_t> pidTblSet; // Using unordered_set in order to reduce search cost to O(1)
    crawlAllActiveCores(true, [&](const AffinityResourceCore& core) {
        const size_t currPid = core.getPid();
        if (currPid == pidOfMyProc) {
            doesExistMyProc = true;
            return true; // skip
        }
        if (pidTblSet.find(currPid) == pidTblSet.end()) {
            // found new Pid
            pidTblSet.insert(currPid);
            total++;
        }
        return true;
    });
    return total;
}

bool
AffinityResourceNumaNode::isBelongCoreId(const unsigned coreId) const
{
    // We should flip the result of crawlAllCores()
    return !crawlAllCores([&](const AffinityResourceCore& core) {
        // Return value false makes early exit of crawlAllCores() internal loop
        // We want to do an early exit if found core.
        return core.getCoreId() != coreId;
    });
}

std::string
AffinityResourceNumaNode::show() const
{
    std::ostringstream ostr;
    ostr << "AffinityResourceNumaNode {\n"
         << "  mNumaNodeId:" << mNumaNodeId << '\n'
         << str_util::addIndent(showCoreTbl()) << '\n'
         << "  mWeight:" << mWeight << '\n'
         << "}";
    return ostr.str();
}

std::string
AffinityResourceNumaNode::showCoreTbl() const
{
    if (mCoreTbl.empty()) return "mCoreTbl is empty";

    const size_t tblSize = mCoreTbl.size();
    const int wTblId = str_util::getNumberOfDigits(tblSize);
    const int wCoreId = str_util::getNumberOfDigits(getMaxCoreId());
    const int wPid = str_util::getNumberOfDigits(getMaxPid());

    auto showCore = [&](const size_t tblId) {
        const AffinityResourceCore& core = mCoreTbl[tblId];
        // tblId:coreid/pid
        std::ostringstream ostr;
        ostr << std::setw(wTblId) << tblId << ':'
             << std::setw(wCoreId) << core.getCoreId() << '/';
        if (core.getUsedFlag()) ostr << std::setw(wPid) << core.getPid();
        else                    ostr << std::setfill('.') << std::setw(wPid) << '.';
        return ostr.str();
    };

    constexpr size_t totalItemsOneLine = 4;

    std::ostringstream ostr;
    ostr << "mCoreTbl (size:" << tblSize << ") tblId:coreId/pid {";
    for (size_t tblId = 0; tblId < tblSize; ++tblId) {
        if (tblId % totalItemsOneLine == 0) ostr << "\n  ";  
        ostr << showCore(tblId) << ' ';
    }
    ostr << "\n}";
    return ostr.str();
}

int
AffinityResourceNumaNode::calcAvailableCoreTotal() const
{
    int total = 0;
    crawlAllActiveCores(false, [&](const AffinityResourceCore& core) {
        total++; 
        return true;
    });
    return total;
}

unsigned
AffinityResourceNumaNode::getMaxCoreId() const
{
    unsigned maxCoreId = 0;
    crawlAllCores([&](const AffinityResourceCore& core) {
        maxCoreId = std::max(maxCoreId, core.getCoreId());
        return true;
    });
    return maxCoreId;
}

size_t
AffinityResourceNumaNode::getMaxPid() const
{
    size_t maxPid = 0;
    crawlAllActiveCores(true, [&](const AffinityResourceCore& core) {
        maxPid = std::max(maxPid, core.getPid());
        return true;
    });
    return maxPid;
}

//------------------------------------------------------------------------------------------

AffinityResourceSocket::AffinityResourceSocket(const unsigned socketId,
                                               const CpuSocketUtil& cpuSocketUtil,
                                               const NumaUtil& numaUtil)
    : mSocketId {socketId}
{
    const std::vector<unsigned> numaNodeIdTbl = calcNumaNodeIdTbl(mSocketId, cpuSocketUtil, numaUtil);
    const size_t numaNodeTotal = numaNodeIdTbl.size();
    mNumaNodeTbl.reserve(numaNodeTotal);
    for (size_t tblId = 0; tblId < numaNodeTotal; ++tblId) {
        mNumaNodeTbl.emplace_back(numaNodeIdTbl[tblId], numaUtil);
    }
}

void
AffinityResourceSocket::resetWeight(const bool onlySocket)
{
    mWeight = -1;
    if (!onlySocket) {
        resetWeightNumaNode();
    }
}

void
AffinityResourceSocket::calcSelectionWeight(const size_t pidOfMyProc,
                                            const int otherProcTotalOfThisHost,
                                            const bool onlySocket)
{
    if (calcAvailableCoreTotal() == 0) {
        mWeight = -1;
        return;
    }

    //
    // Select the Socket on which as few other processes as possible are running.
    // However, if a process with the same PID as myself already exists, give priority
    // to that Socket
    //
    bool doesExistMyProc;
    const int currOtherProcTotal = calcTotalOtherProcesses(pidOfMyProc, doesExistMyProc);
    mWeight = otherProcTotalOfThisHost - currOtherProcTotal; // Weight range : 0 ~ otherProcTotalOfThisHost 
    if (doesExistMyProc) {
        // We want to pick socket that has my process. Boost weight here.
        // Weight range : otherProcTotalOfThisHost + 1 ~ 2*otherProcTotalOfThisHost + 1
        mWeight += otherProcTotalOfThisHost + 1; 
    }

    /* for debug
    std::cerr << ">> AffinityResourceControl.cc SOCKET calcSelectionWeight() {\n"
              << "  currOtherProcTotal:" << currOtherProcTotal << '\n'
              << "  otherProcTotalOfThisHost:" << otherProcTotalOfThisHost << '\n'
              << "  mWeight:" << mWeight << '\n'
              << "}\n";
    */

    if (onlySocket) return;

    crawlAllNumaNodes([&](AffinityResourceNumaNode& numaNode) {
        numaNode.calcSelectionWeight(pidOfMyProc, currOtherProcTotal);
        return true;
    });
}

int
AffinityResourceSocket::calcAvailableCoreTotal() const
{
    int total = 0;
    crawlAllActiveCores(false, [&](const AffinityResourceCore& core) {
        total++;
        return true;
    });
    return total;
}

int
AffinityResourceSocket::singleCoreAllocation(const size_t pidOfMyProc,
                                             std::string& errMsg)
{
    //
    // We want to pick a single core from this socket.
    // To pick the core, first of all, recompute the weight for all of the nodes of this socket.
    //
    resetWeightNumaNode();

    bool doesExistMyProc;
    const int currOtherProcTotal = calcTotalOtherProcesses(pidOfMyProc, doesExistMyProc);
    crawlAllNumaNodes([&](AffinityResourceNumaNode& numaNode) {
        numaNode.calcSelectionWeight(pidOfMyProc, currOtherProcTotal);
        return true;
    });

    //
    // pick up the node and get a core
    //
    AffinityResourceNumaNode* const numaNode = pickNumaNodeCandidate();
    if (!numaNode) {
        std::ostringstream ostr;
        ostr << "New core allocation failed. No more core resources at NumaNode level. {\n"
             << str_util::addIndent(show()) << '\n'
             << "}";
        errMsg = ostr.str();
        return -1;
    }
    return numaNode->singleCoreAllocation(pidOfMyProc, errMsg);
}

int
AffinityResourceSocket::calcTotalOtherProcesses(const size_t pidOfMyProc,
                                                bool& doesExistMyProc) const
{
    doesExistMyProc = false;
    int total = 0;
    std::unordered_set<size_t> pidTblSet; // Using unordered_set in order to reduce search cost to O(1)
    crawlAllActiveCores(true, [&](const AffinityResourceCore& core) {
        const size_t currPid = core.getPid();
        if (currPid == pidOfMyProc) {
            doesExistMyProc = true;
            return true; // skip
        }
        if (pidTblSet.find(currPid) == pidTblSet.end()) {
            // found new Pid
            pidTblSet.insert(currPid);
            total++;
        }
        return true;
    });
    return total;
}

bool
AffinityResourceSocket::isBelongCoreId(const unsigned coreId) const
{
    // We should flip the result of crawlAllNumaNodes()
    return !crawlAllNumaNodes([&](const AffinityResourceNumaNode& node) {
        // Return value false makes early exit of crawlAllNumaNodes() internal loop
        // We want to do an early exit if found numaNode
        return !node.isBelongCoreId(coreId);
    });
}

std::string
AffinityResourceSocket::show() const
{
    std::ostringstream ostr;
    ostr << "AffinityResouceSocket {\n"
         << "  mSocketId:" << mSocketId << '\n'
         << str_util::addIndent(showNumaNodeTbl()) << '\n'
         << "  mWeight:" << mWeight << '\n'
         << "}";
    return ostr.str();
}

std::string
AffinityResourceSocket::showNumaNodeTbl() const
{
    if (mNumaNodeTbl.empty()) return "mNumaNodeTbl is empty";

    const size_t tblSize = mNumaNodeTbl.size();
    std::ostringstream ostr;
    ostr << "mNumaNodeTbl (size:" << tblSize << ") {\n";
    for (size_t tblId = 0; tblId < tblSize; ++tblId) {
        std::ostringstream ostr2;
        ostr2 << "tblId:" << tblId << ' ';
        ostr << str_util::addIndent(ostr2.str() + mNumaNodeTbl[tblId].show()) << '\n';
    }
    ostr << "}";
    return ostr.str();
}

std::string
AffinityResourceSocket::showAllNumaNodeWeight() const
{
    if (mNumaNodeTbl.empty()) return "mNumaNodeTbl is empty";

    const size_t tblSize = mNumaNodeTbl.size();
    std::ostringstream ostr;
    ostr << "mNumaNodeTbl (size:" << tblSize << ") {\n";
    for (size_t tblId = 0; tblId < tblSize; ++tblId) {
        const AffinityResourceNumaNode& numaNode = mNumaNodeTbl[tblId];
        ostr << "  tblId:" << tblId
             << " numaNodeId:" << numaNode.getNumaNodeId()
             << " weight:" << numaNode.getWeight() << '\n';
    }
    ostr << "}";
    return ostr.str();
}

void
AffinityResourceSocket::resetWeightNumaNode()
{
    crawlAllNumaNodes([&](AffinityResourceNumaNode& numaNode) {
        numaNode.resetWeight();
        return true;
    });
}

AffinityResourceNumaNode*
AffinityResourceSocket::pickNumaNodeCandidate()
//
// This function pick one of the numaNode based on the numaNode selection weight
//
{
    AffinityResourceNumaNode* candidate = nullptr;
    crawlAllNumaNodes([&](AffinityResourceNumaNode& numaNode) {
        if (numaNode.getWeight() < 0) return true; // skip
        if (!candidate) {
            candidate = &numaNode;
        } else {
            if (numaNode.getWeight() > candidate->getWeight()) {
                candidate = &numaNode;
            }
        }
        return true;
    });
    return candidate;
}

//------------------------------------------------------------------------------------------

AffinityResourceControl::AffinityResourceControl(const CpuSocketUtil& cpuSocketUtil,
                                                 const NumaUtil& numaUtil,
                                                 const ShmAffinityInfo& affinityInfo)
    : mAffinityInfo {affinityInfo}
{
    const size_t socketTotal = cpuSocketUtil.getTotalSockets();
    mSocketTbl.reserve(socketTotal);
    for (size_t socketId = 0; socketId < socketTotal; ++socketId) {
        mSocketTbl.emplace_back(socketId, cpuSocketUtil, numaUtil);
    }

    const size_t totalCores = cpuSocketUtil.getTotalCores();
    if (mAffinityInfo.getNumCores() != totalCores) {
        std::ostringstream ostr;
        ostr << "internal core number mismatch."
             << " cpuSocketUtil.getTotalCores():" << cpuSocketUtil.getTotalCores() << " !="
             << " mAffinityInfo.getNumCores():" << mAffinityInfo.getNumCores();
        throw ostr.str();
    }

    mCoreTbl.resize(totalCores);
    crawlAllSockets([&](AffinityResourceSocket& socket) {
        socket.crawlAllNumaNodes([&](AffinityResourceNumaNode& numaNode) {
            numaNode.crawlAllCores([&](AffinityResourceCore& core) {
                mCoreTbl[core.getCoreId()] = &core;
                return true;
            });
            return true;
        });
        return true;
    });
 
    mMyPid = static_cast<size_t>(getpid());

    parserConfigure();
}

int
AffinityResourceControl::calcAvailableCoreTotal() const
{
    int total = 0;
    crawlAllSockets([&](const AffinityResourceSocket& socket) {
        total += socket.calcAvailableCoreTotal();
        return true;
    });
    return total;
}

std::vector<unsigned>
AffinityResourceControl::coreAllocation(const int numCores,
                                        const bool verifyMode)
//
// Throw exception(std::string err) if error
//
{
    std::vector<unsigned> coreIdTable;
    coreIdTable.reserve(numCores);

    resetPid(); // All the coreTbl info is overwritten by the current ShmAffinityInfo
    for (int i = 0; i < numCores; ++i) {
        std::string errMsg;
        int coreId = singleCoreAllocation(errMsg);
        if (coreId < 0) {
            std::ostringstream ostr;
            ostr << "AffinityResourceControl::coreAllocation() failed."
                 << " i:" << i << "/numCores:" << numCores << " err=>{\n"
                 << str_util::addIndent(errMsg) << '\n'
                 << "}";
            throw ostr.str();
        }
        if (verifyMode) {
            if (!verifyAllocation(coreId, errMsg)) {
                std::ostringstream ostr;
                ostr << "AffinityResourceControl::coreAllocation() VerifyAllocation failed."
                     << " i:" << i << "/numCores:" << numCores << " err=>{\n"
                     << str_util::addIndent(errMsg) << '\n'
                     << "}";
                throw ostr.str();
            }
        }
        coreIdTable.emplace_back(coreId);

        /* for debug
        std::cerr << ">> coreIdTable size:" << coreIdTable.size() << '\n';
        for (size_t id = 0; id < coreIdTable.size(); ++id) {
            std::cerr << "id:" << id << ' ' << coreIdTable[id] << '\n';
        }
        */
    }

    return coreIdTable;
}

std::string
AffinityResourceControl::show() const
{
    std::ostringstream ostr;
    ostr << "AffinityResourceControl {\n"
         << "  mMyPid:" << mMyPid << '\n'
         << str_util::addIndent(showSocketTbl()) << '\n'
         << str_util::addIndent(show2CoreTbl()) << '\n'
         << "}";
    return ostr.str();
}

std::string    
AffinityResourceControl::showSocketTbl() const
{
    if (mSocketTbl.empty()) return "mSocketTbl is empty";

    const size_t tblSize = mSocketTbl.size();
    std::ostringstream ostr;
    ostr << "mSocketTbl (size:" << tblSize << ") {\n";
    for (size_t tblId = 0; tblId < tblSize; ++tblId) {
        std::ostringstream ostr2;
        ostr2 << "tblId:" << tblId << ' ';
        ostr << str_util::addIndent(ostr2.str() + mSocketTbl[tblId].show()) << '\n';
    }
    ostr << "}";
    return ostr.str();
}

std::string
AffinityResourceControl::showCoreTbl() const
{
    if (mCoreTbl.empty()) return "mCoreTbl is empty";

    const size_t tblSize = mCoreTbl.size();
    std::ostringstream ostr;
    ostr << "mCoreTbl (size:" << tblSize << ") {\n";
    for (size_t tblId = 0; tblId < tblSize; ++tblId) {
        std::ostringstream ostr2;
        ostr2 << "tblId:" << tblId << ' ';
        ostr << str_util::addIndent(ostr2.str() + mCoreTbl[tblId]->show()) << '\n';
    }
    ostr << "}";
    return ostr.str();
}

std::string
AffinityResourceControl::show2CoreTbl() const
{
    if (mCoreTbl.empty()) return "mCoreTbl is empty";

    const size_t tblSize = mCoreTbl.size();
    const int wTblId = str_util::getNumberOfDigits(tblSize);
    const int wCoreId = str_util::getNumberOfDigits(getMaxCoreId());
    const int wNumaNodeId = str_util::getNumberOfDigits(getMaxNumaNodeId());
    const int wSocketId = str_util::getNumberOfDigits(getMaxSocketId());
    const int wPid = str_util::getNumberOfDigits(getMaxPid());

    const int maxSocketWeight = getMaxSocketWeight();
    const int wSocketWeight = ((maxSocketWeight < 0) ?
                               1 :
                               str_util::getNumberOfDigits(static_cast<unsigned>(maxSocketWeight)));
    const int maxNumaNodeWeight = getMaxNumaNodeWeight();
    const int wNumaNodeWeight = ((maxNumaNodeWeight < 0) ?
                                 1 :
                                 str_util::getNumberOfDigits(static_cast<unsigned>(maxNumaNodeWeight)));

    constexpr size_t totalItemsOneLine = 4;
    std::ostringstream ostr;

    auto showCore = [&](const unsigned tblId) {
        auto showWeight = [](const int wWeight, const int weight) {
            std::ostringstream ostr;
            if (weight < 0) {
                ostr << std::setfill('-') << std::setw(wWeight) << '-';
            } else {
                ostr << std::setw(wWeight) << weight;
            }
            return ostr.str();
        };

        const AffinityResourceCore* currCore = mCoreTbl[tblId];
        const unsigned coreId = currCore->getCoreId();
        const AffinityResourceNumaNode* currNumaNode = getNumaNodeByCoreId(coreId);
        const unsigned numaNodeId = currNumaNode->getNumaNodeId();
        const unsigned numaNodeWeight = currNumaNode->getWeight();
        const AffinityResourceSocket* currSocket = getSocketByCoreId(coreId);
        const unsigned socketId = currSocket->getSocketId();
        const unsigned socketWeight = currSocket->getWeight();

        // tblId[coreId/numaNodeId/socketId](numaNodeWeight/socketWeight)pid
        std::ostringstream ostr;
        ostr << std::setw(wTblId) << tblId << '['
             << std::setw(wCoreId) << coreId << '/'
             << std::setw(wNumaNodeId) << numaNodeId << '/'
             << std::setw(wSocketId) << socketId << ']';
        ostr << '('
             << showWeight(wNumaNodeWeight, numaNodeWeight) << '/'
             << showWeight(wSocketWeight, socketWeight)
             << ')';
        if (currCore->getUsedFlag()) {
            ostr << std::setw(wPid) << currCore->getPid();
        } else {
            ostr << std::setfill('-') << std::setw(wPid) << '-';
        }
        return ostr.str();
    };

    ostr << "mCoreTbl (size:" << tblSize << ") tblId[coreId/nodeId/socketId](nodeW/socketW)Pid {";
    for (unsigned tblId = 0; tblId < tblSize; ++tblId) {
        if (tblId % totalItemsOneLine == 0) ostr << "\n  ";
        ostr << showCore(tblId) << ' ';
    }
    ostr << "\n}";
    return ostr.str();
}

unsigned
AffinityResourceControl::getMaxCoreId() const
{
    unsigned id = 0;
    crawlAllCores([&](const AffinityResourceCore* core) {
        id = std::max(id, core->getCoreId());
        return true;
    });
    return id;
}

unsigned
AffinityResourceControl::getMaxNumaNodeId() const
{
    unsigned id = 0;
    crawlAllNumaNodes([&](const AffinityResourceNumaNode& numaNode) {
        id = std::max(id, numaNode.getNumaNodeId());
        return true;
    });
    return id;
}

unsigned
AffinityResourceControl::getMaxSocketId() const
{
    unsigned id = 0;
    crawlAllSockets([&](const AffinityResourceSocket& socket) {
        id = std::max(id, socket.getSocketId());
        return true;
    });
    return id;
}

size_t
AffinityResourceControl::getMaxPid() const
{
    size_t id = 0;
    crawlAllActiveCores(true, [&](const AffinityResourceCore* core) {
        id = std::max(id, core->getPid());
        return true;
    });
    return id;
}

int
AffinityResourceControl::getMaxSocketWeight() const
{
    int maxWeight = 0;
    crawlAllSockets([&](const AffinityResourceSocket& socket) {
        if (socket.getWeight() >= 0) maxWeight = std::max(maxWeight, socket.getWeight());
        return true;
    });
    return maxWeight;
}

int
AffinityResourceControl::getMaxNumaNodeWeight() const
{
    int maxWeight = 0;
    crawlAllNumaNodes([&](const AffinityResourceNumaNode& numaNode) {
        if (numaNode.getWeight() >= 0) maxWeight = std::max(maxWeight, numaNode.getWeight());
        return true;
    });
    return maxWeight;
}

const AffinityResourceSocket*
AffinityResourceControl::getSocketByCoreId(const unsigned coreId) const
{
    const AffinityResourceSocket* resultSocket = nullptr;
    crawlAllSockets([&](const AffinityResourceSocket& socket) {
        if (socket.isBelongCoreId(coreId)) {
            resultSocket = &socket;
            return false; // early exit 
        }
        return true;
    });
    return resultSocket;
}

const AffinityResourceNumaNode*
AffinityResourceControl::getNumaNodeByCoreId(const unsigned coreId) const
{
    const AffinityResourceNumaNode* resultNumaNode = nullptr;
    crawlAllNumaNodes([&](const AffinityResourceNumaNode& numaNode) {
        if (numaNode.isBelongCoreId(coreId)) {
            resultNumaNode = &numaNode;
            return false; // early exit
        }
        return true;
    });
    return resultNumaNode;
}

int
AffinityResourceControl::singleCoreAllocation(std::string& errMsg)
//
// return coreId if allocated
// return -1 when error and set error messages into errMsg
//    
{
    resetWeight(true); // only socket level
    calcSelectionWeight(true); // only socket level

    AffinityResourceSocket* socket = pickSocketCandidate();
    if (!socket) {
        std::ostringstream ostr;
        ostr << "New core allocation failed. No more core resources at Socket level. {\n"
             << str_util::addIndent(show()) << '\n'
             << "}";
        errMsg = ostr.str();
        return -1;
    }

    return socket->singleCoreAllocation(mMyPid, errMsg);
}

AffinityResourceSocket*
AffinityResourceControl::pickSocketCandidate()
//
// This function pick one of the socket based on the socket selection weight
//
{
    AffinityResourceSocket* candidate = nullptr;
    crawlAllSockets([&](AffinityResourceSocket& socket) {
        if (socket.getWeight() < 0) return true; // early exit
        if (!candidate) {
            candidate = &socket;
        } else {
            if (socket.getWeight() > candidate->getWeight()) {
                candidate = &socket;
            }
        }
        return true;
    });
    return candidate;
}

void
AffinityResourceControl::resetPid()
//
// All the coreTbl info is overwritten by the current ShmAffinityInfo
//
{
    const unsigned numCores = mAffinityInfo.getNumCores();
    for (unsigned coreId = 0; coreId < numCores; ++coreId) {
        bool occupancy;
        size_t pid;
        if (mAffinityInfo.getCoreInfo(coreId, occupancy, pid)) {
            if (occupancy) {
                mCoreTbl[coreId]->setUsedFlag(true);
                mCoreTbl[coreId]->setPid(pid);
            } else {
                mCoreTbl[coreId]->reset(); // usedFlag=false, pid=0
            }
        }
    }
}

void
AffinityResourceControl::resetWeight(const bool onlySocket)
{
    crawlAllSockets([&](AffinityResourceSocket& socket) {
        socket.resetWeight(onlySocket);
        return true;
    });
}

void
AffinityResourceControl::calcSelectionWeight(const bool onlySocket)
{
    const int otherProcTotal = calcTotalOtherProcesses();
    crawlAllSockets([&](AffinityResourceSocket& socket) {
        socket.calcSelectionWeight(mMyPid, otherProcTotal, onlySocket);
        return true;
    });
}

int
AffinityResourceControl::calcTotalOtherProcesses() const
{
    int total = 0;
    std::unordered_set<size_t> pidTblSet; // Using unordered_set in order to reduce search cost to O(1)
    crawlAllActiveCores(true, [&](const AffinityResourceCore* core) {
        const size_t currPid = core->getPid();
        if (currPid == mMyPid) return true; // skip
        if (pidTblSet.find(currPid) == pidTblSet.end()) {
            // found new Pid
            pidTblSet.insert(currPid);
            total++;
        }
        return true;
    });
    return total;
}

bool
AffinityResourceControl::verifyAllocation(const int targetCoreId, std::string& err) const
{
    auto showCoreCondition = [&](const CoreCondition& core) {
        std::ostringstream ostr;
        ostr << "CoreCondition {\n"
             << "  mCoreId:" << core.mCoreId << '\n'
             << "  mSockMyProc:" << str_util::boolStr(core.mSockMyProc) << '\n'
             << "  mSockOtherProcTotal:" << core.mSockOtherProcTotal << '\n'
             << "  mNodeMyProc:" << str_util::boolStr(core.mNodeMyProc) << '\n'
             << "  mNodeOtherProcTotal:" << core.mNodeOtherProcTotal << '\n'
             << "}";
        return ostr.str();
    };

    CoreCondition targetCore = computeCoreCondition(targetCoreId);

    bool result = true;
    crawlAllActiveCores
        (false,
         [&](const AffinityResourceCore* core) {
             if (core->getCoreId() == targetCoreId) {
                 return true; // skip targetCore
             } 
             CoreCondition trialCore = computeCoreCondition(core->getCoreId());
             if (shouldPickUpTrialRatherThanTarget(targetCore, trialCore)) {
                 // We found better candidate to pick up. This means verify is failed.
                 result = false; // set verify failed condition
                 std::ostringstream ostr;
                 ostr << "VERIFY-FAILED : We found better candidate to pick up. {\n"
                      << str_util::addIndent("targetCore " + showCoreCondition(targetCore)) << '\n'
                      << str_util::addIndent("betterCore " + showCoreCondition(trialCore)) << '\n'
                      << "}";
                 err = ostr.str();
                 return false; // early exit of crawl loop
             }
             return true;
         });

    return result;
}

AffinityResourceControl::CoreCondition
AffinityResourceControl::computeCoreCondition(const unsigned coreId) const
{
    const AffinityResourceSocket* socket = getSocketByCoreId(coreId);
    const AffinityResourceNumaNode* numaNode = getNumaNodeByCoreId(coreId);

    CoreCondition condition;
    condition.mCoreId = coreId;
    condition.mSockOtherProcTotal = socket->calcTotalOtherProcesses(mMyPid, condition.mSockMyProc);
    condition.mNodeOtherProcTotal = numaNode->calcTotalOtherProcesses(mMyPid, condition.mNodeMyProc);
    return condition;
}

bool
AffinityResourceControl::shouldPickUpTrialRatherThanTarget(const CoreCondition& targetCore,
                                                           const CoreCondition& trialCore) const
{
    // Investigate NUMA-node level first.
    if (targetCore.mNodeMyProc) {
        // NUMA-node of targetCore has myProc
        if (!trialCore.mNodeMyProc) {
            // NUMA-node of trialCore does not have myProc, this trialCore should not be picked up.
            return false;
        } else {
            // NUMA-node of trialCore has myProc
            if (trialCore.mNodeOtherProcTotal < targetCore.mNodeOtherProcTotal) {
                // NUMA-node of trialCore has less number of other-processes,
                // the trialCore is better than the targetCore
                return true;
            } else if (trialCore.mNodeOtherProcTotal > targetCore.mNodeOtherProcTotal) {
                // NUMA-node of trialCore has more the other-processes than targetCore's,
                // this trialCore should not be picked up.
                return false;
            } else {
                // In this case, targetCore and trialCore have the same opportunities,
                // and we can equally pick up one of them.
                return false;
            }
        }
    } else {
        // NUMA-node of targetCore does not have myProc
        if (trialCore.mNodeMyProc) {
            // NUMA-node of trialCore has myProc, this trialCore is better than the targetCore.
            return true;
        }
        // need to investigate more
    }

    // Investigate Socket-level condition
    if (targetCore.mSockMyProc) {
        // Socket of targetCore has myProc
        if (!trialCore.mSockMyProc) {
            // Socket of trialCore does not have myProc, this trailCore should not be picked up.
            return false;
        } else {
            // Socket of trialCore has myProc
            if (trialCore.mSockOtherProcTotal < targetCore.mSockOtherProcTotal) {
                // Socket of trialCore has less number of other-processes,
                // the trialCore is better than the targetCore
                return true;
            } else if (trialCore.mSockOtherProcTotal > targetCore.mSockOtherProcTotal) {
                // Socket of trialCore has more the other-processes than targetCore's,
                // this trialCore should not be picked up
                return false;
            } else {
                // In this case, targetCore and trialCore have the same opportunities,
                // and we can equally pick up one of them.
                return false;
            }
        }
    } else {
        // Socket of targetCore does not have myProc
        if (trialCore.mSockMyProc) {
            // Socket of trialCore has myProc, this trialCore is better than the targetCore.
            return true;
        }
        // Need to investigate more
    }

    // TargetCore condition does not have myProc in the NUMA-node and Socket
    // Investigate Socket-level condition of no-myProc here.
    if (trialCore.mSockOtherProcTotal < targetCore.mSockOtherProcTotal) {
        // Socket of trialCore has less number of other-processes,
        // the trialCore is better than the targetCore
        return true;
    } else if (trialCore.mSockOtherProcTotal > targetCore.mSockOtherProcTotal) {
        // Socket of trialCore has more the other-processes than targetCore's,
        // this trialCore should not be picked up
        return false;
    }

    // Socket of trialCore nad targetCore has the same number of the other-processes.
    // Need to investigate NUMA-node level
    if (trialCore.mNodeOtherProcTotal < targetCore.mNodeOtherProcTotal) {
        // NUMA-node of trialCore has less number of other-processes,
        // the trialCore is better than the targetCore
        return true;
    } else if (trialCore.mNodeOtherProcTotal > targetCore.mNodeOtherProcTotal) {
        // NUMA-node of trialCore has more the other-processes than targetCore's,
        // this trialCore should not be picked up
        return false;
    }

    // In this case, targetCore and trialCore has the same opportunities,
    // we can equally pick up one of them.
    return false;
}

void
AffinityResourceControl::parserConfigure()
{
    mParser.description("AffinityResourceControl command");
    mParser.opt("showAll", "", "show all info",
                [&](Arg& arg) { return arg.msg(show() + '\n'); });
    mParser.opt("showSocketTbl", "", "show socketTbl",
                [&](Arg& arg) { return arg.msg(showSocketTbl() + '\n'); });
    mParser.opt("showCoreTbl", "", "show coreTbl",
                [&](Arg& arg) { return arg.msg(showCoreTbl() + '\n'); });
    mParser.opt("show2CoreTbl", "", "show2 coreTbl",
                [&](Arg& arg) { return arg.msg(show2CoreTbl() + '\n'); });
    mParser.opt("testWeight", "", "run all weight computation only (for testing)",
                [&](Arg& arg) { return testWeight([&](const std::string& msg) { return arg.msg(msg); }); });
    mParser.opt("testCoreAllocation", "<numCores> <verify-on|off>", "core allocation",
                [&](Arg& arg) {
                    int numCores = (arg++).as<int>(0);
                    bool verify = (arg++).as<bool>(0);
                    return testCoreAllocation(numCores, verify,
                                              [&](const std::string& msg) { return arg.msg(msg); });
                });
}

bool
AffinityResourceControl::testWeight(const MsgFunc& msgFunc)
{
    msgFunc("===>>> test weight calculation for all levels. (i.e. socket/numaNode) <<<===\n");

    resetPid();

    resetWeight(false);
    // msgFunc(show2CoreTbl() + '\n'); // for debug

    calcSelectionWeight(false);

    return true;
}

bool
AffinityResourceControl::testCoreAllocation(const int numCores, const bool verify, const MsgFunc& msgFunc)
{
    auto showTbl = [](const std::string& title, const std::vector<unsigned>& tbl) {
        const int wId = str_util::getNumberOfDigits(tbl.size());
        const int wValue = str_util::getNumberOfDigits(*std::max_element(tbl.begin(), tbl.end()));
        auto showItem = [&](const size_t id) {
            std::ostringstream ostr;
            ostr << std::setw(wId) << id << ':' << std::setfill('0') << std::setw(wValue) << tbl[id];
            return ostr.str();
        };
        constexpr size_t numItemsOneLine = 16;
        std::ostringstream ostr;
        ostr << title << " (size:" << tbl.size() << ") {";
        for (size_t id = 0; id < tbl.size(); ++id) {
            if (id % numItemsOneLine == 0) ostr << "\n  ";
            ostr << showItem(id) << ' ';
        }
        ostr << "\n}";
        return ostr.str();
    };

    msgFunc("===>>> test core allocation <<<===\n");
    std::vector<unsigned> coreIdTable;
    try {
        coreIdTable = coreAllocation(numCores, verify);
    }
    catch(const std::string& err) {
        std::ostringstream ostr;
        ostr << "coreAllocation failed. err=>{\n"
             << str_util::addIndent(err) << '\n'
             << "}";
        msgFunc(ostr.str() + '\n');
        return false;
    }
    std::ostringstream ostr;
    ostr << "coreAllocation OK. numCores:" << numCores << '\n'
         << show2CoreTbl() << '\n'
         << showTbl("coreIdTable", coreIdTable);
    msgFunc(ostr.str() + '\n');
    
    return true;
}

} // namespace grid_util
} // namespace scene_rdl2
