// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

//
// -- Active pixels information for entire image --
//
// Keep active or not status for each pixels for one image
// Internal data is using tiled format and each tile size is 8x8.
// Pixel itself is represented by one bit and one tile is 64bit (=uint64_t)
// This activePixels information is used by several different places like
// Tile extrapolation, packTile (ProgressiveFrame message) and others.
//

#include <scene_rdl2/common/platform/Platform.h> // finline

#include <string>
#include <vector>
#include <cstring>

#include <nmmintrin.h>          // _mm_popcnt_u64()

namespace scene_rdl2 {
namespace fb_util {

class ActivePixels
{
public:
    ActivePixels()
        : mOriginalWidth(0)
        , mOriginalHeight(0)
        , mAlignedWidth(0)
        , mAlignedHeight(0)
        , mNumTilesX(0)
        , mNumTilesY(0)
    {}
    ActivePixels(const ActivePixels &src) { copy(src); }

    finline void init(const unsigned width, const unsigned height); // original width and height (not need to tile aligned)
    finline void cleanUp();                                         // free internal memory

    bool isActive() const { return (mOriginalWidth && mOriginalHeight)? true: false; }
    finline bool isSameSize(const ActivePixels &activePixels) const;

    void reset() { for (size_t i = 0; i < mTiles.size(); ++i) mTiles[i] = static_cast<uint64_t>(0x0); }
    finline void reset(const std::vector<char> &activeTilesTbl);

    void setTileMask(const unsigned tileId, const uint64_t mask) { mTiles[tileId] = mask; }
    uint64_t getTileMask(const unsigned tileId) const { return mTiles[tileId]; }

    unsigned getWidth() const { return mOriginalWidth; }
    unsigned getHeight() const { return mOriginalHeight; }
    unsigned getAlignedWidth() const { return mAlignedWidth; }
    unsigned getAlignedHeight() const { return mAlignedHeight; }

    unsigned getNumTiles() const { return static_cast<unsigned>(mTiles.size()); }
    unsigned getNumTilesX() const { return mNumTilesX; }
    unsigned getNumTilesY() const { return mNumTilesY; }

    finline unsigned getActiveTileTotal() const; // count active tile total
    finline unsigned getActivePixelTotal() const; // count active pixel total for debug

    finline void copy(const ActivePixels &src);
    bool compare(const ActivePixels &target) const; // for debug
    finline bool orOp(const ActivePixels &src); // OR bit operation
    void orOp(const unsigned tileId, const uint64_t mask) { mTiles[tileId] |= mask; }

    std::string show(const std::string &hd) const;
    std::string showFullInfo(const std::string &hd) const;
    static std::string showMask(const std::string &hd, const uint64_t mask64);
    std::string show() const;
    std::string showTile(unsigned tileId) const;

    // for debugging
    template <typename F>
    static size_t crawlAllActivePixels(const ActivePixels &activePixels, F activePixelFunc) {
        size_t totalActivePix = 0;
        uint64_t mask = 0x0;
        for (unsigned tileId = 0; tileId < activePixels.getNumTiles(); ++tileId) {
            if ((mask = activePixels.getTileMask(tileId))) {
                unsigned tilePixOffset = tileId << 6;
                for (unsigned pixOffset = 0; pixOffset < 64; ++pixOffset) {
                    if (!mask) break; // early exit
                    if (mask & static_cast<uint64_t>(0x1)) {
                        totalActivePix++;
                        unsigned currPixOffset = tilePixOffset + pixOffset;
                        activePixelFunc(currPixOffset);
                    } // active pixel
                    mask >>= 1;
                } // pixOffset
            } // active tile
        } // tileId
        return totalActivePix;
    }

    bool verifyReset(const std::vector<char> *partialMergeTilesTbl) const; // for debug

protected:

    unsigned calcNumTilesX(const unsigned width) const { return width >> 3; }
    unsigned calcNumTilesY(const unsigned height) const { return height >> 3; }

    unsigned tileIdOffset(const unsigned tileIdX, const unsigned tileIdY) const { return tileIdX + tileIdY * mNumTilesX; }

