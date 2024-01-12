// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "LightSet.h"

namespace scene_rdl2 {
namespace rdl2 {

/**
 * The ShadowSet inherits from the LightSet. Just like the LighSet, it is a
 * collection of lights with no duplicates. It it used for per part assignments
 * in the Layer. It can be reused for multiple Layer assignments.
 *
 * The purpose of the ShadowSet is to specify which lights an object does not cast
 * a shadow from. For example ObjectA is assigned LightSetA. LightSetA contains LightA
 * and LightB. ObjectA is also assigned ShadowSetA, which contains just LightA. This setup
 * means ObjectA is illuminated by LightA and LightB, but is only casts a shadow from LightB.
 */
class ShadowSet : public LightSet
{
public:

    ShadowSet(const SceneClass& sceneClass, const std::string& name);
    static SceneObjectInterface declare(SceneClass& sceneClass);

    bool haveLightsChanged() { return hasChanged(sLightsKey); }

};

template <>
inline const ShadowSet*
SceneObject::asA() const
{
    return isA<ShadowSet>() ? static_cast<const ShadowSet*>(this) : nullptr;
}

template <>
inline ShadowSet*
SceneObject::asA()
{
    return isA<ShadowSet>() ? static_cast<ShadowSet*>(this) : nullptr;
}


} // namespace rdl2
} // namespace scene_rdl2

