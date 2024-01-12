// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "ValueContainerDequeue.h"

namespace scene_rdl2 {
namespace cache {

class CacheDequeue : private ValueContainerDequeue
{
public:
    using ValueContainerDequeue::seekSet;
    using ValueContainerDequeue::deq;          // template
    using ValueContainerDequeue::deqBool;
    using ValueContainerDequeue::deqChar;
    using ValueContainerDequeue::deqFloat;
    using ValueContainerDequeue::deqFloat12;
    using ValueContainerDequeue::deqDouble;
    using ValueContainerDequeue::deqString;
    using ValueContainerDequeue::deqByteData;
    using ValueContainerDequeue::deqVector;    // template
    using ValueContainerDequeue::deqVLInt;     // 32bit
    using ValueContainerDequeue::deqVLUInt;
    using ValueContainerDequeue::deqVLLong;    // 64bit
    using ValueContainerDequeue::deqVLULong;
    using ValueContainerDequeue::deqVLSizeT;   // = deqVLULong
    using ValueContainerDequeue::getRestSize;
    using ValueContainerDequeue::getCurrDataAddress;
    using ValueContainerDequeue::deqAlignPad;

    enum class SizeCheckMode : int {
        DISABLE = 0,
        ENABLE
    };

    explicit CacheDequeue(const void *addr, const size_t dataSize) :
        ValueContainerDequeue(addr, dataSize),
        mSkipDataTotal(0)
    {}

    explicit CacheDequeue(const void *addr, const size_t dataSize, SizeCheckMode sizeCheck) :
        ValueContainerDequeue(addr, dataSize, (sizeCheck == SizeCheckMode::DISABLE)? false: true),
        mSkipDataTotal(0)
    {}

    inline const void *skipByteData(const size_t dataSize)
    {
        mSkipDataTotal += dataSize; // for statistical analyze
        return ValueContainerDequeue::skipByteData(dataSize);
    }

    inline bool isSameEncodedData(const CacheDequeue &src) const
    {
        return ValueContainerDequeue::isSameEncodedData(src);
    }

    std::string show() const;

private:

    size_t mSkipDataTotal;      // byte : for statistical analyze purpose.
};

} // namespace cache
} // namespace scene_rdl2
