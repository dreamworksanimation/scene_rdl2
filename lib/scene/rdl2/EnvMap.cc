// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "EnvMap.h"

#include "SceneClass.h"
#include "Types.h"

#include <string>

namespace scene_rdl2 {
namespace rdl2 {

EnvMap::EnvMap(const SceneClass& sceneClass, const std::string& name) :
    Parent(sceneClass, name)
{
    // Add the EnvMap interface.
    mType |= INTERFACE_ENVMAP;
}

EnvMap::~EnvMap()
{
}

SceneObjectInterface
EnvMap::declare(SceneClass& sceneClass)
{
    auto interface = Parent::declare(sceneClass);

    return interface | INTERFACE_ENVMAP;
}

} // namespace rdl2
} // namespace scene_rdl2

