// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#pragma once

//
// -- Active pixel information for AOV buffer --
//
// This FbActivePixelsAov is stored active pixel information for AOV buffer.
// This data is mainly used for result of fb snapshot operation and indicates which pixel is different between
// previous result.
//

#include "FbReferenceType.h"

#include <scene_rdl2/common/fb_util/ActivePixels.h>
#include <scene_rdl2/common/platform/Platform.h> // finline

namespace scene_rdl2 {
namespace grid_util {

class FbActivePixelsAov {
public:
    using ActivePixels = fb_util::ActivePixels;

    FbActivePixelsAov(const std::string &aovName) :
        mStatus(true),
        mAovName(aovName),
        mReferenceType(FbReferenceType::UNDEF)
    {}

    void setActive() { mStatus = true; }
    void reset() { mStatus = false; }
    finline void init(const unsigned width, const unsigned height); // for regular AOV buffer (i.e. non reference type)
    finline void init(const FbReferenceType referenceType); // for reference type AOV buffer
    finline bool garbageCollectUnusedBuffers(); // return active condition (i.e. = mStatus)

    bool getStatus() const { return mStatus; }
    const std::string &getAovName() const { return mAovName; }

    ActivePixels &getActivePixels() { return mActivePixels; }

protected:
    bool mStatus;               // active or not
    std::string mAovName;

    FbReferenceType mReferenceType;

    ActivePixels mActivePixels;
}; // FbActivePixelsAov

finline void
FbActivePixelsAov::init(const unsigned width, const unsigned height)
{
    mStatus = true;
    mReferenceType = FbReferenceType::UNDEF;
    mActivePixels.init(width, height);
    mActivePixels.reset();
}

finline void
FbActivePixelsAov::init(const FbReferenceType referenceType)
{
    mStatus = true;
    mReferenceType = referenceType;
    mActivePixels.cleanUp();
}

finline bool
FbActivePixelsAov::garbageCollectUnusedBuffers()
{
    if (!mStatus) {
        mAovName.clear();
        mAovName.shrink_to_fit();

        mActivePixels.cleanUp();
    }

    return mStatus;
}

} // namespace grid_util
} // namespace scene_rdl2

