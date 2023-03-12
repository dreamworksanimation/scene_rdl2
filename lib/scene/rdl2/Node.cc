// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "Node.h"

#include "AttributeKey.h"
#include "SceneClass.h"
#include "Types.h"

#include <string>

namespace scene_rdl2 {
namespace rdl2 {

AttributeKey<Mat4d> Node::sNodeXformKey;

Node::Node(const SceneClass& sceneClass, const std::string& name) :
    Parent(sceneClass, name)
{
    // Add the Node interface.
    mType |= INTERFACE_NODE;
}

Node::~Node()
{
}

SceneObjectInterface
Node::declare(SceneClass& sceneClass)
{
    auto interface = Parent::declare(sceneClass);

    sNodeXformKey = sceneClass.declareAttribute<Mat4d>("node_xform",
            FLAGS_BLURRABLE, INTERFACE_GENERIC, { "node xform" });
    sceneClass.setMetadata(sNodeXformKey, "label", "node xform");
    sceneClass.setMetadata(sNodeXformKey, SceneClass::sComment,
            "The 4x4 matrix describing the transformation from local space to world space.");

    return interface | INTERFACE_NODE;
}

} // namespace rdl2
} // namespace scene_rdl2

