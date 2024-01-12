// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include <scene_rdl2/scene/rdl2/rdl2.h>

using namespace scene_rdl2;

RDL2_DSO_ATTR_DECLARE

    rdl2::AttributeKey<rdl2::Float> attrPizza;
    rdl2::AttributeKey<rdl2::Int> attrCookie;

RDL2_DSO_ATTR_DEFINE(rdl2::Map)

    attrPizza =
        sceneClass.declareAttribute<rdl2::Float>("pizza", 1.23, rdl2::FLAGS_BINDABLE);

    attrCookie =
        sceneClass.declareAttribute<rdl2::Int>("cookie", 12);

RDL2_DSO_ATTR_END

