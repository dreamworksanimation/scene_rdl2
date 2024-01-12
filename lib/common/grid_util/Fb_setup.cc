// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#include "Fb.h"

namespace scene_rdl2 {
namespace grid_util {

void
Fb::setupPixelInfo(const PartialMergeTilesTbl *partialMergeTilesTbl,
                   const std::string &name)
{
    if (mPixelInfoName != name) {
        mPixelInfoName = name;
    }

    setupBufferMain(partialMergeTilesTbl,
                    mPixelInfoStatus,
                    mActivePixelsPixelInfo,
                    2,          // numOfBuffers
                    [&](unsigned bufferId,
                        unsigned width, unsigned height,
                        unsigned alignedWidth, unsigned alignedHeight) { // resizeBufferFunc
                        if (bufferId == 0) {
                            mActivePixelsPixelInfo.init(width, height);
                        } else {
                            mPixelInfoBufferTiled.init(alignedWidth, alignedHeight);
                        }
                    },
                    [&](unsigned bufferId) { // initWholeBufferFunc
                        if (bufferId == 0) {
                            mActivePixelsPixelInfo.reset();
                        } else {
                            mPixelInfoBufferTiled.clear(PixelInfo(FLT_MAX));
                        }
                    },
                    [&](unsigned bufferId) { // initPartialBufferFunc
                        if (bufferId == 0) {
                            mActivePixelsPixelInfo.reset(*partialMergeTilesTbl);
                        } else {
                            partialMergeTilesTblCrawler
                                (*partialMergeTilesTbl,
                                 [&](unsigned pixOffset) {
                                    bufferTileClearFloat(mPixelInfoBufferTiled.getData() + pixOffset, FLT_MAX);
                                });
                        }
                    });
}

void
Fb::setupHeatMap(const PartialMergeTilesTbl *partialMergeTilesTbl,
                 const std::string &name)
{
    if (mHeatMapName != name) {
        mHeatMapName = name;
    }

    setupBufferMain(partialMergeTilesTbl,
                    mHeatMapStatus,
                    mActivePixelsHeatMap,
                    3,          // numOfBuffers
                    [&](unsigned bufferId,
                        unsigned width, unsigned height,
                        unsigned alignedWidth, unsigned alignedHeight) { // resizeBufferFunc
                        switch (bufferId) {
                        case 0 : mActivePixelsHeatMap.init(width, height); break;
                        case 1 : mHeatMapSecBufferTiled.init(alignedWidth, alignedHeight); break;
                        case 2 : mHeatMapNumSampleBufferTiled.init(alignedWidth, alignedHeight); break;
                        }
                    },
                    [&](unsigned bufferId) { // initWholeBufferFunc
                        switch (bufferId) {
                        case 0 : mActivePixelsHeatMap.reset(); break;
                        case 1 : mHeatMapSecBufferTiled.clear(); break;
                        case 2 : mHeatMapNumSampleBufferTiled.clear(); break;
                        }
                    },
                    [&](unsigned bufferId) { // initPartialBufferFunc
                        switch (bufferId) {
                        case 0 : mActivePixelsHeatMap.reset(*partialMergeTilesTbl);
                            break;
                        case 1 :
                            partialMergeTilesTblCrawler
                                (*partialMergeTilesTbl,
                                 [&](unsigned pixOffset) {
                                    bufferTileClear(mHeatMapSecBufferTiled.getData() + pixOffset);
                                });
                            break;
                        case 2 :
                            partialMergeTilesTblCrawler
                                (*partialMergeTilesTbl,
                                 [&](unsigned pixOffset) {
                                    bufferTileClear(mHeatMapNumSampleBufferTiled.getData() + pixOffset);
                                });
                            break;
                        }
                    });
}

void
Fb::setupWeightBuffer(const PartialMergeTilesTbl *partialMergeTilesTbl,
                      const std::string &name)
{
    if (mWeightBufferName != name) {
        mWeightBufferName = name;
    }

    setupBufferMain(partialMergeTilesTbl,
                    mWeightBufferStatus,
                    mActivePixelsWeightBuffer,
                    2,          // numOfBuffers
                    [&](unsigned bufferId,
                        unsigned width, unsigned height,
                        unsigned alignedWidth, unsigned alignedHeight) { // resizeBufferFunc
                        if (bufferId == 0) mActivePixelsWeightBuffer.init(width, height);
                        else mWeightBufferTiled.init(alignedWidth, alignedHeight);
                    },
                    [&](unsigned bufferId) { // initWholeBufferFunc
                        if (bufferId == 0) mActivePixelsWeightBuffer.reset();
                        else mWeightBufferTiled.clear();
                    },
                    [&](unsigned bufferId) { // initPartialBufferFunc
                        if (bufferId == 0) mActivePixelsWeightBuffer.reset(*partialMergeTilesTbl);
                        else {
                            partialMergeTilesTblCrawler
                                (*partialMergeTilesTbl,
                                 [&](unsigned pixOffset) {
                                    bufferTileClear(mWeightBufferTiled.getData() + pixOffset);
                                });
                        }
                    });
}

void
Fb::setupRenderBufferOdd(const PartialMergeTilesTbl *partialMergeTilesTbl)
{
    setupBufferMain(partialMergeTilesTbl,
                    mRenderBufferOddStatus,
                    mActivePixelsRenderBufferOdd,
                    3,          // numOfBuffers
                    [&](unsigned bufferId,
                        unsigned width, unsigned height,
                        unsigned alignedWidth, unsigned alignedHeight) { // resizeBufferFunc
                        switch (bufferId) {
                        case 0 : mActivePixelsRenderBufferOdd.init(width, height); break;
                        case 1 : mRenderBufferOddTiled.init(alignedWidth, alignedHeight); break;
                        case 2 : mRenderBufferOddNumSampleBufferTiled.init(alignedWidth, alignedHeight); break;
                        }
                    },
                    [&](unsigned bufferId) { // initWholeBufferFunc
                        switch (bufferId) {
                        case 0 : mActivePixelsRenderBufferOdd.reset(); break;
                        case 1 : mRenderBufferOddTiled.clear(); break;
                        case 2 : mRenderBufferOddNumSampleBufferTiled.clear(); break;
                        }
                    },
                    [&](unsigned bufferId) { // initPartialBufferFunc
                        switch (bufferId) {
                        case 0 : mActivePixelsRenderBufferOdd.reset(*partialMergeTilesTbl);
                            break;
                        case 1 :
                            partialMergeTilesTblCrawler
                                (*partialMergeTilesTbl,
                                 [&](unsigned pixOffset) {
                                    bufferTileClear(mRenderBufferOddTiled.getData() + pixOffset);
                                });
                            break;
                        case 2 :
                            partialMergeTilesTblCrawler
                                (*partialMergeTilesTbl,
                                 [&](unsigned pixOffset) {
                                    bufferTileClear(mRenderBufferOddNumSampleBufferTiled.getData() + pixOffset);
                                });
                            break;
                        }
                    });
}

//---------------------------------------------------------------------------------------------------------------

template <typename ResizeBuffFunc, typename InitWholeBuffFunc, typename InitPartialBuffFunc>
void
Fb::setupBufferMain(const PartialMergeTilesTbl *partialMergeTilesTbl,
                    bool &bufferStatus,
                    const ActivePixels &activePixels,
                    unsigned numOfBuffers,
                    ResizeBuffFunc resizeBuffFunc,
                    InitWholeBuffFunc initWholeBuffFunc,
                    InitPartialBuffFunc initPartialBuffFunc)
//
// This function finally setup internal memory and clear data and set buffer condition as active
// (i.e. bufferStatus goes to true).
// 
{
    bool needPartialInit = false;
    bool needWholeInit = false;
    if (!bufferStatus) {
        // Previous bufferStatus condition is false. This means we have to properly setup memory inside
        // this function. In order to this, we need at least one of the needPartialInit and needWholeInit
        // flag should be true. First of all, set needpartialInit = true here. If this condition is not
        // good, needWholeInit will be on instead later.
        needPartialInit = true;
    }

    //
    // resize buffer (i.e. memory allocation)
    //
    if (activePixels.getWidth() != mRezedViewport.width() ||
        activePixels.getHeight() != mRezedViewport.height()) {
        //
        // Regardless of current buffer condition (bufferStatus = true/false), we have to setup this buffer
        // because we have to make this buffer as access ready condition (bufferStatus = true).
        //
        // We only check activePixels resolution because other associated buffer
        // always has same resolution of activePixels actually.
        //
        unsigned width = mRezedViewport.width();
        unsigned height = mRezedViewport.height();
        unsigned alignedWidth = (width + 7) & ~7; // tile aligned (8x8) size
        unsigned alignedHeight = (height + 7) & ~7; // tile aligned (8x8) size
#       ifdef SINGLE_THREAD
        for (unsigned bufferId = 0; bufferId < numOfBuffers; ++bufferId) {
            resizeBuffFunc(bufferId, width, height, alignedWidth, alignedHeight);
        }
#       else // else SINGLE_THREAD
        tbb::parallel_for((unsigned)0, numOfBuffers, [&](unsigned bufferId) {
                resizeBuffFunc(bufferId, width, height, alignedWidth, alignedHeight);
            });
#       endif // end !SINGLE_THREAD

        needPartialInit = false;
        needWholeInit = true;
    }

    //
    // initialize buffer
    //
    if (!partialMergeTilesTbl) {
        if (needPartialInit) {
            needWholeInit = true;
            needPartialInit = false;
        }
    }

    if (needWholeInit) {
#       ifdef SINGLE_THREAD
        for (unsigned bufferId = 0; bufferId < numOfBuffers; ++bufferId) {
            initWholeBuffFunc(bufferId);
        }
#       else // else SINGLE_THREAD
        tbb::parallel_for((unsigned)0, numOfBuffers, [&](unsigned bufferId) {
                initWholeBuffFunc(bufferId);
            });
#       endif // end !SINGLE_THREAD
    } else if (needPartialInit) {
#       ifdef SINGLE_THREAD
        for (unsigned bufferId = 0; bufferId < numOfBuffers; ++bufferId) {
            initPartialBuffFunc(bufferId);
        }
#       else // else SINGLE_THREAD
        tbb::parallel_for((unsigned)0, numOfBuffers, [&](unsigned bufferId) {
                initPartialBuffFunc(bufferId);
            });
#       endif // end !SINGLE_THREAD
    }

    bufferStatus = true;
}
    
} // namespace grid_util
} // namespace scene_rdl2

