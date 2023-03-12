// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "AttributeKey.h"
#include "Node.h"
#include "SceneClass.h"
#include "SceneObject.h"
#include "Types.h"

#include <scene_rdl2/common/except/exceptions.h>

#include <stdexcept>
#include <string>

namespace scene_rdl2 {
namespace rdl2 {

class NoProjectionException : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

class Camera : public Node
{
public:
    typedef Node Parent;

    Camera(const SceneClass& sceneClass, const std::string& name);
    virtual ~Camera();
    static SceneObjectInterface declare(SceneClass& sceneClass);

    void setNear(float n) { set<Float>(sNearKey, n); }
    void setFar(float f) { set<Float>(sFarKey, f); }
    virtual void setFocalLength(float /*length*/) { }
    virtual void setFilmApertureWidth(float /*width*/) { }
    
    const Material* getMediumMaterial() const;
    Geometry* getMediumGeometry() const;

    /// Compute a projection matrix for this camera (c2s).
    /// Screen space is defined as the 3D space that maps
    /// the extents of the camera frustum into [-1, -1, -1] x [1, 1, 1].
    /// In other words, it is a post-perspective NDC space.
    /// The window arguments specifies the aspect ratio.
    [[noreturn]] virtual math::Mat4f computeProjectionMatrix(float t,
                                                             const std::array<float, 4>& window,
                                                             float interocularOffset) const
    {
        throw NoProjectionException("Projection information not implemented");
    }
    virtual bool doesSupportProjectionMatrix() const
    {
        return false;
    }

    static AttributeKey<Float> sNearKey;
    static AttributeKey<Float> sFarKey;

    static AttributeKey<Float> sMbShutterOpenKey;
    static AttributeKey<Float> sMbShutterCloseKey;
    static AttributeKey<Float> sMbShutterBiasKey;

    static AttributeKey<String> sPixelSampleMap;

    static AttributeKey<SceneObject*> sMediumMaterial;
    static AttributeKey<SceneObject*> sMediumGeometry;

private:
    friend class SceneContext;
};

template <>
inline const Camera*
SceneObject::asA() const
{
    return isA<Camera>() ? static_cast<const Camera*>(this) : nullptr;
}

template <>
inline Camera*
SceneObject::asA()
{
    return isA<Camera>() ? static_cast<Camera*>(this) : nullptr;
}

} // namespace rdl2
} // namespace scene_rdl2

