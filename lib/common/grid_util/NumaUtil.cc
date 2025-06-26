// Copyright 2024-2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "NumaUtil.h"
#include "CpuSocketUtil.h"

#include <scene_rdl2/common/except/exceptions.h>
#include <scene_rdl2/render/util/StrUtil.h>

#include <algorithm>
#include <fstream>
#include <iostream> // debug
#include <sys/mman.h> // mmap, munmap
#include <sys/syscall.h> // __MR_mbind
#include <unistd.h> // syscall(), sysconf()

#ifdef PLATFORM_APPLE
#include <errno.h>
#include <sys/sysctl.h>
#include <thread>
#else // !PLATFORM_APPLE
#include <climits> // CHAR_BIT
#include <linux/mempolicy.h> // MPOL_BIND
#endif // end of !PLATFORM_APPLE

//#define DEBUG_MSG

namespace {

#ifdef DEBUG_MSG
template <typename T>
std::string
showVec(const std::string& msg, const std::vector<T>& tbl)
{
    constexpr size_t maxOneLineItems = 8;
    const int wi = scene_rdl2::str_util::getNumberOfDigits(tbl.size());
    int wv {0};
    if constexpr (std::is_unsigned_v<T>) {
        wv = scene_rdl2::str_util::getNumberOfDigits(static_cast<size_t>(*std::max_element(tbl.begin(), tbl.end())));
    }
    std::ostringstream ostr;
    ostr << msg << " (size:" << tbl.size() << ") {\n";
    for (size_t i = 0; i < tbl.size(); ++i) {
        if (i % maxOneLineItems == 0) ostr << "  ";
        ostr << "i:" << std::setw(wi) << i << ' ' << std::setw(wv) << tbl[i] << ' ';
        if ((i + 1) % maxOneLineItems == 0) ostr << '\n';
    }
    if (tbl.size() % maxOneLineItems != 0) ostr << '\n';
    ostr << "} ";
    if constexpr (std::is_unsigned_v<T>) {
        ostr << scene_rdl2::grid_util::CpuSocketUtil::idTblToDefStr(tbl);
    } else if constexpr (std::is_signed_v<T>) {
        ostr << '{';
        for (size_t i = 0; i < tbl.size(); ++i) {
            ostr << tbl[i];
            if (i != tbl.size() - 1) ostr << ',';
        }
        ostr << '}';
    }
    return ostr.str();
}
#endif // end DEBUG_MSG

void
unknownNumaIdAndThrowException(const std::string& msg, const std::string& modeStr, const unsigned numaNodeId)
{
    std::ostringstream ostr;
    ostr << msg << " unknown numaNodeId:" << modeStr << " modeStr:" << modeStr;

    throw scene_rdl2::except::RuntimeError(ostr.str());
}

#ifndef PLATFORM_APPLE
std::string
getSingleLine(const std::string& fileName, std::string& errMsg)
{
    errMsg.clear();

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
#endif // end of !PLATFORM_APPLE

#ifndef PLATFORM_APPLE
std::vector<unsigned>
getIdTbl(const std::string& infoFileName)
//
// Might throw an except::RuntimeError() if an error occurs.
//
{
    std::vector<unsigned> idTbl;

    std::string errMsg;
    const std::string line = getSingleLine(infoFileName, errMsg);
    if (line.empty()) {
        if (!errMsg.empty()) {
            std::ostringstream ostr;
            ostr << "NumaUtil::getIdTbl() failed. err:" << errMsg;
            throw scene_rdl2::except::RuntimeError(ostr.str());
        }
        return idTbl; // empty
    }

    if (!scene_rdl2::grid_util::CpuSocketUtil::parseIdDef(line, idTbl, errMsg)) {
        std::ostringstream ostr;
        ostr << "NumaUtil::getIdTbl() failed. err:" << errMsg;
        throw scene_rdl2::except::RuntimeError(ostr.str());
    }
    return idTbl; // idTbl is already sorted
}
#endif // end of !PLATFORM_APPLE

//------------------------------------------------------------------------------------------

std::vector<unsigned>
getNumaNodeIdTbl(const std::string& modeStr)
//
// Might throw an exception scene_rdl2::except::RuntimeError if an error occurs
//
{
    std::vector<unsigned> tbl;
    auto setTbl = [&](const size_t size) {
        tbl.resize(size);
        for (size_t i = 0; i < size; ++i) tbl[i] = static_cast<unsigned>(i);
    };
    
    if (modeStr == "localhost") {
#ifdef PLATFORM_APPLE
        setTbl(1); // Mac is UMA, we return single id. 
#else // !PLATFORM_APPLE    
        constexpr const char* infoFileName = "/sys/devices/system/node/online";
        tbl = getIdTbl(infoFileName); // sorted
#endif // end of !PLATFORM_APPLE
    } else if (modeStr == "ag") {
        setTbl(8);
    } else if (modeStr == "tin") {
        setTbl(2);
    } else if (modeStr == "cobalt") {
        setTbl(1);
    } else {
        std::ostringstream ostr;
        ostr << "getNumaNodeIdTbl() failed. unknown modeStr:" << modeStr;
        throw scene_rdl2::except::RuntimeError(ostr.str());
    }

#ifdef DEBUG_MSG
    std::cerr << showVec("getNumaNodeIdTbl()", tbl) << '\n';
#endif // end DEBUG_MSG

    return tbl;
}

size_t
getLocalhostNumaNodeMemSize(const unsigned numaNodeId)
//
// Might throw an exception scene_rdl2::except::RuntimeError() if an error occurs
//
{    
#ifdef PLATFORM_APPLE
    // We assume Mac is single NUMA-node
    int64_t memSize = 0;
    size_t memSizeLen = sizeof(memSize);
    if (sysctlbyname("hw.memsize", &memSize, &memSizeLen, nullptr, 0) != 0) {
        std::ostringstream ostr;
        ostr << "NumaUtil::getNumaNodeMemSize() sysctlbyname(\"hw.memsize\") failed. error=>{\n"
             << scene_rdl2::str_util::addIndent(strerror(errno)) << '\n'
             << "}";
        throw scene_rdl2::except::RuntimeError(ostr.str());
    }
    return static_cast<size_t>(memSize);
#else // !PLATFORM_APPLE    
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
#endif // end of !PLATFORM_APPLE
}

size_t
getEmulatedNumaNodeMemSize(const std::string& modeStr, const unsigned numaNodeId)
//
// Might throw an exception scene_rdl2::except::RuntimeError() if an error occurs
//
{
    auto unknownNumaId = [&]() {
        unknownNumaIdAndThrowException("getEmulatedNumaNodeMemSize() failed.", modeStr, numaNodeId);
    };

    size_t size {0};
    if (modeStr == "ag") {
        switch (numaNodeId) {
        case 0 : size = 100589060096; break;
        case 1 : size = 101455962112; break;
        case 2 : size = 101455966208; break;
        case 3 : size = 101455962112; break;
        case 4 : size = 101455966208; break;
        case 5 : size = 101455962112; break;
        case 6 : size = 101455966208; break;
        case 7 : size = 101335265280; break;
        default : unknownNumaId();
        }
    } else if (modeStr == "tin") {
        switch (numaNodeId) {
        case 0 : size = 99433930752; break;
        case 1 : size = 101452263424; break;
        default : unknownNumaId();
        }
    } else if (modeStr == "cobalt") {
        if (numaNodeId == 0) size = 269522509824;
        else unknownNumaId();
    } else {
        std::ostringstream ostr;
        ostr << "getEmulatedNumaNodeMemSize() failed. unknown modeStr:" << modeStr;
        throw scene_rdl2::except::RuntimeError(ostr.str());
    }
    return size;
}

size_t
getNumaNodeMemSize(const std::string& modeStr, const unsigned numaNodeId)
//
// Might throw an exception except::RuntimeError() if an error occurs
//
{
    size_t size {0};
    if (modeStr == "localhost") {
        size = getLocalhostNumaNodeMemSize(numaNodeId);
    } else {
        size = getEmulatedNumaNodeMemSize(modeStr, numaNodeId);
    }

#ifdef DEBUG_MSG    
    std::cerr << "getNumaNodeMemSize():" << size << '\n';
#endif // end DEBUG_MSG
    return size;
}

std::vector<unsigned>
getLocalhostNumaNodeCpuIdTbl(const unsigned numaNodeId)
{
#ifdef PLATFORM_APPLE
    // We have to get total CPU count and assume all CPUs are belong to the single NUMA-node.
    unsigned totalCpu = std::thread::hardware_concurrency();
    std::vector<unsigned> cpuIdTbl;
    cpuIdTbl.reserve(totalCpu);
    for (unsigned i = 0; i < totalCpu; ++i) {
        cpuIdTbl.push_back(i);
    }
    return cpuIdTbl;
#else // !PLATFORM_APPLE    
    auto getInfoFileName = [&]() {
        constexpr const char* infoFilePrefix = "/sys/devices/system/node/node";
        std::ostringstream ostr;
        ostr << infoFilePrefix << numaNodeId << "/cpulist";
        return ostr.str();
    };
    return getIdTbl(getInfoFileName());
#endif // end of !PLATFORM_APPLE 
}

std::vector<unsigned>
getEmulatedNumaNodeCpuIdTbl(const std::string& modeStr, const unsigned numaNodeId)
//
// Might throw an exception scene_rdl2::except::RuntimeError() if an error occurs
// 
{
    auto unknownNumaId = [&]() {
        unknownNumaIdAndThrowException("getEmulatedNumaNodeCpuIdTbl() failed.", modeStr, numaNodeId);
    };
    auto makeTbl = [&](const std::string& defStr) {
        std::vector<unsigned> tbl;
        std::string errMsg;
        if (!scene_rdl2::grid_util::CpuSocketUtil::parseIdDef(defStr, tbl, errMsg)) {
            std::ostringstream ostr;
            ostr << "getEmulatedNumaNodeCpuIdTbl() makeTbl failed."
                 << " defStr:" << defStr << " modeStr:" << modeStr << " numaNodeId:" << numaNodeId << " err=>{\n"
                 << scene_rdl2::str_util::addIndent(errMsg) << '\n'
                 << "}";
            throw scene_rdl2::except::RuntimeError(ostr.str());
        }
        return tbl;
    };

    std::vector<unsigned> tbl;
    if (modeStr == "ag") {
        switch (numaNodeId) {
        case 0 : tbl = makeTbl("0-23,192-215"); break;
        case 1 : tbl = makeTbl("24-47,216-239"); break;
        case 2 : tbl = makeTbl("48-71,240-263"); break;
        case 3 : tbl = makeTbl("72-95,264-287"); break;
        case 4 : tbl = makeTbl("96-119,288-311"); break;
        case 5 : tbl = makeTbl("120-143,312-335"); break;
        case 6 : tbl = makeTbl("144-167,336-359"); break;
        case 7 : tbl = makeTbl("168-191,360-383"); break;
        default : unknownNumaId();
        }
    } else if (modeStr == "tin") {
        switch (numaNodeId) {
        case 0 : tbl = makeTbl("0-23,48-71"); break;
        case 1 : tbl = makeTbl("24-47,72-95"); break;
        default : unknownNumaId();
        }
    } else if (modeStr == "cobalt") {
        if (numaNodeId == 0) tbl = makeTbl("0-127");
        else unknownNumaId();
    } else {
        std::ostringstream ostr;
        ostr << "getEmulatedNumaNodeCpuIdTbl() failed. unknown modeStr:" << modeStr;
        throw scene_rdl2::except::RuntimeError(ostr.str());
    }

    return tbl;
}

std::vector<unsigned>
getNumaNodeCpuIdTbl(const std::string& modeStr, const unsigned numaNodeId)
//
// Might throw an exception scene_rdl2::except::RuntimeError() if an error occurs
// 
{
    std::vector<unsigned> tbl;
    if (modeStr == "localhost") {
        tbl = getLocalhostNumaNodeCpuIdTbl(numaNodeId);
    } else {
        tbl = getEmulatedNumaNodeCpuIdTbl(modeStr, numaNodeId);
    }

#ifdef DEBUG_MSG
    std::cerr << showVec("getNumaNodeCpuIdTbl (numaNodeId:" + std::to_string(numaNodeId) + ')', tbl) << '\n';
#endif // end DEBUG_MSG

    return tbl;
}

std::vector<int>
getLocalhostNumaNodeDistance(const unsigned numaNodeId)
//
// Might throw an exception scene_rdl2::except::RuntimeError if an error occurs
//
{
#ifdef PLATFORM_APPLE
    // We assume Mac as Single item with distance 10
    std::vector<int> distanceTbl(1,10);
    return distanceTbl;
#else // !PLATFORM_APPLE    
    auto getInfoFileName = [&]() {
        constexpr const char* infoFilePrefix = "/sys/devices/system/node/node";
        return infoFilePrefix + std::to_string(numaNodeId) + "/distance";
    };

    std::vector<int> distanceTbl;

    std::ostringstream ostr;
    std::string errMsg;
    std::string line = getSingleLine(getInfoFileName(), errMsg);
    if (line.empty()) {
        if (!errMsg.empty()) {
            ostr << "NumaUtil::getNumaNodeDistance() failed. err:" << errMsg;
            throw scene_rdl2::except::RuntimeError(ostr.str());
        }
        return distanceTbl; // empty
    }
    
    std::stringstream sstr(line);
    while (true) {
        int val;
        if (!(sstr >> val)) break;
        distanceTbl.push_back(val);
    }

    return distanceTbl;
#endif // end of !PLATFORM_APPLE
}

std::vector<int>
getEmulatedNumaNodeDistance(const std::string& modeStr, const unsigned numaNodeId)
{
    auto unknownNumaId = [&]() {
        unknownNumaIdAndThrowException("getEmulatedNumaNodeDistance() failed.", modeStr, numaNodeId);
    };

    std::vector<int> distance;
    if (modeStr == "ag") {
        switch (numaNodeId) {
        case 0 : distance = {10,12,12,12,32,32,32,32}; break;
        case 1 : distance = {12,10,12,12,32,32,32,32}; break;
        case 2 : distance = {12,12,10,12,32,32,32,32}; break;
        case 3 : distance = {12,12,12,10,32,32,32,32}; break;
        case 4 : distance = {32,32,32,32,10,12,12,12}; break;
        case 5 : distance = {32,32,32,32,12,10,12,12}; break;
        case 6 : distance = {32,32,32,32,12,12,10,12}; break;
        case 7 : distance = {32,32,32,32,12,12,12,10}; break;
        default : unknownNumaId();
        }
    } else if (modeStr == "tin") {
        switch (numaNodeId) {
        case 0 : distance = {10,21}; break;
        case 1 : distance = {21,10}; break;
        default : unknownNumaId();
        }
    } else if (modeStr == "cobalt") {
        if (numaNodeId == 0) distance = {10};
        else unknownNumaId();
    } else {
        std::ostringstream ostr;
        ostr << "getEmulatedNumaNodeDistance() failed. unknown modeStr:" << modeStr;
        throw scene_rdl2::except::RuntimeError(ostr.str());
    }
    return distance;
}

std::vector<int>
getNumaNodeDistance(const std::string& modeStr, const unsigned numaNodeId)
//
// Might throw an exception except::RuntimeError() if an error occurs
//
{ 
    std::vector<int> distanceTbl;
    if (modeStr == "localhost") {
        distanceTbl = getLocalhostNumaNodeDistance(numaNodeId);
    } else {
        distanceTbl = getEmulatedNumaNodeDistance(modeStr, numaNodeId);
    }
    
#ifdef DEBUG_MSG
    std::cerr << showVec("getNumaNodeDistance() (numaNodeId:" + std::to_string(numaNodeId) + ')', distanceTbl)
              << '\n';
#endif // end of DEBUG_MSG

    return distanceTbl;
}

//------------------------------------------------------------------------------------------

#ifndef PLATFORM_APPLE
long
sysCallMBind(void* const addr,
             const unsigned long size,
             const int mode,
             const unsigned long* nodeMask,
             const unsigned long maxNode,
             const unsigned flags)
//
// NUMA (Non-Uniform Memory Access) memory bind system call
//
{
    return syscall(__NR_mbind, addr, size, mode, nodeMask, maxNode, flags);
}
#endif // end !PLATFORM_APPLE

#ifndef PLATFORM_APPLE
void*
numaNodeMBind(const unsigned numaNodeId,
              void* const memory,
              const size_t size)
//
// Might throw an except::RuntimeError() if an error occurs
//
{
    //
    // Just in case, this code supports more than sizeof(unsigned long) * CHAR_BIT count of NUMA-node
    //
    constexpr size_t BITS_PER_ULONG = sizeof(unsigned long) * CHAR_BIT;
    size_t nodeMaskSize = (numaNodeId + 1 + BITS_PER_ULONG - 1) / BITS_PER_ULONG; // to apply the ceiling

    std::vector<unsigned long> nodeMask(nodeMaskSize, 0x0);
    const size_t maskId = numaNodeId / BITS_PER_ULONG;
    const int maskShift = numaNodeId % BITS_PER_ULONG;
    nodeMask[maskId] = static_cast<unsigned long>(0x1) << maskShift;

    if (sysCallMBind(memory,
                     size,
                     MPOL_BIND, // Memory Policy: Bind to the particular NUMA-node
                     nodeMask.data(),
                     nodeMask.size() * BITS_PER_ULONG,
                     0) != 0) {
        munmap(memory, size);
        std::ostringstream ostr;
        ostr << "numaNodeMBInd() sysCallMBind() failed. numaNodeId:" << numaNodeId << " size:" << size;
        throw scene_rdl2::except::RuntimeError(ostr.str());
    }
    return memory;
}
#endif // end of !PLATFORM_APPLE

#ifndef PLATFORM_APPLE
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
#endif // end of !PLATFORM_APPLE

void*
mmapMemory(const size_t size)
//
// Might throw an exception except::RuntimeError() if an error occurs
//
{
    // mmap returned address is always aligned by pagesize.
#ifdef PLATFORM_APPLE
    void* const memory = mmap(nullptr,
                              size,
                              PROT_READ | PROT_WRITE,
                              MAP_PRIVATE | MAP_ANON,
                              -1,
                              0);
#else // !PLATFORM_APPLE
    void* const memory = mmap(nullptr,
                              size,
                              PROT_READ | PROT_WRITE,
                              MAP_PRIVATE | MAP_ANONYMOUS,
                              -1,
                              0);
#endif // end of !PLATFORM_APPLE
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
    const long pageSize = sysconf(_SC_PAGESIZE); // This works on both of Linux and MacOS
    if (pageSize == -1) return 0;
    return static_cast<size_t>(pageSize);
}

} // namespace

//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------

namespace scene_rdl2 {
namespace grid_util {

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
// Might throw an exception except::RuntimeError(std::string) if an error occurs
//
{
#ifdef PLATFORM_APPLE
    return mmapMemory(size);
#else // !PLATFORM_APPLE
    return numaNodeMBind(mNodeId, mmapMemory(size), size);
#endif // end of !PLATFORM_APPLE
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
// Might throw an exception except::RuntimeError(std::string) if an error occurs
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
#ifdef PLATFORM_APPLE
    return true; // always true
#else // !PLATFORM_APPLE    
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
#endif // end of !PLATFORM_APPLE
}

bool
NumaNode::isBelongCpu(const unsigned cpuId) const
{
    if (isEmptyCPU()) return false;

#ifdef PLATFORM_APPLE
    return true; // always true
#else // !PLATFORM_APPLE    
    if (cpuId < mCpuIdList.front() || mCpuIdList.back() < cpuId) return false;
    return std::find(mCpuIdList.begin(), mCpuIdList.end(), cpuId) != mCpuIdList.end();
#endif // end of !PLATFORM_APPLE
}

bool
NumaNode::alignmentSizeCheck(const size_t alignment) const
//
// All the memory is allocated by mmap and the address is always aligned by pagesize.
// This function checks that the requested alignment is valid by pagesize alignment.
// We can safely use the address allocated by NumaNode::alloc() as an aligned address
// if the aligned size is passed by this test.
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
         << str_util::addIndent(scene_rdl2::grid_util::CpuSocketUtil::showCpuIdTbl("mCpuIdList", mCpuIdList)) << '\n'
         << str_util::addIndent(showDistanceTbl()) << '\n'
         << "}";
    return ostr.str();
}

//------------------------------------------------------------------------------------------

NumaUtil::NumaUtil() // Might throw an exception scene_rdl2::except::RuntimeError() if an error occurs
{
    reset("localhost");

    parserConfigure();
}

void
NumaUtil::reset(const std::string& modeStr)
{
    const std::vector<unsigned> nodeIdTbl = getNumaNodeIdTbl(modeStr); // sorted

    mNumaNodeTbl.clear();
    const unsigned totalNode = *(std::max_element(nodeIdTbl.begin(), nodeIdTbl.end())) + 1;
    for (unsigned id = 0; id < nodeIdTbl.size(); ++id) {
        mNumaNodeTbl.emplace_back(nodeIdTbl[id],
                                  totalNode,
                                  getNumaNodeMemSize(modeStr, id),
                                  getNumaNodeCpuIdTbl(modeStr, id),
                                  getNumaNodeDistance(modeStr, id));
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
#ifdef PLATFORM_APPLE
    std::vector<unsigned> numaNodeIdTbl(1, 0);
    return numaNodeIdTbl;
#else // !PLATFORM_APPLE    
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
#endif // end of !PLATFORM_APPLE
}

// static function
unsigned
NumaUtil::findNumaNodeByMemAddr(void* addr)
{
#ifdef PLATFORM_APPLE
    return 0; // return nodeId = 0
#else // !PLATFORM_APPLE    
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
#endif // end of !PLATFORM_APPLE
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

void
NumaUtil::parserConfigure()
{
    mParser.description("NumaUtil command");

    mParser.opt("show", "", "show all info",
                [&](Arg& arg) { return arg.msg(show() + '\n'); });
    mParser.opt("reset", "<localhost|ag|tin|cobalt>", "reset internal mSocketInfoTbl by given argument mode",
                [&](Arg& arg) {
                    const std::string modeStr = (arg++)();
                    return resetCmd(modeStr, [&](const std::string& msg) { return arg.msg(msg); });
                });
}

bool
NumaUtil::resetCmd(const std::string& modeStr, const MsgFunc& msgCallBack)
{
    try {
        reset(modeStr);
    }
    catch (const except::RuntimeError& e) {
        std::ostringstream ostr;
        ostr << "reset() failed. error=>{\n"
             << str_util::addIndent(e.what()) << '\n'
             << "}";
        msgCallBack(ostr.str() + '\n');
        return false;
    }
    return true;
}

} // namespace grid_util
} // namespace scene_rdl2
