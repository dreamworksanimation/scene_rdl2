// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "NormalMap.h"

#include "SceneClass.h"
#include "Types.h"

#include <string>

namespace scene_rdl2 {
namespace rdl2 {

NormalMap::NormalMap(const SceneClass& sceneClass, const std::string& name) :
    Parent(sceneClass, name),
    mSampleNormalFunc(nullptr),
    mSampleNormalFuncv(nullptr),
    mOriginalSampleNormalFunc(nullptr),
    mOriginalSampleNormalFuncv(nullptr)
{
    // Add the Map interface.
    mType |= INTERFACE_NORMALMAP;

}

NormalMap::~NormalMap()
{
}

SceneObjectInterface
NormalMap::declare(SceneClass& sceneClass)
{
    auto interface = Parent::declare(sceneClass);

    return interface | INTERFACE_NORMALMAP;
}

} // namespace rdl2
} // namespace scene_rdl2

