// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "ValueContainerEnqueue.h"

#include <vector>

namespace scene_rdl2 {
namespace cache {

class CacheEnqueue : private ValueContainerEnqueue
{
public:
    using ValueContainerEnqueue::enq;         // template
    using ValueContainerEnqueue::enqBool;
    using ValueContainerEnqueue::enqChar;
    using ValueContainerEnqueue::enqFloat;
    using ValueContainerEnqueue::enqFloat12;
    using ValueContainerEnqueue::enqDouble;
    using ValueContainerEnqueue::enqString;
    using ValueContainerEnqueue::enqByteData;
    using ValueContainerEnqueue::enqAlignPad;
    using ValueContainerEnqueue::enqVector;   // template
    using ValueContainerEnqueue::enqVLInt;    // 32bit
    using ValueContainerEnqueue::enqVLUInt;
    using ValueContainerEnqueue::enqVLLong;   // 64bit
    using ValueContainerEnqueue::enqVLULong;
    using ValueContainerEnqueue::enqVLSizeT;  // = enqVLULong
    using ValueContainerEnqueue::enqReserveMem;
    using ValueContainerEnqueue::finalize;
    using ValueContainerEnqueue::currentSize;

    explicit CacheEnqueue(std::string *bytes) :
        ValueContainerEnqueue(bytes),
        mRuntimeVerify(false)
    {}

    //------------------------------
    //
    // analyze scene purpose APIs for debug
    //
    void incrementPrimitiveTypeCounter(const unsigned typeId);
    const std::vector<unsigned> &getPrimitiveTypeCounter() const { return mPrimitiveTypeCounter; }

    std::string show() const;
    std::string showDebug() const;

private:
    bool mRuntimeVerify; // not used yet.

    std::vector<unsigned> mPrimitiveTypeCounter; // statistical information for debug
};

} // namespace cache
} // namespace scene_rdl2
