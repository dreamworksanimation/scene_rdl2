// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <scene_rdl2/scene/rdl2/ValueContainerEnq.h>

#include <vector>

namespace scene_rdl2 {
namespace cache {

class CacheEnqueue : private rdl2::ValueContainerEnq
{
public:
    using rdl2::ValueContainerEnq::enq;         // template
    using rdl2::ValueContainerEnq::enqBool;
    using rdl2::ValueContainerEnq::enqFloat;
    using rdl2::ValueContainerEnq::enqFloat12;
    using rdl2::ValueContainerEnq::enqString;
    using rdl2::ValueContainerEnq::enqByteData;
    using rdl2::ValueContainerEnq::enqAlignPad;
    using rdl2::ValueContainerEnq::enqVector;   // template
    using rdl2::ValueContainerEnq::enqVLInt;    // 32bit
    using rdl2::ValueContainerEnq::enqVLUInt;
    using rdl2::ValueContainerEnq::enqVLLong;   // 64bit
    using rdl2::ValueContainerEnq::enqVLULong;
    using rdl2::ValueContainerEnq::enqVLSizeT;  // = enqVLULong
    using rdl2::ValueContainerEnq::enqReserveMem;
    using rdl2::ValueContainerEnq::finalize;
    using rdl2::ValueContainerEnq::currentSize;

    explicit CacheEnqueue(std::string *bytes) :
        rdl2::ValueContainerEnq(bytes),
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
