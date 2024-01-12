// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include <scene_rdl2/scene/rdl2/rdl2.h>

#include <string>

#include "attributes.cc"

using namespace scene_rdl2;

namespace {

class DeclareAndCreateObject : public rdl2::SceneObject
{
public:
    typedef rdl2::SceneObject Parent;

    DeclareAndCreateObject(const rdl2::SceneClass& sceneClass,
                           const std::string& name) :
        Parent(sceneClass, name)
    {
    }
};

} // namespace

extern "C"
RDL2_DSO_EXPORT
rdl2::SceneObject*
rdl2_create(const rdl2::SceneClass& sceneClass, const std::string& name)
{
    return new DeclareAndCreateObject(sceneClass, name);
}

// Intentially do not define rdl_destroy(). This is for testing of lazy loading
// of the create() and destroy() symbols.

