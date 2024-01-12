// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include <scene_rdl2/scene/rdl2/rdl2.h>

#include "../ImaginaryLib.h"

#include "attributes.cc"

using namespace scene_rdl2;

namespace {

void
fakeSample(const rdl2::NormalMap*, moonray::shading::TLState *tls, const moonray::shading::State&, math::Vec3f*)
{
}

} // namespace

RDL2_DSO_CLASS_BEGIN(LibLadenMap, rdl2::NormalMap)

public:
    LibLadenMap(const rdl2::SceneClass& sceneClass, const std::string& name);
    ~LibLadenMap();

private:
    ImaginaryThing* mThing;

RDL2_DSO_CLASS_END(LibLadenMap)

LibLadenMap::LibLadenMap(const rdl2::SceneClass& sceneClass,
                         const std::string& name) :
    Parent(sceneClass, name),
    mThing(new ImaginaryThing)
{
    mSampleNormalFunc = fakeSample;
    mThing->doTheThing();
}

LibLadenMap::~LibLadenMap()
{
    delete mThing;
}

