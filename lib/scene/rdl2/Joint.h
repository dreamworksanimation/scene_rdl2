// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "SceneClass.h"
#include "Node.h"
#include "Types.h"

#include <scene_rdl2/common/except/exceptions.h>

#include <string>

namespace scene_rdl2 {
namespace rdl2 {

class Joint : public Node
{
public:
    typedef Node Parent;

    Joint(const SceneClass& sceneClass, const std::string& name);
    virtual ~Joint();
    static SceneObjectInterface declare(SceneClass& sceneClass);
};

template <>
inline const Joint*
SceneObject::asA() const
{
    return isA<Joint>() ? static_cast<const Joint*>(this) : nullptr;
}

template <>
inline Joint*
SceneObject::asA()
{
    return isA<Joint>() ? static_cast<Joint*>(this) : nullptr;
}

} // namespace rdl2
} // namespace scene_rdl2

