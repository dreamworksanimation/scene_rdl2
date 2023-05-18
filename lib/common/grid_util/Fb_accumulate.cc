// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#include "Fb.h"
#include <scene_rdl2/render/logging/logging.h>

namespace scene_rdl2 {
namespace grid_util {

void
Fb::accumulateRenderBuffer(const PartialMergeTilesTbl* partialMergeTilesTbl,
                           const Fb& src)
{
    operatorOnPartialTiles(partialMergeTilesTbl, [&](int tileId) {
            accumulateRenderBufferOneTile(src, tileId);

            /* for debug
            if (!partialMergeTilesTbl) {
                verifyAccumulateNumSampleTile(tileId, src, "X X X X X");
            }
            */
        });

    /* for debug
    if (!partialMergeTilesTbl) {
        std::cerr << ">> Fb_accumulate.cc accumulateRenderBuffer() runtime verify start\n";
        verifyAccumulateNumSample(src, "Y Y Y Y Y");
    }
    */
}

void
Fb::accumulatePixelInfo(const PartialMergeTilesTbl* partialMergeTilesTbl,
                        const Fb& src)
{
    if (!src.getPixelInfoStatus()) return;
    setupPixelInfo(partialMergeTilesTbl, src.getPixelInfoName());

    operatorOnPartialTiles(partialMergeTilesTbl, [&](int tileId) {
            accumulatePixelInfoOneTile(src, tileId);                
        });
} 

void
Fb::accumulateHeatMap(const PartialMergeTilesTbl* partialMergeTilesTbl,
                      const Fb& src)
{
    if (!src.getHeatMapStatus()) return;
    setupHeatMap(partialMergeTilesTbl, src.getHeatMapName());

    operatorOnPartialTiles(partialMergeTilesTbl, [&](int tileId) {
            accumulateHeatMapOneTile(src, tileId);
        });
}

void
Fb::accumulateWeightBuffer(const PartialMergeTilesTbl* partialMergeTilesTbl, const Fb& src)
{
    if (!src.getWeightBufferStatus()) return;
    setupWeightBuffer(partialMergeTilesTbl, src.getWeightBufferName());

    operatorOnPartialTiles(partialMergeTilesTbl, [&](int tileId) {
            accumulateWeightBufferOneTile(src, tileId);
        });
}

void
Fb::accumulateRenderBufferOdd(const PartialMergeTilesTbl* partialMergeTilesTbl, const Fb& src)
{
    if (!src.getRenderBufferOddStatus()) return;
    setupRenderBufferOdd(partialMergeTilesTbl);

    operatorOnPartialTiles(partialMergeTilesTbl, [&](int tileId) {
            accumulateRenderBufferOddOneTile(src, tileId);
        });
}

void
Fb::accumulateRenderOutput(const PartialMergeTilesTbl* partialMergeTilesTbl,
                           const Fb& srcFb)
// This function is used on progmcrt_merge computation
{
    if (!srcFb.getRenderOutputStatus()) return;

    operatorOnAllActiveAov(srcFb, [&](const FbAovShPtr& srcFbAov, FbAovShPtr& dstFbAov) {
            // activeAovFunc
            if (srcFbAov->getReferenceType() == FbReferenceType::UNDEF) {
                // Non-Reference type buffer
                // We have to update fbAov information and accumulate data based on activeTile information

                // need to setup default value before call setup()
                dstFbAov->setDefaultValue(srcFbAov->getDefaultValue());

                // We always need to process numSampleData on merge computation
                constexpr bool storeNumSampleData = true;

                dstFbAov->setup(partialMergeTilesTbl,
                                srcFbAov->getFormat(),
                                srcFbAov->getWidth(),
                                srcFbAov->getHeight(), // setup memory and clean if needed
                                storeNumSampleData);

                // setup closestFilter condition
                dstFbAov->setClosestFilterStatus(srcFbAov->getClosestFilterStatus());

                // We always update numSampleData here regardless of storeNumSampleData condition.
                switch (srcFbAov->getFormat()) {
                case VariablePixelBuffer::FLOAT :
                    operatorOnPartialTiles(partialMergeTilesTbl, [&](int tileId) {
                            accumulateFloat1AovOneTile(dstFbAov, srcFbAov, tileId);
                        });
                    break;
                case VariablePixelBuffer::FLOAT2 :
                    operatorOnPartialTiles(partialMergeTilesTbl, [&](int tileId) {
                            accumulateFloat2AovOneTile(dstFbAov, srcFbAov, tileId);
                        });
                    break;
                case VariablePixelBuffer::FLOAT3 :
                    operatorOnPartialTiles(partialMergeTilesTbl, [&](int tileId) {
                            accumulateFloat3AovOneTile(dstFbAov, srcFbAov, tileId);
                        });
                    break;
                case VariablePixelBuffer::FLOAT4 :
                    operatorOnPartialTiles(partialMergeTilesTbl, [&](int tileId) {
                            accumulateFloat4AovOneTile(dstFbAov, srcFbAov, tileId);
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

void
Fb::accumulateAllFbs(const int numMachines,
                     const std::vector<char>& received,
                     const std::vector<grid_util::Fb>& srcFbs)
//
// Test function for tile base MT task distribution. Still work in progress.
// This function is used on progmcrt_merge computation
//
{
    auto bufferSetupFunc = [&](unsigned bufferId, const Fb& src) {
        switch (bufferId) {
        case 0 : {
            if (src.getPixelInfoStatus()) setupPixelInfo(nullptr, src.getPixelInfoName());
        } break;
        case 1 : {
            if (src.getHeatMapStatus()) setupHeatMap(nullptr, src.getHeatMapName());
        } break;
        case 2 : {
            if (src.getWeightBufferStatus()) setupWeightBuffer(nullptr, src.getWeightBufferName());
        } break;
        case 3 : {
            if (src.getRenderBufferOddStatus()) setupRenderBufferOdd(nullptr);
        } break;
        case 4 : {
            if (src.getRenderOutputStatus()) {
                operatorOnAllActiveAov
                    (src,
                     [&](const FbAovShPtr& srcFbAov, FbAovShPtr& dstFbAov) {
                        if (srcFbAov->getReferenceType() == FbReferenceType::UNDEF) {
                            // Non-Reference type buffer
                            // We have to update fbAov information and accumulate data based on
                            // activeTile information

                            // We always need to process numSampleData on merge computation
                            constexpr bool storeNumSampleData = true;

                            // need to setup default value before call setup()
                            dstFbAov->setDefaultValue(srcFbAov->getDefaultValue());
                            dstFbAov->setup(nullptr,
                                            srcFbAov->getFormat(),
                                            srcFbAov->getWidth(),
                                            srcFbAov->getHeight(), // setup memory and clean if needed
                                            storeNumSampleData);

                            // setup closestFilter condition
                            dstFbAov->setClosestFilterStatus(srcFbAov->getClosestFilterStatus());

                        } else {
                            // Reference type buffer
                            // Just setup fbAov w/ referenceType information.
                            // We don't have any actual data for reference buffer type inside fbAov.
                            dstFbAov->setup(srcFbAov->getReferenceType());
                        }
                    });
            }
        } break;
        }
    };

    // setup all buffer memory first. This should be done for every machineId once
    // (not for every tile of each machineId).
    for (int machineId = 0; machineId < numMachines; ++machineId) {
        if (!received[machineId]) continue;        
        const Fb& src = srcFbs[machineId];

#       ifdef SINGLE_THREAD
        for (unsigned int bufferId = 0; bufferId < 5; ++bufferId) {
            bufferSetupFunc(bufferId, src);
        }
#       else // else SINGLE_THREAD
        // probably we should try to parallel run for machineId loop as well. : TODO
        tbb::parallel_for(0, 5, [&](unsigned bufferId) { bufferSetupFunc(bufferId, src); });
#       endif // end !SINGLE_THREAD        
    }

    // merge all buffers
    operatorOnPartialTiles(nullptr, [&](int tileId) {
            for (int machineId = 0; machineId < numMachines; ++machineId) {
                if (!received[machineId]) continue;

                const Fb& src = srcFbs[machineId];

                accumulateRenderBufferOneTile(src, tileId);
                if (src.getPixelInfoStatus()) accumulatePixelInfoOneTile(src, tileId);
                if (src.getHeatMapStatus()) accumulatePixelInfoOneTile(src, tileId);
                if (src.getWeightBufferStatus()) accumulateWeightBufferOneTile(src, tileId);
                if (src.getRenderBufferOddStatus()) accumulateRenderBufferOddOneTile(src, tileId);
                if (src.getRenderOutputStatus()) {
                    operatorOnAllActiveAov(src, [&](const FbAovShPtr& srcFbAov, FbAovShPtr& dstFbAov) {
                            if (srcFbAov->getReferenceType() != FbReferenceType::UNDEF) return;

                            switch (srcFbAov->getFormat()) {
                            case VariablePixelBuffer::FLOAT :
                                accumulateFloat1AovOneTile(dstFbAov, srcFbAov, tileId);
                                break;
                            case VariablePixelBuffer::FLOAT2 :
                                accumulateFloat2AovOneTile(dstFbAov, srcFbAov, tileId);
                                break;
                            case VariablePixelBuffer::FLOAT3 :
                                accumulateFloat3AovOneTile(dstFbAov, srcFbAov, tileId);
                                break;
                            case VariablePixelBuffer::FLOAT4 :
                                accumulateFloat4AovOneTile(dstFbAov, srcFbAov, tileId);
                                break;
                            default :
                                break;
                            }
                        });
                }
            }
        });
}

//---------------------------------------------------------------------------------------------------------------

template <typename T>
void
Fb::accumulateTile(T* dstFirstValOfTile,
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

            unsigned int totalSample = currDstNumSampleTotal + currSrcNumSampleTotal;
            T ave;
            if (totalSample > 0) {
                ave =
                    (currDstVal * static_cast<float>(currDstNumSampleTotal) +
                     currSrcVal * static_cast<float>(currSrcNumSampleTotal)) /
                    static_cast<float>(totalSample);
            } else {
                // just in case
                std::memset(reinterpret_cast<void *>(&ave), 0x0, sizeof(T));
            }

            currDstVal = ave;
            currDstNumSampleTotal = totalSample;
        });
}

template <typename T>
void
Fb::accumulateTileClosestFilter(T* dstFirstValOfTile,
                                unsigned int* dstFirstNumSampleTotalOfTile,
                                uint64_t srcMask,
                                const T* srcFirstValOfTile,
                                const unsigned int* srcFirstNumSampleTotalOfTile) const
//
// special accumulateTile function for the case of using closestFilter
//
{
    unsigned int depthId = T::N - 1; // depth value is last component
    
    operatorOnActivePixOfTile(srcMask, [&](unsigned pixId) { // operatePixFunc
            T& currDstVal = dstFirstValOfTile[pixId];
            unsigned int& currDstNumSampleTotal = dstFirstNumSampleTotalOfTile[pixId];
            const T& currSrcVal = srcFirstValOfTile[pixId];
            const unsigned int& currSrcNumSampleTotal = srcFirstNumSampleTotalOfTile[pixId];

            unsigned int totalSample = currDstNumSampleTotal + currSrcNumSampleTotal;
            if (totalSample > 0) {
                if (currDstNumSampleTotal == 0) {
                    currDstVal = currSrcVal;
                } else {
                    if (currSrcVal[depthId] < currDstVal[depthId]) {
                        // replace currDstVal if currSrcVal's closestFilter depth is closer
                        currDstVal = currSrcVal;
                    }
                }
                currDstNumSampleTotal = totalSample;
            }
        });
}

//---------------------------------------------------------------------------------------------------------------

void
Fb::accumulateRenderBufferOneTile(const Fb& src, const int tileId)
{
    operatorOnActiveOneTile
        (mActivePixels,
         src.mActivePixels,
         tileId,
         [&](uint64_t srcMask, int pixOffset) { // accumulateTile function
            accumulateTile(mRenderBufferTiled.getData() + pixOffset,
                           mNumSampleBufferTiled.getData() + pixOffset,
                           srcMask,
                           src.mRenderBufferTiled.getData() + pixOffset,
                           src.mNumSampleBufferTiled.getData() + pixOffset);
        });
}

void
Fb::accumulatePixelInfoOneTile(const Fb& src, const int tileId)
{
    operatorOnActiveOneTile
        (mActivePixelsPixelInfo,
         src.mActivePixelsPixelInfo,
         tileId,
         [&](uint64_t srcMask, int pixOffset) { // accumulateTile function
            accumulatePixelInfoTile(mPixelInfoBufferTiled.getData() + pixOffset,
                                    srcMask,
                                    src.mPixelInfoBufferTiled.getData() + pixOffset);
        });
}

void
Fb::accumulateHeatMapOneTile(const Fb& src, const int tileId)
{
    operatorOnActiveOneTile
        (mActivePixelsHeatMap,
         src.mActivePixelsHeatMap,
         tileId,
         [&](uint64_t srcMask, int pixOffset) { // accumulateTile function
            accumulateTile(mHeatMapSecBufferTiled.getData() + pixOffset,
                           mHeatMapNumSampleBufferTiled.getData() + pixOffset,
                           srcMask,
                           src.mHeatMapSecBufferTiled.getData() + pixOffset,
                           src.mHeatMapNumSampleBufferTiled.getData() + pixOffset);
        });
}

void
Fb::accumulateWeightBufferOneTile(const Fb& src, const int tileId)
{
    operatorOnActiveOneTile
        (mActivePixelsWeightBuffer,
         src.mActivePixelsWeightBuffer,
         tileId,
         [&](uint64_t srcMask, int pixOffset) { // accumulateTile function
            accumulateWeightBufferTile(mWeightBufferTiled.getData() + pixOffset,
                                       srcMask,
                                       src.mWeightBufferTiled.getData() + pixOffset);
        });
}

void
Fb::accumulateRenderBufferOddOneTile(const Fb& src, const int tileId)
{
    operatorOnActiveOneTile
        (mActivePixelsRenderBufferOdd,
         src.mActivePixelsRenderBufferOdd,
         tileId,
         [&](uint64_t srcMask, int pixOffset) { // accumulateTile function
            accumulateTile(mRenderBufferOddTiled.getData() + pixOffset,
                           mRenderBufferOddNumSampleBufferTiled.getData() + pixOffset,
                           srcMask,
                           src.mRenderBufferOddTiled.getData() + pixOffset,
                           src.mRenderBufferOddNumSampleBufferTiled.getData() + pixOffset);
        });
}

void
Fb::accumulateFloat1AovOneTile(FbAovShPtr& dstFbAov, const FbAovShPtr& srcFbAov, const int tileId)
{
    operatorOnActiveOneTile
        (dstFbAov->getActivePixels(),
         srcFbAov->getActivePixels(),
         tileId,
         [&](uint64_t srcMask, int pixOffset) { // accumulateTile function
            accumulateTile(dstFbAov->getBufferTiled().getFloatBuffer().getData() + pixOffset,
                           dstFbAov->getNumSampleBufferTiled().getData() + pixOffset,
                           srcMask,
                           srcFbAov->getBufferTiled().getFloatBuffer().getData() + pixOffset,
                           srcFbAov->getNumSampleBufferTiled().getData() + pixOffset);
        });
}

void
Fb::accumulateFloat2AovOneTile(FbAovShPtr& dstFbAov, const FbAovShPtr& srcFbAov, const int tileId)
{
    operatorOnActiveOneTile
        (dstFbAov->getActivePixels(),
         srcFbAov->getActivePixels(),
         tileId,
         [&](uint64_t srcMask, int pixOffset) { // accumulateTile function
            if (srcFbAov->getClosestFilterStatus()) {
                accumulateTileClosestFilter
                    (dstFbAov->getBufferTiled().getFloat2Buffer().getData() + pixOffset,
                     dstFbAov->getNumSampleBufferTiled().getData() + pixOffset,
                     srcMask,
                     srcFbAov->getBufferTiled().getFloat2Buffer().getData() + pixOffset,
                     srcFbAov->getNumSampleBufferTiled().getData() + pixOffset);
            } else {
                accumulateTile
                    (dstFbAov->getBufferTiled().getFloat2Buffer().getData() + pixOffset,
                     dstFbAov->getNumSampleBufferTiled().getData() + pixOffset,
                     srcMask,
                     srcFbAov->getBufferTiled().getFloat2Buffer().getData() + pixOffset,
                     srcFbAov->getNumSampleBufferTiled().getData() + pixOffset);
            }
        });
}

void
Fb::accumulateFloat3AovOneTile(FbAovShPtr& dstFbAov, const FbAovShPtr& srcFbAov, const int tileId)
{
    operatorOnActiveOneTile
        (dstFbAov->getActivePixels(),
         srcFbAov->getActivePixels(),
         tileId,
         [&](uint64_t srcMask, int pixOffset) { // accumulateTile function
            if (srcFbAov->getClosestFilterStatus()) {
                accumulateTileClosestFilter
                    (dstFbAov->getBufferTiled().getFloat3Buffer().getData() + pixOffset,
                     dstFbAov->getNumSampleBufferTiled().getData() + pixOffset,
                     srcMask,
                     srcFbAov->getBufferTiled().getFloat3Buffer().getData() + pixOffset,
                     srcFbAov->getNumSampleBufferTiled().getData() + pixOffset);
            } else {
                accumulateTile
                    (dstFbAov->getBufferTiled().getFloat3Buffer().getData() + pixOffset,
                     dstFbAov->getNumSampleBufferTiled().getData() + pixOffset,
                     srcMask,
                     srcFbAov->getBufferTiled().getFloat3Buffer().getData() + pixOffset,
                     srcFbAov->getNumSampleBufferTiled().getData() + pixOffset);
            }
        });
}

void
Fb::accumulateFloat4AovOneTile(FbAovShPtr& dstFbAov, const FbAovShPtr& srcFbAov, const int tileId)
{
    operatorOnActiveOneTile
        (dstFbAov->getActivePixels(),
         srcFbAov->getActivePixels(),
         tileId,
         [&](uint64_t srcMask, int pixOffset) { // accumulateTile function
            if (srcFbAov->getClosestFilterStatus()) {
                accumulateTileClosestFilter
                    (dstFbAov->getBufferTiled().getFloat4Buffer().getData() + pixOffset,
                     dstFbAov->getNumSampleBufferTiled().getData() + pixOffset,
                     srcMask,
                     srcFbAov->getBufferTiled().getFloat4Buffer().getData() + pixOffset,
                     srcFbAov->getNumSampleBufferTiled().getData() + pixOffset);
            } else {
                accumulateTile
                    (dstFbAov->getBufferTiled().getFloat4Buffer().getData() + pixOffset,
                     dstFbAov->getNumSampleBufferTiled().getData() + pixOffset,
                     srcMask,
                     srcFbAov->getBufferTiled().getFloat4Buffer().getData() + pixOffset,
                     srcFbAov->getNumSampleBufferTiled().getData() + pixOffset);
            }
        });
}

void
Fb::accumulatePixelInfoTile(PixelInfo* dstFirstPixelInfoOfTile,
                            uint64_t srcMask,
                            const PixelInfo* srcFirstPixelInfoOfTile) const
{
    operatorOnActivePixOfTile(srcMask, [&](unsigned pixId) {
            PixelInfo& currDstPixelInfo = dstFirstPixelInfoOfTile[pixId];
            const PixelInfo& currSrcPixelInfo = srcFirstPixelInfoOfTile[pixId];

            if (currDstPixelInfo.depth > currSrcPixelInfo.depth) {
                currDstPixelInfo.depth = currSrcPixelInfo.depth;
            }
        });
}

void
Fb::accumulateWeightBufferTile(float* dstFirstPixelInfoOfTile,
                               uint64_t srcMask,
                               const float* srcFirstPixelInfoOfTile) const
{
    operatorOnActivePixOfTile(srcMask, [&](unsigned pixId) {
            float& currDstPixelInfo = dstFirstPixelInfoOfTile[pixId];
            const float& currSrcPixelInfo = srcFirstPixelInfoOfTile[pixId];

            currDstPixelInfo += currSrcPixelInfo;
        });
}

bool
Fb::verifyAccumulateNumSampleTile(uint64_t srcMask,
                                  const unsigned int* srcFirstNumSampleTotalOfTile,
                                  const unsigned int* dstFirstNumSampleTotalOfTile,
                                  const std::string& msg) const
{
    bool resultFlag = true;
    operatorOnActivePixOfTile(srcMask, [&](unsigned pixId) {
            unsigned int srcNumSample = srcFirstNumSampleTotalOfTile[pixId];
            unsigned int dstNumSample = dstFirstNumSampleTotalOfTile[pixId];
            if (dstNumSample < srcNumSample) {
                std::cerr << ">> Fb_accumulate.cc accumulateTile() RUNTIME numSample verify failed."
                          << " localPixId:" << pixId
                          << " srcNumSample:" << srcNumSample
                          << " dstNumSample:" << dstNumSample
                          << ' ' << msg << '\n';
                resultFlag = false;
            }
        });
    return resultFlag;
}
    
bool
Fb::verifyAccumulateNumSampleTile(int tileId, const Fb& src, const std::string& msg) const
{
    uint64_t srcMask = mActivePixels.getTileMask(tileId);
    int pixOffset = tileId << 6;
    const unsigned int* srcFirstNumSampleTotalOfTile = src.mNumSampleBufferTiled.getData() + pixOffset;
    const unsigned int* dstFirstNumSampleTotalOfTile = mNumSampleBufferTiled.getData() + pixOffset;
    return verifyAccumulateNumSampleTile(srcMask, srcFirstNumSampleTotalOfTile, dstFirstNumSampleTotalOfTile, msg);
}

bool
Fb::verifyAccumulateNumSample(const Fb& src, const std::string& msg) const
{
    for (int tileId = 0; tileId < static_cast<int>(getTotalTiles()); ++tileId) {
        if (!verifyAccumulateNumSampleTile(tileId, src, msg)) return false;
    }
    return true;
}

} // namespace grid_util
} // namespace scene_rdl2

