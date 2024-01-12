// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "ShadowSet.h"

namespace scene_rdl2 {
namespace rdl2 {

ShadowSet::ShadowSet(const SceneClass& sceneClass, const std::string& name) :
    LightSet(sceneClass, name)
{
    // Add the ShadowSet interface.
    mType |= INTERFACE_SHADOWSET;
}

SceneObjectInterface
ShadowSet::declare(SceneClass& sceneClass)
{
    auto interface = SceneObject::declare(sceneClass);

    sLightsKey = sceneClass.declareAttribute<SceneObjectVector>("lights", FLAGS_NONE, INTERFACE_LIGHT);
    sceneClass.setMetadata(sLightsKey, "comment",
        "List of lights that belong to this ShadowSet");

    return interface | INTERFACE_SHADOWSET;
}

} // namespace rdl2
} // namespace scene_rdl2

