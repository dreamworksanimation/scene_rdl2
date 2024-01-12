// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include <scene_rdl2/scene/rdl2/rdl2.h>

#include <exception>

#include "attributes.cc"

using namespace scene_rdl2;

RDL2_DSO_CLASS_BEGIN(ThrowDuringConstruct, rdl2::SceneObject)

public:
    ThrowDuringConstruct(const rdl2::SceneClass& sceneClass,
                         const std::string& name);

RDL2_DSO_CLASS_END(ThrowDuringConstruct)

ThrowDuringConstruct::ThrowDuringConstruct(const rdl2::SceneClass& sceneClass,
                                           const std::string& name) :
    Parent(sceneClass, name)
{
    throw std::runtime_error("Something went wrong.");
}

