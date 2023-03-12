// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include <scene_rdl2/scene/rdl2/rdl2.h>

using namespace scene_rdl2;

RDL2_DSO_ATTR_DECLARE

    rdl2::AttributeKey<rdl2::SceneObject *> attrMatA;

RDL2_DSO_ATTR_DEFINE(rdl2::Material)

    attrMatA =
        sceneClass.declareAttribute<rdl2::SceneObject *>("mat_a", rdl2::FLAGS_NONE,
                                                         rdl2::INTERFACE_DWABASELAYERABLE, { "mat A" });

    rdl2_dso_interface |= rdl2::INTERFACE_DWABASELAYERABLE;
RDL2_DSO_ATTR_END

