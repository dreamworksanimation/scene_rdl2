// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "PixelBuffer.h"

#include <scene_rdl2/common/math/simd.h>
#include <scene_rdl2/common/math/Vec2.h>
#include <scene_rdl2/common/math/Vec4.h>

#include <cstdint>              // uint8_t
#include <ostream>

namespace scene_rdl2 {
namespace fb_util {

typedef math::Vec4f RenderColor; // Must match pbr::RenderColor!

// Useful pixel types.
struct ByteColor
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

struct ByteColor4
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

// For now this PixelInfo struct just holds a float representing the depth.
// Can be expanded to contain other stuff.
struct PixelInfo
{
    explicit PixelInfo(float d = 0.f) : depth(d) {}
    float depth;
};

// Useful buffer types.
typedef PixelBuffer<RenderColor> RenderBuffer;
typedef PixelBuffer<ByteColor> Rgb888Buffer;
typedef PixelBuffer<ByteColor4> Rgba8888Buffer;
typedef PixelBuffer<PixelInfo> PixelInfoBuffer;
typedef PixelBuffer<float> FloatBuffer;
typedef PixelBuffer<math::Vec2f> Float2Buffer;
typedef PixelBuffer<math::Vec3f> Float3Buffer;
typedef PixelBuffer<math::Vec4f> Float4Buffer;
typedef PixelBuffer<int64_t> HeatMapBuffer;

//
// We have a tile structure here to allow us to specify an explicit list of tiles we want
// rendered and the order we want them rendered in. This allows us to support region of
// interest, and also gives us the ability to swizzle the tiles for increased cache
// coherency. A tile is always 8*8 pixels in size unless it's on the border of a clipped
// viewport. Unlike viewports, the max x and max y coordinates of a tile are non-inclusive.
//
struct Tile
{
    Tile() :
        mMinX(0),
        mMaxX(1),
        mMinY(0),
        mMaxY(1) {}

    Tile(unsigned minX, unsigned maxX, unsigned minY, unsigned maxY) :
        mMinX(minX),
        mMaxX(maxX),
        mMinY(minY),
        mMaxY(maxY) {}

    unsigned getExtentX() const  { return mMaxX - mMinX; }
    unsigned getExtentY() const  { return mMaxY - mMinY; }
    unsigned getArea() const     { return getExtentX() * getExtentY(); }

    unsigned    mMinX;      // Inclusive.
    unsigned    mMaxX;      // Non-inclusive.
    unsigned    mMinY;      // Inclusive.
    unsigned    mMaxY;      // Non-inclusive.
};

inline bool operator==(const Tile& lhs, const Tile &rhs)
{
    return lhs.mMinX == rhs.mMinX &&
           lhs.mMaxX == rhs.mMaxX &&
           lhs.mMinY == rhs.mMinY &&
           lhs.mMaxY == rhs.mMaxY;
}

inline bool operator!=(const Tile& lhs, const Tile &rhs)
{
    return !(lhs == rhs);
}

inline std::ostream& operator<<(std::ostream& outs, const Tile& t)
{
    return outs << '[' << t.mMinX << ", " << t.mMaxX << "] [" <<
                          t.mMinY << ", " << t.mMaxY << ']';
}

} // namespace fb_util
} // namespace scene_rdl2

