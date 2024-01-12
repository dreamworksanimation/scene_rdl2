// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "GeometrySet.h"

#include "AttributeKey.h"
#include "Geometry.h"
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

AttributeKey<SceneObjectIndexable> GeometrySet::sGeometriesKey;

GeometrySet::GeometrySet(const SceneClass& sceneClass, const std::string& name) :
    Parent(sceneClass, name)
{
    // Add the GeometrySet interface.
    mType |= INTERFACE_GEOMETRYSET;

    // By default, a GeometrySet is included in the BVH.
    mIncludeInBVH = true;
}

SceneObjectInterface
GeometrySet::declare(SceneClass& sceneClass)
{
    auto interface = Parent::declare(sceneClass);

    sGeometriesKey = sceneClass.declareAttribute<SceneObjectIndexable>("geometries", FLAGS_NONE, INTERFACE_GEOMETRY);

    return interface | INTERFACE_GEOMETRYSET;
}

void
GeometrySet::add(Geometry* geometry)
{
    if (!mUpdateActive) {
        std::stringstream errMsg;
        errMsg << "Geometry '" << geometry->getName() << "' can only be added"
            " to GeometrySet '" << mName << "' between beginUpdate() and"
            " endUpdate() calls.";
        throw except::RuntimeError(errMsg.str());
    }

    // Retrieve a mutable reference to the geometries attribute.
    SceneObjectIndexable& geometries = getMutable(sGeometriesKey);

    // Search for the geometry.
    const auto iters = geometries.equal_range(const_cast<Geometry*>(geometry));

    // If it's already in the set, we're done
    if (iters.first != iters.second) {
        return;
    }

    // Otherwise, append geometry.
    geometries.push_back(geometry);

    // When a geometry is added to a geometry set in a delta file,
    // we must make a request to update it.
    geometry->requestUpdate();

    // Manually turn on the set flag and dirty flag since we didn't go through
    // the set() method.
    mAttributeUpdateMask.set(sGeometriesKey.mIndex, true);
    mAttributeSetMask.set(sGeometriesKey.mIndex, true);
    mDirty = true;
}

void
GeometrySet::remove(Geometry* geometry)
{
    if (!mUpdateActive) {
        std::stringstream errMsg;
        errMsg << "Geometry '" << geometry->getName() << "' can only be removed"
            " from GeometrySet '" << mName << "' between beginUpdate() and"
            " endUpdate() calls.";
        throw except::RuntimeError(errMsg.str());
    }

    // Retrieve a mutable reference to the geometries attribute.
    SceneObjectIndexable& geometries = getMutable(sGeometriesKey);

    // Search for the geometry.
    const auto iters = geometries.equal_range(const_cast<Geometry*>(geometry));

    // If found, remove it.
    if (iters.first != iters.second) {
        geometries.erase(geometries.begin() + *iters.first);
    
        // Manually turn on the set flag and dirty flag since we didn't go
        // through the set() method.
        mAttributeUpdateMask.set(sGeometriesKey.mIndex, true);
        mAttributeSetMask.set(sGeometriesKey.mIndex, true);
        mDirty = true;
    }
}

bool
GeometrySet::contains(const Geometry* geometry) const
{
    // Retrieve a const reference to the geometries attribute.
    const SceneObjectIndexable& geometries = get(sGeometriesKey);

    // Search for the geometry.
    const auto iters = geometries.equal_range(const_cast<Geometry*>(geometry));

    return iters.first != iters.second;
}

void
GeometrySet::clear()
{
    if (!mUpdateActive) {
        throw except::RuntimeError(util::buildString("GeometrySet '", mName,
            "' can only be cleared between beginUpdate() and endUpdate() calls."));
    }

    // Retrieve a mutable reference to the geometries attribute.
    SceneObjectIndexable& geometries = getMutable(sGeometriesKey);

    geometries.clear();

    // Manually turn on the set flag and dirty flag since we didn't go
    // through the set() method.
    mAttributeUpdateMask.set(sGeometriesKey.mIndex, true);
    mAttributeSetMask.set(sGeometriesKey.mIndex, true);
    mDirty = true;
}

bool
GeometrySet::isStatic() const
{
    // Retrieve a const reference to the geometries attribute.
    const SceneObjectIndexable& geometries = get(sGeometriesKey);

    for (auto iter = geometries.begin(); iter != geometries.end(); ++iter) {
        if (!(*iter)->asA<Geometry>()->isStatic()) {
            return false;
        }
    }

    return true;
}

bool
GeometrySet::updatePrepFast(UpdateHelper& sceneObjects, int depth)
{
    MNRY_ASSERT_REQUIRE(!mUpdateActive);

    if (mUpdatePrepApplied &&
            (sceneObjects.getDepth(this) >=depth || sceneObjects.isLeaf(this))) {
        return updateRequired();
    }
    mUpdatePrepApplied = true;

    const SceneObjectIndexable& geometries = get(sGeometriesKey);
    bool attributeTreeChanged = false;
    bool bindingTreeChanged = false;
    // check for any geometry in this set has been updated.
    for (auto iter = geometries.begin(); iter != geometries.end(); ++iter) {
        Geometry* geom = (*iter)->asA<Geometry>();
        if (geom->attributeTreeChanged()) {
            attributeTreeChanged = true;
        }
        if (geom->bindingTreeChanged()) {
            bindingTreeChanged = true;
        }
        // terminate early if both are true
        if (attributeTreeChanged && bindingTreeChanged) {
            break;
        }
    }
    mAttributeTreeChanged = attributeTreeChanged || mAttributeUpdateMask.any();
    mBindingTreeChanged = bindingTreeChanged || mBindingUpdateMask.any();

    if (updateRequired()) {
        sceneObjects.insert(this, depth);
    }
    return updateRequired();
}

} // namespace rdl2
} // namespace scene_rdl2

