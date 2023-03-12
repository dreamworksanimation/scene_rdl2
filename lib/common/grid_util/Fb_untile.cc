// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#include "Fb.h"
#include "FbUtils.h"

#include <scene_rdl2/common/fb_util/GammaF2C.h>
#include <scene_rdl2/common/fb_util/SrgbF2C.h>
#include <scene_rdl2/common/rec_time/RecTime.h>

#include <functional>

//
// Following directives are used to control dumping timing result mainly used for debug/optimizing
//
#define UNTILE_TIMING_TEST_UC_BEAUTYRGB    0 // beauty-rgb      8bit
#define UNTILE_TIMING_TEST_UC_ALPHA        0 // alpha           8bit
#define UNTILE_TIMING_TEST_UC_PIXELINFO    0
#define UNTILE_TIMING_TEST_UC_HEATMAP      0
#define UNTILE_TIMING_TEST_UC_WEIGHTBUFFER 0
#define UNTILE_TIMING_TEST_UC_BEAUTYAUX    0 // beautyAux-rgb   8bit
#define UNTILE_TIMING_TEST_UC_ALPHAAUX     0 // alphaAux        8bit
#define UNTILE_TIMING_TEST_UC_RENDEROUTPUT 0

#define UNTILE_TIMING_TEST_F_BEAUTY        0 // beauty-rgba    32bit
#define UNTILE_TIMING_TEST_F_BEAUTYRGB     0 // beauty-rgb     32bit
#define UNTILE_TIMING_TEST_F_ALPHA         0 // alpha          32bit
#define UNTILE_TIMING_TEST_F_PIXELINFO     0
#define UNTILE_TIMING_TEST_F_HEATMAP       0
#define UNTILE_TIMING_TEST_F_WEIGHTBUFFER  0
#define UNTILE_TIMING_TEST_F_BEAUTYODD     0 // beautyOdd-rgba 32bit (beautyAux+alphaAux)
#define UNTILE_TIMING_TEST_F_BEAUTYAUX     0 // beautyAux-rgb  32bit
#define UNTILE_TIMING_TEST_F_ALPHAAUX      0 // alphaAux       32bit
#define UNTILE_TIMING_TEST_F_RENDEROUTPUT  0

#define UNTILE_TIMING_TEST_F4_BEAUTYRGB     0 // beauty-rgb     32bit
#define UNTILE_TIMING_TEST_F4_ALPHA         0 // alpha          32bit
#define UNTILE_TIMING_TEST_F4_HEATMAP       0
#define UNTILE_TIMING_TEST_F4_WEIGHTBUFFER  0
#define UNTILE_TIMING_TEST_F4_BEAUTYAUX     0 // beautyAux-rgb  32bit
#define UNTILE_TIMING_TEST_F4_ALPHAAUX      0 // alphaAux       32bit
#define UNTILE_TIMING_TEST_F4_RENDEROUTPUT  0

