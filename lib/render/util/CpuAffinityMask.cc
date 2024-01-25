// Copyright 2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "CpuAffinityMask.h"

#include <scene_rdl2/common/except/exceptions.h>
#include <scene_rdl2/render/util/StrUtil.h>

#include <cassert>
#include <sstream>
#include <thread>

namespace scene_rdl2 {

CpuAffinityMask::CpuAffinityMask()
    : mNumCpu { std::thread::hardware_concurrency() }
    , mMaskSize { sizeof(cpu_set_t) }
{
    // If we need more capacity than CPU_SETSIZE, we should allocate the expected memory using CPU_ALLOC
    if (mNumCpu > CPU_SETSIZE) {
        std::ostringstream ostr;
        ostr << "ERROR : CpuAffinityMask() constructor mNumCpu:" << mNumCpu << " > maxSize:" << CPU_SETSIZE;
        throw except::RuntimeError(ostr.str());
    }
    reset();
}

CpuAffinityMask::CpuAffinityMask(const CpuAffinityMask& src)
    : mNumCpu {src.mNumCpu}
    , mMaskSize {src.mMaskSize}
{
    memcpy(&mMask, &src.mMask, src.mMaskSize);
}

void
CpuAffinityMask::setFull()
{
    for (unsigned cpuId = 0; cpuId < mNumCpu; ++cpuId) {
        CPU_SET(cpuId, &mMask);
    }
}

bool
CpuAffinityMask::isSame(const CpuAffinityMask& mask) const
{
    if (mNumCpu != mask.mNumCpu ||
        mMaskSize != mask.mMaskSize) {
        return false;
    }
    return (CPU_EQUAL(&mMask, &mask.mMask) != 0);
}

std::string
CpuAffinityMask::showMask() const
{
    auto showCpuIdRange = [&](const int w, const int startCpuId, int endCpuId) {
        std::ostringstream ostr;
        endCpuId = (endCpuId >= mNumCpu) ? mNumCpu - 1 : endCpuId; 
        ostr << "cpuId(" << std::setw(w) << endCpuId << '~' << std::setw(w) << startCpuId << ")";
        return ostr.str();
    };
    auto showMaskRangeBit = [&](const int startCpuId, const int endCpuId) {
        auto showBit = [&](const int cpuId) -> std::string {
            return (cpuId >= mNumCpu) ? " " : (isSet(cpuId) ? "1" : "0");
        };
        auto showBitSeparator = [](const int cpuId) -> std::string {
            return (cpuId % 16 == 0) ? "/" : ((cpuId % 4 == 0) ? "-" : "");
        };
        std::ostringstream ostr;
        ostr << "bit(";
        for (int cpuId = endCpuId; cpuId >= startCpuId; --cpuId) {
            ostr << showBit(cpuId) << ((cpuId != startCpuId) ? showBitSeparator(cpuId) : "");
        }
        ostr << ')';
        return ostr.str();
    };
    auto showMaskRangeHex = [&](const int startCpuId, const int endCpuId) {
        assert(startCpuId <= endCpuId);
        std::ostringstream ostr;
        ostr << "hex(";
        int v = 0x0;
        int b = 0x1 << ((endCpuId - startCpuId) % 4);
        for (int cpuId = endCpuId; cpuId >= startCpuId; --cpuId) {
            if (cpuId < mNumCpu && isSet(cpuId)) v |= b;
            b >>= 1;
            if (b == 0) {
                if (v != 0x0) ostr << std::hex << v;
                else          ostr << ' ';
                if (cpuId > startCpuId && cpuId % 16 == 0) ostr << '-';
                v = 0x0;
                b = 0x8;
            }
        }
        ostr << ')';
        return ostr.str();
    };
    auto showRange = [&](const int w, const int startCpuId, const int endCpuId) {
        std::ostringstream ostr;
        ostr
        << showCpuIdRange(w, startCpuId, endCpuId) << ' '
        << showMaskRangeBit(startCpuId, endCpuId) << ' '
        << showMaskRangeHex(startCpuId, endCpuId);
        return ostr.str();
    };

    constexpr int cpuRangeSize = 32; // single line displays 32 cpu info
    int rangeLoopCount = mNumCpu / cpuRangeSize;
    if (mNumCpu % cpuRangeSize != 0) ++rangeLoopCount;
    int w = str_util::getNumberOfDigits(mNumCpu - 1);

    std::ostringstream ostr;
    ostr << "CpuAffinityMask (cpuTotal:" << mNumCpu << ") {\n";
    for (int rangeLoopId = rangeLoopCount - 1; rangeLoopId >= 0; --rangeLoopId) {
        int startRangeId = rangeLoopId * cpuRangeSize;
        int endRangeId = startRangeId + cpuRangeSize - 1;
        ostr << str_util::addIndent(showRange(w, startRangeId, endRangeId)) << '\n';
    }
    ostr << "}";
    return ostr.str();
}

} // namespace scene_rdl2
