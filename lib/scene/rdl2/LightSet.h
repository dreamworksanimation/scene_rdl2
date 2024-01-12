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
 * The LightSet represents a collection of Lights with no duplicates. It's
 * used in Part/Material assignments in the Layer, where you can define which
 * Lights affect a particular Part/Material assignment. That collection of
 * Lights is the LightSet. LightSets can be reused for many Part/Material
 * assignments.
 *
 * It only has one attribute, named "lights", which is a SceneObjectVector.
 * We provide convenience functions on the LightSet to add and remove Lights
 * from the set, as well as check if a Light is contained in the set. Please use these
 * functions to maintain the light's uniqueness invariant. When setting "lights"
 * directly, make sure the lights are unique and their order is deterministic.
 *
 * You can get the Lights as a const SceneObjectVector& with the getLights() method call.
 */
class LightSet : public SceneObject
{
public:
    typedef SceneObject Parent;

    LightSet(const SceneClass& sceneClass, const std::string& name);
    static SceneObjectInterface declare(SceneClass& sceneClass);

    /// Retrieves the set of unique Lights in this LightSet.
    finline const SceneObjectVector& getLights() const;

    /**
     * Adds the given Light to the LightSet, if it is not already a member of
     * the set. If it is already a member of the set, this does nothing.
     *
     * @param   light   The Light to add to the LightSet.
     */
    void add(Light* light);

    /**
     * Removes the given Light from the LightSet, if it is already a member of
     * the set. If it is not a member of the set, this does nothing.
     *
     * @param   light   The Light to remove from the LightSet.
     */
    void remove(Light* light);

    /**
     * Returns true if the given Light is a member of the LightSet. There's
     * no need to call this before calling add() or remove(), as they will
     * gracefully handle those edge cases.
     *
     * @param   light   The Light to check for membership.
     * @return  True if the light is a member of the LightSet.
     */
    bool contains(const Light* light) const;

    /**
     * Check if any of the following that are modified: Lights
     * in the LightSet, SceneObject attributes, and the LightSet itself.
     * Should only be called after all UpdateGuards.
     *
     * @return  True if the light list changed
     */
    bool updatePrepLight(UpdateHelper& sceneObjects, size_t depth);

    /**
     * Alphabetizes light list by name
     */
    void update() override;

    /// Completely empties the LightSet so that it doesn't contain anything.
    void clear();

protected:
    static AttributeKey<SceneObjectVector> sLightsKey;
};

const SceneObjectVector& LightSet::getLights() const { return get(sLightsKey); }

template <>
inline const LightSet*
SceneObject::asA() const
{
    return isA<LightSet>() ? static_cast<const LightSet*>(this) : nullptr;
}

template <>
inline LightSet*
SceneObject::asA()
{
    return isA<LightSet>() ? static_cast<LightSet*>(this) : nullptr;
}

} // namespace rdl2
} // namespace scene_rdl2

