// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "AttributeKey.h"
#include "SceneClass.h"
#include "SceneObject.h"
#include "Types.h"

#include <scene_rdl2/common/except/exceptions.h>

#include <string>

namespace scene_rdl2 {
namespace rdl2 {

class Node : public SceneObject
{
public:
    typedef SceneObject Parent;

    Node(const SceneClass& sceneClass, const std::string& name);
    virtual ~Node();
    static SceneObjectInterface declare(SceneClass& sceneClass);

    // Attributes common to all Nodes.
    static AttributeKey<Mat4d> sNodeXformKey;
};

template <>
inline const Node*
SceneObject::asA() const
{
    return isA<Node>() ? static_cast<const Node*>(this) : nullptr;
}

template <>
inline Node*
SceneObject::asA()
{
    return isA<Node>() ? static_cast<Node*>(this) : nullptr;
}

} // namespace rdl2
} // namespace scene_rdl2

