// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include <scene_rdl2/scene/rdl2/rdl2.h>

#include "../ImaginaryLib.h"

#include "attributes.cc"

using namespace scene_rdl2;

RDL2_DSO_CLASS_BEGIN(LibLadenLightFilter, rdl2::LightFilter)

public:
    LibLadenLightFilter(const rdl2::SceneClass& sceneClass, const std::string& name);
    ~LibLadenLightFilter();

private:
    ImaginaryThing* mThing;

RDL2_DSO_CLASS_END(LibLadenLightFilter)

LibLadenLightFilter::LibLadenLightFilter(const rdl2::SceneClass& sceneClass,
                                         const std::string& name) :
    Parent(sceneClass, name),
    mThing(new ImaginaryThing)
{
    mThing->doTheThing();
}

LibLadenLightFilter::~LibLadenLightFilter()
{
    delete mThing;
}

