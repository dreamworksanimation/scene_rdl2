// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "ActivePixels.h"

#include <iomanip>
#include <iostream>
#include <sstream>

namespace scene_rdl2 {
namespace fb_util {

bool
ActivePixels::compare(const ActivePixels &target) const // for debug
{
    if (target.mOriginalWidth != mOriginalWidth ||
        target.mOriginalHeight != mOriginalHeight ||
        target.mAlignedWidth != mAlignedWidth ||
        target.mAlignedHeight != mAlignedHeight ||
        target.mNumTilesX != mNumTilesX ||
        target.mNumTilesY != mNumTilesY) {
        return false;
    }

    if (target.mTiles.size() != mTiles.size()) {
        return false;
    }

    const uint64_t * __restrict ptrA = &target.mTiles[0];
    const uint64_t * __restrict ptrB = &mTiles[0];
    for (size_t tileId = 0; tileId < mTiles.size(); ++tileId) {
        if (*ptrA != *ptrB) return false;
        ptrA++;
        ptrB++;
    }
    return true;
}

bool
ActivePixels::isActivePixel(const unsigned sx, const unsigned sy) const
{
    if (sx >= getWidth() || sy >= getHeight()) return false;

    constexpr unsigned tileSize = 8;
    unsigned tileX = sx / tileSize;
    unsigned tileY = sy / tileSize;
    uint64_t tileMask = getTile(tileX, tileY);

    unsigned offX = sx % tileSize;
    unsigned offY = sy % tileSize;
    unsigned offset = offY * tileSize + offX;
    return ((uint64_t)(tileMask >> offset) & 0x1) != (uint64_t)(0x0);
}

std::string
ActivePixels::show(const std::string &hd) const
{
    std::ostringstream ostr;

    ostr << hd << "ActivePixels (w:" << mOriginalWidth << " h:" << mOriginalHeight
         << " mNumTileX:" << mNumTilesX << " mNumTileY:" << mNumTilesY << ") {\n";
    for (int tileIdY = static_cast<int>(mNumTilesY) - 1; tileIdY >= 0; --tileIdY) {
        ostr << hd << "  ";
        for (int tileIdX = 0; tileIdX < static_cast<int>(mNumTilesX); ++tileIdX) {
            if (getTile(tileIdX, tileIdY)) ostr << "* ";
            else ostr << ". ";
        }
        ostr << '\n';
    }
    ostr << hd << '}';
    return ostr.str();
}

std::string
ActivePixels::showFullInfo(const std::string &hd) const
{
    std::ostringstream ostr;

    ostr << hd << "ActivePixels (w:" << mOriginalWidth << " h:" << mOriginalHeight << ") {\n";
    ostr << hd << "  totalActiveTiles:" << getActiveTileTotal() << '\n';
    for (size_t tileId = 0; tileId < mTiles.size(); ++tileId) {
        const uint64_t &currMask = mTiles[tileId];
        if (currMask) {
            ostr << hd << "  mTiles[" << tileId
                 << "] = 0x" << std::hex << std::setw(16) << std::setfill('0') << currMask << std::dec
                 << ";\n";
        }
    }
    ostr << hd << '}';
    return ostr.str();
}

// static function
std::string
ActivePixels::showMask(const std::string &hd, const uint64_t mask64)
{
    std::ostringstream ostr;

    ostr << hd << "mask 0x" << std::setw(16) << std::setfill('0') << std::hex << mask64 << " {\n";
    for (int y = 7; y >= 0; --y) {
        ostr << hd << "  ";
        for (int x = 0; x < 8; ++x) {
            if (mask64 & (static_cast<uint64_t>(0x1) << ((y << 3) + x))) {
                // ostr << "* ";
                ostr << std::setw(2) << std::oct << ((y << 3) + x) << ' ';
            } else {
                // ostr << ". ";
                ostr << " . ";
            }
        }
        ostr << '\n';
    }
    ostr << hd << "}";
    return ostr.str();
}

std::string
ActivePixels::show() const
{
    std::ostringstream ostr;
    ostr << "ActivePixels {\n"
         << "  mOriginalWidth:" << mOriginalWidth << '\n'
         << "  mOriginalHeight:" << mOriginalHeight << '\n'
         << "  mAlignedWidth:" << mAlignedWidth << '\n'
         << "  mAlignedHeight:" << mAlignedHeight << '\n'
         << "  mNumTilesX:" << mNumTilesX << '\n'
         << "  mNumTilesY:" << mNumTilesY << '\n'
         << "  mTiles.size():" << mTiles.size() << '\n'
         << "  getActiveTileTotal():" << getActiveTileTotal() << '\n'
         << "  getActivePixelTotal():" << getActivePixelTotal() << '\n'
         << "}";
    return ostr.str();
}

std::string
ActivePixels::showTile(unsigned tileId) const
{
    if (tileId > getNumTiles() - 1) {
        std::ostringstream ostr;
        ostr << "tileId:" << tileId << " outside range. numTiles:" << getNumTiles();
        return ostr.str();
    }

    uint64_t currMask = getTileMask(tileId);
    return showMask("", currMask);
}

bool
ActivePixels::verifyReset(const std::vector<char> *partialMergeTilesTbl) const
{
    if (!partialMergeTilesTbl) return true;

    if (partialMergeTilesTbl->size() != mTiles.size()) {
        return false;
    }

    for (size_t id = 0; id < partialMergeTilesTbl->size(); ++id) {
        if ((*partialMergeTilesTbl)[id]) {
            if (mTiles[id] != 0x0) {
                return false;
            }
        }
    }

    return true;
}

} // namespace fb_util
} // namespace scene_rdl2
