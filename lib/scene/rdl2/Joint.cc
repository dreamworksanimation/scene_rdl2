// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "Joint.h"

#include "AttributeKey.h"
#include "SceneClass.h"
#include "Types.h"

#include <string>

namespace scene_rdl2 {
namespace rdl2 {

Joint::Joint(const SceneClass& sceneClass, const std::string& name) :
    Parent(sceneClass, name)
{
    // Add the Joint interface.
    mType |= INTERFACE_JOINT;
}

Joint::~Joint()
{
}

SceneObjectInterface
Joint::declare(SceneClass& sceneClass)
{
    auto interface = Parent::declare(sceneClass);

    return interface | INTERFACE_JOINT;
}

} // namespace rdl2
} // namespace scene_rdl2

