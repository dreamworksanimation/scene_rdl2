// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include <scene_rdl2/scene/rdl2/rdl2.h>

#include "attributes.cc"

using namespace scene_rdl2;

RDL2_DSO_CLASS_BEGIN(LibLadenDwaBaseLayerable, rdl2::Material)

public:
    LibLadenDwaBaseLayerable(const rdl2::SceneClass& sceneClass, const std::string& name);

RDL2_DSO_CLASS_END(LibLadenDwaBaseLayerable)

LibLadenDwaBaseLayerable::LibLadenDwaBaseLayerable(const rdl2::SceneClass& sceneClass,
                                                   const std::string& name) :
    Parent(sceneClass, name)
{
    mType |= rdl2::INTERFACE_DWABASELAYERABLE;
}

