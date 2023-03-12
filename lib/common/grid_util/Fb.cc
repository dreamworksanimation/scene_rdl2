// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#include "Fb.h"
#include "PackTiles.h"

#include <iomanip>


namespace scene_rdl2 {
namespace grid_util {

void
Fb::garbageCollectUnusedBuffers()
{
    if (!mPixelInfoStatus) {
        mActivePixelsPixelInfo.cleanUp();
        mPixelInfoBufferTiled.cleanUp();
    }

    if (!mHeatMapStatus) {
        mActivePixelsHeatMap.cleanUp();
        mHeatMapSecBufferTiled.cleanUp();
        mHeatMapNumSampleBufferTiled.cleanUp();
    }

    if (!mWeightBufferStatus) {
        mActivePixelsWeightBuffer.cleanUp();
        mWeightBufferTiled.cleanUp();
    }

    if (!mRenderBufferOddStatus) {
        mActivePixelsRenderBufferOdd.cleanUp();
        mRenderBufferOddTiled.cleanUp();
        mRenderBufferOddNumSampleBufferTiled.cleanUp();
    }

    // try to do garbage collect AOV buffers
    {
        unsigned totalActiveAovBuffer = 0;
        auto itr = mRenderOutput.begin();
        while (1) {
            if (itr == mRenderOutput.end()) break;
            
            if ((itr->second)->garbageCollectUnusedBuffers()) {
                totalActiveAovBuffer++;
                itr++;
            } else {
                itr = mRenderOutput.erase(itr); // erase this entry
            }
        }
        mRenderOutputStatus = (totalActiveAovBuffer)? true: false; // just in case we update condition
    }
}

//------------------------------------------------------------------------------

std::string
Fb::show(const std::string &hd) const
{
    std::ostringstream ostr;
    ostr << hd << "Fb {\n"; {
        ostr << hd << "  mAlignedWidth:" << mAlignedWidth << '\n';
        ostr << hd << "  mAlignedHeight:" << mAlignedHeight << '\n';

        ostr << mActivePixels.show(hd + "  ") << '\n';
        ostr << showRenderBuffer(hd + "  ") << '\n';
    }
    ostr << hd << "}";
    return ostr.str();
}

void
Fb::verifyRenderBufferAccessTest() const // for debug
{
    std::cerr << ">> Fb.cc verifyRenderBufferAccessTest() start..." << std::endl;
    if (!PackTiles::verifyRenderBufferAccessTest(mRenderBufferTiled)) {
        std::cerr << ">> Fb.cc verifyRenderBufferAccessTest() failed" << std::endl;
    }
}

unsigned
Fb::getNonBlackRenderBufferPixelTotal() const
//
// for debug
//
{
    unsigned total = 0;
    ActivePixels::crawlAllActivePixels
        (mActivePixels,
         [&](unsigned currPixOffset) {
            const fb_util::RenderColor *v = mRenderBufferTiled.getData() + currPixOffset;
            if ((*v)[0] != 0.0f || (*v)[1] != 0.0f || (*v)[2] != 0.0f) total++;
        });
    return total;
}

std::string 
Fb::showDebugMinMaxActiveWeightPixelInfo() const
//
// for debug
//
{
    unsigned total = 0;
    float min, max;
    ActivePixels::crawlAllActivePixels
        (mActivePixelsWeightBuffer,
         [&](unsigned currPixOffset) {
            const float *v = mWeightBufferTiled.getData() + currPixOffset;
            if ((*v) != 0.0f) {
                if (!total) {
                    min = *v;
                    max = *v;
                } else {
                    if (*v < min) min = *v;
                    if (max < *v) max = *v;
                }
                total++;
            }
        });

    std::ostringstream ostr;
    ostr << "weightBuffer activeTile:" << mActivePixelsWeightBuffer.getActiveTileTotal()
         << " activePixel:" << mActivePixelsWeightBuffer.getActivePixelTotal()
         << " nonZero:" << total;
    if (total) {
        ostr << " min:" << min << " max:" << max;
    }
    return ostr.str();
}

//------------------------------------------------------------------------------

// static function
Fb::TileExtrapolation &
Fb::getTileExtrapolation()
{
    static TileExtrapolation tileExtrapolation;
    return tileExtrapolation;
}

//------------------------------------------------------------------------------

std::string
Fb::showRenderBuffer(const std::string &hd) const
{
    int numTilesX = static_cast<int>(getNumTilesX());
    int numTilesY = static_cast<int>(getNumTilesY());
    int totalTiles = static_cast<int>(getTotalTiles());

    static const int maxActiveTileToShow = 10;

    std::ostringstream ostr;
    ostr << hd << "mRenderBufferTiled {\n";
    ostr << hd << "  width:" << mRenderBufferTiled.getWidth() << '\n';
    ostr << hd << "  height:" << mRenderBufferTiled.getHeight() << '\n';
    ostr << hd << "  numTilesX:" << numTilesX << '\n';
    ostr << hd << "  numTilesY:" << numTilesY << '\n';
    int activeTile = 0;
    for (int tileId = 0; tileId < totalTiles; ++tileId) {
        int pixOffset = tileId << 6;
        uint64_t mask = mActivePixels.getTileMask(tileId);
        if (mask) {
            const RenderColor *firstRenderColorOfTile = mRenderBufferTiled.getData() + pixOffset;
            ostr << "  tileId:" << tileId << '\n';
            if (activeTile < maxActiveTileToShow) {
                ostr << showRenderBufferTile(hd + "  ", mask, firstRenderColorOfTile) << '\n';
                activeTile++;
                if (activeTile == maxActiveTileToShow) {
                    ostr << "  ... too many active tiles -> skip ...\n";
                }
            }
        }
    }
    ostr << hd << "}";
    return ostr.str();
}

std::string
Fb::showRenderBufferTile(const std::string &hd,
                         const uint64_t mask,
                         const RenderColor *firstRenderColorOfTile) const
{
    std::ostringstream ostr;
    ostr << hd << "RenderBufferTile {\n";
    if (!mask) {
        ostr << hd << "  empty tile\n";
    } else {
        for (int pixY = 7; pixY >= 0; --pixY) {
            ostr << hd << "  ";
            for (int pixX = 0; pixX < 8; ++pixX) {
                int pixOffset = pixY * 8 + pixX;
                if (mask & (0x1 << pixOffset)) {
                    const RenderColor &currPix = firstRenderColorOfTile[pixOffset];
                    int v = static_cast<int>(currPix[0] * 255.0f); // red
                    if (v < 0) v = 0;
                    else if (v > 255) v = 255;
                    ostr << std::setw(2) << std::hex << std::setfill('0') << v << ' ';
                } else {
                    ostr << " . ";
                }
            }
            ostr << '\n';
        }
    }
    ostr << hd << "}";
    return ostr.str();
}

} // namespace grid_util
} // namespace scene_rdl2

