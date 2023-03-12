// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "AttributeKey.h"
#include "SceneObject.h"
#include "Types.h"

namespace scene_rdl2 {
namespace rdl2 {

/**
 * The LightFilterSet represents a collection of LightFilters with no duplicates. It's
 * used in Part/Material assignments in the Layer, where you can define which
 * LightFilters affect a particular Part/Material assignment. That collection of
 * LightFilters is the LightFilterSet. LightFilterSets can be reused for many Part/Material
 * assignments.
 *
 * It only has one attribute, named "lightfilters", which is a SceneObjectVector.
 * We provide convenience functions on the LightFilterSet to add and remove LightFilters
 * from the set, as well as check if a LightFilter is contained in the set. Please use these
 * functions to maintain the lightfilter's uniqueness invariant. When setting "lightfilters"
 * directly, make sure the light filters are unique and their order is deterministic.
 *
 * You can get the LightFilters as a const SceneObjectVector& with the getLightFilters() method call.
 */
class LightFilterSet : public SceneObject
{
public:
    typedef SceneObject Parent;

    LightFilterSet(const SceneClass& sceneClass, const std::string& name);
    static SceneObjectInterface declare(SceneClass& sceneClass);

    /// Retrieves the set of unique LightFilters in this LightFilterSet.
    finline const SceneObjectVector& getLightFilters() const;

    /**
     * Adds the given LightFilter to the LightFilterSet, if it is not already a member of
     * the set. If it is already a member of the set, this does nothing.
     *
     * @param   lightfilter   The LightFilter to add to the LightFilterSet.
     */
    void add(LightFilter* lightfilter);

    /**
     * Removes the given LightFilter from the LightFilterSet, if it is already a member of
     * the set. If it is not a member of the set, this does nothing.
     *
     * @param   lightfilter   The LightFilter to remove from the LightFilterSet.
     */
    void remove(LightFilter* lightfilter);

    /**
     * Returns true if the given LightFilter is a member of the LightFilterSet. There's
     * no need to call this before calling add() or remove(), as they will
     * gracefully handle those edge cases.
     *
     * @param   lightfilter   The LightFilter to check for membership.
     * @return  True if the lightfilter is a member of the LightFilterSet.
     */
    bool contains(const LightFilter* lightfilter) const;

    /**
     * Check if any of the following that are modified: LightFilters
     * in the LightFilterSet, SceneObject attributes, and the LightFilterSet itself.
     * Should only be called after all UpdateGuards.
     *
     * @return  True if the lightfilter list changed
     */
    bool updatePrepLightFilter(UpdateHelper& sceneObjects, size_t depth);

    /**
     * Alphabetizes light filter list by name
     */
    void update() override;

    /// Completely empties the LightFilterSet so that it doesn't contain anything.
    void clear();

private:
    static AttributeKey<SceneObjectVector> sLightFiltersKey;
};

const SceneObjectVector& LightFilterSet::getLightFilters() const { return get(sLightFiltersKey); }

template <>
inline const LightFilterSet*
SceneObject::asA() const
{
    return isA<LightFilterSet>() ? static_cast<const LightFilterSet*>(this) : nullptr;
}

template <>
inline LightFilterSet*
SceneObject::asA()
{
    return isA<LightFilterSet>() ? static_cast<LightFilterSet*>(this) : nullptr;
}

} // namespace rdl2
} // namespace scene_rdl2

