// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#include "Fb.h"
#include <scene_rdl2/render/logging/logging.h>

namespace scene_rdl2 {
namespace grid_util {

void
Fb::accumulateRenderBuffer(const PartialMergeTilesTbl *partialMergeTilesTbl,
                           const Fb &src)
{
    accumulatePartialTiles(partialMergeTilesTbl, [&](int tileId) {
            accumulateRenderBufferOneTile(src, tileId);            
        });
}

void
Fb::accumulatePixelInfo(const PartialMergeTilesTbl *partialMergeTilesTbl,
                        const Fb &src)
{
    if (!src.getPixelInfoStatus()) return;
    setupPixelInfo(partialMergeTilesTbl, src.getPixelInfoName());

    accumulatePartialTiles(partialMergeTilesTbl, [&](int tileId) {
            accumulatePixelInfoOneTile(src, tileId);                
        });
} 

void
Fb::accumulateHeatMap(const PartialMergeTilesTbl *partialMergeTilesTbl,
                      const Fb &src)
{
    if (!src.getHeatMapStatus()) return;
    setupHeatMap(partialMergeTilesTbl, src.getHeatMapName());

    accumulatePartialTiles(partialMergeTilesTbl, [&](int tileId) {
            accumulateHeatMapOneTile(src, tileId);
        });
}

void
Fb::accumulateWeightBuffer(const PartialMergeTilesTbl *partialMergeTilesTbl, const Fb &src)
{
    if (!src.getWeightBufferStatus()) return;
    setupWeightBuffer(partialMergeTilesTbl, src.getWeightBufferName());

    accumulatePartialTiles(partialMergeTilesTbl, [&](int tileId) {
            accumulateWeightBufferOneTile(src, tileId);
        });
}

void
Fb::accumulateRenderBufferOdd(const PartialMergeTilesTbl *partialMergeTilesTbl, const Fb &src)
{
    if (!src.getRenderBufferOddStatus()) return;
    setupRenderBufferOdd(partialMergeTilesTbl);

    accumulatePartialTiles(partialMergeTilesTbl, [&](int tileId) {
            accumulateRenderBufferOddOneTile(src, tileId);
        });
}

void
Fb::accumulateRenderOutput(const PartialMergeTilesTbl *partialMergeTilesTbl,
                           const Fb &srcFb)
// This function is used on progmcrt_merge computation
{
    if (!srcFb.getRenderOutputStatus()) return;

    accumulateAllActiveAov(srcFb, [&](const FbAovShPtr &srcFbAov, FbAovShPtr &dstFbAov) {
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

                switch (srcFbAov->getFormat()) {
                case VariablePixelBuffer::FLOAT :
                    accumulatePartialTiles(partialMergeTilesTbl, [&](int tileId) {
                            accumulateFloat1AovOneTile(dstFbAov, srcFbAov, tileId);
                        });
                    break;
                case VariablePixelBuffer::FLOAT2 :
                    accumulatePartialTiles(partialMergeTilesTbl, [&](int tileId) {
                            accumulateFloat2AovOneTile(dstFbAov, srcFbAov, tileId);
                        });
                    break;
                case VariablePixelBuffer::FLOAT3 :
                    accumulatePartialTiles(partialMergeTilesTbl, [&](int tileId) {
                            accumulateFloat3AovOneTile(dstFbAov, srcFbAov, tileId);
                        });
                    break;
                case VariablePixelBuffer::FLOAT4 :
                    accumulatePartialTiles(partialMergeTilesTbl, [&](int tileId) {
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
                     const std::vector<char> &received,
                     const std::vector<grid_util::Fb> &srcFbs)
//
// Test function for tile base MT task distribution. Still work in progress.
// This function is used on progmcrt_merge computation
//
{
    auto bufferSetupFunc = [&](unsigned bufferId, const Fb &src) {
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
                accumulateAllActiveAov
                    (src,
                     [&](const FbAovShPtr &srcFbAov, FbAovShPtr &dstFbAov) {
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
        const Fb &src = srcFbs[machineId];

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
    accumulatePartialTiles(nullptr, [&](int tileId) {
            for (int machineId = 0; machineId < numMachines; ++machineId) {
                if (!received[machineId]) continue;

                const Fb &src = srcFbs[machineId];

                accumulateRenderBufferOneTile(src, tileId);
                if (src.getPixelInfoStatus()) accumulatePixelInfoOneTile(src, tileId);
                if (src.getHeatMapStatus()) accumulatePixelInfoOneTile(src, tileId);
                if (src.getWeightBufferStatus()) accumulateWeightBufferOneTile(src, tileId);
                if (src.getRenderBufferOddStatus()) accumulateRenderBufferOddOneTile(src, tileId);
                if (src.getRenderOutputStatus()) {
                    accumulateAllActiveAov(src, [&](const FbAovShPtr &srcFbAov, FbAovShPtr &dstFbAov) {
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

#ifdef SINGLE_THREAD
template <typename F>
void
Fb::accumulatePartialTiles(const PartialMergeTilesTbl *partialMergeTilesTbl,
                           F accumTileFunc) const
{
    if (!partialMergeTilesTbl) {
        // If partialMergeTilesTbl is empty, we accumulate all the tiles.
        for (int tileId = 0; tileId < static_cast<int>(getTotalTiles()); ++tileId) {
            accumTileFunc(tileId);
        }
    } else {
        // Only accumulate tile which specified by partialMergeTilesTbl
        for (int tileId = 0; tileId < static_cast<int>(getTotalTiles()); ++tileId) {
            if ((*partialMergeTilesTbl)[tileId]) {
                accumTileFunc(tileId);
            }
        }
    }
}
#else // else SINGLE_THREAD
template <typename F>
void
Fb::accumulatePartialTiles(const PartialMergeTilesTbl *partialMergeTilesTbl,
                           F accumTileFunc) const
{
    if (!partialMergeTilesTbl) {
        // If partialMergeTilesTbl is empty, we accumulate all the tiles.
        if (!getTotalTiles()) return;
        // Based on several different grain size test (2,4,16,32,64,128,256,512,1024,2048,4096)
        // and found 64 is somehow reasonable for 1K or more resolution image in this parallel_for loop
        tbb::blocked_range<size_t> range(0, getTotalTiles(), 64);
        tbb::parallel_for(range, [&](const tbb::blocked_range<size_t> &tileRange) {
                for (size_t tileId = tileRange.begin(); tileId < tileRange.end(); ++tileId) {
                    accumTileFunc(tileId);
                }
            });
    } else {
        // Only accumulate tile which specified by partialMergeTilesTbl
        std::vector<unsigned> partialMergeTilesId;
        for (size_t tileId = 0; tileId < partialMergeTilesTbl->size(); ++tileId) {
            if ((*partialMergeTilesTbl)[tileId]) {
                partialMergeTilesId.push_back(tileId);
            }
        }
        if (!partialMergeTilesId.size()) return;

        // Based on several different grain size test (2,4,16,32,64,128,256,512,1024,2048,4096)
        // and found 16 is somehow reasonable for 1K or more resolution image in this parallel_for loop
        tbb::blocked_range<size_t> range(0, partialMergeTilesId.size(), 16);
        tbb::parallel_for(range, [&](const tbb::blocked_range<size_t> &idRange) {
                for (size_t id = idRange.begin(); id < idRange.end(); ++id) {
                    accumTileFunc(partialMergeTilesId[id]);
                }
            });
    }
}
#endif // end !SINGLE_THREAD

#ifdef SINGLE_THREAD
template <typename F>
void
Fb::accumulateAllActiveAov(const Fb &srcFb, F activeAovFunc)
{
    for (const auto &itr : srcFb.mRenderOutput) {
        const FbAovShPtr &srcFbAov = itr.second;
        if (!srcFbAov->getStatus()) continue; // skip non active aov
        // real data AOV buffer or Reference type
        const std::string &aovName = srcFbAov->getAovName();

        FbAovShPtr &dstFbAov = getAov(aovName);
        activeAovFunc(srcFbAov, dstFbAov);
        mRenderOutputStatus = true;
    }
}
#else // else SINGLE_THREAD
template <typename F>
void
Fb::accumulateAllActiveAov(const Fb &srcFb, F activeAovFunc)
{
    std::vector<std::string> activeAovNameArray;
    for (const auto &itr : srcFb.mRenderOutput) {
        const FbAovShPtr &srcFbAov = itr.second;
        if (!srcFbAov->getStatus()) continue; // skip non active aov
        // real data AOV buffer or Reference type
        activeAovNameArray.push_back(srcFbAov->getAovName());
    }
    if (!activeAovNameArray.size()) return;

    tbb::blocked_range<size_t> range(0, activeAovNameArray.size());
    tbb::parallel_for(range, [&](const tbb::blocked_range<size_t> &r) {
            for (size_t activeAovNameId = r.begin(); activeAovNameId < r.end(); ++activeAovNameId) {
                const std::string &aovName = activeAovNameArray[activeAovNameId];
                if (!srcFb.findAov(aovName)) {
                    std::ostringstream ostr;
                    ostr << ">> ============ Fb.h findAov failed. aovName:>" << aovName << "<";
                    logging::Logger::error(ostr.str());
                    continue;
                }
                const FbAovShPtr &srcFbAov = srcFb.mRenderOutput.at(aovName);

                FbAovShPtr dstFbAov = getAov(aovName);
                activeAovFunc(srcFbAov, dstFbAov);
                mRenderOutputStatus = true;
            }
        });
}
#endif // end !SINGLE_THREAD

template <typename F>
void
Fb::accumulateActiveOneTile(ActivePixels &dstActivePixels,
                            const ActivePixels &srcActivePixels,
                            const int tileId,
                            F accumTileFunc) const
{
    int pixOffset = tileId << 6;

    uint64_t srcMask = srcActivePixels.getTileMask(tileId);
    if (srcMask) {
        uint64_t dstMask = dstActivePixels.getTileMask(tileId);
        dstMask |= srcMask; // update destination activePixels mask
        dstActivePixels.setTileMask(tileId, dstMask);

        accumTileFunc(srcMask, pixOffset);
    }
}

template <typename T>
void
Fb::accumulateTile(T *dstFirstValOfTile,
                   unsigned int *dstFirstNumSampleTotalOfTile,
                   uint64_t srcMask,
                   const T *srcFirstValOfTile,
                   const unsigned int *srcFirstNumSampleTotalOfTile) const
{
    T ave;
    for (unsigned int pixId = 0; pixId < sPixelsPerTile; ++pixId) {
        if (!srcMask) break;    // early exit
        if (srcMask & static_cast<uint64_t>(0x1)) {
            T &currDstVal = dstFirstValOfTile[pixId];
            unsigned int &currDstNumSampleTotal = dstFirstNumSampleTotalOfTile[pixId];
            const T &currSrcVal = srcFirstValOfTile[pixId];
            const unsigned int &currSrcNumSampleTotal = srcFirstNumSampleTotalOfTile[pixId];

            unsigned int totalSample = currDstNumSampleTotal + currSrcNumSampleTotal;
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
        }
        srcMask >>= 1;
    }
}

template <typename T>
void
Fb::accumulateTileClosestFilter(T *dstFirstValOfTile,
                                unsigned int *dstFirstNumSampleTotalOfTile,
                                uint64_t srcMask,
                                const T *srcFirstValOfTile,
                                const unsigned int *srcFirstNumSampleTotalOfTile) const
//
// special accumulateTile function for the case of using closestFilter
//
{
    unsigned int depthId = T::N - 1; // depth value is last component
    
    for (unsigned int pixId = 0; pixId < sPixelsPerTile; ++pixId) {
        if (!srcMask) break;    // early exit
        if (srcMask & static_cast<uint64_t>(0x1)) {
            T &currDstVal = dstFirstValOfTile[pixId];
            unsigned int &currDstNumSampleTotal = dstFirstNumSampleTotalOfTile[pixId];

            const T &currSrcVal = srcFirstValOfTile[pixId];
            const unsigned int &currSrcNumSampleTotal = srcFirstNumSampleTotalOfTile[pixId];

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
        }
        srcMask >>= 1;
    }
}

//---------------------------------------------------------------------------------------------------------------

void
Fb::accumulateRenderBufferOneTile(const Fb &src, const int tileId)
{
    accumulateActiveOneTile
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
Fb::accumulatePixelInfoOneTile(const Fb &src, const int tileId)
{
    accumulateActiveOneTile
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
Fb::accumulateHeatMapOneTile(const Fb &src, const int tileId)
{
    accumulateActiveOneTile
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
Fb::accumulateWeightBufferOneTile(const Fb &src, const int tileId)
{
    accumulateActiveOneTile
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
Fb::accumulateRenderBufferOddOneTile(const Fb &src, const int tileId)
{
    accumulateActiveOneTile
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
Fb::accumulateFloat1AovOneTile(FbAovShPtr &dstFbAov, const FbAovShPtr &srcFbAov, const int tileId)
{
    accumulateActiveOneTile
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
Fb::accumulateFloat2AovOneTile(FbAovShPtr &dstFbAov, const FbAovShPtr &srcFbAov, const int tileId)
{
    accumulateActiveOneTile
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
Fb::accumulateFloat3AovOneTile(FbAovShPtr &dstFbAov, const FbAovShPtr &srcFbAov, const int tileId)
{
    accumulateActiveOneTile
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
Fb::accumulateFloat4AovOneTile(FbAovShPtr &dstFbAov, const FbAovShPtr &srcFbAov, const int tileId)
{
    accumulateActiveOneTile
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
Fb::accumulatePixelInfoTile(PixelInfo *dstFirstPixelInfoOfTile,
                            uint64_t srcMask,
                            const PixelInfo *srcFirstPixelInfoOfTile) const
{
    for (unsigned int pixId = 0; pixId < sPixelsPerTile; ++pixId) {
        if (!srcMask) break;    // early exit
        if (srcMask & static_cast<uint64_t>(0x1)) {
            PixelInfo &currDstPixelInfo = dstFirstPixelInfoOfTile[pixId];
            const PixelInfo &currSrcPixelInfo = srcFirstPixelInfoOfTile[pixId];

            if (currDstPixelInfo.depth > currSrcPixelInfo.depth) {
                currDstPixelInfo.depth = currSrcPixelInfo.depth;
            }
        }
        srcMask >>= 1;
    }
}

void
Fb::accumulateWeightBufferTile(float *dstFirstPixelInfoOfTile,
                               uint64_t srcMask,
                               const float *srcFirstPixelInfoOfTile) const
{
    for (unsigned int pixId = 0; pixId < sPixelsPerTile; ++pixId) {
        if (!srcMask) break;    // early exit
        if (srcMask & static_cast<uint64_t>(0x1)) {
            float &currDstPixelInfo = dstFirstPixelInfoOfTile[pixId];
            const float &currSrcPixelInfo = srcFirstPixelInfoOfTile[pixId];

            currDstPixelInfo += currSrcPixelInfo;
        }
        srcMask >>= 1;
    }
}
    
} // namespace grid_util
} // namespace scene_rdl2

