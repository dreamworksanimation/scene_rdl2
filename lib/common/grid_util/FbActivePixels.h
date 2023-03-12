// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#pragma once

//
// -- Active pixel information --
//
// FbActivePixels stores all active pixel information for beauty, pixelInfo, heatMap and AOVs.
// This data is mainly used for result of fb snapshot operation and indicates which pixel is different between
// previous result.
//

#include "FbActivePixelsAov.h"

#include <scene_rdl2/common/fb_util/ActivePixels.h>
#include <scene_rdl2/common/platform/Platform.h> // finline

#include <functional>
#include <memory>               // shared_ptr
#include <mutex>
#include <unordered_map>

namespace scene_rdl2 {
namespace grid_util {

class FbActivePixels
{
public:
    using ActivePixels = fb_util::ActivePixels;
    
    using FbActivePixelsAovShPtr = std::shared_ptr<FbActivePixelsAov>;

    FbActivePixels() :
        mWidth(0), mHeight(0), mAlignedWidth(0), mAlignedHeight(0),
        mPixelInfoStatus(false), mHeatMapStatus(false), mWeightBufferStatus(false),
        mRenderBufferOddStatus(false),
        mRenderOutputStatus(false)
    {}

    // set false to pixelInfo/heatMap/weightBuffer/renderOutput condition
    finline void init(const unsigned width, const unsigned height);

    finline void initPixelInfo();       // should be called after init()
    finline void initHeatMap();         // should be called after init()
    finline void initWeightBuffer();    // should be called after init()
    finline void initRenderBufferOdd(); // should be called after init()

    // void reset(); // not used yet
    // void garbageCollectUnusedBuffers(); // not used yet

    ActivePixels &getActivePixels() { return mActivePixels; }
    ActivePixels &getActivePixelsPixelInfo() { return mActivePixelsPixelInfo; }
    ActivePixels &getActivePixelsHeatMap() { return mActivePixelsHeatMap; }
    ActivePixels &getActivePixelsWeightBuffer() { return mActivePixelsWeightBuffer; }
    ActivePixels &getActivePixelsRenderBufferOdd() { return mActivePixelsRenderBufferOdd; }
    finline FbActivePixelsAovShPtr &getAov(const std::string &aovName); // MTsafe

    finline void updateRenderOutputStatus(std::function<bool(const std::string &aovName,
                                                             bool status)> evalStatusFunc);
    finline void activeRenderOutputCrawler(std::function<void(const std::string &aovName,
                                                              const ActivePixels &activePixels)> func);

    std::string showAllAov() const; // for debug

protected:
    unsigned mWidth;
    unsigned mHeight;

    unsigned mAlignedWidth;     // tile aligned (8 pixel) width
    unsigned mAlignedHeight;    // tile aligned (8 pixel) height

    ActivePixels mActivePixels; // for renderBuffer (beauty/alpha) buffer

    bool mPixelInfoStatus;
    ActivePixels mActivePixelsPixelInfo; // for pixelInfo

    bool mHeatMapStatus;
    ActivePixels mActivePixelsHeatMap;   // for heatMap

    bool mWeightBufferStatus;
    ActivePixels mActivePixelsWeightBuffer; // for weightBuffer

    bool mRenderBufferOddStatus;
    ActivePixels mActivePixelsRenderBufferOdd; // for renderBufferOdd (beautyAux/alphaAux)

    bool mRenderOutputStatus;
    std::unordered_map<std::string, FbActivePixelsAovShPtr> mActivePixelsRenderOutput; // for renderOutput
    std::mutex mMutex;

    //------------------------------

    finline void resetAllRenderOutput();
}; // FbActivePixels

finline void
FbActivePixels::init(const unsigned width, const unsigned height)
{
    mWidth = width;
    mHeight = height;
    mAlignedWidth = (width + 7) & ~7;
    mAlignedHeight = (height + 7) & ~7;

    mActivePixels.init(mWidth, mHeight);
    mActivePixels.reset();

    mPixelInfoStatus = false;
    mHeatMapStatus = false;
    mWeightBufferStatus = false;
    mRenderBufferOddStatus = false;
    resetAllRenderOutput();
}

finline void
FbActivePixels::initPixelInfo()
//
// You should call init() first before call this initPixelInfo()
//
{
    mPixelInfoStatus = true;
    mActivePixelsPixelInfo.init(mWidth, mHeight);
    mActivePixelsPixelInfo.reset();
}

finline void
FbActivePixels::initHeatMap()
//
// You should call init() first before call this initHeatMap()
//
{
    mHeatMapStatus = true;
    mActivePixelsHeatMap.init(mWidth, mHeight);
    mActivePixelsHeatMap.reset();
}

finline void
FbActivePixels::initWeightBuffer()
//
// You should call init() first before call this initWeightBuffer()
//
{
    mWeightBufferStatus = true;
    mActivePixelsWeightBuffer.init(mWidth, mHeight);
    mActivePixelsWeightBuffer.reset();
}

finline void
FbActivePixels::initRenderBufferOdd()
//
// You should call init() first before call this initRenderBufferOdd()
//
{
    mRenderBufferOddStatus = true;
    mActivePixelsRenderBufferOdd.init(mWidth, mHeight);
    mActivePixelsRenderBufferOdd.reset();
}

finline FbActivePixels::FbActivePixelsAovShPtr &
FbActivePixels::getAov(const std::string &aovName)
{
    std::lock_guard<std::mutex> lock(mMutex);

    if (mActivePixelsRenderOutput.find(aovName) == mActivePixelsRenderOutput.end()) {
        // Very first time to access this AOV -> create
        mActivePixelsRenderOutput[aovName] = FbActivePixelsAovShPtr(new FbActivePixelsAov(aovName));
    }
    mActivePixelsRenderOutput[aovName]->setActive();
    mRenderOutputStatus = true;

    return mActivePixelsRenderOutput[aovName];
}

finline void
FbActivePixels::updateRenderOutputStatus(std::function<bool(const std::string &aovName, bool status)> evalStatusFunc)
{
    int totalActiveAov = 0;
    for (auto &itr : mActivePixelsRenderOutput) {
        if (evalStatusFunc((itr.second)->getAovName(), (itr.second)->getStatus())) {
            totalActiveAov++;
        } else {
            (itr.second)->reset(); // mark as non-active (and not free internal memory yet)
        }
    }
    mRenderOutputStatus = (totalActiveAov)? true: false;
}

finline void
FbActivePixels::activeRenderOutputCrawler(std::function<void(const std::string &aovName,
                                                             const ActivePixels &activePixels)> func)
{
    for (auto &itr : mActivePixelsRenderOutput) {
        if (!(itr.second)->getStatus()) continue; // skip non active aov
        func((itr.second)->getAovName(), (itr.second)->getActivePixels());
    }
}

finline void
FbActivePixels::resetAllRenderOutput()
{
    for (auto &itr : mActivePixelsRenderOutput) {
        (itr.second)->reset();
    }
    mRenderOutputStatus = false;
}

} // namespace grid_util
} // namespace scene_rdl2

