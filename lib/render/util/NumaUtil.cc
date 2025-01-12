// Copyright 2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "NumaUtil.h"
#include "CpuSocketUtil.h"

#include <scene_rdl2/common/except/exceptions.h>
#include <scene_rdl2/render/util/StrUtil.h>

#include <algorithm>
#include <fstream>
#include <linux/mempolicy.h> // MPOL_BIND
#include <sys/mman.h> // mmap, munmap
#include <sys/syscall.h> // __MR_mbind
#include <unistd.h> // syscall(), sysconf()

namespace {

std::string
getSingleLine(const std::string& fileName, std::string& errMsg)
{
    std::ifstream ifs(fileName);
    if (!ifs) {
        std::ostringstream ostr;
        ostr << "NumaUtil::getSingleLine() Can not open file:" << fileName;
        errMsg = ostr.str();
        return "";
    }
    std::string line;
    if (!std::getline(ifs, line)) {
        std::ostringstream ostr;
        ostr << "NumaUtil::getSingleLine() File read failed. file:" << fileName;
        errMsg = ostr.str();
        return "";
    }
    return line;
}

std::vector<unsigned>
getIdTbl(const std::string& infoFileName)
//
// Might throw except::RuntimeError() when error
//
{
    std::ostringstream ostr;

    std::string errMsg;
    std::string line = getSingleLine(infoFileName, errMsg);
    if (line.empty()) {
        ostr << "NumaUtil::getIdTbl() failed. err:" << errMsg;
        throw scene_rdl2::except::RuntimeError(ostr.str());
    }

    std::vector<unsigned> idTbl;
    if (!scene_rdl2::CpuSocketUtil::parseIdDef(line, idTbl, errMsg)) {
        ostr << "NumaUtil::getIdTbl() failed. err:" << errMsg;
        throw scene_rdl2::except::RuntimeError(ostr.str());
    }
    return idTbl; // idTbl is already sorted
}

std::vector<unsigned>
getNumaNodeIdTbl()
{
    constexpr const char* infoFileName = "/sys/devices/system/node/online";
    return getIdTbl(infoFileName); // sorted
}

size_t
getNumaNodeMemSize(const unsigned numaNodeId)
//
// Might throw except::RuntimeError() when error
//
{
    auto getInfoFileName = [&]() {
        constexpr const char* infoFilePrefix = "/sys/devices/system/node/node";
        std::ostringstream ostr;
        ostr << infoFilePrefix << numaNodeId << "/meminfo";
        return ostr.str();
    };

    std::ifstream ifs(getInfoFileName());
    if (!ifs) {
        std::ostringstream ostr;
        ostr << "NumaUtil::getNumaNodeMemSize() Can not open file:" << getInfoFileName();
        throw scene_rdl2::except::RuntimeError(ostr.str());
    }

    std::string str;
    while (std::getline(ifs, str)) {
        if (str.find("MemTotal") != std::string::npos) {
            // found memory total entry
            std::stringstream sstr(str);
            std::string work0, work1, work2;
            size_t memSizeKB;;
            sstr >> work0 >> work1 >> work2 >> memSizeKB;
            return memSizeKB * 1024; // return byte
        }
    }
    return 0;
}

std::vector<int>
getNumaNodeDistance(const unsigned numaNodeId)
//
// Might throw except::RuntimeError() when error
//
{
    auto getInfoFileName = [&]() {
        constexpr const char* infoFilePrefix = "/sys/devices/system/node/node";
        return infoFilePrefix + std::to_string(numaNodeId) + "/distance";
    };

    std::ostringstream ostr;
    std::string errMsg;
    std::string line = getSingleLine(getInfoFileName(), errMsg);
    if (line.empty()) {
        ostr << "NumaUtil::getNumaNodeDistance() failed. err:" << errMsg;
        throw scene_rdl2::except::RuntimeError(ostr.str());
    }
    
    std::vector<int> distanceTbl;
    std::stringstream sstr(line);
    while (true) {
        int val;
        if (!(sstr >> val)) break;
        distanceTbl.push_back(val);
    }
    return distanceTbl;
}

std::vector<unsigned>
getNumaNodeCpuIdTbl(const unsigned numaNodeId)
{
    auto getInfoFileName = [&]() {
        constexpr const char* infoFilePrefix = "/sys/devices/system/node/node";
        std::ostringstream ostr;
        ostr << infoFilePrefix << numaNodeId << "/cpulist";
        return ostr.str();
    };

    return getIdTbl(getInfoFileName());
}

long
sysCallMBind(void* const addr,
             const unsigned long size,
             const int mode,
             const unsigned long* nodeMask,
             const unsigned long maxNode,
             const unsigned flags)
//
// NUMA (Non-Uniform Memory Access) memmory bind system call
//
{
    return syscall(__NR_mbind, addr, size, mode, nodeMask, maxNode, flags);
}

void*
numaNodeMBind(const unsigned numaNodeId,
              void* const memory,
              const size_t size)
//
// Might throw except::RuntimeError() when error
//
{
    //
    // Just in case, this code supports more than sizeof(unsigned long) count of NUMA-node
    //
    size_t nodeMaskSize = (numaNodeId + 1) / sizeof(unsigned long);
    if ((numaNodeId + 1) % sizeof(unsigned long) != 0) nodeMaskSize++;

    std::vector<unsigned long> nodeMask(nodeMaskSize, 0x0);
    const size_t maskId = numaNodeId / sizeof(unsigned long);
    const int maskShift = numaNodeId % sizeof(unsigned long);
    nodeMask[maskId] = static_cast<unsigned long>(0x1) << maskShift;

    if (sysCallMBind(memory,
                     size,
                     MPOL_BIND, // Memory Policy: Bind to the particular NUMA-node
                     nodeMask.data(),
                     nodeMask.size() * sizeof(unsigned long) * 8,
                     0) != 0) {
        std::ostringstream ostr;
        ostr << "numaNodeMBInd() sysCallMBind() failed. numaNodeId:" << numaNodeId << " size:" << size;
        munmap(memory, size);
        throw scene_rdl2::except::RuntimeError(ostr.str());
    }
    return memory;
}

long
sysCallMovePages(const int pId,
                 const unsigned long count,
                 void** const pages,
                 int* const nodes,
                 int* const status,
                 const int flags)
{
    return syscall(__NR_move_pages, pId, count, pages, nodes, status, flags);
}

void*
mmapMemory(const size_t size)
//
// Might throw except::RuntimeError() when error
//
{
    // mmap returned address is always aligned by pagesize.
    void* const memory = mmap(nullptr,
                              size,
                              PROT_READ | PROT_WRITE,
                              MAP_PRIVATE | MAP_ANONYMOUS,
                              -1,
                              0);
    if (memory == MAP_FAILED) {
        std::ostringstream ostr;
        ostr << "file:" << __FILE__ << " line:" << __LINE__ << " func:" << __FUNCTION__
             << " mmap failed. size:" << size;
        throw scene_rdl2::except::RuntimeError(ostr.str());
    }
    return memory;
}

size_t
getPageSize()
{
    const long pageSize = sysconf(_SC_PAGESIZE);
    if (pageSize == -1) return 0;
    return static_cast<size_t>(pageSize);
}

} // namespace

