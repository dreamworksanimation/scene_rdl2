// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "PixelBuffer.h"
#include "Tiler.h"

#include <scene_rdl2/common/platform/Platform.h>

#include <type_traits>

namespace scene_rdl2 {
namespace fb_util {

// 
// Sparse tile buffer functionality. 
//

// Pack sparse tile data into the supplied memory block, given a tiled source
// buffer and the corresponding tile list. 
// The buffer passed in much be numTiles * 64 * sizeof(PIXEL_TYPE) in length.
// No allocation or deallocation is performed inside of this function.
template<typename PIXEL_TYPE>
inline bool
packSparseTiles(PIXEL_TYPE *dstPackedBuffer,
                PixelBuffer<PIXEL_TYPE> const &srcTiledBuffer,
                const std::vector<Tile> &tiles)
{
    MNRY_ASSERT(dstPackedBuffer);
    MNRY_ASSERT(srcTiledBuffer.getWidth() % 8 == 0);
    MNRY_ASSERT(srcTiledBuffer.getHeight() % 8 == 0);

    unsigned numTiles = (unsigned)tiles.size();
    if (numTiles == 0) {
        return false;
    }

    Tiler tiler(srcTiledBuffer.getWidth(), srcTiledBuffer.getHeight());

    const PIXEL_TYPE *src = srcTiledBuffer.getData();
    PIXEL_TYPE *dst = dstPackedBuffer;
    for (unsigned i = 0; i < numTiles; ++i) {
        const Tile &tile = tiles[i];
        unsigned ofs = tiler.linearCoordsToCoarseTileOffset(tile.mMinX, tile.mMinY);
        memcpy(dst, src + ofs, sizeof(PIXEL_TYPE) * 64);
        dst += 64;
    }

    MNRY_ASSERT(dst == dstPackedBuffer + numTiles * 64);

    return true;
}

// Unpacked tile data to a destination tiled buffer.
// dst_tiled_buffer must be pre-initialized to be desired (tiled) dimensions.
template<typename PIXEL_TYPE>
inline bool
unpackSparseTiles(PixelBuffer<PIXEL_TYPE> *dstTiledBuffer,
                  const PIXEL_TYPE *srcPackedData,
                  const std::vector<Tile> &tiles)
{
    MNRY_ASSERT(dstTiledBuffer);
    MNRY_ASSERT(dstTiledBuffer->getWidth() % 8 == 0);
    MNRY_ASSERT(dstTiledBuffer->getHeight() % 8 == 0);

    unsigned numTiles = (unsigned)tiles.size();
    if (numTiles == 0 || dstTiledBuffer->getArea() == 0) {
        return false;
    }

    unsigned w = dstTiledBuffer->getWidth();
    unsigned h = dstTiledBuffer->getHeight();

    Tiler tiler(w, h);

    const PIXEL_TYPE *src = srcPackedData;
    PIXEL_TYPE *dst = dstTiledBuffer->getData();
    for (unsigned i = 0; i < numTiles; ++i) {
        const Tile &tile = tiles[i];
        MNRY_ASSERT(tile.mMaxX <= w && tile.mMaxY <= h);
        unsigned ofs = tiler.linearCoordsToCoarseTileOffset(tile.mMinX, tile.mMinY);
        memcpy(dst + ofs, src, sizeof(PIXEL_TYPE) * 64);
        src += 64;
    }

    MNRY_ASSERT(src == srcPackedData + numTiles * 64);

    return true;
}

} // namespace fb_util
} // namespace scene_rdl2


