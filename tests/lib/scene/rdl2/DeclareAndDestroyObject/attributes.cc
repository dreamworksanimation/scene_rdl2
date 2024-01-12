// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include <scene_rdl2/scene/rdl2/rdl2.h>

using namespace scene_rdl2;

RDL2_DSO_ATTR_DECLARE

    rdl2::AttributeKey<rdl2::Int> attrDeclareAndDestroy;

RDL2_DSO_ATTR_DEFINE(rdl2::SceneObject)

    attrDeclareAndDestroy =
        sceneClass.declareAttribute<rdl2::Int>("declare_and_destroyness", 11, { "declare and destroyness" });

RDL2_DSO_ATTR_END

