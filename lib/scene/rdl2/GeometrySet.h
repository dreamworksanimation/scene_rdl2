// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

// Include this before any other includes!
#include <scene_rdl2/common/platform/Platform.h>

#include "AttributeKey.h"
#include "SceneClass.h"
#include "SceneObject.h"
#include "Types.h"
#include "UpdateHelper.h"

#include <scene_rdl2/common/except/exceptions.h>

#include <string>

namespace scene_rdl2 {
namespace rdl2 {

/**
 * The GeometrySet represents a collection of Geometries with no duplicates.
 * It's used for building BVHs over a set of spatially local Geometries, where
 * building trees for each Geometry would result in a lot of spatial overlap.
 *
 * It only has one attribute, named "geometries", which is a SceneObjectIndexable.
 * We provide convenience functions on the GeometrySet to add and remove Geometries
 * from the set, as well as check if a Geometry is contained in the set. Please use these
 * functions to maintain the geometries' uniqueness invariant. When setting "geometries"
 * directly, make sure the light filters are unique and their order is deterministic.
 *
 * You can get the Geometries as a const SceneObjectIndexable& with the getGeometries() method call.
 */
class GeometrySet : public SceneObject
{
public:
    typedef SceneObject Parent;

    GeometrySet(const SceneClass& sceneClass, const std::string& name);
    static SceneObjectInterface declare(SceneClass& sceneClass);

    /// Retrieves the set of unique Geometry in this GeometrySet.
    finline const SceneObjectIndexable& getGeometries() const;

    /**
     * Adds the given Geometry to the GeometrySet, if it is not already a member
     * of the set. If it is already a member of the set, this does nothing.
     *
     * @param   geometry    The Geometry to add to the GeometrySet.
     */
    void add(Geometry* geometry);

    /**
     * Removes the given Geometry from the GeometrySet, if it is already a
     * member of the set. If it is not a member of the set, this does nothing.
     *
     * @param   geometry    The Geometry to remove from the GeometrySet.
     */
    void remove(Geometry* geometry);

    /**
     * Returns true if the given Geometry is a member of the GeometrySet.
     * There's no need to call this before calling add() or remove(), as they
     * will gracefully handle those edge cases.
     *
     * @param   geometry    The Geometry to check for membership.
     * @return  True if the geometry is a member of the GeometrySet.
     */
    bool contains(const Geometry* geometry) const;

    /// Completely empties the GeometrySet so that it doesn't contain anything.
    void clear();

    /// Returns true if all Geometry objects in the set are themselves static.
    bool isStatic() const;

    /**
     * This is called internally when needed. You should not have to call this
     * manually on a specific object (see SceneContext::applyUpdates())
     *
     * This is a non-recursive version of updatePrep(). This is needed to handle the
     * case for Layer and GeometrySet, for which we have already looped on
     * their dependencies (see SceneContext::applyUpdates() and
     * Layer::updatePrepAssignments() for details), but the Layer and GeometrySet
     * objects themselves still need to be prepared for update.
     * @param   sceneObjets   UpdateHelper passed from SceneContext.cc
     * @param   depth         depth of current object which is set 
     *                        automatically and recursively  
     * @return  True if the object itself or any of its dependencies has been 
     *          changed since last call of resetUpdate() and this object
     *          needs update
     */
    bool updatePrepFast(UpdateHelper& sceneObjects, int depth);

    bool haveGeometriesChanged() const { return hasChanged(sGeometriesKey); }

    bool includeInBVH() const { return mIncludeInBVH; }

protected:
    static AttributeKey<SceneObjectIndexable> sGeometriesKey;
    bool mIncludeInBVH;
};

const SceneObjectIndexable& GeometrySet::getGeometries() const { return get(sGeometriesKey); }

template <>
inline const scene_rdl2::rdl2::GeometrySet*
SceneObject::asA() const
{
    return isA<GeometrySet>() ? static_cast<const scene_rdl2::rdl2::GeometrySet*>(this) : nullptr;
}

template <>
inline scene_rdl2::rdl2::GeometrySet*
SceneObject::asA()
{
    return isA<GeometrySet>() ? static_cast<scene_rdl2::rdl2::GeometrySet*>(this) : nullptr;
}

} // namespace rdl2
} // namespace scene_rdl2

