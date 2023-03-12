// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#pragma once

#include <scene_rdl2/scene/rdl2/ValueContainerDeq.h>

namespace scene_rdl2 {
namespace cache {

class CacheDequeue : private rdl2::ValueContainerDeq
{
public:
    using rdl2::ValueContainerDeq::seekSet;
    using rdl2::ValueContainerDeq::deq;          // template
    using rdl2::ValueContainerDeq::deqBool;
    using rdl2::ValueContainerDeq::deqFloat12;
    using rdl2::ValueContainerDeq::deqString;
    using rdl2::ValueContainerDeq::deqVector;    // template
    using rdl2::ValueContainerDeq::deqVLInt;
    using rdl2::ValueContainerDeq::deqVLUInt;
    using rdl2::ValueContainerDeq::deqVLSizeT;
    using rdl2::ValueContainerDeq::getRestSize;
    using rdl2::ValueContainerDeq::getCurrDataAddress;
    using rdl2::ValueContainerDeq::deqAlignPad;

    enum class SizeCheckMode : int {
        DISABLE = 0,
        ENABLE
    };

    explicit CacheDequeue(const void *addr, const size_t dataSize) :
        rdl2::ValueContainerDeq(addr, dataSize),
        mSkipDataTotal(0)
    {}

    explicit CacheDequeue(const void *addr, const size_t dataSize, SizeCheckMode sizeCheck) :
        rdl2::ValueContainerDeq(addr, dataSize, (sizeCheck == SizeCheckMode::DISABLE)? false: true),
        mSkipDataTotal(0)
    {}

    inline const void *skipByteData(const size_t dataSize)
    {
        mSkipDataTotal += dataSize; // for statistical analyze
        return rdl2::ValueContainerDeq::skipByteData(dataSize);
    }

    inline bool isSameEncodedData(const CacheDequeue &src) const
    {
        return rdl2::ValueContainerDeq::isSameEncodedData(src);
    }

    std::string show() const;

private:

    size_t mSkipDataTotal;      // byte : for statistical analyze purpose.
};

} // namespace cache
} // namespace scene_rdl2

