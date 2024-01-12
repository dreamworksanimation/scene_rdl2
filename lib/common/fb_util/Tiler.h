// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
// This class abstracts the conversions between linear and the minimal tiling
// scheme which is implemented here. 
//
// Terminology used:
//
//  Linear coordinates: (x, y) coordinates which are in scanline format.
//  For example (0, 0) would refer to the bottom left pixel and (w - 1, h - 1)
//  would refer to the top right pixel. Incrementing x coordinates always go
//  left to right, and incrementing y coordinates always go bottom to top.
//
//  Tiled coordinates: The memory layout of a buffer is not linear.
//  Tiling is done on 8x8 quads. The 64 pixels of each tile are laid out
//  contiguously in memory in a linear bottom-left to top-right fashion.
//  The tiles themselves also follow a linear pattern the context of the
//  whole image. 
//
//  Tiled offset: A tiled offset refers to the offset from the start of a
//  tiled buffer to a particular pixel in memory.
//
//  Coarse tiled offset: A tiled offset refers to the offset from the start of
//  a tiled buffer to the start of a particular tile in memory.
//
#pragma once

#include "FbTypes.h"
#include "PixelBuffer.h"

#include <tbb/parallel_for.h>

// This can't be changed but avoids magic numbers in client code.
const unsigned COARSE_TILE_SIZE = 8u;

namespace scene_rdl2 {
namespace fb_util {

class Tiler
{
public:
    Tiler() :
        mOriginalW(0),
        mOriginalH(0),
        mAlignedW(0),
        mAlignedH(0),
        mNumTiles(0)
    {
    }

    // Pass in desired (potentially unaligned) width and height.
    Tiler(unsigned w, unsigned h) :
        mOriginalW(w),
        mOriginalH(h),
        mAlignedW((w + 7) & ~7),
        mAlignedH((h + 7) & ~7),
        mNumTiles((mAlignedW * mAlignedH) >> 6)
    {
    }

    unsigned linearCoordsToCoarseTileOffset(unsigned lx, unsigned ly) const
    {
        unsigned tileIdx = (ly >> 3) * (mAlignedW >> 3) + (lx >> 3);
        return tileIdx << 6;
    }

    unsigned linearCoordsToTiledOffset(unsigned lx, unsigned ly) const
    {
        return linearCoordsToCoarseTileOffset(lx, ly) + ((ly & 7) << 3) + (lx & 7);
    }

    void linearToTiledCoords(unsigned lx, unsigned ly, unsigned *tx, unsigned *ty) const
    {
        MNRY_ASSERT(lx < mOriginalW && ly < mOriginalH);

        unsigned tileOfs = linearCoordsToTiledOffset(lx, ly);
        getTiledCoords(tileOfs, tx, ty);

        MNRY_ASSERT(*tx < mAlignedW && *ty < mAlignedH);
    }

    bool tiledToLinearCoords(unsigned tx, unsigned ty, unsigned *lx, unsigned *ly) const
    {
        MNRY_ASSERT(tx < mAlignedW && ty < mAlignedH);

        unsigned tileOfs = mAlignedW * ty + tx;

        unsigned a = tileOfs >> 6;
        unsigned b = mAlignedW >> 3;

        unsigned tileY = a / b;
        unsigned tileX = a % b;

        *lx = (tileX << 3) + (tileOfs & 7);
        *ly = (tileY << 3) + ((tileOfs & 63) >> 3);

        // Returns false if linear coords are outside of the valid range.
        return (*lx < mOriginalW && *ly < mOriginalH);
    }

    unsigned getTiledOffset(unsigned tx, unsigned ty) const
    {
        MNRY_ASSERT(tx < mAlignedW && ty < mAlignedH);
        return mAlignedW * ty + tx;
    }

    void getTiledCoords(unsigned tileOfs, unsigned *tx, unsigned *ty) const
    {
        MNRY_ASSERT(tileOfs < (mAlignedW * mAlignedH));

        *ty = tileOfs / mAlignedW;
        *tx = tileOfs % mAlignedW;

        MNRY_ASSERT(getTiledOffset(*tx, *ty) == tileOfs);
    }

