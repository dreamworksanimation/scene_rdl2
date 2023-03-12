// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include <scene_rdl2/scene/rdl2/rdl2.h>

#include "attributes.cc"

using namespace scene_rdl2;

namespace {

} // namespace

RDL2_DSO_CLASS_BEGIN(FakeVolumeShader, rdl2::VolumeShader)

public:
    FakeVolumeShader(const rdl2::SceneClass& sceneClass, const std::string& name);
    finline unsigned getProperties() const override { return 0; }

    math::Color extinct(moonray::shading::TLState *tls,
                        const moonray::shading::State &state,
                        const math::Color& density) const override
    {
        return math::sBlack;
    }

    math::Color albedo(moonray::shading::TLState *tls,
                       const moonray::shading::State &state,
                       const math::Color& density) const override
    {
        return math::sBlack;
    }

    math::Color emission(moonray::shading::TLState *tls,
                         const moonray::shading::State &state,
                         const math::Color& density) const override
    {
        return math::sBlack;
    }

    float anisotropy(moonray::shading::TLState *tls,
                     const moonray::shading::State &state) const override
    {
        return 0.0f;
    }

    virtual bool hasExtinctionMapBinding() const override
    {
        return false;
    }

    virtual bool updateBakeRequired() const override
    {
        return false;
    }

RDL2_DSO_CLASS_END(FakeVolumeShader)

FakeVolumeShader::FakeVolumeShader(const rdl2::SceneClass& sceneClass,
                                   const std::string& name) :
    Parent(sceneClass, name)
{

}

