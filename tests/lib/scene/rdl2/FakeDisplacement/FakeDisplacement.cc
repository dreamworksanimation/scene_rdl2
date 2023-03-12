// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include <scene_rdl2/scene/rdl2/rdl2.h>

#include "attributes.cc"

using namespace scene_rdl2;

namespace {

void
fakeDisplace(const rdl2::Displacement*, moonray::shading::TLState *tls, const moonray::shading::State&, math::Vec3f*)
{
}

} // namespace

RDL2_DSO_CLASS_BEGIN(FakeDisplacement, rdl2::Displacement)

public:
    FakeDisplacement(const rdl2::SceneClass& sceneClass, const std::string& name);

RDL2_DSO_CLASS_END(FakeDisplacement)

FakeDisplacement::FakeDisplacement(const rdl2::SceneClass& sceneClass,
                                  const std::string& name) :
    Parent(sceneClass, name)
{
    mDisplaceFunc = fakeDisplace;
}

