// Copyright 2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <sched.h> // cpu_set_t
#include <string>

namespace scene_rdl2 {
#ifdef __APPLE__
#define CPU_ZERO(a)
#define CPU_SET(a, b)
#define CPU_EQUAL(a,b) false
#define CPU_ISSET(a, b) false
#define CPU_AND(a, b, c)
#define CPU_OR(a, b, c)
#define CPU_COUNT(a) 0
#define CPU_SETSIZE 1024
typedef uint64_t cpu_set_t;
#endif

class CpuAffinityMask
//
// This wrapper class of cpu_set_t mask is used to set up CPU affinity control
//
{
public:
    CpuAffinityMask(); // Throw except::RuntimeError if internally failed.
    CpuAffinityMask(const CpuAffinityMask& src);

    CpuAffinityMask& operator = (const CpuAffinityMask& src) { new(this) CpuAffinityMask(src); return *this; }

    void reset() { CPU_ZERO(&mMask); }

    // Caller need to set bindCpuId by proper range. (0 ~ mNumCpu - 1).
    void set(const unsigned bindCpuId) // You can call multiple times
    {
        if (bindCpuId < mNumCpu) CPU_SET(bindCpuId, &mMask);
    }
    void setFull(); // set all cpuId

    bool isEmpty() const { return CPU_COUNT(&mMask) == 0; }
    bool isSet(unsigned cpuId) const { return (cpuId < mNumCpu) ? CPU_ISSET(cpuId, &mMask) : false; }
    bool isSame(const CpuAffinityMask& mask) const;

    unsigned getNumCpu() const { return mNumCpu; }
    size_t getMaskSize() const { return mMaskSize; }
    cpu_set_t* getMaskPtr() { return &mMask; }

    std::string showMask() const; // for debugging

private:
    const unsigned mNumCpu {0};
    const size_t mMaskSize {0}; // byte
    cpu_set_t mMask;
};

} // namespace scene_rdl2
