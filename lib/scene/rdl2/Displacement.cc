// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "Displacement.h"

#include "AttributeKey.h"
#include "SceneClass.h"
#include "SceneContext.h"
#include "Types.h"

#include <string>

namespace scene_rdl2 {
namespace rdl2 {

AttributeKey<Float> Displacement::sBoundPadding;

Displacement::Displacement(const SceneClass& sceneClass, const std::string& name) :
    Parent(sceneClass, name),
    mDisplaceFunc(nullptr),
    mDisplaceFuncv(nullptr)
{
    // Add the Displacement interface.
    mType |= INTERFACE_DISPLACEMENT;
}

Displacement::~Displacement()
{
}

SceneObjectInterface
Displacement::declare(SceneClass& sceneClass)
{
    auto interface = Parent::declare(sceneClass);

    sBoundPadding = sceneClass.declareAttribute<Float>(
        "bound_padding", 0.0f, { "bound padding" });
    sceneClass.setMetadata(sBoundPadding, "label", "bound padding");
    sceneClass.setMetadata(sBoundPadding, "comment",
        "bound padding defines how much to extend the bounding box "
        "of the object. Keep this value as low as possible unless the geometry "
        "skips tessellation because control cage bounding box is out of camera "
        "frustum but the displacement stretch out of the original "
        "object bounding box (pre-displacement). Setting the bound padding "
        "too large will consume more memory and tessellation time.");

    return interface | INTERFACE_DISPLACEMENT;
}

} // namespace rdl2
} // namespace scene_rdl2