//------------------------------------------------------------------------------------------

namespace scene_rdl2 {

NumaNode::NumaNode(const unsigned nodeId,
                   const unsigned totalNode,
                   const size_t memSize,
                   const std::vector<unsigned>& cpuIdList,
                   const std::vector<int>& nodeDistance)
    : mNodeId {nodeId}
    , mTotalNode {totalNode}
    , mMemSize {memSize}
    , mPageSize {getPageSize()}
    , mCpuIdList {cpuIdList}
    , mNodeDistance {nodeDistance}
{
}

void*
NumaNode::alloc(const size_t size) const
//
// might throw except::RuntimeError(std::string) when error
//
{
    return numaNodeMBind(mNodeId, mmapMemory(size), size);
}

void
NumaNode::free(void* const memory, const size_t size) const
{
    munmap(memory, size);
}

#ifdef NOT_USED_SO_FAR // but keep this code for future references

void*
NumaNode::alignedAlloc(const size_t size, const size_t align) const
//
// might throw except::RuntimeError(std::string) when error
//
{
    const size_t mapSize = size + align;
    const uintptr_t mapAddr = reinterpret_cast<uintptr_t>(mmapMemory(mapSize));
    const uintptr_t alignedAddr = (mapAddr + align - 1) & ~(align - 1);
    void* const alignedPtr = reinterpret_cast<void*>(alignedAddr);
    return numaNodeMBind(mNodeId, alignedPtr, size);
}
#endif // end NOT_USED_SO_FAR

#ifdef NOT_USED_SO_FAR // but keep this code for future references
void
NumaNode::alignedFree(void* const alignedMemory, const size_t size) const
{
    // We have to munmap original mmap address not as an aligned address.
    // However, there is no way to track the original mmap address now.
    // Needs more work.
}
#endif // end NOT_USED_SO_FAR

bool
NumaNode::isBelongMem(void* const memory, const size_t size) const
{
    const size_t pageSize = getPageSize();
    size_t totalPages = size / pageSize;
    if (totalPages * pageSize < size) totalPages++;

    std::vector<void*> ptrTbl(totalPages);
    for (size_t i = 0; i < ptrTbl.size(); ++i) {
        ptrTbl[i] = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(memory) + i * pageSize);
    }

