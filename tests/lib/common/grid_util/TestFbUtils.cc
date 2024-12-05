// Copyright 2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "TestFbUtils.h"

#include <scene_rdl2/common/fb_util/Tiler.h>
#include <scene_rdl2/common/grid_util/FbUtils.h>
#include <scene_rdl2/common/math/Viewport.h>
#include <scene_rdl2/render/cache/ValueContainerUtils.h>
#include <scene_rdl2/render/util/StrUtil.h>

//#include <iostream>

namespace scene_rdl2 {
namespace grid_util {
namespace unittest {

void
TestFbUtils::testUntileSinglePixelLoop()
{
    CPPUNIT_ASSERT("testUntileSinglePixelLoop" && testUntileSinglePixelLoopMain());
}

bool
TestFbUtils::testUntileSinglePixelLoopMain() const
{
    const unsigned w = 1920;
    const unsigned h = 1080;
    /* for debug
    const unsigned w = 16;
    const unsigned h = 8;
    */

    bool result = true;
    if (!runTestUntileSinglePixel(w, h, false, false, 0, 0, 0, 0)) result = false;
    if (!runTestUntileSinglePixel(w, h, true, false, 0, 0, 0, 0)) result = false;
    if (!runTestUntileSinglePixel(w, h, false, true, 10, 20, w - 10, h - 20)) result = false;
    if (!runTestUntileSinglePixel(w, h, true, true, 10, 20, w - 10, h - 20)) result = false;

    return result;
}

bool
TestFbUtils::runTestUntileSinglePixel(const unsigned width,
                                      const unsigned height,
                                      const bool top2Btm,
                                      const bool roiFlag,
                                      const unsigned minX,
                                      const unsigned minY,
                                      const unsigned maxX,
                                      const unsigned maxY) const
{
    std::ostringstream ostr;
    ostr << "runTestUntileSinglePixel w:" << width << " h:" << height << " top2Btm:" << str_util::boolStr(top2Btm);
    if (roiFlag) {
        ostr << " roi(minX:" << minX << " minY:" << minY << " maxX:" << maxX << " maxY:" << maxY << ")";
    }

    fb_util::VariablePixelBuffer buffTiled = setupDummyTiledBuffer(width, height, roiFlag, minX, minY, maxX, maxY);
    if (!verifyUntileSinglePixel(buffTiled, top2Btm, roiFlag, minX, minY, maxX, maxY)) {
        std::cerr << ostr.str() << " => NG\n";
        return false;
    }
    std::cerr << ostr.str() << " => OK\n";
    return true;
}

fb_util::VariablePixelBuffer
TestFbUtils::setupDummyTiledBuffer(const unsigned width, const unsigned height,
                                   const bool roiFlag,
                                   const unsigned minX, const unsigned minY,
                                   const unsigned maxX, const unsigned maxY) const
{
    auto clamp = [](const unsigned v, const unsigned lo, const unsigned hi) -> unsigned {
        return std::min<unsigned>(std::max<unsigned>(lo, v), hi);
    };

    constexpr unsigned chanTotal = 3;

    const unsigned sx = (roiFlag) ? clamp(minX, 0, width - 1) : 0;
    const unsigned ex = (roiFlag) ? clamp(maxX, 0, width - 1) + 1 : width;
    const unsigned sy = (roiFlag) ? clamp(minY, 0, height - 1) : 0;
    const unsigned ey = (roiFlag) ? clamp(maxY, 0, height - 1) + 1 : height;

    fb_util::VariablePixelBuffer buffTiled;
    buffTiled.init(fb_util::VariablePixelBuffer::RGB888, width, height);

    fb_util::Rgb888Buffer& rgb888Buff = buffTiled.getRgb888Buffer();
    unsigned char* rgb888Addr = reinterpret_cast<unsigned char*>(rgb888Buff.getData());

    fb_util::Tiler tiler(width, height);
    unsigned char uc = 0;
    for (unsigned ly = sy; ly < ey; ++ly) {
        for (unsigned lx = sx; lx < ex; ++lx) {
            const unsigned offsetTiled = tiler.linearCoordsToTiledOffset(lx, ly);
            const unsigned offset = offsetTiled * chanTotal;
            for (unsigned c = 0; c < chanTotal; ++c) {
                rgb888Addr[offset + c] = uc;
            }
            uc++;
        }
    }

    /* for debug
    const size_t buffSize = width * height * 3;
    std::cerr << cache::ValueContainerUtil::hexDump("rgb888Buff", rgb888Buff.getData(), buffSize) << '\n';
    */

    return buffTiled;
}

bool
TestFbUtils::verifyUntileSinglePixel(const fb_util::VariablePixelBuffer& buffTiled, const bool top2Btm,
                                     const bool roiFlag,
                                     const unsigned minX, const unsigned minY,
                                     const unsigned maxX, const unsigned maxY) const
{
    const fb_util::Rgb888Buffer& rgb888Buff = buffTiled.getRgb888Buffer();
    const unsigned char* rgb888Addr = reinterpret_cast<const unsigned char*>(rgb888Buff.getData());

    math::Viewport roi(minX, minY, maxX, maxY);
    math::Viewport* roiPtr = (roiFlag) ? &roi : nullptr;

    bool result = true;
    const unsigned width = buffTiled.getWidth();
    const unsigned height = buffTiled.getHeight();
    const unsigned currW = (roiFlag) ? (maxX - minX + 1) : width;
    const unsigned currH = (roiFlag) ? (maxY - minY + 1) : height;
    const unsigned chanTotal = 3;
    unsigned errorTotal = 0;
    constexpr unsigned errorMax = 10;
    untileSinglePixelMainLoop(width, height, roiPtr, chanTotal,
                              [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) {
                                  const unsigned char* srcPix = rgb888Addr + (tileOfs + pixOfs) * 3;
                                  const unsigned char uc0 = srcPix[0];
                                  const unsigned char uc1 = srcPix[1];
                                  const unsigned char uc2 = srcPix[2];
                                  if (uc0 != uc1 || uc0 != uc2) {
                                      result = false;
                                      if (errorTotal < errorMax) {
                                          std::cerr << ">> ERROR-A : dstOfs:" << std::dec << dstOfs << '\n';
                                          errorTotal++;
                                      }
                                  } else {
                                      const unsigned id = dstOfs / chanTotal;
                                      const unsigned outY = id / currW;
                                      const unsigned tgtY = (top2Btm) ? (currH - outY - 1) : outY;
                                      const unsigned outX = id % currW;
                                      const unsigned tgtPixId = tgtY * currW + outX;
                                      const unsigned char tgtV = tgtPixId % 256;
                                      if (tgtV != uc0) {
                                          result = false;
                                          if (errorTotal < errorMax) {
                                              std::cerr << ">> ERROR-B :"
                                                        << " tileOfs:" << tileOfs << " pixOfs:" << pixOfs
                                                        << " dstOfs:" << std::dec << dstOfs
                                                        << " (id:" << id << " outY:" << outY << " outX:" << outX << ")"
                                                        << std::hex
                                                        << " tgtV:0x" << (int)tgtV << " uc0:0x" << (int)uc0 << '\n';
                                              errorTotal++;
                                          }
                                      }
                                  }
                              },
                              top2Btm);

    return result;
}

} // namespace unittest
} // namespace grid_util
} // namespace scene_rdl2
