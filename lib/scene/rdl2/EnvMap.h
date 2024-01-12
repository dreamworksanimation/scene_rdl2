// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "Node.h"
#include "SceneClass.h"
#include "SceneObject.h"
#include "Types.h"

#include <scene_rdl2/common/except/exceptions.h>

#include <string>

namespace scene_rdl2 {
namespace rdl2 {

class EnvMap : public Node
{
public:
    typedef Node Parent;

    EnvMap(const SceneClass& sceneClass, const std::string& name);
    virtual ~EnvMap();
    static SceneObjectInterface declare(SceneClass& sceneClass);
};

template <>
inline const EnvMap*
SceneObject::asA() const
{
    return isA<EnvMap>() ? static_cast<const EnvMap*>(this) : nullptr;
}

template <>
inline EnvMap*
SceneObject::asA()
{
    return isA<EnvMap>() ? static_cast<EnvMap*>(this) : nullptr;
}

} // namespace rdl2
} // namespace scene_rdl2

