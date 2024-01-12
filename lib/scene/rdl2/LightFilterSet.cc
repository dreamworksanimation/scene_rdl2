// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "LightFilterSet.h"

#include "LightFilter.h"
#include "SceneClass.h"
#include "SceneObject.h"
#include "UpdateHelper.h"

#include <scene_rdl2/render/util/Strings.h>
#include <scene_rdl2/common/except/exceptions.h>

#include <sstream>
#include <string>

namespace scene_rdl2 {
namespace rdl2 {

AttributeKey<SceneObjectVector> LightFilterSet::sLightFiltersKey;

LightFilterSet::LightFilterSet(const SceneClass& sceneClass, const std::string& name) :
    Parent(sceneClass, name)
{
    // Add the LightFilterSet interface.
    mType |= INTERFACE_LIGHTFILTERSET;
}

SceneObjectInterface
LightFilterSet::declare(SceneClass& sceneClass)
{
    auto interface = Parent::declare(sceneClass);

    sLightFiltersKey = sceneClass.declareAttribute<SceneObjectVector>("lightfilters", FLAGS_NONE,
                                                                      INTERFACE_LIGHTFILTER);
    sceneClass.setMetadata(sLightFiltersKey, "comment", 
        "List of light filters that belong to this LightFilterSet");

    return interface | INTERFACE_LIGHTFILTERSET;
}

void
LightFilterSet::add(LightFilter* lightfilter)
{
    if (!mUpdateActive) {
        std::stringstream errMsg;
        errMsg << "LightFilter '" << lightfilter->getName() << "' can only be added to"
            " LightFilterSet '" << mName << "' between beginUpdate() and endUpdate()"
            " calls.";
        throw except::RuntimeError(errMsg.str());
    }

    // Retrieve a mutable reference to the lights attribute.
    SceneObjectVector& lightfilters = getMutable(sLightFiltersKey);

    // Binary search for the insertion point.
    SceneObjectVector::iterator insertPoint = lowerBoundByName(lightfilters.begin(),
                                                               lightfilters.end(),
                                                               lightfilter);

    // Is the lightfilter at the insertion point the same? If so, it's already in the
    // set.
    if (insertPoint != lightfilters.end() && *insertPoint == lightfilter) {
        return;
    }

    // Otherwise, do the insert.
    lightfilters.insert(insertPoint, lightfilter);

    // Manually turn on the set flag and dirty flag since we didn't go through
    // the set() method.
    mAttributeUpdateMask.set(sLightFiltersKey.mIndex, true);
    mAttributeSetMask.set(sLightFiltersKey.mIndex, true);
    mDirty = true;
}

void
LightFilterSet::remove(LightFilter* lightfilter)
{
    if (!mUpdateActive) {
        std::stringstream errMsg;
        errMsg << "LightFilter '" << lightfilter->getName() << "' can only be removed from"
            " LightFilterSet '" << mName << "' between beginUpdate() and endUpdate()"
            " calls.";
        throw except::RuntimeError(errMsg.str());
    }

    // Retrieve a mutable reference to the lights attribute.
    SceneObjectVector& lightfilters = getMutable(sLightFiltersKey);

    // Binary search for the lightfilter.
    SceneObjectVector::iterator removePoint = lowerBoundByName(lightfilters.begin(),
                                                               lightfilters.end(),
                                                               lightfilter);

    // If found, remove it.
    if (removePoint != lightfilters.end() && *removePoint == lightfilter) {
        lightfilters.erase(removePoint);

        // Manually turn on the set flag and dirty flag since we didn't go
        // through the set() method.
        mAttributeUpdateMask.set(sLightFiltersKey.mIndex, true);
        mAttributeSetMask.set(sLightFiltersKey.mIndex, true);
        mDirty = true;
    }
}

bool
LightFilterSet::contains(const LightFilter* lightfilter) const
{
    // Retrieve a const reference to the lights attribute.
    const SceneObjectVector& lightfilters = get(sLightFiltersKey);

    // Binary search for the lightfilter.
    SceneObjectVector::const_iterator iter = lowerBoundByName(lightfilters.begin(),
                                                              lightfilters.end(),
                                                              lightfilter);

    return iter != lightfilters.end() && *iter == lightfilter;
}

bool
LightFilterSet::updatePrepLightFilter(UpdateHelper& sceneObjects, size_t depth)
{
    MNRY_ASSERT_REQUIRE(!mUpdateActive);
    return updatePrep(sceneObjects, depth);
}

void
LightFilterSet::update()
{
    // Retrieve a mutable reference to the light filters attribute.
    SceneObjectVector& lightfilters = getMutable(sLightFiltersKey);

    // Sort light filters by name.
    std::sort(lightfilters.begin(), lightfilters.end(),
        [](const SceneObject* a, const SceneObject* b) {
            return a->getName() < b->getName();
        }
    );
}

void
LightFilterSet::clear()
{
    if (!mUpdateActive) {
        throw except::RuntimeError(util::buildString("LightFilterSet '", mName,
            "' can only be cleared between beginUpdate() and endUpdate() calls."));
    }

    // Retrieve a mutable reference to the light filters attribute.
    SceneObjectVector& lightfilters = getMutable(sLightFiltersKey);

    lightfilters.clear();

    // Manually turn on the set flag and dirty flag since we didn't go
    // through the set() method.
    mAttributeUpdateMask.set(sLightFiltersKey.mIndex, true);
    mAttributeSetMask.set(sLightFiltersKey.mIndex, true);
    mDirty = true;
}

} // namespace rdl2
} // namespace scene_rdl2

