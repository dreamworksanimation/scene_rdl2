// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Camera.h"
#include "Displacement.h"
#include "DisplayFilter.h"
#include "EnvMap.h"
#include "Geometry.h"
#include "Joint.h"
#include "Light.h"
#include "LightFilter.h"
#include "Map.h"
#include "NormalMap.h"
#include "Material.h"

#include <scene_rdl2/common/except/exceptions.h>
#include <scene_rdl2/render/util/Alloc.h>

#include <sstream>
#include <string>

namespace scene_rdl2 {
namespace rdl2 {

/**
 * Displacement, CameraProxy, EnvMapProxy, GeometryProxy, LightProxy, LightFilterProxy,
 * MapProxy, MaterialProxy, VolumeShaderProxy and SceneObjectProxy define proxy classes
 * for objects of each customization point in RDL2.
 *
 * Effectively these objects will invoke the proper chain of constructors and
 * have the same set of attributes as the objects they are standing in for,
 * but don't provide the rich interface of those objects. As such, they don't
 * drag in any library dependencies.
 *
 * This is useful if you want to create objects of those types, but don't want
 * to link with or distribute the huge chain of dependencies that your DSOs
 * might have. Those are still needed for rendering, but for a content tool
 * which just needs to set attribute data those dependencies are overkill.
 *
 * Built in classes that come for free with RDL (like the GeometrySet, Layer,
 * LightSet, and SceneVariables) never need to be proxied, because they are
 * always fully available and have no extra dependencies.
 */

// -------- CameraProxy ------------------------------------------------------

class CameraProxy : public Camera
{
public:
    finline CameraProxy(const SceneClass& sceneClass, const std::string& name);
};

CameraProxy::CameraProxy(const SceneClass& sceneClass, const std::string& name)
    : Camera(sceneClass, name)
{
}

// -------- EnvMapProxy ------------------------------------------------------

class EnvMapProxy : public EnvMap
{
public:
    finline EnvMapProxy(const SceneClass& sceneClass, const std::string& name);
};

EnvMapProxy::EnvMapProxy(const SceneClass& sceneClass, const std::string& name)
    : EnvMap(sceneClass, name)
{
}

// -------- GeometryProxy ----------------------------------------------------

class GeometryProxy : public Geometry
{
public:
    finline GeometryProxy(const SceneClass& sceneClass, const std::string& name);
    finline moonray::geom::Procedural* createProcedural() const override;
    finline void destroyProcedural() const override;
    bool deformed() const override { return false; }
    void resetDeformed() override {}
};

GeometryProxy::GeometryProxy(const SceneClass& sceneClass, const std::string& name)
    : Geometry(sceneClass, name)
{
}

moonray::geom::Procedural*
GeometryProxy::createProcedural() const
{
    std::stringstream errMsg;
    errMsg << "You cannot invoke createProcedural() on SceneObject '" <<
        mName << "', SceneClass '" << mSceneClass.getName() << " because it is"
        " a GeometryProxy.";
    throw except::RuntimeError(errMsg.str());
}

void
GeometryProxy::destroyProcedural() const
{
}

// -------- LightProxy -------------------------------------------------------

class LightProxy : public Light
{
public:
    finline LightProxy(const SceneClass& sceneClass, const std::string& name);
};

LightProxy::LightProxy(const SceneClass& sceneClass, const std::string& name)
    : Light(sceneClass, name)
{
}

// -------- LightFilterProxy -------------------------------------------------

class LightFilterProxy : public LightFilter
{
public:    
    finline LightFilterProxy(const SceneClass& sceneClass, const std::string& name);
};

LightFilterProxy::LightFilterProxy(const SceneClass& sceneClass, const std::string& name)
    : LightFilter(sceneClass, name)
{
}

// -------- MapProxy ---------------------------------------------------------

class MapProxy : public Map
{
public:
    finline MapProxy(const SceneClass& sceneClass, const std::string& name);

    static finline void sample(const rdl2::Map* self,
                               moonray::shading::TLState *tls,
                               const moonray::shading::State& st,
                               math::Color* result);
};

MapProxy::MapProxy(const SceneClass& sceneClass, const std::string& name)
    : Map(sceneClass, name)
{
    mSampleFunc = MapProxy::sample;
}

void
MapProxy::sample(const rdl2::Map* self, moonray::shading::TLState *tls,
                   const moonray::shading::State& st,
                   math::Color* result)
{
    const MapProxy* me = static_cast<const MapProxy*>(self);
    std::stringstream errMsg;
    errMsg << "You cannot invoke sample() on SceneObject '" << me->mName <<
        "', SceneClass '" << me->mSceneClass.getName() << " because it is a"
        " MapProxy.";
    throw except::RuntimeError(errMsg.str());
}

// -------- NormalMapProxy ---------------------------------------------------------

class NormalMapProxy : public NormalMap
{
public:
    finline NormalMapProxy(const SceneClass& sceneClass, const std::string& name);

