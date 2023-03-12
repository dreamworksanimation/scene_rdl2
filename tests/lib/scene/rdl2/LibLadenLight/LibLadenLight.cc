// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include <scene_rdl2/scene/rdl2/rdl2.h>

#include "../ImaginaryLib.h"

#include "attributes.cc"

using namespace scene_rdl2;

RDL2_DSO_CLASS_BEGIN(LibLadenLight, rdl2::Light)

public:
    LibLadenLight(const rdl2::SceneClass& sceneClass, const std::string& name);
    ~LibLadenLight();

private:
    ImaginaryThing* mThing;

RDL2_DSO_CLASS_END(LibLadenLight)

LibLadenLight::LibLadenLight(const rdl2::SceneClass& sceneClass,
                             const std::string& name) :
    Parent(sceneClass, name),
    mThing(new ImaginaryThing)
{
    mThing->doTheThing();
}

LibLadenLight::~LibLadenLight()
{
    delete mThing;
}

