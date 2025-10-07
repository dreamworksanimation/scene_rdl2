// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#include "Fb.h"

#include <scene_rdl2/common/fb_util/GammaF2C.h>
#include <scene_rdl2/common/fb_util/SrgbF2C.h>

#include <functional>
#include <thread>

// Basically we should use multi-thread version.
// This single thread mode is used debugging and performance comparison reason mainly.
//#define SINGLE_THREAD

#ifndef SINGLE_THREAD
#include <tbb/parallel_for.h>
#include <thread>
#endif // end SINGLE_THREAD

namespace scene_rdl2 {
namespace grid_util {

template <typename ConvPixFunc>
void
conv888Main(const Fb::FArray &srcArray,
            const unsigned numChannels,
            Fb::UCArray &dstArray,
            ConvPixFunc convPixFunc)
//
// convert float array to unsigned char array data main loop
//
{
    unsigned pixTotal = srcArray.size() / numChannels;
    unsigned dstSize = pixTotal * 3; // destination buffer is always 3 components (rgb)
    if (dstArray.size() != dstSize) {
        dstArray.resize(dstSize);
    }

#   ifdef SINGLE_THREAD
    for (unsigned pixOfs = 0; pixOfs < pixTotal; ++pixOfs) {
        const float *srcPix = &(srcArray[pixOfs * numChannels]);
        unsigned char *dstPix = &(dstArray[pixOfs * 3]);
        convPixFunc(srcPix, dstPix);
    }
#   else // else SINGLE_THREAD    
    size_t taskSize = std::max(pixTotal / (std::thread::hardware_concurrency() * 10), 1U);
    tbb::blocked_range<size_t> range(0, pixTotal, taskSize);
    tbb::parallel_for(range, [&](const tbb::blocked_range<size_t> &r) {
            for (size_t pixOfs = r.begin(); pixOfs < r.end(); ++pixOfs) {
                const float *srcPix = &(srcArray[pixOfs * numChannels]);
                unsigned char *dstPix = &(dstArray[pixOfs * 3]);
                convPixFunc(srcPix, dstPix);
            }
        });
#   endif // end !SINGLE_THREAD
}

//---------------------------------------------------------------------------------------------------------------    

// static function
void    
Fb::conv888Beauty(const FArray &srcRgba,
                  const bool isSrgb,
                  UCArray &dstRgb888)
{
    std::function<unsigned char(float)> f2ucConversion =
        (!isSrgb)? fb_util::GammaF2C::g22: fb_util::SrgbF2C::sRGB;

    conv888Main(srcRgba, (unsigned)4, dstRgb888,
                [&](const float *srcPix, unsigned char *dstPix) {
                    dstPix[0] = f2ucConversion(srcPix[0]);
                    dstPix[1] = f2ucConversion(srcPix[1]);
                    dstPix[2] = f2ucConversion(srcPix[2]);
                });
}

void
Fb::conv888BeautyRGB(const FArray &srcRgb,
                     const bool isSrgb,
                     UCArray &dstRgb888) const
{
    std::function<unsigned char(float)> f2ucConversion =
        (!isSrgb)? fb_util::GammaF2C::g22: fb_util::SrgbF2C::sRGB;

    conv888Main(srcRgb, (unsigned)3, dstRgb888,
                [&](const float *srcPix, unsigned char *dstPix) {
                    dstPix[0] = f2ucConversion(srcPix[0]);
                    dstPix[1] = f2ucConversion(srcPix[1]);
                    dstPix[2] = f2ucConversion(srcPix[2]);
                });
}

void
Fb::conv888Alpha(const FArray &srcData,
                 const bool isSrgb,
                 UCArray &dstRgb888) const
{
    std::function<unsigned char(float)> f2ucConversion =
        (!isSrgb)? fb_util::GammaF2C::g22: fb_util::SrgbF2C::sRGB;

    conv888Main(srcData, (unsigned)1, dstRgb888,
                [&](const float *srcPix, unsigned char *dstPix) {
                    uint8_t uc = f2ucConversion(srcPix[0]);
                    dstPix[0] = uc;
                    dstPix[1] = uc;
                    dstPix[2] = uc;
                });
}

void
Fb::conv888PixelInfo(const FArray &srcData,
                     const bool isSrgb,
                     UCArray &dstRgb888) const
{
    // This logic is same as Fb::computeMinMaxPixelInfoForDisplay()
    auto calcDepthMinMax = [&](float &min, float &max) {
        //
        // First step
        //
        min = FLT_MAX;
        float maxLimit = FLT_MIN;
        for (size_t i = 0; i < srcData.size(); ++i) {
            const float &v = srcData[i];
            if (v < min) min = v;
            if (maxLimit < v) maxLimit = v;
        }
        max = FLT_MIN;
        if (min == FLT_MAX) return; // no active pixels

        //
        // Second step
        //
        for (size_t i = 0; i < srcData.size(); ++i) {
            const float &v = srcData[i];
            if (v < maxLimit * 0.9f) {
                if (max < v) max = v;
            }
        }
        if (maxLimit * 0.85 < max) {
            max = maxLimit;
        }
    };

    auto normalizedDepth = [](float depth, float minDepth, float maxDepth) -> float {
        return ((minDepth != FLT_MAX)?
                (1.0f - (depth - minDepth) / (maxDepth - minDepth)): // return normalized depth
                0.0f); // empty data, return 0.0
    };

    std::function<unsigned char(float)> f2ucConversion =
        (!isSrgb)? fb_util::GammaF2C::g22: fb_util::SrgbF2C::sRGB;

    float minDepth, maxDepth;
    calcDepthMinMax(minDepth, maxDepth);
    
    conv888Main(srcData, (unsigned)1, dstRgb888,
                [&](const float *srcPix, unsigned char *dstPix) {
                    float currDepth = normalizedDepth(*srcPix, minDepth, maxDepth);;
                    unsigned char uc = f2ucConversion(currDepth);
                    dstPix[0] = uc;
                    dstPix[1] = uc;
                    dstPix[2] = uc;
                });
}

void
Fb::conv888HeatMap(const FArray &srcData,
                   bool isSrgb,
                   UCArray &dstRgb888) const
{
    // This logic is same as Fb::computeMinMaxHeatMapForDisplay()
    auto calcMinMax = [&](float &min, float &max) {
        //
        // First step
        //
        float dataMin = FLT_MAX;
        float dataMax = FLT_MIN;
        for (size_t i = 0; i < srcData.size(); ++i) {
            const float &v = srcData[i];
            if (v > 0.0f) {
                if (v < dataMin) dataMin = v;
                if (dataMax < v) dataMax = v;
            }
        }

        //
        // Second step
        //
        constexpr int size = 32;
        std::vector<float> maxBin(size, FLT_MIN);
        std::vector<unsigned> totalBin(size, 0);
        float dataStep = (dataMax - dataMin) / (float)size;
        unsigned activePixTotal = 0;
        for (size_t i = 0; i < srcData.size(); ++i) {
            const float &v = srcData[i];
            if (dataMin <= v && v <= dataMax) {
                int id = (int)((v - dataMin) / dataStep);
                if (maxBin[id] < v) maxBin[id] = v;
                totalBin[id]++;
                activePixTotal++;
            }
        }

        //
        // Third step
        //
        constexpr float rmFrac = 0.001f; // rm top 0.1%
        unsigned rmPixTotal = (unsigned)((float)activePixTotal * rmFrac);
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
    };

    float min, max;
    calcMinMax(min, max);

    conv888Main(srcData, (unsigned)1, dstRgb888,
                [&](const float *srcPix, unsigned char *dstPix) {
                    float v;
                    if (min == FLT_MAX) {
                        v = 0.0f;   // no active data
                    } else {
                        v = ((*srcPix) - min) / (max - min);
                    }
                    f2HeatMapCol255(v, isSrgb, dstPix);
                });
}

void
Fb::conv888WeightBuffer(const FArray &srcData,
                        const bool isSrgb,
                        UCArray &dstRgb888) const
{
    // This logic is same as Fb::computeMaxWeightBufferForDisplay()
    auto calcMax = [&](float &max) -> size_t {
        size_t totalNonZeroPixels = 0;
        float dataMax = FLT_MIN;
        for (size_t i = 0; i < srcData.size(); ++i) {
            const float &v = srcData[i];
            if (v > 0.0f) {
                if (dataMax < v) dataMax = v;
                totalNonZeroPixels++;
            }
        }
        max = dataMax;
        return totalNonZeroPixels;
    };

    std::function<unsigned char(float)> f2ucConversion =
        (!isSrgb)? fb_util::GammaF2C::g22: fb_util::SrgbF2C::sRGB;

    float max;
    size_t totalNonZeroPixels = calcMax(max);

    conv888Main(srcData, 1, dstRgb888,
                [&](const float *srcPix, unsigned char *dstPix) {
                    float v;
                    if (!totalNonZeroPixels) {
                        v = 0.0f;   // no active data
                    } else {
                        v = (*srcPix) / max;
                    }
                    uint8_t uc = f2ucConversion(v);
                    dstPix[0] = uc;
                    dstPix[1] = uc;
                    dstPix[2] = uc;
                });
}

void    
Fb::conv888BeautyOdd(const FArray &srcRgba,
                     const bool isSrgb,
                     UCArray &dstRgb888) const
{
    conv888Beauty(srcRgba, isSrgb, dstRgb888);
}

void    
Fb::conv888BeautyAux(const FArray &srcRgb,
                     const bool isSrgb,
                     UCArray &dstRgb888) const
{
    conv888BeautyRGB(srcRgb, isSrgb, dstRgb888);
}

void    
Fb::conv888AlphaAux(const FArray &srcData,
                    const bool isSrgb,
                    UCArray &dstRgb888) const
{
    conv888Alpha(srcData, isSrgb, dstRgb888);
}

bool
Fb::conv888RenderOutput(const int aovId,
                        const FArray &srcData,
                        const bool isSrgb,
                        const bool closestFilterDepthOutput,
                        UCArray &dstRgb888) const
{
    FbAovShPtr fbAov;
    if (!getAov2(aovId, fbAov)) return false;
    conv888RenderOutput(fbAov, srcData, isSrgb, closestFilterDepthOutput, dstRgb888);
    return true;
}

bool
Fb::conv888RenderOutput(const std::string &aovName,
                        const FArray &srcData,
                        const bool isSrgb,
                        const bool closestFilterDepthOutput,
                        UCArray &dstRgb888) const
{
    FbAovShPtr fbAov;
    if (!getAov2(aovName, fbAov)) return false;
    conv888RenderOutput(fbAov, srcData, isSrgb, closestFilterDepthOutput, dstRgb888);
    return true;
}

void
Fb::conv888RenderOutput(const FbAovShPtr fbAov,
                        const FArray &srcData,
                        const bool isSrgb,
                        const bool closestFilterDepthOutput,
                        UCArray &dstRgb888) const
{
    switch (fbAov->getReferenceType()) {
    case grid_util::FbReferenceType::UNDEF :
        fbAov->conv888(srcData, isSrgb, closestFilterDepthOutput, dstRgb888);
        break;
    case grid_util::FbReferenceType::BEAUTY : conv888BeautyRGB(srcData, isSrgb, dstRgb888); break;
    case grid_util::FbReferenceType::ALPHA : conv888Alpha(srcData, isSrgb, dstRgb888); break;
    case grid_util::FbReferenceType::HEAT_MAP : conv888HeatMap(srcData, isSrgb, dstRgb888); break;
    case grid_util::FbReferenceType::WEIGHT : conv888WeightBuffer(srcData, isSrgb, dstRgb888); break;
    case grid_util::FbReferenceType::BEAUTY_AUX : conv888BeautyAux(srcData, isSrgb, dstRgb888); break;
    case grid_util::FbReferenceType::ALPHA_AUX : conv888AlphaAux(srcData, isSrgb, dstRgb888); break;
    }
}

} // namespace grid_util
} // namespace scene_rdl2