    static finline void sampleNormal(const rdl2::NormalMap* self,
                                     moonray::shading::TLState *tls,
                                     const moonray::shading::State& st,
                                     math::Vec3f* result);
};

NormalMapProxy::NormalMapProxy(const SceneClass& sceneClass, const std::string& name)
    : NormalMap(sceneClass, name)
{
    mSampleNormalFunc = NormalMapProxy::sampleNormal;
}

void
NormalMapProxy::sampleNormal(const rdl2::NormalMap* self,
                             moonray::shading::TLState *tls,
                             const moonray::shading::State& st,
                             math::Vec3f* result)
{
    const NormalMapProxy* me = static_cast<const NormalMapProxy*>(self);
    std::stringstream errMsg;
    errMsg << "You cannot invoke sample() on SceneObject '" << me->mName <<
        "', SceneClass '" << me->mSceneClass.getName() << " because it is a"
        " NormalMapProxy.";
    throw except::RuntimeError(errMsg.str());
}

// -------- MaterialProxy ----------------------------------------------------

class MaterialProxy : public Material
{
public:
    finline MaterialProxy(const SceneClass& sceneClass, const std::string& name);

    static finline void shade(const rdl2::Material* self,
                              moonray::shading::TLState *tls,
                              const moonray::shading::State& state,
                              moonray::shading::BsdfBuilder& bsdfBuilder);
    // MOONRAY-1949???
};

MaterialProxy::MaterialProxy(const SceneClass& sceneClass, const std::string& name)
    : Material(sceneClass, name)
{
    mShadeFunc = MaterialProxy::shade;
}

void
MaterialProxy::shade(const rdl2::Material* self,
                     moonray::shading::TLState *tls,
                     const moonray::shading::State& state,
                     moonray::shading::BsdfBuilder& bsdfBuilder)
{
    const MaterialProxy* me = static_cast<const MaterialProxy*>(self);
    std::stringstream errMsg;
    errMsg << "You cannot invoke shade() on SceneObject '" << me->mName <<
        "', SceneClass '" << me->mSceneClass.getName() << " because it is a"
        " MaterialProxy.";
    throw except::RuntimeError(errMsg.str());
}

// -------- DwaBaseLayerableProxy ----------------------------------------------------

class DwaBaseLayerableProxy : public Material
{
public:
    finline DwaBaseLayerableProxy(const SceneClass& sceneClass, const std::string& name);
};

DwaBaseLayerableProxy::DwaBaseLayerableProxy(const SceneClass& sceneClass, const std::string& name)
    : Material(sceneClass, name)
{
    mType |= INTERFACE_DWABASELAYERABLE;
}

// -------- DwaBaseHairLayerableProxy ----------------------------------------------------

class DwaBaseHairLayerableProxy : public Material
{
public:
    finline DwaBaseHairLayerableProxy(const SceneClass& sceneClass, const std::string& name);
};

DwaBaseHairLayerableProxy::DwaBaseHairLayerableProxy(const SceneClass& sceneClass, const std::string& name)
    : Material(sceneClass, name)
{
    mType |= INTERFACE_DWABASEHAIRLAYERABLE;
}

// -------- DisplacmentProxy ----------------------------------------------------

class DisplacementProxy : public Displacement
{
public:
    finline DisplacementProxy(const SceneClass& sceneClass, const std::string& name);

    static finline void displace(const Displacement* self, moonray::shading::TLState *tls,
                         const moonray::shading::State& state, math::Vec3f* displace);
};

DisplacementProxy::DisplacementProxy(const SceneClass& sceneClass, const std::string& name)
    : Displacement(sceneClass, name)
{
    mDisplaceFunc = DisplacementProxy::displace;
}

void
DisplacementProxy::displace(const Displacement* self,
                            moonray::shading::TLState *tls,
                            const moonray::shading::State& state,
                            math::Vec3f* displace)
{
    const DisplacementProxy* me = static_cast<const DisplacementProxy*>(self);
    std::stringstream errMsg;
    errMsg << "You cannot invoke displace() on SceneObject '" << me->mName <<
        "', SceneClass '" << me->mSceneClass.getName() << " because it is a"
        " DisplacementProxy.";
    throw except::RuntimeError(errMsg.str());
}

// -------- VolumeShaderProxy ----------------------------------------------------

class VolumeShaderProxy : public VolumeShader
{
public:
    finline VolumeShaderProxy(const SceneClass& sceneClass, const std::string& name);

    finline unsigned getProperties() const override
    {
        return 0;
    }

    math::Color extinct(moonray::shading::TLState *tls,
                        const moonray::shading::State &state,
                        const math::Color& density,
                        float rayVolumeDepth) const override
    {
        return math::sBlack;
    }

    math::Color albedo(moonray::shading::TLState *tls,
                       const moonray::shading::State &state,
                       const math::Color& density,
                       float rayVolumeDepth) const override
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
};

VolumeShaderProxy::VolumeShaderProxy(const SceneClass& sceneClass, const std::string& name)
    : VolumeShader(sceneClass, name)
{
}

// -------- DisplayFilterProxy --------------------------------------------------
class DisplayFilterProxy : public DisplayFilter
{
public:
    DisplayFilterProxy(const SceneClass &sceneClass, const std::string &name):
        DisplayFilter(sceneClass, name)
    {
    }

    virtual void getInputData(const moonray::displayfilter::InitializeData& initData,
                              moonray::displayfilter::InputData& inputData) const override
    {
    }
};

// -------- SceneObjectProxy ----------------------------------------------------

class SceneObjectProxy : public SceneObject
{
public:
    finline SceneObjectProxy(const SceneClass& sceneClass, const std::string& name);
};

SceneObjectProxy::SceneObjectProxy(const SceneClass& sceneClass, const std::string& name)
    : SceneObject(sceneClass, name)
{
}

} // namespace rdl2
} // namespace scene_rdl2

