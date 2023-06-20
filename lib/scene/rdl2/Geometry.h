// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "Camera.h"
#include "Node.h"
#include "SceneClass.h"
#include "SceneObject.h"
#include "Types.h"

#include <scene_rdl2/common/except/exceptions.h>
#include <scene_rdl2/common/math/Xform.h>

#include <string>

namespace moonray { namespace geom { class Procedural; } }

namespace scene_rdl2 {
namespace rdl2 {


// This enum is used in multiple attributes.cc files and we require those files to have a minimum of
// depedencies on moonray itself. So this basically provides a convenient place to put a shared definition.
enum class MotionBlurType {
    STATIC           =  0,
    VELOCITY         =  1,
    FRAME_DELTA      =  2,
    ACCELERATION     =  3,
    HERMITE          =  4,
    STATIC_DUPLICATE =  5,

    BEST             = -1
};
// Note about STATIC_DUPLICATE:  This is not a motion blur type per se but is a way to create a vertex
// buffer with two identical motion steps.   The vertices are then replaced with modified values such
// as in the moonshine WrapDeformGeometry procedural.

enum class PrimitiveAttributeFrame {
    FIRST_MOTION_STEP = 0,
    SECOND_MOTION_STEP = 1,
    BOTH_MOTION_STEPS = 2,
};


class Geometry : public Node
{
public:
    typedef Node Parent;
    enum SideType { TWO_SIDED = 0, SINGLE_SIDED, MESH_DEFAULT_SIDED };

    Geometry(const SceneClass& sceneClass, const std::string& name);
    virtual ~Geometry();
    static SceneObjectInterface declare(SceneClass& sceneClass);

    /// Invokes createProcedural() and captures the returned procedural.
    finline void loadProcedural();

    /// destroy the loaded procedural
    finline void unloadProcedural();

    /// Returns the loaded procedural or null.
    finline const moonray::geom::Procedural* getProcedural() const;
    finline moonray::geom::Procedural* getProcedural();

    /// Set the render to object transform cache, This should
    /// be set by the renderer during geometry update or creation.
    finline void setRender2Object(const math::Xform3f &render2Object);

    /// Return the render2Object transform cache set by the renderer
    finline math::Xform3f getRender2Object() const;

    /// Convenience function for checking if the Geometry is static.
    finline bool isStatic() const;

    /// Returns the sidedness of the mesh.
    finline SideType getSideType() const;

    /// Check if normals of the mesh are reversed.
    finline bool getReverseNormals() const;

    // See if user has set a RayEpsilon (default 0.0f)
    // If so, renderer should use this value.  Otherwise, use renderer's estimate
    finline float getRayEpsilon() const;

    // See if user has set a ShadowRayEpsilon (default 0.0f)
    // If so, renderer should use this value.
    finline float getShadowRayEpsilon() const;

    finline void setContainsCamera();

    // Get the shadow receiver label string, used to look for matches with a correspoinding shadow caster label
    finline const std::string& getShadowReceiverLabel() const;

    // Get the shadow exclusion mappping string
    finline const std::string& getShadowExclusionMappings() const;

    /// Returns the mesh visibility Mask.
    int getVisibilityMask() const;

    /// Returns the dicing camera
    finline const Camera* getDicingCamera() const;

    /// Returns whether use_local_motion_blur is enabled
    finline bool getUseLocalMotionBlur() const;

    // TODO this is a temporary band-aid to avoid attribute modification
    // that doesn't require geometry to regenerate causing long regenerate wait
    // during interactive workflow. One solution would be to have a more
    // generic way to classify attributes for corresponding renderer behavior
    // after attributes update.
    // Or we should move such attributes out of the geometry object.
    //
    /// Returns whether the attributes graph this geometry depends on contains
    /// update that requires geometry to regenerate/tessellate/construct accelerator
    bool requiresGeometryUpdate(UpdateHelper& sceneObjects, int depth);

