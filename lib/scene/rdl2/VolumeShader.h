// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "RootShader.h"

#include <scene_rdl2/common/math/Color.h>

#include <string>

namespace scene_rdl2 {
namespace rdl2 {

class VolumeShader : public RootShader
{
public:
    typedef RootShader Parent;

    enum Properties {
        IS_EXTINCTIVE =      1 << 0,
        HOMOGENOUS_EXTINC  = 1 << 1,
        IS_SCATTERING =      1 << 2,
        HOMOGENOUS_ALBEDO =  1 << 3,
        ISOTROPIC_PHASE =    1 << 4,
        IS_EMISSIVE =        1 << 5,
        HOMOGENOUS_EMISS =   1 << 6,
    };

    VolumeShader(const SceneClass& sceneClass, const std::string& name);
    virtual ~VolumeShader();
    static SceneObjectInterface declare(SceneClass& sceneClass);

    /// Properties specify what kind of volume this is and what to sample.
    virtual finline unsigned getProperties() const
    {
        MNRY_ASSERT(0, "not implemented");
        // no properties for generic volume shader.
        return 0;
    }

    virtual math::Color extinct(moonray::shading::TLState *tls,
                                const moonray::shading::State &state,
                                const math::Color& density) const = 0;

    virtual math::Color albedo(moonray::shading::TLState *tls,
                               const moonray::shading::State &state,
                               const math::Color& density) const = 0;

    virtual math::Color emission(moonray::shading::TLState *tls,
                                 const moonray::shading::State &state,
                                 const math::Color& density) const = 0;

    virtual float anisotropy(moonray::shading::TLState *tls,
                             const moonray::shading::State &state) const = 0;

    int getBakeResolutionMode() const
    {
        return get(sBakeResolutionMode);
    }

    int getBakeDivisions() const
    {
        return get(sBakeDivisions);
    }

    float getBakeVoxelSize() const
    {
        return get(sBakeVoxelSize);
    }

    bool isHomogenous() const;

    virtual bool hasExtinctionMapBinding() const = 0;

    virtual bool updateBakeRequired() const = 0;

    virtual bool isCutout() const { return false; }

    float surfaceOpacityThreshold() const
    {
        return get(sSurfaceOpacityThreshold);
    }

    static AttributeKey<String> sLabel;
    static AttributeKey<Int> sBakeResolutionMode;
    static AttributeKey<Int> sBakeDivisions;
    static AttributeKey<Float> sBakeVoxelSize;
    static AttributeKey<Float> sSurfaceOpacityThreshold;
};


template <>
inline const VolumeShader*
SceneObject::asA() const
{
    return isA<VolumeShader>() ? static_cast<const VolumeShader*>(this) : nullptr;
}


template <>
inline VolumeShader*
SceneObject::asA()
{
    return isA<VolumeShader>() ? static_cast<VolumeShader*>(this) : nullptr;
}

} // namespace rdl2
} // namespace scene_rdl2

