// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "ShadowReceiverSet.h"

namespace scene_rdl2 {
namespace rdl2 {


AttributeKey<Bool> ShadowReceiverSet::sComplementKey;

ShadowReceiverSet::ShadowReceiverSet(const SceneClass& sceneClass, const std::string& name) :
    GeometrySet(sceneClass, name)
{
    // Add the ShadowReceiverSet interface.
    mType |= INTERFACE_SHADOWRECEIVERSET;

    // ShadowReceiverSets should not appear in the BVH.
    mIncludeInBVH = false;
}

SceneObjectInterface
ShadowReceiverSet::declare(SceneClass& sceneClass)
{
    auto interface = SceneObject::declare(sceneClass);

    sGeometriesKey = sceneClass.declareAttribute<SceneObjectIndexable>("geometries", FLAGS_NONE, INTERFACE_GEOMETRY);
    sComplementKey = sceneClass.declareAttribute<Bool>("complement", false);

    sceneClass.setGroup("Properties", sComplementKey);

    return interface | INTERFACE_SHADOWRECEIVERSET;
}

} // namespace rdl2
} // namespace scene_rdl2

