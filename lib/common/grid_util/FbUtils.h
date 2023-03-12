// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#pragma once

#include <scene_rdl2/common/math/Viewport.h>

#include <algorithm>

// Basically we should use multi-thread version.
// This single thread mode is used debugging and performance comparison reason mainly.
//#define SINGLE_THREAD

namespace scene_rdl2 {
namespace grid_util {

template <typename F>
void untileSinglePixelMainLoop(const unsigned w,
                               const unsigned h,
                               const math::Viewport *roi,
                               const unsigned dstNumChan,
                               F untileMain,
                               const bool top2bottom)
{
    if (roi) {
        untileSinglePixelLoopROI(w, h,
                                 std::max<unsigned>(0, roi->mMinX), std::max<unsigned>(0, roi->mMinY),
                                 std::max<unsigned>(0, roi->mMaxX), std::max<unsigned>(0, roi->mMaxY),
                                 dstNumChan, untileMain, top2bottom);
    } else {
        untileSinglePixelLoop(w, h, dstNumChan, untileMain, top2bottom);
    }
}

#ifdef SINGLE_THREAD
template <typename F>
void untileSinglePixelLoopROI(const unsigned w,
                              const unsigned h,
                              const unsigned minX,
                              const unsigned minY,
                              const unsigned maxX,
                              const unsigned maxY,
                              const unsigned dstNumChan,
                              F untileMain,
                              const bool top2bottom)
{
    fb_util::Tiler tiler(w, h);
    unsigned sx = clamp(std::min<unsigned>(minX, maxX), 0, w - 1);
    unsigned ex = clamp(std::max<unsigned>(minX, maxX), 0, w - 1) + 1;
    unsigned sy = clamp(std::min<unsigned>(minY, maxY), 0, h - 1);
    unsigned ey = clamp(std::max<unsigned>(minY, maxY), 0, h - 1) + 1;
    unsigned currW = ex - sx;
    unsigned currH = ey - sy;
    for (unsigned y = sy; y < ey; ++y) {
        unsigned currSx = (sx >> 3) << 3;
        for (unsigned x = currSx; x < ex; x += 8) {
            unsigned tileOfs = tiler.linearCoordsToTiledOffset(x, y);
            unsigned scanLength = std::min<unsigned>(ex - x, 8);
            for (unsigned pixOfs = 0; pixOfs < scanLength; ++pixOfs) {
                if (x + pixOfs < sx) continue;

                unsigned dstOfs = 0;
                if (top2bottom) {
                    dstOfs = ((currH - 1 - (y - sy)) * currW + (x - sx) + pixOfs) * dstNumChan;
                } else {
                    dstOfs = ((y - sy) * currW + (x - sx) + pixOfs) * dstNumChan;
                }
                untileMain(tileOfs, pixOfs, dstOfs);
            }
        }
    }
}
#else // else SINGLE_THREAD
template <typename F>
void untileSinglePixelLoopROI(const unsigned w,
                              const unsigned h,
                              const unsigned minX,
                              const unsigned minY,
                              const unsigned maxX,
                              const unsigned maxY,
                              const unsigned dstNumChan,
                              F untileMain,
                              const bool top2bottom)
{
    auto clamp = [](unsigned v, unsigned lo, unsigned hi) -> unsigned {
        return std::min<unsigned>(std::max<unsigned>(lo, v), hi);
    };

    fb_util::Tiler tiler(w, h);
    unsigned sx = clamp(std::min<unsigned>(minX, maxX), 0, w - 1);
    unsigned ex = clamp(std::max<unsigned>(minX, maxX), 0, w - 1) + 1;
    unsigned sy = clamp(std::min<unsigned>(minY, maxY), 0, h - 1);
    unsigned ey = clamp(std::max<unsigned>(minY, maxY), 0, h - 1) + 1;
    unsigned currW = ex - sx;
    unsigned currH = ey - sy;
    tbb::blocked_range<unsigned> range(sy, ey, 8);
    tbb::parallel_for(range, [&](const tbb::blocked_range<unsigned> &r) {
            for (unsigned y = r.begin(); y < r.end(); ++y) {
                unsigned currSx = (sx >> 3) << 3;
                for (unsigned x = currSx; x < ex; x += 8) {
                    unsigned tileOfs = tiler.linearCoordsToTiledOffset(x, y);
                    unsigned scanLength = std::min<unsigned>(ex - x, 8);
                    for (unsigned pixOfs = 0; pixOfs < scanLength; ++pixOfs) {
                        if (x + pixOfs < sx) continue;

                        unsigned dstOfs = 0;
                        if (top2bottom) {
                            dstOfs = ((currH - 1 - (y - sy)) * currW + (x - sx) + pixOfs) * dstNumChan;
                        } else {
                            dstOfs = ((y - sy) * currW + (x - sx) + pixOfs) * dstNumChan;
                        }
                        untileMain(tileOfs, pixOfs, dstOfs);
                    }
                }
            }
        });
}
#endif // end !SINGLE_THREAD

#ifdef SINGLE_THREAD
template <typename F>
void untileSinglePixelLoop(const unsigned w,
                           const unsigned h,
                           const unsigned dstNumChan,
                           F untileMain,
                           const bool top2bottom)
{
    fb_util::Tiler tiler(w, h);
    for (unsigned y = 0; y < h; ++y) {
        for (unsigned x = 0; x < w; x += 8) {
            unsigned tileOfs = tiler.linearCoordsToTiledOffset(x, y);
            unsigned scanLength = std::min<unsigned>(w - x, 8);
            for (unsigned pixOfs = 0; pixOfs < scanLength; ++pixOfs) {
                unsigned dstOfs = 0;
                if (top2bottom) {
                    dstOfs = ((h - 1 - y) * w + x + pixOfs) * dstNumChan;
                } else {
                    dstOfs = (y * w + x + pixOfs) * dstNumChan;
                }
                untileMain(tileOfs, pixOfs, dstOfs);
            }
        }
    }
}
#else // else SINGLE_THREAD
template <typename F>
void untileSinglePixelLoop(const unsigned w,
                           const unsigned h,
                           const unsigned dstNumChan,
                           F untileMain,
                           const bool top2bottom)
{
    fb_util::Tiler tiler(w, h);
    tbb::blocked_range<unsigned> range(0, h, 8);
    tbb::parallel_for(range, [&](const tbb::blocked_range<unsigned> &r) {
            for (unsigned y = r.begin(); y < r.end(); ++y) {
                for (unsigned x = 0; x < w; x += 8) {
                    unsigned tileOfs = tiler.linearCoordsToTiledOffset(x, y);
                    unsigned scanLength = std::min<unsigned>(w - x, 8);
                    for (unsigned pixOfs = 0; pixOfs < scanLength; ++pixOfs) {
                        unsigned dstOfs = 0;
                        if (top2bottom) {
                            dstOfs = ((h - 1 - y) * w + x + pixOfs) * dstNumChan;
                        } else {
                            dstOfs = (y * w + x + pixOfs) * dstNumChan;
                        }
                        untileMain(tileOfs, pixOfs, dstOfs);
                    }
                }
            }
        });
}
#endif // end !SINGLE_THREAD

#ifdef SINGLE_THREAD
template <typename F>
void untileDualPixelLoop(const unsigned w,
                         const unsigned h,
                         const unsigned dstNumChan,
                         F untileMain,
                         const bool top2bottom)
{
    fb_util::Tiler tiler(w, h);
    for (unsigned y = 0; y < h; ++y) {
        for (unsigned x = 0; x < w; x += 8) {
            unsigned tileOfs = tiler.linearCoordsToTiledOffset(x, y);
            unsigned scanLength = std::min<unsigned>(w - x, 8);
            for (unsigned pixOfs = 0; pixOfs < scanLength; pixOfs += 2) {
                unsigned dstOfs = 0;
                if (top2bottom) {
                    dstOfs = ((h - 1 - y) * w + x + pixOfs) * dstNumChan;
                } else {
                    dstOfs = (y * w + x + pixOfs) * dstNumChan;
                }
                untileMain(tileOfs, pixOfs, dstOfs);
            }
        }
    }
}
#else // else SINGLE_THREAD
template <typename F>
void untileDualPixelLoop(const unsigned w,
                         const unsigned h,
                         const unsigned dstNumChan,
                         F untileMain,
                         const bool top2bottom)
{
    fb_util::Tiler tiler(w, h);
    tbb::blocked_range<unsigned> range(0, h, 8);
    tbb::parallel_for(range, [&](const tbb::blocked_range<unsigned> &r) {
            for (unsigned y = r.begin(); y < r.end(); ++y) {
                for (unsigned x = 0; x < w; x += 8) {
                    unsigned tileOfs = tiler.linearCoordsToTiledOffset(x, y);
                    unsigned scanLength = std::min<unsigned>(w - x, 8);
                    for (unsigned pixOfs = 0; pixOfs < scanLength; pixOfs += 2) {
                        unsigned dstOfs = 0;
                        if (top2bottom) {
                            dstOfs = ((h - 1 - y) * w + x + pixOfs) * dstNumChan;
                        } else {
                            dstOfs = (y * w + x + pixOfs) * dstNumChan;
                        }
                        untileMain(tileOfs, pixOfs, dstOfs);
                    }
                }
            }
        });
}
#endif // end !SINGLE_THREAD    

} // namespace grid_util
} // namespace scene_rdl2

