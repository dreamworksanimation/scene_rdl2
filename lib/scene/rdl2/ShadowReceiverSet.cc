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
    sceneClass.setMetadata(sGeometriesKey, "comment",
        "List of geometries that belong to this ShadowReceiverSet");

    sComplementKey = sceneClass.declareAttribute<Bool>("complement", false);
    sceneClass.setMetadata(sComplementKey, "comment",
        "If false, shadows from designated casters will be suppressed on a given receiver "
        "if the receiver is in the ShadowReceiverSet.\n"
        "If true, those shadows will be suppressed if the receiver is NOT in the ShadowReceiverSet.");

    sceneClass.setGroup("Properties", sComplementKey);

    return interface | INTERFACE_SHADOWRECEIVERSET;
}

} // namespace rdl2
} // namespace scene_rdl2