namespace scene_rdl2 {
namespace grid_util {

//---------------------------------------------------------------------------------------------------------------
//    
// 8bit pixel value APIs
//
//---------------------------------------------------------------------------------------------------------------
    
void
Fb::untileBeauty(const bool isSrgb,
                 const bool top2bottom,
                 const math::Viewport *roi,
                 UCArray &rgbFrame) const
{
    std::function<void(const float *, uint8_t [3])> f4ToUc3Conversion;
    if (!isSrgb) {
        f4ToUc3Conversion = [](const float *rgba, uint8_t out[3]) {
            out[0] = fb_util::GammaF2C::g22(rgba[0]);
            out[1] = fb_util::GammaF2C::g22(rgba[1]);
            out[2] = fb_util::GammaF2C::g22(rgba[2]);
        };
    } else {
        f4ToUc3Conversion = [](const float *rgba, uint8_t out[3]) {
            out[0] = fb_util::SrgbF2C::sRGB(rgba[0]);
            out[1] = fb_util::SrgbF2C::sRGB(rgba[1]);
            out[2] = fb_util::SrgbF2C::sRGB(rgba[2]);
        };
    }

    untileMain<(bool)UNTILE_TIMING_TEST_UC_BEAUTYRGB>
        ((unsigned)3, // output numChannels
         top2bottom,
         roi,
         [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) { // untilePixFunc()
            const float *srcPix =
                reinterpret_cast<const float *>(mRenderBufferTiled.getData()) + (tileOfs + pixOfs) * 4;
            f4ToUc3Conversion(srcPix, &rgbFrame[dstOfs]);
         },
         "untileBeauty(uc) untile",
         rgbFrame);
}

void
Fb::untileAlpha(const bool isSrgb,
                const bool top2bottom,
                const math::Viewport *roi,
                UCArray &rgbFrame) const
{
    std::function<unsigned char(float)> f2ucConversion =
        (!isSrgb)? fb_util::GammaF2C::g22: fb_util::SrgbF2C::sRGB;

    untileMain<(bool)UNTILE_TIMING_TEST_UC_ALPHA>
        ((unsigned)3, // output numChannels
         top2bottom,
         roi,
         [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) { // untilePixFunc()
            const float *srcPix =
                reinterpret_cast<const float *>(mRenderBufferTiled.getData()) + (tileOfs + pixOfs) * 4;
            uint8_t uc = f2ucConversion(srcPix[3]);
            rgbFrame[dstOfs  ] = uc;
            rgbFrame[dstOfs+1] = uc;
            rgbFrame[dstOfs+2] = uc;
         },
         "untileAlpha(uc) untile",
         rgbFrame);
}

void
Fb::untilePixelInfo(const bool isSrgb,
                    const bool top2bottom,
                    const math::Viewport *roi,
                    UCArray &rgbFrame) const
{
    std::function<unsigned char(float)> f2ucConversion =
        (!isSrgb)? fb_util::GammaF2C::g22: fb_util::SrgbF2C::sRGB;

    float min, max;
    untileExecMain<(bool)UNTILE_TIMING_TEST_UC_PIXELINFO>
        ([&]() { // execFunc()
            computeMinMaxPixelInfoForDisplay(min, max);
         }, "untilePixelInfo(uc) minMax");

    untileMain<(bool)UNTILE_TIMING_TEST_UC_PIXELINFO>
        ((unsigned)3, // output numChannls
         top2bottom,
         roi,
         [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) { // untilePixFunc()
            const float *srcPix =
                reinterpret_cast<const float *>(mPixelInfoBufferTiled.getData()) + (tileOfs + pixOfs);
            float v;
            if (min == FLT_MAX) {
                v = 0.0f;   // no active data
            } else {
                v = 1.0f - ((*srcPix) - min) / (max - min);
            }
            unsigned char uc = f2ucConversion(v);
            rgbFrame[dstOfs    ] = uc;
            rgbFrame[dstOfs + 1] = uc;                
            rgbFrame[dstOfs + 2] = uc;            
         },
         "untilePixelInfo(uc) untile",
         rgbFrame);
}

void
Fb::untileHeatMap(const bool isSrgb,
                  const bool top2bottom,
                  const math::Viewport *roi,
                  UCArray &rgbFrame) const
{
    float min, max;
    untileExecMain<(bool)UNTILE_TIMING_TEST_UC_HEATMAP>
        ([&]() { // execFunc()
            computeMinMaxHeatMapForDisplay(min, max);
         }, "untileHeatMap(uc) minMax");

    untileMain<(bool)UNTILE_TIMING_TEST_UC_HEATMAP>
        ((unsigned)3, // output numChannels
         top2bottom, roi,
         [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) { // untilePixFunc()
            const float *srcPix =
                reinterpret_cast<const float *>(mHeatMapSecBufferTiled.getData()) + (tileOfs + pixOfs);
            float v;
            if (min == FLT_MAX) {
                v = 0.0f;   // no active data
            } else {
                v = ((*srcPix) - min) / (max - min);
            }
            f2HeatMapCol255(v, isSrgb, &rgbFrame[dstOfs]);
         },
         "untileHeatMap(uc) untile",
         rgbFrame);
}

void
Fb::untileWeightBuffer(const bool isSrgb,
                       const bool top2bottom,
                       const math::Viewport *roi,
                       UCArray &rgbFrame) const
{
    float max;
    size_t totalNonZeroPixels = 0;
    untileExecMain<(bool)UNTILE_TIMING_TEST_UC_WEIGHTBUFFER>
        ([&]() { // execFunc()
            totalNonZeroPixels = computeMaxWeightBufferForDisplay(max);
         }, "untileWeightBuffer(uc) max");

    std::function<unsigned char(float)> f2ucConversion =
        (!isSrgb)? fb_util::GammaF2C::g22: fb_util::SrgbF2C::sRGB;

    untileMain<(bool)UNTILE_TIMING_TEST_UC_WEIGHTBUFFER>
        ((unsigned)3, // output numChannels
         top2bottom,
         roi,
         [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) { // untilePixFunc()
            const float *srcPix =
                reinterpret_cast<const float *>(mWeightBufferTiled.getData()) + (tileOfs + pixOfs);
            float v;
            if (!totalNonZeroPixels) {
                v = 0.0f;   // no active data
            } else {
                v = (*srcPix) / max;
            }
            uint8_t uc = f2ucConversion(v);
            rgbFrame[dstOfs    ] = uc;
            rgbFrame[dstOfs + 1] = uc;
            rgbFrame[dstOfs + 2] = uc;
         },
         "untileWeightBuffer(uc) untile",
         rgbFrame);
}

void
Fb::untileBeautyAux(const bool isSrgb,
                    const bool top2bottom,
                    const math::Viewport *roi,
                    UCArray &rgbFrame) const
{
    std::function<void(const float *, uint8_t [3])> f4ToUc3Conversion;
    if (!isSrgb) {
        f4ToUc3Conversion = [](const float *rgba, uint8_t out[3]) {
            out[0] = fb_util::GammaF2C::g22(rgba[0]);
            out[1] = fb_util::GammaF2C::g22(rgba[1]);
            out[2] = fb_util::GammaF2C::g22(rgba[2]);
        };
    } else {
        f4ToUc3Conversion = [](const float *rgba, uint8_t out[3]) {
            out[0] = fb_util::SrgbF2C::sRGB(rgba[0]);
            out[1] = fb_util::SrgbF2C::sRGB(rgba[1]);
            out[2] = fb_util::SrgbF2C::sRGB(rgba[2]);
        };
    }

    untileMain<(bool)UNTILE_TIMING_TEST_UC_BEAUTYAUX>
        ((unsigned)3, // output numChannels
         top2bottom,
         roi,
         [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) { // untilePixFunc()
            const float *srcPix =
                reinterpret_cast<const float *>(mRenderBufferOddTiled.getData()) + (tileOfs + pixOfs) * 4;
            f4ToUc3Conversion(srcPix, &rgbFrame[dstOfs]);
         },
         "untileBeautyAux(uc) untile",
         rgbFrame);
}

void
Fb::untileAlphaAux(const bool isSrgb,
                   const bool top2bottom,
                   const math::Viewport *roi,
                   UCArray &rgbFrame) const
{
    std::function<unsigned char(float)> f2ucConversion =
        (!isSrgb)? fb_util::GammaF2C::g22: fb_util::SrgbF2C::sRGB;

    untileMain<(bool)UNTILE_TIMING_TEST_UC_ALPHAAUX>
        ((unsigned)3, // output numChannels
         top2bottom,
         roi,
         [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) { // untilePixFunc()
            const float *srcPix =
                reinterpret_cast<const float *>(mRenderBufferOddTiled.getData()) + (tileOfs + pixOfs) * 4;
            uint8_t uc = f2ucConversion(srcPix[3]);
            rgbFrame[dstOfs  ] = uc;
            rgbFrame[dstOfs+1] = uc;
            rgbFrame[dstOfs+2] = uc;
         },
         "untileAlphaAux(uc) untile",
         rgbFrame);
}

void
Fb::untileRenderOutput(const int aovId,
                       const bool isSrgb,
                       const bool top2bottom,
                       const math::Viewport *roi,
                       const bool closestFilterDepthOutput,
                       UCArray &rgbFrame) const
{
    FbAovShPtr fbAov;
    if (!getAov2(aovId, fbAov)) {
        unsigned dataSize = 0;
        if (roi) dataSize = roi->width() * roi->height() * 3;
        else     dataSize = getWidth() * getHeight() * 3;
        memset(static_cast<void *>(rgbFrame.data()), 0x0, dataSize);
        return;
    }

    untileRenderOutputMain(fbAov, isSrgb, top2bottom, roi, closestFilterDepthOutput, rgbFrame);
}

void
Fb::untileRenderOutput(const std::string &aovName,
                       const bool isSrgb,
                       const bool top2bottom,
                       const math::Viewport *roi,
                       const bool closestFilterDepthOutput,
                       UCArray &rgbFrame) const
{
    FbAovShPtr fbAov;
    if (!getAov2(aovName, fbAov)) {
        unsigned dataSize = 0;
        if (roi) dataSize = roi->width() * roi->height() * 3;
        else     dataSize = getWidth() * getHeight() * 3;
        memset(static_cast<void *>(rgbFrame.data()), 0x0, dataSize);
        return;
    }

    untileRenderOutputMain(fbAov, isSrgb, top2bottom, roi, closestFilterDepthOutput, rgbFrame);
}

//---------------------------------------------------------------------------------------------------------------
//    
// 32bit pixel value APIs
//
//---------------------------------------------------------------------------------------------------------------

void
Fb::untileBeauty(const bool top2bottom,
                 const math::Viewport *roi,
                 FArray &rgba) const
{
    untileMain<(bool)UNTILE_TIMING_TEST_F_BEAUTY>
        ((unsigned)4, // output numChannels
         top2bottom,
         roi,
         [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) { // untilePixFunc()
            const float *srcPix =
                reinterpret_cast<const float *>(mRenderBufferTiled.getData()) + (tileOfs + pixOfs) * 4;
            rgba[dstOfs    ] = srcPix[0];
            rgba[dstOfs + 1] = srcPix[1];
            rgba[dstOfs + 2] = srcPix[2];
            rgba[dstOfs + 3] = srcPix[3];
         },
         "untileBeauty(f) untile",
         rgba);
}

void
Fb::untileBeautyRGB(const bool top2bottom,
                    const math::Viewport *roi,
                    FArray &rgb) const
{
    untileMain<(bool)UNTILE_TIMING_TEST_F_BEAUTYRGB>
        ((unsigned)3, // output numChannels
         top2bottom,
         roi,
         [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) { // untilePixFunc()
            const float *srcPix =
                reinterpret_cast<const float *>(mRenderBufferTiled.getData()) + (tileOfs + pixOfs) * 4;
            rgb[dstOfs    ] = srcPix[0];
            rgb[dstOfs + 1] = srcPix[1];
            rgb[dstOfs + 2] = srcPix[2];
         },
         "untileBeautyRGB(f) untile",
         rgb);
}

void
Fb::untileBeautyRGBF4(const bool top2bottom,
                      const math::Viewport *roi,
                      FArray &data) const
{
    untileMain<(bool)UNTILE_TIMING_TEST_F4_BEAUTYRGB>
        ((unsigned)4, // output numChannels
         top2bottom,
         roi,
         [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) { // untilePixFunc()
            const float *srcPix =
                reinterpret_cast<const float *>(mRenderBufferTiled.getData()) + (tileOfs + pixOfs) * 4;
            data[dstOfs    ] = srcPix[0];
            data[dstOfs + 1] = srcPix[1];
            data[dstOfs + 2] = srcPix[2];
            data[dstOfs + 3] = 0.0f;
         },
         "untileBeautyRGBF4(f) untile",
         data);
}

void
Fb::untileAlpha(const bool top2bottom,
                const math::Viewport *roi,
                FArray &alpha) const
{
    untileMain<(bool)UNTILE_TIMING_TEST_F_ALPHA>
        ((unsigned)1, // output numChannels
         top2bottom,
         roi,
         [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) { // untilePixFunc()
            const float *srcPix =
                reinterpret_cast<const float *>(mRenderBufferTiled.getData()) + (tileOfs + pixOfs) * 4;
            alpha[dstOfs] = srcPix[3];
         },
         "untileAlpha(f) untile",
         alpha);
}

void
Fb::untileAlphaF4(const bool top2bottom,
                  const math::Viewport *roi,
                  FArray &data) const
{
    untileMain<(bool)UNTILE_TIMING_TEST_F4_ALPHA>
        ((unsigned)4, // output numChannels
         top2bottom,
         roi,
         [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) { // untilePixFunc()
            const float *srcPix =
                reinterpret_cast<const float *>(mRenderBufferTiled.getData()) + (tileOfs + pixOfs) * 4;
            data[dstOfs    ] = srcPix[3];
            data[dstOfs + 1] = srcPix[3];
            data[dstOfs + 2] = srcPix[3];
            data[dstOfs + 3] = srcPix[3];
         },
         "untileAlphaF4(f) untile",
         data);
}

void
Fb::untilePixelInfo(const bool top2bottom,
                    const math::Viewport *roi,
                    FArray &data) const
{
    untileMain<(bool)UNTILE_TIMING_TEST_F_PIXELINFO>
        ((unsigned)1, // output numChannels
         top2bottom,
         roi,
         [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) { // untilePixFunc()
            const float *srcPix =
                reinterpret_cast<const float *>(mPixelInfoBufferTiled.getData()) + (tileOfs + pixOfs);
            data[dstOfs] = srcPix[0];
         },
         "untilePixelInfo(f) untile",
         data);
}

void
Fb::untileHeatMap(const bool top2bottom,
                  const math::Viewport *roi,
                  FArray &data) const
{
    untileMain<(bool)UNTILE_TIMING_TEST_F_HEATMAP>
        ((unsigned)1, // output numChannels
         top2bottom,
         roi,
         [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) { // untilePixFunc()
            const float *srcPix =
                reinterpret_cast<const float *>(mHeatMapSecBufferTiled.getData()) + (tileOfs + pixOfs);
            data[dstOfs] = srcPix[0];
         },
         "untileHeatMap(f) untile",
         data);
}

void
Fb::untileHeatMapF4(const bool top2bottom,
                    const math::Viewport *roi,
                    FArray &data) const
{
    untileMain<(bool)UNTILE_TIMING_TEST_F4_HEATMAP>
        ((unsigned)4, // output numChannels
         top2bottom,
         roi,
         [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) { // untilePixFunc()
            const float *srcPix =
                reinterpret_cast<const float *>(mHeatMapSecBufferTiled.getData()) + (tileOfs + pixOfs);
            data[dstOfs    ] = srcPix[0];
            data[dstOfs + 1] = srcPix[0];
            data[dstOfs + 2] = srcPix[0];
            data[dstOfs + 3] = srcPix[0];
         },
         "untileHeatMapF4(f) untile",
         data);
}

void
Fb::untileWeightBuffer(const bool top2bottom,
                       const math::Viewport *roi,
                       FArray &data) const
{
    untileMain<(bool)UNTILE_TIMING_TEST_F_WEIGHTBUFFER>
        ((unsigned)1, // output numChannels
         top2bottom,
         roi,
         [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) { // untilePixFunc()
            const float *srcPix =
                reinterpret_cast<const float *>(mWeightBufferTiled.getData()) + (tileOfs + pixOfs);
            data[dstOfs] = srcPix[0];
         },
         "untileWeightBuffer(f) untile",
         data);
}

void
Fb::untileWeightBufferF4(const bool top2bottom,
                         const math::Viewport *roi,
                         FArray &data) const
{
    untileMain<(bool)UNTILE_TIMING_TEST_F4_WEIGHTBUFFER>
        ((unsigned)4, // output numChannels
         top2bottom,
         roi,
         [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) { // untilePixFunc()
            const float *srcPix =
                reinterpret_cast<const float *>(mWeightBufferTiled.getData()) + (tileOfs + pixOfs);
            data[dstOfs    ] = srcPix[0];
            data[dstOfs + 1] = srcPix[0];
            data[dstOfs + 2] = srcPix[0];
            data[dstOfs + 3] = srcPix[0];
         },
         "untileWeightBufferF4(f) untile",
         data);
}

void
Fb::untileBeautyOdd(const bool top2bottom,
                    const math::Viewport *roi,
                    FArray &rgba) const
{
    untileMain<(bool)UNTILE_TIMING_TEST_F_BEAUTYODD>
        ((unsigned)4, // output numChannels
         top2bottom,
         roi,
         [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) { // untilePixFunc()
            const float *srcPix =
                reinterpret_cast<const float *>(mRenderBufferOddTiled.getData()) + (tileOfs + pixOfs) * 4;
            rgba[dstOfs    ] = srcPix[0];
            rgba[dstOfs + 1] = srcPix[1];
            rgba[dstOfs + 2] = srcPix[2];
            rgba[dstOfs + 3] = srcPix[3];
         },
         "untileBeautyOdd(f) untile",
         rgba);
}

void    
Fb::untileBeautyAux(const bool top2bottom,
                    const math::Viewport *roi,
                    FArray &rgb) const
{
    untileMain<(bool)UNTILE_TIMING_TEST_F_BEAUTYAUX>
        ((unsigned)3, // output numChannels
         top2bottom,
         roi,
         [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) { // untilePixFunc()
            const float *srcPix =
                reinterpret_cast<const float *>(mRenderBufferOddTiled.getData()) + (tileOfs + pixOfs) * 4;
            rgb[dstOfs    ] = srcPix[0];
            rgb[dstOfs + 1] = srcPix[1];
            rgb[dstOfs + 2] = srcPix[2];
         },
         "untileBeautyAux(f) untile",
         rgb);
}

void    
Fb::untileBeautyAuxF4(const bool top2bottom,
                      const math::Viewport *roi,
                      FArray &data) const
{
    untileMain<(bool)UNTILE_TIMING_TEST_F4_BEAUTYAUX>
        ((unsigned)4, // output numChannels
         top2bottom,
         roi,
         [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) { // untilePixFunc()
            const float *srcPix =
                reinterpret_cast<const float *>(mRenderBufferOddTiled.getData()) + (tileOfs + pixOfs) * 4;
            data[dstOfs    ] = srcPix[0];
            data[dstOfs + 1] = srcPix[1];
            data[dstOfs + 2] = srcPix[2];
            data[dstOfs + 3] = 0.0f;
         },
         "untileBeautyAuxF4(f) untile",
         data);
}

void    
Fb::untileAlphaAux(const bool top2bottom,
                   const math::Viewport *roi,
                   FArray &alpha) const
{
    untileMain<(bool)UNTILE_TIMING_TEST_F_ALPHAAUX>
        ((unsigned)1, // output numChannels
         top2bottom,
         roi,
         [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) { // untilePixFunc()
            const float *srcPix =
                reinterpret_cast<const float *>(mRenderBufferOddTiled.getData()) + (tileOfs + pixOfs) * 4;
            alpha[dstOfs] = srcPix[3];
         },
         "untileAlphaAux(f) untile",
         alpha);
}

void    
Fb::untileAlphaAuxF4(const bool top2bottom,
                     const math::Viewport *roi,
                     FArray &data) const
{
    untileMain<(bool)UNTILE_TIMING_TEST_F4_ALPHAAUX>
        ((unsigned)4, // output numChannels
         top2bottom,
         roi,
         [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) { // untilePixFunc()
            const float *srcPix =
                reinterpret_cast<const float *>(mRenderBufferOddTiled.getData()) + (tileOfs + pixOfs) * 4;
            data[dstOfs    ] = srcPix[3];
            data[dstOfs + 1] = srcPix[3];
            data[dstOfs + 2] = srcPix[3];
            data[dstOfs + 3] = srcPix[3];
         },
         "untileAlphaAuxF4(f) untile",
         data);
}

int
Fb::untileRenderOutput(const int aovId,
                       const bool top2bottom,
                       const math::Viewport *roi,
                       const bool closestFilterDepthOutput,
                       FArray &data) const
{
    FbAovShPtr fbAov;
    if (!getAov2(aovId, fbAov)) {
        return 0;
    }

    return untileRenderOutputMain(fbAov, top2bottom, roi, closestFilterDepthOutput, data);
}

int
Fb::untileRenderOutput(const std::string &aovName,
                       const bool top2bottom,
                       const math::Viewport *roi,
                       const bool closestFilterDepthOutput,
                       FArray &data) const
{
    FbAovShPtr fbAov;
    if (!getAov2(aovName, fbAov)) {
        return 0;
    }

    return untileRenderOutputMain(fbAov, top2bottom, roi, closestFilterDepthOutput, data);
}

int
Fb::untileRenderOutputF4(const int aovId,
                         const bool top2bottom,
                         const math::Viewport* roi,
                         const bool closestFilterDepthOutput,
                         FArray& data) const
//
// Special RenderOutput data untile function for denoise operation. Set data into float4 pixel buffer
//
{
    FbAovShPtr fbAov;
    if (!getAov2(aovId, fbAov)) {
        return 0;
    }

    return untileRenderOutputMainF4(fbAov, top2bottom, roi, closestFilterDepthOutput, data);
}

int
Fb::untileRenderOutputF4(const std::string& aovName,
                         const bool top2bottom,
                         const math::Viewport* roi,
                         const bool closestFilterDepthOutput,
                         FArray& data) const
//
// Special RenderOutput data untile function for denoise operation. Set data into float4 pixel buffer
//
{
    FbAovShPtr fbAov;
    if (!getAov2(aovName, fbAov)) {
        return 0;
    }

    return untileRenderOutputMainF4(fbAov, top2bottom, roi, closestFilterDepthOutput, data);
}

//---------------------------------------------------------------------------------------------------------------

template <bool timingTest, typename T, typename UntilePixFunc>
void
Fb::untileMain(const unsigned numChannels, // outputData's numChannel
               const bool top2bottom,
               const math::Viewport *roi,
               UntilePixFunc untilePixFunc,
               const char *timingTestMsg,
               std::vector<T> &outData) const
//
// timingTestMsg is only used when timingTest = true
//
{
    unsigned w = getWidth();
    unsigned h = getHeight();
    if (roi) {
        outData.resize(roi->width() * roi->height() * numChannels);
    } else {
        outData.resize(w * h * numChannels);
    }

    auto untileMainFunc = [&]() {
        /* test code for AVX2
        //
        // gamma correct 2 pixels at once (AVX2 version) : This code is not support ROI yet.
        //
        untileDualPixelLoop(w, h, 3,
                            [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) {
                                untilePixFunc(tileOfs, pixOfs, dstOfs);
                            });
        */
        untileSinglePixelMainLoop(w, h, roi, numChannels,
                                  [&](unsigned tileOfs, unsigned pixOfs, unsigned dstOfs) {
                                      untilePixFunc(tileOfs, pixOfs, dstOfs);
                                  }, top2bottom);
    };

    untileExecMain<timingTest>(untileMainFunc, timingTestMsg);
}

template <bool timingTest, typename ExecFunc>
void
Fb::untileExecMain(ExecFunc execFunc,
                   const char *timingTestMsg) const
//
// timingTestMsg is only used when timingTest = true
//
{
    if (timingTest) {
        //
        // show measured timing result for debug
        //
        static rec_time::RecTimeLog recTimeLog;
        rec_time::RecTime recTime;
        recTime.start();

        execFunc();

        recTimeLog.add(recTime.end());
        if (recTimeLog.getTotal() == 24) {
            // every 24 call, dump information and reset
            std::cerr << ">> Fb_untile.cc " << timingTestMsg << ' '
                      << recTimeLog.getAverage() * 1000.0f << " ms" << std::endl;
            recTimeLog.reset();
        }
    } else {
        //
        // non timing measurement
        //
        execFunc();
    }
}

//---------------------------------------------------------------------------------------------------------------

void
Fb::f2HeatMapCol255(const float v, const bool isSrgb, unsigned char rgb[3]) const
//
// v : 0.0 ~ 1.0
//
{
    std::function<unsigned char(float)> f2ucConversion =
        (!isSrgb)? fb_util::GammaF2C::g22: fb_util::SrgbF2C::sRGB;

    static math::Vec3f red(1.0f, 0.0f, 0.0f);
    static math::Vec3f blue(0.0f, 0.0f, 1.0f);

    math::Vec3f c = (red - blue) * v + blue;
    rgb[0] = f2ucConversion(c[0]);
    rgb[1] = f2ucConversion(c[1]);
    rgb[2] = f2ucConversion(c[2]);
}

//---------------------------------------------------------------------------------------------------------------    

void
Fb::untileRenderOutputMain(const FbAovShPtr &fbAov,
                           const bool isSrgb,
                           const bool top2bottom,
                           const math::Viewport *roi,
                           const bool closestFilterDepthOutput,
                           UCArray &rgbFrame) const // pixTotal * 3 channel
{
    if (!fbAov->getStatus()) return; // just in case

    untileExecMain<(bool)UNTILE_TIMING_TEST_UC_RENDEROUTPUT>
        ([&]() {
            switch (fbAov->getReferenceType()) {
            case grid_util::FbReferenceType::UNDEF :
                if (roi) rgbFrame.resize(roi->width() * roi->height() * 3);
                else     rgbFrame.resize(getWidth() * getHeight() * 3);
                fbAov->untile(isSrgb, top2bottom, roi, closestFilterDepthOutput, rgbFrame);
                break;
            case grid_util::FbReferenceType::BEAUTY :
                untileBeauty(isSrgb, top2bottom, roi, rgbFrame);
                break;
            case grid_util::FbReferenceType::ALPHA :
                untileAlpha(isSrgb, top2bottom, roi, rgbFrame);
                break;
            case grid_util::FbReferenceType::HEAT_MAP :
                untileHeatMap(isSrgb, top2bottom, roi, rgbFrame);
                break;
            case grid_util::FbReferenceType::WEIGHT :
                untileWeightBuffer(isSrgb, top2bottom, roi, rgbFrame);
                break;
            case grid_util::FbReferenceType::BEAUTY_AUX :
                untileBeautyAux(isSrgb, top2bottom, roi, rgbFrame);
                break;
            case grid_util::FbReferenceType::ALPHA_AUX :
                untileAlphaAux(isSrgb, top2bottom, roi, rgbFrame);
                break;
            }
         }, "untileRenderOutputMain(uc) untile");
}

int
Fb::untileRenderOutputMain(const FbAovShPtr &fbAov,
                           const bool top2bottom,
                           const math::Viewport *roi,
                           const bool closestFilterDepthOutput,
                           FArray &data) const
{
    if (!fbAov->getStatus()) return 0; // just in case

    int numChan = 0;
    untileExecMain<(bool)UNTILE_TIMING_TEST_F_RENDEROUTPUT>
        ([&]() {
            switch (fbAov->getReferenceType()) {
            case grid_util::FbReferenceType::UNDEF :
                {
                    int numChan = (fbAov->getClosestFilterStatus() && closestFilterDepthOutput)?
                        1: // depth single channel
                        fbAov->getNumChan(); // this API already consider closestFilter status internally
                    int pixTotal = (roi)?
                        roi->width() * roi->height():
                        getWidth() * getHeight();
                    data.resize(pixTotal * numChan);
                }
                numChan = fbAov->untile(top2bottom, roi, closestFilterDepthOutput, data);
                break;
            case grid_util::FbReferenceType::BEAUTY :
                untileBeautyRGB(top2bottom, roi, data);
                numChan = 3;
                break;
            case grid_util::FbReferenceType::ALPHA :
                untileAlpha(top2bottom, roi, data);
                numChan = 1;
                break;
            case grid_util::FbReferenceType::HEAT_MAP :
                untileHeatMap(top2bottom, roi, data);
                numChan = 1;
                break;
            case grid_util::FbReferenceType::WEIGHT :
                untileWeightBuffer(top2bottom, roi, data);
                numChan = 1;
                break;
            case grid_util::FbReferenceType::BEAUTY_AUX :
                untileBeautyAux(top2bottom, roi, data);
                numChan = 3;
                break;
            case grid_util::FbReferenceType::ALPHA_AUX :
                untileAlphaAux(top2bottom, roi, data);
                numChan = 1;
                break;
            }
         }, "untileRenderOutputMain(f) untile");

    return numChan;
}

int
Fb::untileRenderOutputMainF4(const FbAovShPtr &fbAov,
                             const bool top2bottom,
                             const math::Viewport *roi,
                             const bool closestFilterDepthOutput,
                             FArray &data) const
{
    if (!fbAov->getStatus()) return 0; // just in case

    int numChan = 0;
    untileExecMain<(bool)UNTILE_TIMING_TEST_F4_RENDEROUTPUT>
        ([&]() {
            switch (fbAov->getReferenceType()) {
            case grid_util::FbReferenceType::UNDEF :
                {
                    int pixTotal = (roi) ? (roi->width() * roi->height()) : (getWidth() * getHeight());
                    data.resize(pixTotal * 4);
                }
                numChan = fbAov->untileF4(top2bottom, roi, closestFilterDepthOutput, data);
                break;
            case grid_util::FbReferenceType::BEAUTY :
                untileBeautyRGBF4(top2bottom, roi, data);
                numChan = 3;
                break;
            case grid_util::FbReferenceType::ALPHA :
                untileAlphaF4(top2bottom, roi, data);
                numChan = 1;
                break;
            case grid_util::FbReferenceType::HEAT_MAP :
                untileHeatMapF4(top2bottom, roi, data);
                numChan = 1;
                break;
            case grid_util::FbReferenceType::WEIGHT :
                untileWeightBufferF4(top2bottom, roi, data);
                numChan = 1;
                break;
            case grid_util::FbReferenceType::BEAUTY_AUX :
                untileBeautyAuxF4(top2bottom, roi, data);
                numChan = 3;
                break;
            case grid_util::FbReferenceType::ALPHA_AUX :
                untileAlphaAuxF4(top2bottom, roi, data);
                numChan = 1;
                break;
            }
         }, "untileRenderOutputMainF4(f) untile");

    return numChan;
}

void
Fb::computeMinMaxPixelInfoForDisplay(float &min, float &max) const
//
// Very heuristic logic to find min and max pixelInfo value for display
//
{
    //
    // First step, we will get min and maxLimit (= actual max value of image) value
    //
    min = FLT_MAX;
    float maxLimit = FLT_MIN;
    activeTileCrawler(mActivePixelsPixelInfo, [&](uint64_t tileMask, int pixOffset) {
            // tileFunc
            const PixelInfo *srcPixelInfo = mPixelInfoBufferTiled.getData() + pixOffset;
            activePixelCrawler(tileMask, reinterpret_cast<const float *>(srcPixelInfo), [&](const float &v) {
                    // pixFunc
                    min = fminf(min, v);
                    maxLimit = fmaxf(maxLimit, v);
                });
        });

    max = FLT_MIN;
    if (min == FLT_MAX) {
        return;                 // no active pixels
    }

    //
    // Second step, we will try to get secondary max value (which has less than 90% of max distance).
    // This secondary max depth is useful if image has no hit condition.
    //
    activeTileCrawler(mActivePixelsPixelInfo, [&](uint64_t tileMask, int pixOffset) {
            // tileFunc
            const PixelInfo *srcPixelInfo = mPixelInfoBufferTiled.getData() + pixOffset;
            activePixelCrawler(tileMask, reinterpret_cast<const float *>(srcPixelInfo), [&](const float &v) {
                    // pixFunc
                    if (v < maxLimit * 0.9f) {
                        max = fmaxf(max, v);
                    }
                });
        });

    if (maxLimit * 0.85 < max) {
        // If max is very close to maxLimit, we should pick up maxLimit value as max.
        // This is heuristic logic.
        max = maxLimit;
    }
}

void
Fb::computeMinMaxHeatMapForDisplay(float &min, float &max) const
//
// Very heuristic logic to find min and max heatMap value for display
//
{
    //
    // First step, try to get min and max value
    //
    float dataMin = FLT_MAX;
    float dataMax = FLT_MIN;
    activeTileCrawler(mActivePixelsHeatMap, [&](uint64_t tileMask, int pixOffset) {
            const float *src = mHeatMapSecBufferTiled.getData() + pixOffset;
            activePixelCrawler(tileMask, src, [&](const float &v) {
                    if (v > 0.0f) {
                        dataMin = fminf(dataMin, v);
                        dataMax = fmaxf(dataMax, v);
                    }
                });
        }); 

    //
    // Seond step, we sill divide min~max range into some number (=size) of bins and
    // find out each bin max value. Also try to get pixel total.
    //
    const int size = 32;
    std::vector<float> maxBin(size, FLT_MIN);
    std::vector<unsigned> totalBin(size, 0);

    float dataStep = (dataMax - dataMin) / (float)size;

    unsigned activePixTotal = 0;
    activeTileCrawler(mActivePixelsHeatMap, [&](uint64_t tileMask, int pixOffset) {
            const float *src = mHeatMapSecBufferTiled.getData() + pixOffset;
            activePixelCrawler(tileMask, src, [&](const float &v) {
                    if (dataMin <= v && v <= dataMax) {
                        int id = (int)((v - dataMin) / dataStep);
                        maxBin[id] = fmaxf(maxBin[id], v);
                        totalBin[id]++;
                        activePixTotal++;
                    }
                });
        });

    //
    // Third step, ignore top some percentage samples (=rmPct) and find max value of rest
    //
    const float rmPct = 0.001f; // rm top 0.1%
    unsigned rmPixTotal = (unsigned)((float)activePixTotal * rmPct);

    min = dataMin;
    {
        unsigned currTotal = 0;
        for (int i = (int)totalBin.size() - 1; i >= 0; --i) {
            currTotal += totalBin[i];
            if (rmPixTotal <= currTotal) {
                // found
                max = maxBin[i];
                return;
            }
        }
    }
    max = FLT_MIN;
}

size_t
Fb::computeMaxWeightBufferForDisplay(float &max) const
//
// logic to find max weight buffer value for display
//
{
    //
    // try to get min and max value
    //
    size_t totalNonZeroPixels = 0;
    float dataMax = FLT_MIN;
    activeTileCrawler(mActivePixelsWeightBuffer, [&](uint64_t tileMask, int pixOffset) {
            const float *src = mWeightBufferTiled.getData() + pixOffset;
            activePixelCrawler(tileMask, src, [&](const float &v) {
                    if (v > 0.0f) {
                        dataMax = fmaxf(dataMax, v);
                        totalNonZeroPixels++;
                    }
                });
        }); 

    max = dataMax;
    return totalNonZeroPixels;
}

} // namespace grid_util
} // namespace scene_rdl2

