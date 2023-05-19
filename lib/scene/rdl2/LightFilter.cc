// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "LightFilter.h"

namespace scene_rdl2 {
namespace rdl2 {

AttributeKey<Bool> LightFilter::sOnKey;

LightFilter::LightFilter(const SceneClass& sceneClass, const std::string& name) :
    Parent(sceneClass, name)
{
    // Add the Light interface.
    mType |= INTERFACE_LIGHTFILTER;
}

LightFilter::~LightFilter()
{
}

SceneObjectInterface
LightFilter::declare(SceneClass& sceneClass)
{
    auto interface = Parent::declare(sceneClass);

    sOnKey = sceneClass.declareAttribute<Bool>("on", true);
    sceneClass.setMetadata(sOnKey, "comment", "Turns the light filter on/off.");

    return interface | INTERFACE_LIGHTFILTER;
}

bool
LightFilter::isOn() const
{
    return get(sOnKey);
}

} // namespace rdl2
} // namespace scene_rdl2

