// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

//
// -- AOV frame buffer information --
//
// This FbAov is stored one AOV related frame buffer information which include ActivePixels
//

#include "FbReferenceType.h"
#include "PackTilesPassPrecision.h"

#include <scene_rdl2/common/fb_util/ActivePixels.h>
#include <scene_rdl2/common/fb_util/FbTypes.h>
#include <scene_rdl2/common/fb_util/Tiler.h>
#include <scene_rdl2/common/fb_util/VariablePixelBuffer.h>

#include <scene_rdl2/common/platform/Platform.h> // finline

#include <tbb/parallel_for.h>

#include <memory>               // shared_ptr

// Basically we should use multi-thread version.
// This single thread mode is used debugging and performance comparison reason mainly.
//#define SINGLE_THREAD

namespace scene_rdl2 {
    namespace math {
        class Viewport;
    }
}

namespace scene_rdl2 {
namespace grid_util {

class FbAov
{
public:
    using ActivePixels = fb_util::ActivePixels;
    using VariablePixelBuffer = fb_util::VariablePixelBuffer;
    using NumSampleBuffer = fb_util::PixelBuffer<unsigned int>;

    using FbAovShPtr = std::shared_ptr<FbAov>;

    using PartialMergeTilesTbl = std::vector<char>;

    using FArray = std::vector<float>;
    using UCArray = std::vector<unsigned char>;

    FbAov(const std::string &aovName) :
        mStatus(true),
        mAovName(aovName),
        mReferenceType(FbReferenceType::UNDEF),
        mDefaultValue(0.0f),
        mClosestFilterStatus(false),
        mCoarsePassPrecision(CoarsePassPrecision::F32),
        mFinePassPrecision(FinePassPrecision::F32)
    {}

    // debugTag for debugging purposes
    void setDebugTag(const std::string& debugTag) { mDebugTag = debugTag; }
    const std::string& getDebugTag() const { return mDebugTag; }

    void setDefaultValue(float v) { mDefaultValue = v; }
    float getDefaultValue() const { return mDefaultValue; }
    
    // setup function for non reference buffer and only do memory allocation and clean if needed
    void setup(const PartialMergeTilesTbl *partialMergeTilesTbl,
               fb_util::VariablePixelBuffer::Format fmt, const unsigned width, const unsigned height,
               bool storeNumSampleData);

    // setup function for reference buffer.
    void setup(FbReferenceType referenceType);

    FbReferenceType getReferenceType() const { return mReferenceType; }

    void setClosestFilterStatus(bool flag) { mClosestFilterStatus = flag; }
    bool getClosestFilterStatus() const { return mClosestFilterStatus; }

    void setCoarsePassPrecision(CoarsePassPrecision p) { mCoarsePassPrecision = p; }
    CoarsePassPrecision getCoarsePassPrecision() const { return mCoarsePassPrecision; }
    void setFinePassPrecision(FinePassPrecision p) { mFinePassPrecision = p; }
    FinePassPrecision getFinePassPrecision() const { return mFinePassPrecision; }

    void setActive() { mStatus = true; }
    void reset() { mStatus = false; }
    bool garbageCollectUnusedBuffers(); // return current mStatus

    bool getStatus() const { return mStatus; }
    VariablePixelBuffer::Format getFormat() const { return mBufferTiled.getFormat(); }
    unsigned getWidth() const { return mActivePixels.getWidth(); }
    unsigned getHeight() const { return mActivePixels.getHeight(); }
    const std::string &getAovName() const { return mAovName; }
    int getNumChan() const;

    ActivePixels &getActivePixels() { return mActivePixels; }
    VariablePixelBuffer &getBufferTiled() { return mBufferTiled; }
    NumSampleBuffer &getNumSampleBufferTiled() { return mNumSampleBufferTiled; }
    int getPix(int sx, int sy, std::vector<float> &out) const; // return numChan
    std::string showInfo() const; // return detailed AOV data information as string

