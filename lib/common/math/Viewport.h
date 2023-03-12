// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "Vec2.h"

/**
 * Viewports represent a rectangular region in pixel space. They may contain
 * positive or negative pixel coordinates, but the min X/Y will always be <=
 * the max X/Y. There are 2 varieties defined here: ClosedViewport uses closed
 * intervals to represent each axis, and HalfOpenViewport uses half-open
 * intervals to represent each axis.
 */

namespace scene_rdl2 {
namespace math {

struct BaseViewport
{
    /**
     * Default construct a viewport, whose region is 1x1 at the origin.
     */
    finline BaseViewport() :
        mMinX(0),
        mMinY(0),
        mMaxX(0),
        mMaxY(0) {}

    /**
     * Construct a viewport from individual min/max X/Y coordinates. The
     * viewport guarantees that the min is actually the min and the max is
     * actually the max.
     *
     * @param   minX    The minimum X coordinate.
     * @param   minY    The minimum Y coordinate.
     * @param   maxX    The maximum X coordinate.
     * @param   maxY    The maximum Y coordinate.
     */
    finline BaseViewport(int minX, int minY, int maxX, int maxY) :
        mMinX(math::min(minX, maxX)),
        mMinY(math::min(minY, maxY)),
        mMaxX(math::max(minX, maxX)),
        mMaxY(math::max(minY, maxY)) {}

    /**
     * Construct a viewport from min and max vectors. The viewport guarantees
     * that the min is actually the min and the max is actually the max.
     *
     * @param   min     The minimum 2D point.
     * @param   max     The maximum 2D point.
     */
    finline BaseViewport(math::Vec2i min, math::Vec2i max) :
        mMinX(math::min(min.x, max.x)),
        mMinY(math::min(min.y, max.y)),
        mMaxX(math::max(min.x, max.x)),
        mMaxY(math::max(min.y, max.y))  {}

    /**
     * Construct a viewport from an array of 4 integers, in minX, minY, maxX,
     * maxY order. The viewport guarantees that the min is actually the min
     * and the max is actually the max.
     *
     * @param   region  An array of 4 integers in minX, minY, maxX, maxY order.
     */
    finline explicit BaseViewport(int region[4]) :
        mMinX(math::min(region[0], region[2])),
        mMinY(math::min(region[1], region[3])),
        mMaxX(math::max(region[0], region[2])),
        mMaxY(math::max(region[1], region[3])) {}

    /// Returns the viewport min as a 2D point.
    finline math::Vec2i min() const
    {
        return math::Vec2i(mMinX, mMinY);
    }

    /// Returns the viewport max as a 2D point.
    finline math::Vec2i max() const
    {
        return math::Vec2i(mMaxX, mMaxY);
    }

    int mMinX;
    int mMinY;
    int mMaxX;
    int mMaxY;

protected:
    finline ~BaseViewport() {}
};

finline bool operator==(const BaseViewport& a, const BaseViewport& b)
{
    return a.mMinX == b.mMinX &&
           a.mMinY == b.mMinY &&
           a.mMaxX == b.mMaxX &&
           a.mMaxY == b.mMaxY;
}

finline bool operator!=(const BaseViewport& a, const BaseViewport& b)
{
    return !(a == b);
}

/*
 * The viewport min and max are both inclusive. In other words, a viewport
 * with a min X of 0 and a max X of 9 has a width of 10 pixels. While half-open
 * intervals are usually more convenient, this is to maintain consistency
 * with how the studio has dealt with viewports in the past. Just make sure to
 * use <= instead of < when iterating between the min and max.
 *
 * For code which assumes half open invervals, see ViewportHalfOpen
 */
struct Viewport : public BaseViewport
{
    finline Viewport() : BaseViewport() {}

    finline Viewport(int minX, int minY, int maxX, int maxY) :
        BaseViewport(minX, minY, maxX, maxY) {}

    finline Viewport(math::Vec2i min, math::Vec2i max) :
        BaseViewport(min, max) {}

    finline Viewport(int region[4]) :
        BaseViewport(region) {}

    /// Returns the width of the viewport, in pixels.
    finline unsigned width() const
    {
        // Guaranteed to be positive since we verified that max > min during
        // construction.
        return mMaxX - mMinX + 1;
    }

    /// Returns the height of the viewport, in pixels.
    finline unsigned height() const
    {
        // Guaranteed to be positive since we verified that max > min during
        // construction.
        return mMaxY - mMinY + 1;
    }

    /// Returns true if the given coordinate (x, y) is included within the
    /// bounds of the viewport.
    finline bool contains(int x, int y) const
    {
        return (x >= mMinX) && (x <= mMaxX) && (y >= mMinY) && (y <= mMaxY);
    }
};


/**
 * Half open scheme. 
 *  
 * A viewport with a width of 10 starting at 0 would have a minX of 0, and a 
 * maxX of 10 under this scheme.
 */
struct HalfOpenViewport : public BaseViewport
{
    finline HalfOpenViewport() : BaseViewport() {}

    finline HalfOpenViewport(int minX, int minY, int maxX, int maxY) :
        BaseViewport(minX, minY, maxX, maxY) {}

    finline HalfOpenViewport(math::Vec2i min, math::Vec2i max) :
        BaseViewport(min, max) {}

    finline HalfOpenViewport(int region[4]) :
        BaseViewport(region) {}

    finline HalfOpenViewport(const std::vector<int>& window, float invRes)
    {
        int width = int(float(window[2] - window[0]) * invRes);
        int height = int(float(window[3] - window[1]) * invRes);
        mMinX = static_cast<int>(static_cast<float>(window[0]) * invRes);
        mMinY = static_cast<int>(static_cast<float>(window[1]) * invRes);
        mMaxX = mMinX + width;
        mMaxY = mMinY + height;
    }

    /// Returns the width of the viewport, in pixels.
    finline unsigned width() const
    {
        // Guaranteed to be positive since we verified that max > min during
        // construction.
        return mMaxX - mMinX;
    }

    /// Returns the height of the viewport, in pixels.
    finline unsigned height() const
    {
        // Guaranteed to be positive since we verified that max > min during
        // construction.
        return mMaxY - mMinY;
    }

    /// Returns true if the given coordinate (x, y) is included within the
    /// bounds of the viewport.
    finline bool contains(int x, int y) const
    {
        return (x >= mMinX) && (x < mMaxX) && (y >= mMinY) && (y < mMaxY);
    }
};

// Conversion functions:
inline Viewport convertToClosedViewport(const HalfOpenViewport &vp)
{
    return Viewport(vp.mMinX, vp.mMinY, vp.mMaxX - 1, vp.mMaxY - 1);
}

inline HalfOpenViewport convertHalfOpenToViewport(const Viewport &vp)
{
    return HalfOpenViewport(vp.mMinX, vp.mMinY, vp.mMaxX + 1, vp.mMaxY + 1);
}

} // namespace math
} // namespace scene_rdl2