    // Attributes common to all Geometries.
    static AttributeKey<String> sLabel;
    static AttributeKey<SceneObjectVector> sReferenceGeometries;
    static AttributeKey<Bool> sStaticKey;
    static AttributeKey<Int> sSideTypeKey;
    static AttributeKey<Bool> sReverseNormals;
    static AttributeKey<Bool> sVisibleCamera;
    static AttributeKey<Bool> sVisibleShadow;
    static AttributeKey<Bool> sVisibleDiffuseReflection;
    static AttributeKey<Bool> sVisibleDiffuseTransmission;
    static AttributeKey<Bool> sVisibleGlossyReflection;
    static AttributeKey<Bool> sVisibleGlossyTransmission;
    static AttributeKey<Bool> sVisibleMirrorReflection;
    static AttributeKey<Bool> sVisibleMirrorTransmission;
    static AttributeKey<Bool> sVisiblePhase;
    static AttributeKey<Float> sRayEpsilon;
    static AttributeKey<Float> sShadowRayEpsilon;
    static AttributeKey<String> sShadowReceiverLabel;
    static AttributeKey<String> sShadowExclusionMappings;
    static AttributeKey<Bool> sContainsCamera;
    static AttributeKey<SceneObject*> sDicingCamera;
    static AttributeKey<Bool> sUseLocalMotionBlur;

    /// Returns whether the internal procedural geometric data has been deformed.
    /// WARNING: assumes that procedural exists, verify that getProcedural
    /// is non-null before calling.
    virtual bool deformed() const { return false; }

    /// Reset the deformed status.
    /// WARNING: assumes that procedural exists, verify that getProcedural
    /// is non-null before calling.
    virtual void resetDeformed() {};

protected:
    // Must be implemented by derived classes.
    virtual moonray::geom::Procedural* createProcedural() const = 0;

    virtual void destroyProcedural() const = 0;

protected:
    moonray::geom::Procedural* mProcedural;
    math::Xform3f mRender2Object;

    template <typename Container>
    bool geometryUpdatePrepSequenceContainer(const Attribute* attribute,
            UpdateHelper& sceneObjects,
            int depth)
    {
        bool updateRequired = false;
        const AttributeKey<Container> key(*attribute);
        const Container& objectVector = get(key);
        for (SceneObject * const object : objectVector) {
            if (object) {
                if (object->isA<Geometry>() && !object->asA<Geometry>()->
                    requiresGeometryUpdate(sceneObjects, depth + 1)) {
                    continue;
                }
                updateRequired |= object->updatePrep(sceneObjects, depth);
            }
        }
        mAttributeTreeChanged |= updateRequired;
        return updateRequired;
    }

private:
    bool mContainsCamera;
};

void
Geometry::loadProcedural()
{
    mProcedural = createProcedural();
}

void
Geometry::unloadProcedural()
{
    destroyProcedural();
    mProcedural = nullptr;
}

const moonray::geom::Procedural*
Geometry::getProcedural() const
{
    return mProcedural;
}

moonray::geom::Procedural*
Geometry::getProcedural()
{
    return mProcedural;
}

void
Geometry::setRender2Object(const math::Xform3f &render2Object)
{
    mRender2Object = render2Object;
}

math::Xform3f
Geometry::getRender2Object() const
{
    return mRender2Object;
}

bool
Geometry::isStatic() const
{
    return get(sStaticKey);
}

Geometry::SideType
Geometry::getSideType() const
{
    return static_cast<SideType>(get(sSideTypeKey));
}

bool
Geometry::getReverseNormals() const
{
    return get(sReverseNormals);
}

float
Geometry::getRayEpsilon() const
{
    return get(sRayEpsilon);
}

float
Geometry::getShadowRayEpsilon() const
{
    return get(sShadowRayEpsilon);
}

void
Geometry::setContainsCamera()
{
    bool& containsCamera = getMutable(sContainsCamera);
    containsCamera = true;
}

const Camera*
Geometry::getDicingCamera() const
{
    SceneObject* so = get(sDicingCamera);
    if (so) {
        Camera* dicingCamera = so->asA<Camera>();
        return dicingCamera;
    }
    return nullptr;
}

bool
Geometry::getUseLocalMotionBlur() const
{
    return get(sUseLocalMotionBlur);
}

const std::string&
Geometry::getShadowReceiverLabel() const
{
    return get(sShadowReceiverLabel);
}

const std::string&
Geometry::getShadowExclusionMappings() const
{
    return get(sShadowExclusionMappings);
}

template <>
inline const Geometry*
SceneObject::asA() const
{
    return isA<Geometry>() ? static_cast<const Geometry*>(this) : nullptr;
}

template <>
inline Geometry*
SceneObject::asA()
{
    return isA<Geometry>() ? static_cast<Geometry*>(this) : nullptr;
}

} // namespace rdl2
} // namespace scene_rdl2

