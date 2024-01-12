// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include <scene_rdl2/scene/rdl2/rdl2.h>

#include "../ImaginaryLib.h"

#include "attributes.cc"

using namespace scene_rdl2;

RDL2_DSO_CLASS_BEGIN(LibLadenDisplayFilter, rdl2::DisplayFilter)

public:
    LibLadenDisplayFilter(const rdl2::SceneClass& sceneClass, const std::string& name);
    ~LibLadenDisplayFilter();
virtual void getInputData(const moonray::displayfilter::InitializeData& initData,
                              moonray::displayfilter::InputData& inputData) const
    {
    }

private:
    ImaginaryThing* mThing;

RDL2_DSO_CLASS_END(LibLadenDisplayFilter)

LibLadenDisplayFilter::LibLadenDisplayFilter(const rdl2::SceneClass& sceneClass,
                                         const std::string& name) :
    Parent(sceneClass, name),
    mThing(new ImaginaryThing)
{
    mThing->doTheThing();
}

LibLadenDisplayFilter::~LibLadenDisplayFilter()
{
    delete mThing;
}

