// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file DisplayFilter.cc

#include "DisplayFilter.h"

namespace scene_rdl2 {
namespace rdl2 {

SceneObjectInterface
DisplayFilter::declare(SceneClass &sceneClass)
{
    SceneObjectInterface interface = Parent::declare(sceneClass);

    // Attribute declarations can go here.  Currently there are none

    return interface | INTERFACE_DISPLAYFILTER;
}

DisplayFilter::DisplayFilter(const SceneClass &sceneClass, const std::string &name):
    Parent(sceneClass, name),
    mFilterFuncv(nullptr)
{
    mType |= INTERFACE_DISPLAYFILTER;
}

DisplayFilter::~DisplayFilter()
{
}


} // namespace rdl2
} // namespace scene_rdl2

