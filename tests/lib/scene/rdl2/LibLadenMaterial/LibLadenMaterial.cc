// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include <scene_rdl2/scene/rdl2/rdl2.h>

#include "../ImaginaryLib.h"

#include "attributes.cc"

using namespace scene_rdl2;

namespace moonray { namespace shading { class BsdfBuilder; }}

namespace {

void
fakeShade(const rdl2::Material*,
         moonray::shading::TLState *tls,
         const moonray::shading::State& state,
         moonray::shading::BsdfBuilder& bsdfBuilder)
{
}

} // namespace

RDL2_DSO_CLASS_BEGIN(LibLadenMaterial, rdl2::Material)

public:
    LibLadenMaterial(const rdl2::SceneClass& sceneClass, const std::string& name);
    ~LibLadenMaterial();

private:
    ImaginaryThing* mThing;

RDL2_DSO_CLASS_END(LibLadenMaterial)

LibLadenMaterial::LibLadenMaterial(const rdl2::SceneClass& sceneClass,
                                   const std::string& name) :
    Parent(sceneClass, name),
    mThing(new ImaginaryThing)
{
    mShadeFunc = fakeShade;
    mThing->doTheThing();
}

LibLadenMaterial::~LibLadenMaterial()
{
    delete mThing;
}