    std::vector<int> nodeIdTbl(totalPages);
    // The move pages system call was designed for moving memory to the particular NUMA-node.
    // But if we set nullptr for nodes, we can ask which NUMA-node the memory belongs to.
    if (sysCallMovePages(0,
                         totalPages,
                         ptrTbl.data(),
                         nullptr,
                         &(nodeIdTbl[0]),
                         0) != 0) {
        return false;
    }

    return std::find_if(nodeIdTbl.begin(), nodeIdTbl.end(),
                        [&](int value) { return value != mNodeId; }) == nodeIdTbl.end();
}

bool
NumaNode::isBelongCpu(const unsigned cpuId) const
{
    if (cpuId < mCpuIdList.front() || mCpuIdList.back() < cpuId) return false;
    return std::find(mCpuIdList.begin(), mCpuIdList.end(), cpuId) != mCpuIdList.end();
}

bool
NumaNode::alignmentSizeCheck(const size_t alignment) const
//
// All the memory is allocated by mmap and the address is always aligned by pagesize.
// This function checks that the requested alignment is valid by pagesize alignment.
// We can safely use the address allocated by NumaNode::alloc() as an aligned address
// if ths aligned size is passed by this test.
//
{
    if (mPageSize < alignment) return false;
    return ((mPageSize % alignment) == 0);
}

std::string
NumaNode::show() const
{
    auto maxDistance = [&]() {
        return *std::max_element(mNodeDistance.begin(), mNodeDistance.end());
    };

    auto showDistanceTbl = [&]() {
        const int w0 = str_util::getNumberOfDigits(mNodeDistance.size());
        const int w1 = str_util::getNumberOfDigits(static_cast<unsigned>(maxDistance()));
        std::ostringstream ostr;
        ostr << "mNodeDistance (size:" << mNodeDistance.size() << ") {\n";
        for (size_t i = 0; i < mNodeDistance.size(); ++i) {
            ostr << "  nodeId:" << std::setw(w0) << i << ' ' << std::setw(w1) << mNodeDistance[i];
            if (i == mNodeId) ostr << " <<- myself";
            ostr << '\n';
        }
        ostr << "}";
        return ostr.str();
    };

    std::ostringstream ostr;
    ostr << "NumaNode {\n"
         << "  mNodeId:" << mNodeId << '\n'
         << "  mTotalNode:" << mTotalNode << '\n'
         << "  mMemSize:" << str_util::byteStr(mMemSize) << " (" << mMemSize << " byte)\n"
         << "  mPageSize:" << mPageSize << " byte\n"
         << str_util::addIndent(scene_rdl2::CpuSocketUtil::showCpuIdTbl("mCpuIdList", mCpuIdList)) << '\n'
         << str_util::addIndent(showDistanceTbl()) << '\n'
         << "}";
    return ostr.str();
}