    unsigned getLinearOffset(unsigned lx, unsigned ly) const
    {
        MNRY_ASSERT(lx < mOriginalW && ly < mOriginalH);
        return mOriginalW * ly + lx;
    }

    void getLinearCoords(unsigned linearOfs, unsigned *lx, unsigned *ly) const
    {
        MNRY_ASSERT(linearOfs < (mOriginalW * mOriginalH));

        *ly = linearOfs / mOriginalW;
        *lx = linearOfs % mOriginalW;

        MNRY_ASSERT(getLinearOffset(*lx, *ly) == linearOfs);
    }

    // Desired user width required. It doesn't have to be tile aligned.
    unsigned mOriginalW;

    // Desired user height required. It doesn't have to be tile aligned.
    unsigned mOriginalH;

    // Tile aligned version of mOriginalW, always >= than mOriginalW.
    unsigned mAlignedW;

    // Tile aligned version of mOriginalH, always >= than mOriginalH.
    unsigned mAlignedH;

    // Total tiles required to cover the buffer.
    unsigned mNumTiles;
};


// General purpose function for untiling any tiled buffer.
// The PIXEL_XFORM_FUNC functor takes SRC_PIXEL_TYPE element and returns a
// DST_PIXEL_TYPE element.
// dstLinearBuffer should equal the original unaligned width and height
// we are targeting.
template<typename DST_PIXEL_TYPE, typename SRC_PIXEL_TYPE, typename PIXEL_XFORM_FUNC>
inline void
untile(PixelBuffer<DST_PIXEL_TYPE> *dstLinearBuffer,
       const PixelBuffer<SRC_PIXEL_TYPE> &srcTiledBuffer,
       const Tiler &tiler,
       bool parallel,
       PIXEL_XFORM_FUNC pixelXform)
{
    MNRY_ASSERT(dstLinearBuffer);

    const unsigned w = dstLinearBuffer->getWidth();
    const unsigned h = dstLinearBuffer->getHeight();

    MNRY_ASSERT(w == tiler.mOriginalW);
    MNRY_ASSERT(h == tiler.mOriginalH);

    const SRC_PIXEL_TYPE *__restrict src = srcTiledBuffer.getData();

    //
    // I don't understand this!!
    //
    // The top block crashes the opt build when running progressive mode in
    // raas_gui (meaning it crashes when executing the non-parallel code path).
    // The ifdef'ed out code block underneath runs fine in all cases and
    // configurations. Everything also runs fine in debug...
    //
    // First time I've witnessed parallel code working fine but the equivalent
    // single threaded code causing a crash.
    //
    // TODO: Check generated asm where crash is happening.
    //

#if 0

    // Should work but doesn't.
    mcrt_common::simpleLoop(parallel, 0u, h, [&](unsigned y) {
        for (unsigned x = 0; x < w; x += 8) {
            unsigned tileOfs = tiler.linearCoordsToTiledOffset(x, y);
            unsigned scanLength = std::min<unsigned>(w - x, 8);
            for (unsigned i = 0; i < scanLength; ++i) {
                dstLinearBuffer->setPixel(x + i, y, pixelXform(src[tileOfs + i], tileOfs + i));
            }
        }
    });

#else

    // In theory this is equivalent to the ifdef'ed out code above.
    if (parallel) {

        tbb::parallel_for (0u, h, [&](unsigned y) {
            for (unsigned x = 0; x < w; x += 8) {
                unsigned tileOfs = tiler.linearCoordsToTiledOffset(x, y);
                unsigned scanLength = std::min<unsigned>(w - x, 8);
                for (unsigned i = 0; i < scanLength; ++i) {
                    dstLinearBuffer->setPixel(x + i, y, pixelXform(src[tileOfs + i], tileOfs + i));
                }
            }
        });

    } else {

        for (unsigned y = 0; y < h; ++y) {
            for (unsigned x = 0; x < w; x += 8) {
                unsigned tileOfs = tiler.linearCoordsToTiledOffset(x, y);
                unsigned scanLength = std::min<unsigned>(w - x, 8);
                for (unsigned i = 0; i < scanLength; ++i) {
                    dstLinearBuffer->setPixel(x + i, y, pixelXform(src[tileOfs + i], tileOfs + i));
                }
            }
        }
    }

#endif
}

} // namespace fb_util
} // namespace scene_rdl2

