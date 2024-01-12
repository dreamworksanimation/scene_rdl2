// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "LightSet.h"

#include "AttributeKey.h"
#include "Light.h"
#include "SceneClass.h"
#include "SceneObject.h"
#include "Types.h"

#include <scene_rdl2/common/except/exceptions.h>
#include <scene_rdl2/render/util/Strings.h>

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

namespace scene_rdl2 {
namespace rdl2 {

AttributeKey<SceneObjectVector> LightSet::sLightsKey;

LightSet::LightSet(const SceneClass& sceneClass, const std::string& name) :
    Parent(sceneClass, name)
{
    // Add the LightSet interface.
    mType |= INTERFACE_LIGHTSET;
}

SceneObjectInterface
LightSet::declare(SceneClass& sceneClass)
{
    auto interface = Parent::declare(sceneClass);

    sLightsKey = sceneClass.declareAttribute<SceneObjectVector>("lights", FLAGS_NONE, INTERFACE_LIGHT);
    sceneClass.setMetadata(sLightsKey, "comment",
        "List of lights that belong to this LightSet");

    return interface | INTERFACE_LIGHTSET;
}

void
LightSet::add(Light* light)
{
    if (!mUpdateActive) {
        std::stringstream errMsg;
        errMsg << "Light '" << light->getName() << "' can only be added to"
            " LightSet '" << mName << "' between beginUpdate() and endUpdate()"
            " calls.";
        throw except::RuntimeError(errMsg.str());
    }

    // Retrieve a mutable reference to the lights attribute.
    SceneObjectVector& lights = getMutable(sLightsKey);

    // Binary search for the insertion point.
    SceneObjectVector::iterator insertPoint = 
            lowerBoundByName(lights.begin(), lights.end(), light);

    // Is the light at the insertion point the same? If so, it's already in the
    // set.
    if (insertPoint != lights.end() && *insertPoint == light) {
        return;
    }

    // Otherwise, do the insert.
    lights.insert(insertPoint, light);

    // Manually turn on the set flag and dirty flag since we didn't go through
    // the set() method.
    mAttributeUpdateMask.set(sLightsKey.mIndex, true);
    mAttributeSetMask.set(sLightsKey.mIndex, true);
    mDirty = true;
}

void
LightSet::remove(Light* light)
{
    if (!mUpdateActive) {
        std::stringstream errMsg;
        errMsg << "Light '" << light->getName() << "' can only be removed from"
            " LightSet '" << mName << "' between beginUpdate() and endUpdate()"
            " calls.";
        throw except::RuntimeError(errMsg.str());
    }

    // Retrieve a mutable reference to the lights attribute.
    SceneObjectVector& lights = getMutable(sLightsKey);

    // Binary search for the light.
    SceneObjectVector::iterator removePoint =
            lowerBoundByName(lights.begin(), lights.end(), light);

    // If found, remove it.
    if (removePoint != lights.end() && *removePoint == light) {
        lights.erase(removePoint);
    
        // Manually turn on the set flag and dirty flag since we didn't go
        // through the set() method.
        mAttributeUpdateMask.set(sLightsKey.mIndex, true);
        mAttributeSetMask.set(sLightsKey.mIndex, true);
        mDirty = true;
    }
}

bool
LightSet::contains(const Light* light) const
{
    // Retrieve a const reference to the lights attribute.
    const SceneObjectVector& lights = get(sLightsKey);

    // Binary search for the light.
    SceneObjectVector::const_iterator iter =
            lowerBoundByName(lights.begin(), lights.end(), light);

    return iter != lights.end() && *iter == light;
}

bool
LightSet::updatePrepLight(UpdateHelper& sceneObjects, size_t depth)
{
    MNRY_ASSERT_REQUIRE(!mUpdateActive);
    return updatePrep(sceneObjects, depth);
}

void
LightSet::update()
{
    // Retrieve a mutable reference to the lights attribute.
    SceneObjectVector& lights = getMutable(sLightsKey);

    // Sort light filters by name.
    std::sort(lights.begin(), lights.end(),
        [](const SceneObject* a, const SceneObject* b) {
            return a->getName() < b->getName();
        }
    );
}

void
LightSet::clear()
{
    if (!mUpdateActive) {
        throw except::RuntimeError(util::buildString("LightSet '", mName,
            "' can only be cleared between beginUpdate() and endUpdate() calls."));
    }

    // Retrieve a mutable reference to the lights attribute.
    SceneObjectVector& lights = getMutable(sLightsKey);

    lights.clear();

    // Manually turn on the set flag and dirty flag since we didn't go
    // through the set() method.
    mAttributeUpdateMask.set(sLightsKey.mIndex, true);
    mAttributeSetMask.set(sLightsKey.mIndex, true);
    mDirty = true;
}

} // namespace rdl2
} // namespace scene_rdl2

