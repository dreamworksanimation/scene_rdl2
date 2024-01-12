// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include <scene_rdl2/scene/rdl2/rdl2.h>

using namespace scene_rdl2;

RDL2_DSO_ATTR_DECLARE

    rdl2::AttributeKey<rdl2::Int> attrThrowHappiness;

RDL2_DSO_ATTR_DEFINE(rdl2::Light)

    attrThrowHappiness =
        sceneClass.declareAttribute<rdl2::Int>("throw_happiness", 9001, { "throw happiness" });

RDL2_DSO_ATTR_END

