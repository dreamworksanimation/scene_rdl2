// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "PathVisSimGlobalInfo.h"

#include <scene_rdl2/render/util/StrUtil.h>

#include <iomanip>
#include <iostream>
#include <sstream>

namespace scene_rdl2 {
namespace grid_util {

void
PathVisSimGlobalInfo::setPathVisActive(const bool st)
{
    mPathVisActive = st;
}

void
PathVisSimGlobalInfo::setSamples(const unsigned pixelX,
                                 const unsigned pixelY,
                                 const unsigned maxDepth,
                                 const unsigned pixelSamples,
                                 const unsigned lightSamples,
                                 const unsigned bsdfSamples)
{
    mPixelX = pixelX;
    mPixelY = pixelY;
    mMaxDepth = maxDepth;
    mPixelSamples = pixelSamples;
    mLightSamples = lightSamples;
    mBsdfSamples = bsdfSamples;
}

void
PathVisSimGlobalInfo::setRayTypeSelection(const bool useSceneSamples,
                                          const bool occlusionRaysOn,
                                          const bool specularRaysOn,
                                          const bool diffuseRaysOn,
                                          const bool bsdfSamplesOn,
                                          const bool lightSamplesOn)
{
    mUseSceneSamples = useSceneSamples;
    mOcclusionRaysOn = occlusionRaysOn;
    mSpecularRaysOn = specularRaysOn;
    mDiffuseRaysOn = diffuseRaysOn;
    mBsdfSamplesOn = bsdfSamplesOn;
    mLightSamplesOn = lightSamplesOn;
}

void
PathVisSimGlobalInfo::setColor(const Color& cameraRayColor,
                               const Color& specularRayColor,
                               const Color& DiffuseRayColor,
                               const Color& bsdfSampleColor,
                               const Color& lightSampleColor)
{
    mCameraRayColor = cameraRayColor;
    mSpecularRayColor = specularRayColor;
    mDiffuseRayColor = DiffuseRayColor;
    mBsdfSampleColor = bsdfSampleColor;
    mLightSampleColor = lightSampleColor;
}

void
PathVisSimGlobalInfo::setLineWidth(const float width)
{
    mLineWidth = width;
}

std::string
PathVisSimGlobalInfo::show() const
{
    std::ostringstream ostr;
    ostr << "PathVisSimGlobalInfo {\n"
         << str_util::addIndent(showSamples()) << '\n'
         << str_util::addIndent(showRayTypeSelection()) << '\n'
         << str_util::addIndent(showColor()) << '\n'
         << "  mLineWidth:" << mLineWidth << '\n'
         << "}";
    return ostr.str();
}

//------------------------------------------------------------------------------------------

std::string
PathVisSimGlobalInfo::showSamples() const
{
    std::ostringstream ostr;
    ostr << "samples {\n"
         << "  mPixelX:" << mPixelX << '\n'
         << "  mPixelY:" << mPixelY << '\n'
         << "  mMaxDepth:" << mMaxDepth << '\n'
         << "  mPixelSamples:" << mPixelSamples << '\n'
         << "  mLightSamples:" << mLightSamples << '\n'
         << "  mBsdfSamples:" << mBsdfSamples << '\n'
         << "}";
    return ostr.str();
}

std::string
PathVisSimGlobalInfo::showRayTypeSelection() const
{
    std::ostringstream ostr;
    ostr << "rayTypeSelection {\n"
         << "  mUseSceneSamples:" << str_util::boolStr(mUseSceneSamples) << '\n'
         << "  mOcclusionRaysOn:" << str_util::boolStr(mOcclusionRaysOn) << '\n'
         << "  mSpecularRaysOn:" << str_util::boolStr(mSpecularRaysOn) << '\n'
         << "  mDiffuseRaysOn:" << str_util::boolStr(mDiffuseRaysOn) << '\n'
         << "  mBsdfSamplesOn:" << str_util::boolStr(mBsdfSamplesOn) << '\n'
         << "  mLightSamplesOn:" << str_util::boolStr(mLightSamplesOn) << '\n'
         << "}";
    return ostr.str();
}

std::string
PathVisSimGlobalInfo::showColor() const
{
    auto showCol = [](const Color& c) {
        auto showV = [](const float f) {
            std::ostringstream ostr;
            ostr << std::setw(10) << std::fixed << std::setprecision(5) << f;
            return ostr.str();
        };
        std::ostringstream ostr;
        ostr << '(' << showV(c.r) << ',' << showV(c.g) << ',' << showV(c.b) << ')';
        return ostr.str();
    };

    std::ostringstream ostr;
    ostr << "color {\n"
         << "    mCameraRayColor:" << showCol(mCameraRayColor) << '\n'
         << "  mSpecularRayColor:" << showCol(mSpecularRayColor) << '\n'
         << "   mDiffuseRayColor:" << showCol(mDiffuseRayColor) << '\n'
         << "   mBsdfSampleColor:" << showCol(mBsdfSampleColor) << '\n'
         << "  mLightSampleColor:" << showCol(mLightSampleColor) << '\n'
         << "}";
    return ostr.str();
}

} // namespace grid_util
} // namespace scene_rdl2
