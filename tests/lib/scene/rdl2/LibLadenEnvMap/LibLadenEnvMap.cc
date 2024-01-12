// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include <scene_rdl2/scene/rdl2/rdl2.h>

#include "../ImaginaryLib.h"

#include "attributes.cc"

using namespace scene_rdl2;

RDL2_DSO_CLASS_BEGIN(LibLadenEnvMap, rdl2::EnvMap)

public:
    LibLadenEnvMap(const rdl2::SceneClass& sceneClass, const std::string& name);
    ~LibLadenEnvMap();

private:
    ImaginaryThing* mThing;

RDL2_DSO_CLASS_END(LibLadenEnvMap)

LibLadenEnvMap::LibLadenEnvMap(const rdl2::SceneClass& sceneClass,
                               const std::string& name) :
    Parent(sceneClass, name),
    mThing(new ImaginaryThing)
{
    mThing->doTheThing();
}

LibLadenEnvMap::~LibLadenEnvMap()
{
    delete mThing;
}

