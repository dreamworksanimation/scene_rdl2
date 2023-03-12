// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include <scene_rdl2/scene/rdl2/rdl2.h>

#include <string>

#include "attributes.cc"

using namespace scene_rdl2;

namespace {

class DeclareAndDestroyObject : public rdl2::SceneObject
{
public:
    typedef rdl2::SceneObject Parent;

    DeclareAndDestroyObject(const rdl2::SceneClass& sceneClass,
                            const std::string& name) :
        Parent(sceneClass, name)
    {
    }
};

} // namespace

// Intentially do not define rdl_create(). This is for testing of lazy loading
// of the create() and destroy() symbols.

extern "C"
RDL2_DSO_EXPORT
void
rdl2_destroy(rdl2::SceneObject* sceneObject)
{
    delete sceneObject;
}

