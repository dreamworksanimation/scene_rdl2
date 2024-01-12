// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#pragma once

//
// -- PackActiveTiles : active tile and related pixelMask information encoding/decoding logic --
//
// PackActiveTiles class is used by pack-tile codec version2. This class focuses on the encoding
// active tile position and active pixel position information. This logic does not include pixel value
// itself.
// pack-tile version1 logic does not use this class. Only used by version2 so far.
//

#include <scene_rdl2/common/fb_util/ActivePixels.h>
#include "RunLenBitTable.h"

namespace scene_rdl2 {

namespace rdl2 {    
    class ValueContainerDeq;
    class ValueContainerEnq;
} // namespace rdl2

namespace grid_util {

class ActiveBitTables;

class PackActiveTiles
{
public:
    using ActivePixels = fb_util::ActivePixels;
    using VContainerDeq = rdl2::ValueContainerDeq;
    using VContainerEnq = rdl2::ValueContainerEnq;

    // return condition definition about both of ActiveBitTables and RunLenBitTable have DumpMode=SKIP_DUMP
    static unsigned char getAllSkipCondition();

    // return combined tileMode and pixMaskMode condition as unsigned char
    // size_t *sizeInfo is output for statistical tracking info purpose.
    // No internal computational overhead if nullptr is set to sizeInfo.
    //     sizeInfo[0] : PackActiveTiles encoded data size (we call this as version 2)
    //     sizeInfo[1] : Size difference between version2 and version1. sizeInfo[1] = sizeInfo[0] - ver1Size
    //                   i.e. ver1Size = sizeInfo[0] - sizeInfo[1]
    static unsigned char enqTileMaskBlock(const ActivePixels &activePixels,
                                          VContainerEnq &vContainerEnq,
                                          int64_t *sizeInfo = nullptr);

    // return true:got_data false:no_data
    static bool deqTileMaskBlock(VContainerDeq &vContainerDeq,
                                 const unsigned activeTileTotal,
                                 ActivePixels &activePixels);

    //------------------------------

    // debug purpose function : generate random pattern activePixels
    static void randomActivePixels(ActivePixels &activePixels, const unsigned totalActivePixel);

    // encode/decode verify test function
    static bool codecVerify(const ActivePixels &activePixels);

protected:    

    // crawl all active tile inside ActivePixels
    template <typename F>
    static void crawlAllActivePixelsTile(const ActivePixels &activePixels, F tileFunc)
    {
        unsigned tileId = 0;
        for (unsigned yTileId = 0; yTileId < activePixels.getNumTilesY(); ++yTileId) {
            for (unsigned xTileId = 0; xTileId < activePixels.getNumTilesX(); ++xTileId) {
                tileFunc(tileId);
                tileId++;
            }
        }
    }

    static void enqPixMaskInfo(const RunLenBitTable::DumpMode pixMaskMode,
                               const RunLenBitTable &pixMaskInfo,
                               VContainerEnq &vContainerEnq);

    //
    // debug purpose functions
    //
    static bool getPix(const ActivePixels &activePixels, const unsigned pixId);
    static void setPix(ActivePixels &activePixels, const unsigned pixId); // set pixel condition as active

    // access pixel by pixId inside activePixels
    template <typename F>
    static void accessPixel(const ActivePixels &activePixels, const unsigned pixId, F tileFunc)
    {
        unsigned pixX   = pixId % activePixels.getWidth();
        unsigned pixY   = pixId / activePixels.getWidth();
        unsigned tileX  = pixX / 8;
        unsigned tileY  = pixY / 8;
        unsigned tileId = tileY * activePixels.getNumTilesX() + tileX;
        unsigned localX = pixX % 8;
        unsigned localY = pixY % 8;
        unsigned shift  = localY * 8 + localX;
        /* useful debug info
        std::cerr << "pix(" << pixX << ',' << pixY << ") tile(" << tileX << ',' << tileY << ')'
                  << " tileId:" << tileId << " local(" << localX << ',' << localY << ')'
                  << " shift:" << shift << std::endl;
        */
        tileFunc(tileId, shift);
    }

    static std::string showDumpMode(const unsigned char dumpMode);
};

} // namespace grid_util
} // namespace scene_rdl2

