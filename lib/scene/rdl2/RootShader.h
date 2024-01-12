// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "SceneClass.h"
#include "SceneContext.h"
#include "SceneObject.h"
#include "Shader.h"
#include "Types.h"

#include <scene_rdl2/common/except/exceptions.h>
#include <scene_rdl2/common/platform/Platform.h>
#include <scene_rdl2/render/util/Alloc.h>
#include <scene_rdl2/render/logging/logging.h>

#include <string>

namespace scene_rdl2 {

namespace geom {
    class Interpolator;
    class Primitive;
}
namespace rdl2 {

/**
 * A RootShader is a Shader object that could potentially be the root of a
 * shader graph. These Shaders can be assigned in a layer, and be looked up
 * from a layer using an assignment id. See Layer.h for more details.
 * Subclasses of RootShader include Displacement, Material, and VolumeShader,
 * which all define the "look" of a geometry.
 */

class RootShader : public Shader
{
public:
    typedef Shader Parent;

    RootShader(const SceneClass& sceneClass, const std::string& name);
    virtual ~RootShader();
    static SceneObjectInterface declare(SceneClass& sceneClass);

    // Checks if any primitive attributes in the shader network have changed.
    bool haveShaderGraphPrimAttributesChanged() const;
    // Caches all primitive attributes in the shader network.
    void cacheShaderGraphPrimAttributes() const;
    // Clears the primitive attribute caches in the shader network.
    void clearShaderGraphCachedPrimAttributes() const;
};

template <>
inline const RootShader*
SceneObject::asA() const
{
    return isA<RootShader>() ? static_cast<const RootShader*>(this) : nullptr;
}

template <>
inline RootShader*
SceneObject::asA()
{
    return isA<RootShader>() ? static_cast<RootShader*>(this) : nullptr;
}

} // namespace rdl2
} // namespace scene_rdl2

