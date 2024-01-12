// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "AttributeKey.h"
#include "Node.h"
#include "SceneClass.h"
#include "SceneObject.h"
#include "RootShader.h"
#include "Types.h"

#include <scene_rdl2/common/except/exceptions.h>

#include <string>

namespace scene_rdl2 { namespace texture { struct TLState; } }

namespace scene_rdl2 {
namespace rdl2 {

class Displacement : public RootShader
{
public:
    typedef RootShader Parent;

    Displacement(const SceneClass& sceneClass, const std::string& name);

    virtual ~Displacement();

    static SceneObjectInterface declare(SceneClass& sceneClass);

    DisplaceFunc mDisplaceFunc;
    DisplaceFuncv mDisplaceFuncv;

    finline void displace(moonray::shading::TLState *tls,
                          const moonray::shading::State &state,
                          math::Vec3f *displace) const
    {
        MNRY_ASSERT(mDisplaceFunc != nullptr);
        mDisplaceFunc(this, tls, state, displace);
    }

    finline void displacev(moonray::shading::TLState *tls,
                           unsigned numStatev,
                           const rdl2::Statev *statev,
                           rdl2::Vec3fv *displace) const
    {
        if (mDisplaceFuncv != nullptr) {
            mDisplaceFuncv(this, tls, numStatev, statev, displace, util::sAllOnMask);
        }
    }

    // Attributes common to all Displacement objects
    static AttributeKey<Float> sBoundPadding;
};

template <>
inline const Displacement*
SceneObject::asA() const
{
    return isA<Displacement>() ? static_cast<const Displacement*>(this) : nullptr;
}

template <>
inline Displacement*
SceneObject::asA()
{
    return isA<Displacement>() ? static_cast<Displacement*>(this) : nullptr;
}

} // namespace rdl2
} // namespace scene_rdl2