    int untile(const bool isSrgb,  // use gamma2.2 when isSrgb = false
               const bool top2bottom,
               const math::Viewport *roi,
               const bool closestFilterDepthOutput,
               std::vector<unsigned char> &rgbFrame) const; // return numChan
    
    int untile(const bool top2bottom,
               const math::Viewport *roi,
               const bool closestFilterDepthOutput,
               std::vector<float> &data) const; // return numChan

    // Special untile function for denoise operation. Set data into float 4 pixel buffer (rgba)
    int untileF4(const bool top2bottom,
                 const math::Viewport *roi,
                 const bool closestFilterDepthOutput,
                 std::vector<float>& data) const; // Returns original data's numChan

    void conv888(const FArray &srcData,
                 const bool isSrgb, // use gamma2.2 when isSrgb = false
                 const bool closestFilterDepthOutput,
                 UCArray &dstRgb888) const; // for debug purpose function

    unsigned nonDefaultPixelTotalFloat() const; // for debug
    unsigned nonZeroNumSamplePixelTotal() const; // for debug

    bool isDepthRelatedAov() const;
    bool isPositionRelatedAov() const;
    bool isBeautyRelatedAov() const;

protected:

    std::string mDebugTag; // for debugging purposes

    bool mStatus; // active status on/off
                  // off means this data is not used
                  // on includes Reference != UNDEF condition.

    std::string mAovName;
    FbReferenceType mReferenceType;
    float mDefaultValue;
    bool mClosestFilterStatus; // Condition of this aov data uses closestFilter or not
    CoarsePassPrecision mCoarsePassPrecision; // required coarse pass precision for packTile codec
    FinePassPrecision mFinePassPrecision;     // required fine pass precision for packTile codec

    ActivePixels mActivePixels;
    VariablePixelBuffer mBufferTiled;      // tiled format : tile aligned resolution : normalized value
    NumSampleBuffer mNumSampleBufferTiled; // tiled format : tile aligned resolution

    //------------------------------

    template <typename F>
    void activeTileCrawler(const F tileFunc) const
    {
        for (unsigned tileId = 0; tileId < mActivePixels.getNumTiles(); ++tileId) {
            uint64_t tileMask = mActivePixels.getTileMask(tileId);
            if (tileMask) {
                int pixOffset = tileId << 6;
                tileFunc(tileMask, pixOffset);
            }
        }
    }

    template <typename T, typename F>
    void activePixelCrawler(uint64_t tileMask, const T *firstDataOfTile, F pixFunc) const
    {
        for (unsigned y = 0; y < 8; ++y) {
            unsigned offset = (y << 3); // y * 8

            uint64_t currTileMask = tileMask >> offset;
            if (!currTileMask) break; // early exit : rest of them are all empty

            const T *currPix = firstDataOfTile + offset;

            uint64_t currTileScanlineMask =
                currTileMask & static_cast<uint64_t>(0xff); // get one scanline mask
            for (unsigned x = 0; x < 8; ++x) {
                if (!currTileScanlineMask) break; // early exit for scanline

                if (currTileScanlineMask & static_cast<uint64_t>(0x1)) {
                    pixFunc(*currPix);
                }
                currPix ++;
                currTileScanlineMask >>= 1;
            }
        }
    }

    template <typename F>
    void
    partialMergeTilesTblCrawler(const PartialMergeTilesTbl &partialMergeTilesTbl, F resetTileFunc) const
    {
        for (size_t tileId = 0; tileId < partialMergeTilesTbl.size(); ++tileId) {
            if (partialMergeTilesTbl[tileId]) {
                uint pixOffset = static_cast<uint>(tileId << 6);
                resetTileFunc(pixOffset);
            }
        }
    }

    template <typename T>
    void
    bufferTileClear(T *dstFirstValOfTile) const
    {
        std::memset(dstFirstValOfTile, 0x0, sizeof(T) * 64);
    }

