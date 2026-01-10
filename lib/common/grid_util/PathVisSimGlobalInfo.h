// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <scene_rdl2/common/math/Color.h>
#include <scene_rdl2/common/math/Vec2.h>

namespace scene_rdl2 {
namespace grid_util {

class PathVisSimGlobalInfo
//
// All the parameters for the PathVisualizer simulation mode
//
{
public:
    using Color = math::Color; 
    using Vec2ui = math::Vec2<unsigned>;

    void setPathVisActive(const bool st);
    void setSamples(const unsigned pixelX, const unsigned pixelY, const unsigned maxDepth,
                    const unsigned pixelSamples, const unsigned lightSamples, const unsigned bsdfSamples);
    void setRayTypeSelection(const bool useSceneSamples,
                             const bool occlusionRaysOn,
                             const bool specularRaysOn,
                             const bool diffuseRaysOn,
                             const bool bsdfSamplesOn,
                             const bool lightSamplesOn);
    void setColor(const Color& cameraRayColor,
                  const Color& specularRayColor,
                  const Color& DiffuseRayColor,
                  const Color& bsdfSampleColor,
                  const Color& lightSampleColor);
    void setLineWidth(const float width);

    bool getPathVisActive() const { return mPathVisActive; }
    Vec2ui getPixelPos() const { return Vec2ui(mPixelX, mPixelY); }
    unsigned getMaxDepth() const { return mMaxDepth; }
    unsigned getPixelSamples() const { return mPixelSamples; }
    unsigned getLightSamples() const { return mLightSamples; }
    unsigned getBsdfSamples() const { return mBsdfSamples; }

    bool getUseSceneSamples() const { return mUseSceneSamples; }
    bool getOcclusionRaysOn() const { return mOcclusionRaysOn; }
    bool getSpecularRaysOn() const { return mSpecularRaysOn; }
    bool getDiffuseRaysOn() const { return mDiffuseRaysOn; }
    bool getBsdfSamplesOn() const { return mBsdfSamplesOn; }
    bool getLightSamplesOn() const { return mLightSamplesOn; }

    const Color& getCameraRayColor() const { return mCameraRayColor; }
    const Color& getSpecularRayColor() const { return mSpecularRayColor; }
    const Color& getDiffuseRayColor() const { return mDiffuseRayColor; }
    const Color& getBsdfSampleColor() const { return mBsdfSampleColor; }
    const Color& getLightSampleColor() const { return mLightSampleColor; }

    float getLineWidth() const { return mLineWidth; }

    std::string show() const;

private:

    std::string showSamples() const;
    std::string showRayTypeSelection() const;
    std::string showColor() const;

    //------------------------------

    bool mPathVisActive {false};

    unsigned mPixelX {0};
    unsigned mPixelY {0};
    unsigned mMaxDepth {1};
    unsigned mPixelSamples {4};
    unsigned mLightSamples {1};
    unsigned mBsdfSamples {1};

    bool mUseSceneSamples {false};
    bool mOcclusionRaysOn {true};
    bool mSpecularRaysOn {true};
    bool mDiffuseRaysOn {true};
    bool mBsdfSamplesOn {true};
    bool mLightSamplesOn {true};

    Color mCameraRayColor {0.0f, 0.0f, 1.0f};
    Color mSpecularRayColor {0.0f, 1.0f, 1.0f};
    Color mDiffuseRayColor {1.0f, 0.0f, 1.0f};
    Color mBsdfSampleColor {1.0f, 0.4f, 0.0f};
    Color mLightSampleColor {1.0f, 1.0f, 0.0f};

    float mLineWidth {1.0f};
};

} // namespace grid_util
} // namespace scene_rdl2