//------------------------------------------------------------------------------------------

NumaUtil::NumaUtil() // Might throw scene_rdl2::except::RuntimeError() when error
{
    std::vector<unsigned> nodeIdTbl = getNumaNodeIdTbl(); // sorted
    const unsigned totalNode = *(std::max_element(nodeIdTbl.begin(), nodeIdTbl.end())) + 1;
    for (unsigned id = 0; id < nodeIdTbl.size(); ++id) {
        mNumaNodeTbl.emplace_back(nodeIdTbl[id],
                                  totalNode,
                                  getNumaNodeMemSize(id),
                                  getNumaNodeCpuIdTbl(id),
                                  getNumaNodeDistance(id));
    }
}

const NumaNode*
NumaUtil::getNumaNode(const unsigned nodeId) const
{
    if (nodeId >= mNumaNodeTbl.size()) return nullptr;
    return &mNumaNodeTbl[nodeId];
}

const NumaNode*
NumaUtil::findNumaNodeByCpuId(const unsigned cpuId) const
{
    for (size_t i = 0; i < mNumaNodeTbl.size(); ++i) {
        if (mNumaNodeTbl[i].isBelongCpu(cpuId)) return &mNumaNodeTbl[i];
    }
    return nullptr;
}

std::vector<unsigned>
NumaUtil::genActiveNumaNodeIdTblByCpuIdTbl(const std::vector<unsigned>& cpuIdTbl) const
{
    std::vector<unsigned> workTbl = cpuIdTbl;
    std::sort(workTbl.begin(), workTbl.end());

    std::vector<unsigned> numaNodeIdTbl;
    for (size_t i = 0; i < workTbl.size(); ++i) {
        const NumaNode* currNode = findNumaNodeByCpuId(workTbl[i]);
        if (currNode) {
            if (numaNodeIdTbl.empty() || numaNodeIdTbl.back() != currNode->getNodeId()) {
                numaNodeIdTbl.push_back(currNode->getNodeId());
            }
        }
    }

    // erase duplicate nodeId info
    std::sort(numaNodeIdTbl.begin(), numaNodeIdTbl.end());
    auto last = std::unique(numaNodeIdTbl.begin(), numaNodeIdTbl.end());
    numaNodeIdTbl.erase(last, numaNodeIdTbl.end());

    /* useful debug dump
    auto showTbl = [](std::vector<unsigned> tbl) {
        std::ostringstream ostr;
        ostr << "showTbl { ";
        for (auto itr : tbl) {
            ostr << itr << ' ';
        }
        ostr << "}";
        return ostr.str();
    };
    std::cerr << __FILE__ << " numaNodeIdTbl " << showTbl(numaNodeIdTbl) << '\n';
    */

    return numaNodeIdTbl;
}

// static function
unsigned
NumaUtil::findNumaNodeByMemAddr(void* addr)
{
    int numaNodeId;
    if (sysCallMovePages(0,
                         1,
                         &addr,
                         nullptr,
                         &numaNodeId,
                         0) != 0) {
        std::ostringstream ostr;
        ostr << "NumaUtil::findNumaNodeByMemAddr() failed. Could not find NUMA-node location. err:"
             << strerror(errno);
        throw except::RuntimeError(ostr.str());
    }
    return numaNodeId;
}

std::string
NumaUtil::show() const
{
    std::ostringstream ostr;
    ostr << "NumaUtil (size:" << mNumaNodeTbl.size() << ") {\n";
    if (mNumaNodeTbl.empty()) {
        ostr << "  empty\n";
    } else {
        const int w = str_util::getNumberOfDigits(mNumaNodeTbl.size());
        for (size_t i = 0; i < mNumaNodeTbl.size(); ++i) {
            std::ostringstream ostr2;
            ostr2 << "i:" << std::setw(w) << i << ' ' << mNumaNodeTbl[i].show();
            ostr << str_util::addIndent(ostr2.str()) << '\n';
        }
    }
    ostr << "}";
    return ostr.str();
}

} // namespace scene_rdl2