    finline void resetActivePixels(const PartialMergeTilesTbl *partialMergeTilesTbl);
    finline void resetNumSampleBufferTiled(const PartialMergeTilesTbl *partialMergeTilesTbl);
    finline void resetBufferTiled(const PartialMergeTilesTbl *partialMergeTilesTbl);

    // for debug of partial reset of each internal buffer data
    bool runtimeVerifySetup(const std::string &msg, const PartialMergeTilesTbl *partialMergeTilesTbl) const;
    bool runtimeVerifySetupTilesBufferTiled(const PartialMergeTilesTbl *partialMergeTilesTbl) const;
    bool runtimeVerifySetupNumSampleBufferTiled(const PartialMergeTilesTbl *partialMergeTilesTbl) const;

    void conv888_computeDepthMinMax(const FArray &srcData,
                                    unsigned pixFloatCount, unsigned depthId,
                                    float &min, float &max) const; // for debug
    void conv888_computePositionMinMax(const FArray &srcData,
                                       unsigned pixFloatCount,
                                       math::Vec3f &min, math::Vec3f &max) const; // for debug

    template <typename T> float floatComponentAccess(T &v, unsigned id) const;
    template <typename T> void computeDepthMinMax(const T *tiledBufferStartAddr, int depthId,
                                                  float &min, float &max) const;
    template <typename T> void computePositionMinMax(const T *tiledBufferStartAddr,
                                                     unsigned calcComponentTotal,
                                                     T &min, T &max) const;

    const char* showVariablePixelBufferFormat(VariablePixelBuffer::Format format) const;
}; // FbAov

finline void
FbAov::resetActivePixels(const PartialMergeTilesTbl *partialMergeTilesTbl)
{
    if (!partialMergeTilesTbl) {
        mActivePixels.reset();
    } else {
        mActivePixels.reset(*partialMergeTilesTbl);
    }
}

finline void
FbAov::resetNumSampleBufferTiled(const PartialMergeTilesTbl *partialMergeTilesTbl)
{
    if (!partialMergeTilesTbl) {
        mNumSampleBufferTiled.clear();
    } else {
        partialMergeTilesTblCrawler
            (*partialMergeTilesTbl,
             [&](unsigned pixOffset) {
                bufferTileClear(mNumSampleBufferTiled.getData() + pixOffset);
            });
    }
}

finline void
FbAov::resetBufferTiled(const PartialMergeTilesTbl *partialMergeTilesTbl)
{
    if (!partialMergeTilesTbl) {
        mBufferTiled.clear();
    } else {
        switch (mBufferTiled.getFormat()) {
        case VariablePixelBuffer::FLOAT :
            partialMergeTilesTblCrawler
                (*partialMergeTilesTbl,
                 [&](unsigned pixOffset) {
                    bufferTileClear(mBufferTiled.getFloatBuffer().getData() + pixOffset);
                });
            break;
        case VariablePixelBuffer::FLOAT2 :
            partialMergeTilesTblCrawler
                (*partialMergeTilesTbl,
                 [&](unsigned pixOffset) {
                    bufferTileClear(mBufferTiled.getFloat2Buffer().getData() + pixOffset);
                });
            break;
        case VariablePixelBuffer::FLOAT3 :
            partialMergeTilesTblCrawler
                (*partialMergeTilesTbl,
                 [&](unsigned pixOffset) {
                    bufferTileClear(mBufferTiled.getFloat3Buffer().getData() + pixOffset);
                });
            break;
        case VariablePixelBuffer::FLOAT4 :
            partialMergeTilesTblCrawler
                (*partialMergeTilesTbl,
                 [&](unsigned pixOffset) {
                    bufferTileClear(mBufferTiled.getFloat4Buffer().getData() + pixOffset);
                });
            break;
        default :
            break;
        }
    }
}

} // namespace grid_util
} // namespace scene_rdl2
