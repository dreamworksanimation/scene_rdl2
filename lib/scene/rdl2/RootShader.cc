// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "RootShader.h"

#include <string>

namespace scene_rdl2 {
namespace rdl2 {

RootShader::RootShader(const SceneClass& sceneClass, const std::string& name) :
    Parent(sceneClass, name)
{
    // Add the RootShader interface.
    mType |= INTERFACE_ROOTSHADER;
}

RootShader::~RootShader()
{
}

SceneObjectInterface
RootShader::declare(SceneClass& sceneClass)
{
    auto interface = Parent::declare(sceneClass);

    return interface | INTERFACE_ROOTSHADER;
}

bool
RootShader::haveShaderGraphPrimAttributesChanged() const
{
    ConstSceneObjectSet b;
    getBindingTransitiveClosure(b);
    for (const rdl2::SceneObject * const o : b) {
        if (o->isA<Shader>()) {
            if (o->asA<Shader>()->hasChangedAttributes()) {
                return true;
            }
        }
    }
    return false;
}

void
RootShader::cacheShaderGraphPrimAttributes() const
{
    ConstSceneObjectSet b;
    getBindingTransitiveClosure(b);
    for (const SceneObject * o : b) {
        if (o->isA<Shader>()) {
            o->asA<Shader>()->cacheAttributes();
        }
    }
}

void
RootShader::clearShaderGraphCachedPrimAttributes() const
{
    ConstSceneObjectSet b;
    getBindingTransitiveClosure(b);
    for (const SceneObject * o : b) {
        if (o->isA<Shader>()) {
            o->asA<Shader>()->clearCachedAttributes();
        }
    }
}

} // namespace rdl2
} // namespace scene_rdl2

