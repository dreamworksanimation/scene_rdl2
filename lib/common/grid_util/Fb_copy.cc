// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "Fb.h"

namespace scene_rdl2 {
namespace grid_util {

void Fb::copy(const PartialMergeTilesTbl* partialMergeTilesTbl,
              const Fb& src)
{
    init(src.getRezedViewport());

    copyRenderBuffer(partialMergeTilesTbl, src);
    copyPixelInfo(partialMergeTilesTbl, src);
    copyHeatMap(partialMergeTilesTbl, src);
    copyWeightBuffer(partialMergeTilesTbl, src);
    copyRenderBufferOdd(partialMergeTilesTbl, src);
    copyRenderOutput(partialMergeTilesTbl, src);
}

void Fb::copyRenderBuffer(const PartialMergeTilesTbl* partialMergeTilesTbl,
                          const Fb& src)
{
    operatorOnPartialTiles(partialMergeTilesTbl, [&](int tileId) {
            copyRenderBufferOneTile(src, tileId);
        });
}

void Fb::copyPixelInfo(const PartialMergeTilesTbl* partialMergeTilesTbl,
                       const Fb& src)
{
    if (!src.getPixelInfoStatus()) return;
    setupPixelInfo(partialMergeTilesTbl, src.getPixelInfoName());

    operatorOnPartialTiles(partialMergeTilesTbl, [&](int tileId) {
            copyPixelInfoOneTile(src, tileId);                
        });
} 

void Fb::copyHeatMap(const PartialMergeTilesTbl* partialMergeTilesTbl,
                     const Fb& src)
{
    if (!src.getHeatMapStatus()) return;
    setupHeatMap(partialMergeTilesTbl, src.getHeatMapName());

    operatorOnPartialTiles(partialMergeTilesTbl, [&](int tileId) {
            copyHeatMapOneTile(src, tileId);
        });
}

void Fb::copyWeightBuffer(const PartialMergeTilesTbl* partialMergeTilesTbl,
                          const Fb& src)
{
    if (!src.getWeightBufferStatus()) return;
    setupWeightBuffer(partialMergeTilesTbl, src.getWeightBufferName());

    operatorOnPartialTiles(partialMergeTilesTbl, [&](int tileId) {
            copyWeightBufferOneTile(src, tileId);
        });
}

void
Fb::copyRenderBufferOdd(const PartialMergeTilesTbl* partialMergeTilesTbl, const Fb& src)
{
    if (!src.getRenderBufferOddStatus()) return;
    setupRenderBufferOdd(partialMergeTilesTbl);

    operatorOnPartialTiles(partialMergeTilesTbl, [&](int tileId) {
            copyRenderBufferOddOneTile(src, tileId);
        });
}

void Fb::copyRenderOutput(const PartialMergeTilesTbl* partialMergeTilesTbl,
                          const Fb& srcFb)
// This API always copy numSampleData internally.
{
    if (!srcFb.getRenderOutputStatus()) return;

    operatorOnAllActiveAov(srcFb, [&](const FbAovShPtr& srcFbAov, FbAovShPtr& dstFbAov) {
            // activeAovFunc
            if (srcFbAov->getReferenceType() == FbReferenceType::UNDEF) {
                // Non-Reference type buffer
                // We have to update fbAov information and copy data based on activeTile information

                // need to setup default value before call setup()
                dstFbAov->setDefaultValue(srcFbAov->getDefaultValue());

                // This API always copy numSampleData
                constexpr bool storeNumSampleData = true;

                dstFbAov->setup(partialMergeTilesTbl,
                                srcFbAov->getFormat(),
                                srcFbAov->getWidth(),
                                srcFbAov->getHeight(), // setup memory and clean if needed
                                storeNumSampleData);

                // setup closestFilter condition
                dstFbAov->setClosestFilterStatus(srcFbAov->getClosestFilterStatus());

                switch (srcFbAov->getFormat()) {
                case VariablePixelBuffer::FLOAT :
                    operatorOnPartialTiles(partialMergeTilesTbl, [&](int tileId) {
                            copyFloat1AovOneTile(dstFbAov, srcFbAov, tileId);
                        });
                    break;
                case VariablePixelBuffer::FLOAT2 :
                    operatorOnPartialTiles(partialMergeTilesTbl, [&](int tileId) {
                            copyFloat2AovOneTile(dstFbAov, srcFbAov, tileId);
                        });
                    break;
                case VariablePixelBuffer::FLOAT3 :
                    operatorOnPartialTiles(partialMergeTilesTbl, [&](int tileId) {
                            copyFloat3AovOneTile(dstFbAov, srcFbAov, tileId);
                        });
                    break;
                case VariablePixelBuffer::FLOAT4 :
                    operatorOnPartialTiles(partialMergeTilesTbl, [&](int tileId) {
                            copyFloat4AovOneTile(dstFbAov, srcFbAov, tileId);
                        });
                    break;
                default :
                    break;
                }
            } else {
                // Reference type buffer
                // Just setup fbAov w/ referenceType information. We don't have any actual data
                // for reference buffer type inside fbAov.
                dstFbAov->setup(srcFbAov->getReferenceType());
            }
        });
}

//------------------------------------------------------------------------------------------

// static function
std::string
Fb::showPartialMergeTilesTbl(const PartialMergeTilesTbl& tbl)
{
    auto calcActiveTiles = [&]() {
        unsigned total = 0;
        for (unsigned i = 0; i < tbl.size(); ++i) {
            if (tbl[i]) total++;
        }
        return total;
    };

    std::ostringstream ostr;
    ostr << "PartialMergeTilesTbl (tblSize:" << tbl.size() << ", activeSize:" << calcActiveTiles() << ") {";
    {
        unsigned startId, endId;
        auto isInit = [&]() -> bool { return (startId == tbl.size()); };
        auto initRange = [&]() { startId = endId = tbl.size(); };
        auto setRange = [&](unsigned i) { startId= endId = i; };
        auto expandRange = [&](unsigned i) { endId = i; };
        auto flushRange = [&]() {
            if (startId != tbl.size()) {
                if (startId == endId) ostr << ' ' << startId;
                else ostr << ' ' << startId << '-' << endId;
            }
            initRange();
        };

        initRange();
        for (unsigned i = 0; i < tbl.size(); ++i) {
            if (tbl[i]) {
                if (isInit()) setRange(i);
                else expandRange(i);
            } else {
                flushRange();
            }
        }
        flushRange();
    }
    ostr << " }";

    return ostr.str();
}

//------------------------------------------------------------------------------------------

template <typename T>
void Fb::copyTile(T* dstFirstValOfTile,
                  unsigned int* dstFirstNumSampleTotalOfTile,
                  uint64_t srcMask,
                  const T* srcFirstValOfTile,
                  const unsigned int* srcFirstNumSampleTotalOfTile) const
{
    operatorOnActivePixOfTile(srcMask, [&](unsigned pixId) {
            T& currDstVal = dstFirstValOfTile[pixId];
            unsigned int& currDstNumSampleTotal = dstFirstNumSampleTotalOfTile[pixId];
            const T& currSrcVal = srcFirstValOfTile[pixId];
            const unsigned int& currSrcNumSampleTotal = srcFirstNumSampleTotalOfTile[pixId];

            currDstVal = currSrcVal;
            currDstNumSampleTotal = currSrcNumSampleTotal;
        });
}

void Fb::copyRenderBufferOneTile(const Fb& src, const int tileId)
{
    operatorOnActiveOneTile
        (mActivePixels,
         src.mActivePixels,
         tileId,
         [&](uint64_t srcMask, int pixOffset) { // accumulateTile function
            copyTile(mRenderBufferTiled.getData() + pixOffset,
                     mNumSampleBufferTiled.getData() + pixOffset,
                     srcMask,
                     src.mRenderBufferTiled.getData() + pixOffset,
                     src.mNumSampleBufferTiled.getData() + pixOffset);
        });
}

void Fb::copyPixelInfoOneTile(const Fb& src, const int tileId)
{
    operatorOnActiveOneTile
        (mActivePixelsPixelInfo,
         src.mActivePixelsPixelInfo,
         tileId,
         [&](uint64_t srcMask, int pixOffset) { // accumulateTile function
            copyPixelInfoTile(mPixelInfoBufferTiled.getData() + pixOffset,
                              srcMask,
                              src.mPixelInfoBufferTiled.getData() + pixOffset);
        });
}

void Fb::copyHeatMapOneTile(const Fb& src, const int tileId)
{
    operatorOnActiveOneTile
        (mActivePixelsHeatMap,
         src.mActivePixelsHeatMap,
         tileId,
         [&](uint64_t srcMask, int pixOffset) { // accumulateTile function
            copyTile(mHeatMapSecBufferTiled.getData() + pixOffset,
                     mHeatMapNumSampleBufferTiled.getData() + pixOffset,
                     srcMask,
                     src.mHeatMapSecBufferTiled.getData() + pixOffset,
                     src.mHeatMapNumSampleBufferTiled.getData() + pixOffset);
        });
}

void Fb::copyWeightBufferOneTile(const Fb& src, const int tileId)
{
    operatorOnActiveOneTile
        (mActivePixelsWeightBuffer,
         src.mActivePixelsWeightBuffer,
         tileId,
         [&](uint64_t srcMask, int pixOffset) { // accumulateTile function
            copyWeightBufferTile(mWeightBufferTiled.getData() + pixOffset,
                                 srcMask,
                                 src.mWeightBufferTiled.getData() + pixOffset);
        });
}

void
Fb::copyRenderBufferOddOneTile(const Fb& src, const int tileId)
{
    operatorOnActiveOneTile
        (mActivePixelsRenderBufferOdd,
         src.mActivePixelsRenderBufferOdd,
         tileId,
         [&](uint64_t srcMask, int pixOffset) { // accumulateTile function
            copyTile(mRenderBufferOddTiled.getData() + pixOffset,
                     mRenderBufferOddNumSampleBufferTiled.getData() + pixOffset,
                     srcMask,
                     src.mRenderBufferOddTiled.getData() + pixOffset,
                     src.mRenderBufferOddNumSampleBufferTiled.getData() + pixOffset);
        });
}

void Fb::copyFloat1AovOneTile(FbAovShPtr& dstFbAov, const FbAovShPtr& srcFbAov, const int tileId)
{
    operatorOnActiveOneTile
        (dstFbAov->getActivePixels(),
         srcFbAov->getActivePixels(),
         tileId,
         [&](uint64_t srcMask, int pixOffset) { // accumulateTile function
            copyTile(dstFbAov->getBufferTiled().getFloatBuffer().getData() + pixOffset,
                     dstFbAov->getNumSampleBufferTiled().getData() + pixOffset,
                     srcMask,
                     srcFbAov->getBufferTiled().getFloatBuffer().getData() + pixOffset,
                     srcFbAov->getNumSampleBufferTiled().getData() + pixOffset);
        });
}

void Fb::copyFloat2AovOneTile(FbAovShPtr& dstFbAov, const FbAovShPtr& srcFbAov, const int tileId)
{
    operatorOnActiveOneTile
        (dstFbAov->getActivePixels(),
         srcFbAov->getActivePixels(),
         tileId,
         [&](uint64_t srcMask, int pixOffset) { // accumulateTile function
            copyTile
                (dstFbAov->getBufferTiled().getFloat2Buffer().getData() + pixOffset,
                 dstFbAov->getNumSampleBufferTiled().getData() + pixOffset,
                 srcMask,
                 srcFbAov->getBufferTiled().getFloat2Buffer().getData() + pixOffset,
                 srcFbAov->getNumSampleBufferTiled().getData() + pixOffset);
        });
}

void Fb::copyFloat3AovOneTile(FbAovShPtr& dstFbAov, const FbAovShPtr& srcFbAov, const int tileId)
{
    operatorOnActiveOneTile
        (dstFbAov->getActivePixels(),
         srcFbAov->getActivePixels(),
         tileId,
         [&](uint64_t srcMask, int pixOffset) { // accumulateTile function
            copyTile
                (dstFbAov->getBufferTiled().getFloat3Buffer().getData() + pixOffset,
                 dstFbAov->getNumSampleBufferTiled().getData() + pixOffset,
                 srcMask,
                 srcFbAov->getBufferTiled().getFloat3Buffer().getData() + pixOffset,
                 srcFbAov->getNumSampleBufferTiled().getData() + pixOffset);
        });
}

void Fb::copyFloat4AovOneTile(FbAovShPtr& dstFbAov, const FbAovShPtr& srcFbAov, const int tileId)
{
    operatorOnActiveOneTile
        (dstFbAov->getActivePixels(),
         srcFbAov->getActivePixels(),
         tileId,
         [&](uint64_t srcMask, int pixOffset) { // accumulateTile function
            copyTile
                (dstFbAov->getBufferTiled().getFloat4Buffer().getData() + pixOffset,
                 dstFbAov->getNumSampleBufferTiled().getData() + pixOffset,
                 srcMask,
                 srcFbAov->getBufferTiled().getFloat4Buffer().getData() + pixOffset,
                 srcFbAov->getNumSampleBufferTiled().getData() + pixOffset);
        });
}

void Fb::copyPixelInfoTile(PixelInfo* dstFirstPixelInfoOfTile,
                           uint64_t srcMask,
                           const PixelInfo* srcFirstPixelInfoOfTile) const
{
    operatorOnActivePixOfTile(srcMask, [&](unsigned pixId) {
            PixelInfo& currDstPixelInfo = dstFirstPixelInfoOfTile[pixId];
            const PixelInfo& currSrcPixelInfo = srcFirstPixelInfoOfTile[pixId];
            currDstPixelInfo = currSrcPixelInfo;
        });
}

void
Fb::copyWeightBufferTile(float* dstFirstPixelInfoOfTile,
                         uint64_t srcMask,
                         const float* srcFirstPixelInfoOfTile) const
{
    operatorOnActivePixOfTile(srcMask, [&](unsigned pixId) {
            float& currDstPixelInfo = dstFirstPixelInfoOfTile[pixId];
            const float& currSrcPixelInfo = srcFirstPixelInfoOfTile[pixId];
            currDstPixelInfo = currSrcPixelInfo;
        });
}

} // namespace grid_util
} // namespace scene_rdl2