    uint64_t getTile(const unsigned tileIdX, const unsigned tileIdY) const { return mTiles[tileIdOffset(tileIdX, tileIdY)]; }

    finline uint64_t countBit64(const uint64_t mask64) const;

    //------------------------------

    unsigned mOriginalWidth, mOriginalHeight;   // original output image size
    unsigned mAlignedWidth, mAlignedHeight;     // aligned 8 pixel boundary size
    unsigned mNumTilesX, mNumTilesY;

    std::vector<uint64_t> mTiles;
}; // ActivePixels

finline void
ActivePixels::init(const unsigned width, const unsigned height)
{
    if (mOriginalWidth == width && mOriginalHeight == height) {
        return;                 // skip
    }

    mOriginalWidth = width;
    mOriginalHeight = height;
    mAlignedWidth = (width + 7) & ~7;
    mAlignedHeight = (height + 7) & ~7;

    mNumTilesX = calcNumTilesX(mAlignedWidth);
    mNumTilesY = calcNumTilesY(mAlignedHeight);
    unsigned totalTiles = mNumTilesX * mNumTilesY;

    mTiles.resize(totalTiles, static_cast<uint64_t>(0x0));
}

finline void
ActivePixels::cleanUp()
{
    mOriginalWidth = 0;
    mOriginalHeight = 0;
    mAlignedWidth = 0;
    mAlignedHeight = 0;
    mNumTilesX = 0;
    mNumTilesY = 0;

    mTiles.clear();
    mTiles.shrink_to_fit();
}

finline void
ActivePixels::reset(const std::vector<char> &activeTilesTbl)
{
    // Basically activeTilesTbl.size() should be same as mTiles.size().
    MNRY_ASSERT(activeTilesTbl.size() == mTiles.size());

    for (size_t i = 0; i < activeTilesTbl.size(); ++i) {
        if (activeTilesTbl[i]) mTiles[i] = 0x0;
    }
}

finline bool
ActivePixels::isSameSize(const ActivePixels &activePixels) const
{
    if (getWidth() != activePixels.getWidth() ||
        getHeight() != activePixels.getHeight()) {
        return false;
    }
    return true;
}

finline unsigned
ActivePixels::getActiveTileTotal() const
{
    unsigned total = 0;
    for (size_t i = 0; i < mTiles.size(); ++i) {
        if (mTiles[i]) total++;
    }
    return total;
}

finline unsigned
ActivePixels::getActivePixelTotal() const
{
    unsigned total = 0;
    for (size_t i = 0; i < mTiles.size(); ++i) {
        if (mTiles[i]) total += static_cast<unsigned>(countBit64(mTiles[i]));
    }
    return total;
}

finline void
ActivePixels::copy(const ActivePixels &src)
{
    init(src.mOriginalWidth, src.mOriginalHeight);
    std::memcpy(mTiles.data(), src.mTiles.data(), mTiles.size() * sizeof(uint64_t));
}

finline bool
ActivePixels::orOp(const ActivePixels &src)
{
    if (src.mOriginalWidth != mOriginalWidth ||
        src.mOriginalHeight != mOriginalHeight ||
        src.mAlignedWidth != mAlignedWidth ||
        src.mAlignedHeight != mAlignedHeight ||
        src.mNumTilesX != mNumTilesX ||
        src.mNumTilesY != mNumTilesY) {
        return false;
    }

    if (src.mTiles.size() != mTiles.size()) {
        return false;
    }

    uint64_t * __restrict dstPtr = &mTiles[0];
    const uint64_t * __restrict srcPtr = &src.mTiles[0];
    for (size_t tileId = 0; tileId < mTiles.size(); ++tileId) {
        *dstPtr++ |= *srcPtr++;
    }
    return true;
}

finline uint64_t
ActivePixels::countBit64(const uint64_t mask64) const
{
    //
    // population count
    //
    return _mm_popcnt_u64(mask64);
}

} // namespace fb_util
} // namespace scene_rdl2
