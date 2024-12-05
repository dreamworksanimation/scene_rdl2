// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <scene_rdl2/common/math/Viewport.h>

#include <algorithm>

// Basically we should use multi-thread version.
// This single thread mode is used debugging and performance comparison reason mainly.
//#define SINGLE_THREAD

namespace scene_rdl2 {
namespace grid_util {

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
    auto clamp = [](unsigned v, unsigned lo, unsigned hi) -> unsigned {
        return std::min<unsigned>(std::max<unsigned>(lo, v), hi);
    };

    fb_util::Tiler tiler(w, h);
    const unsigned sx = clamp(std::min<unsigned>(minX, maxX), 0, w - 1);
    const unsigned ex = clamp(std::max<unsigned>(minX, maxX), 0, w - 1) + 1;
    const unsigned sy = clamp(std::min<unsigned>(minY, maxY), 0, h - 1);
    const unsigned ey = clamp(std::max<unsigned>(minY, maxY), 0, h - 1) + 1;
    const unsigned currW = ex - sx;
    const unsigned currH = ey - sy;
    for (unsigned y = sy; y < ey; ++y) {
        const unsigned currSx = (sx >> 3) << 3;
        const unsigned slOfsPix = ((top2bottom) ? (currH - 1 - (y - sy)) : (y - sy)) * currW;
        for (unsigned x = currSx; x < ex; x += 8) {
            const unsigned tileOfs = tiler.linearCoordsToTiledOffset(x, y);
            const unsigned scanLength = std::min<unsigned>(ex - x, 8);
            const unsigned ofs = (slOfsPix + (x - sx)) * dstNumChan;
            for (unsigned pixOfs = 0; pixOfs < scanLength; ++pixOfs) {
                if (x + pixOfs < sx) continue;
                const unsigned dstOfs = ofs + pixOfs * dstNumChan;
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
    const unsigned sx = clamp(std::min<unsigned>(minX, maxX), 0, w - 1);
    const unsigned ex = clamp(std::max<unsigned>(minX, maxX), 0, w - 1) + 1;
    const unsigned sy = clamp(std::min<unsigned>(minY, maxY), 0, h - 1);
    const unsigned ey = clamp(std::max<unsigned>(minY, maxY), 0, h - 1) + 1;
    const unsigned currW = ex - sx;
    const unsigned currH = ey - sy;
    tbb::blocked_range<unsigned> range(sy, ey, 8);
    tbb::parallel_for(range, [&](const tbb::blocked_range<unsigned> &r) {
            for (unsigned y = r.begin(); y < r.end(); ++y) {
                const unsigned currSx = (sx >> 3) << 3;
                const unsigned slOfsPix = ((top2bottom) ? (currH - 1 - (y - sy)) : (y - sy)) * currW;
                for (unsigned x = currSx; x < ex; x += 8) {
                    const unsigned tileOfs = tiler.linearCoordsToTiledOffset(x, y); 
                    const unsigned scanLength = std::min<unsigned>(ex - x, 8);
                    const unsigned ofs = (slOfsPix + (x - sx)) * dstNumChan;
                    for (unsigned pixOfs = 0; pixOfs < scanLength; ++pixOfs) {
                        if (x + pixOfs < sx) continue;
                        const unsigned dstOfs = ofs + pixOfs * dstNumChan;
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
        const unsigned slOfsPix = ((top2bottom) ? (h - 1 - y) : y) * w;
        for (unsigned x = 0; x < w; x += 8) {
            const unsigned tileOfs = tiler.linearCoordsToTiledOffset(x, y);
            const unsigned scanLength = std::min<unsigned>(w - x, 8);
            unsigned dstOfs = (slOfsPix + x) * dstNumChan;
            for (unsigned pixOfs = 0; pixOfs < scanLength; ++pixOfs) {
                untileMain(tileOfs, pixOfs, dstOfs);
                dstOfs += dstNumChan;
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
                const unsigned slOfsPix = ((top2bottom) ? (h - 1 - y) : y) * w; 
                for (unsigned x = 0; x < w; x += 8) {
                    const unsigned tileOfs = tiler.linearCoordsToTiledOffset(x, y);
                    const unsigned scanLength = std::min<unsigned>(w - x, 8);
                    unsigned dstOfs = (slOfsPix + x) * dstNumChan;
                    for (unsigned pixOfs = 0; pixOfs < scanLength; ++pixOfs) {
                        untileMain(tileOfs, pixOfs, dstOfs);
                        dstOfs += dstNumChan;
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
            const unsigned tileOfs = tiler.linearCoordsToTiledOffset(x, y);
            const unsigned scanLength = std::min<unsigned>(w - x, 8);
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
                    const unsigned tileOfs = tiler.linearCoordsToTiledOffset(x, y);
                    const unsigned scanLength = std::min<unsigned>(w - x, 8);
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

} // namespace grid_util
} // namespace scene_rdl2

