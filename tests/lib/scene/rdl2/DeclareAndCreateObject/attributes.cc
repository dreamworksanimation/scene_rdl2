// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include <scene_rdl2/scene/rdl2/rdl2.h>

using namespace scene_rdl2;

RDL2_DSO_ATTR_DECLARE

    rdl2::AttributeKey<rdl2::Int> attrDeclareAndCreate;

RDL2_DSO_ATTR_DEFINE(rdl2::SceneObject)

    attrDeclareAndCreate =
        sceneClass.declareAttribute<rdl2::Int>("declare_and_createness", 11, { "declare and createness" });

RDL2_DSO_ATTR_END

