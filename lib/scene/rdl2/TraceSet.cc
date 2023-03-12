// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "TraceSet.h"
#include "Geometry.h"

#include <scene_rdl2/common/except/exceptions.h>

namespace scene_rdl2 {
namespace rdl2 {

AttributeKey<SceneObjectIndexable> TraceSet::sGeometriesKey;
AttributeKey<StringVector>         TraceSet::sPartsKey;

TraceSet::TraceSet(const SceneClass& sceneClass, const std::string& name) :
    Parent(sceneClass, name)
{
    // Add the TraceSet interface.
    mType |= INTERFACE_TRACESET;
}

SceneObjectInterface
TraceSet::declare(SceneClass& sceneClass)
{
    auto interface = Parent::declare(sceneClass);

    sGeometriesKey = sceneClass.declareAttribute<SceneObjectIndexable>("geometries", FLAGS_NONE, INTERFACE_GEOMETRY);
    sceneClass.setMetadata(sGeometriesKey, "comment", 
        "Geometry objects that are members of this TraceSet");

    sPartsKey = sceneClass.declareAttribute<StringVector>("parts");
    sceneClass.setMetadata(sPartsKey, "comment", 
        "Part names (one for each geometry object)");

    return interface | INTERFACE_TRACESET;
}

int32_t
TraceSet::getAssignmentCount() const
{
    return static_cast<int32_t>(get(sGeometriesKey).size());
}

int32_t
TraceSet::assign(Geometry* geometry, const String& partName)
{
    if (!mUpdateActive) {
        std::stringstream errMsg;
        errMsg << "Can only make assignment ('" << geometry->getName() <<
            "', '" << partName << "') in TraceSet '" << mName << "' between"
            " beginUpdate() and endUpdate() calls.";
        throw except::RuntimeError(errMsg.str());
    }

    // Get mutable references to the attribute vectors.
    auto& geometries = getMutable(sGeometriesKey);
    auto& parts = getMutable(sPartsKey);

    // We have to do a const cast here, because we have a container of
    // SceneObject*, but equal_range takes a const T&. The compiler does not
    // like us trying to take a const pointer reference. It's complicated and
    // annoying.
    const auto iters = geometries.equal_range(const_cast<Geometry*>(geometry));
    // If the assignment already exists, just return the existing assignment ID.
    for (auto it = iters.first; it != iters.second; ++it) {
        const auto idx = *it;
        if (parts[idx] == partName) {
            return idx;
        }
    }

    // Assignment doesn't exist yet, so create it.
    geometries.push_back(geometry);
    parts.push_back(partName);

    mAttributeUpdateMask.set(sGeometriesKey.mIndex, true);
    mAttributeUpdateMask.set(sPartsKey.mIndex, true);
    mAttributeSetMask.set(sGeometriesKey.mIndex, true);
    mAttributeSetMask.set(sPartsKey.mIndex, true);
    mDirty = true;

    return geometries.size() - 1;
}

TraceSet::GeometryPartPair
TraceSet::lookupGeomAndPart(int32_t assignmentId) const
{
    const auto& geometries = get(sGeometriesKey);
    const auto& parts = get(sPartsKey);

    // Sanity check.
    if (assignmentId < 0 || std::size_t(assignmentId) >= geometries.size()) {
        std::stringstream errMsg;
        errMsg << "Assignment ID '" << assignmentId << "' on trace set '" <<
            getName() << "' is out of range (contains " << geometries.size() <<
            " assignments).";
        throw except::IndexError(errMsg.str());
    }

    return GeometryPartPair(geometries[assignmentId]->asA<Geometry>(),
                            parts[assignmentId]);
}

int32_t
TraceSet::getAssignmentId(const Geometry* geometry, const String& partName) const
{
    const auto& geometries = get(sGeometriesKey);
    const auto& parts = get(sPartsKey);

    // Save the default assignment (part name "") if we come across it during
    // the search.
    int32_t defaultAssignmentId = -1; // Not found.

    // We have to do a const cast here, because we have a container of
    // SceneObject*, but equal_range takes a const T&. The compiler does not
    // like us trying to take a const pointer reference. It's complicated and
    // annoying.
    const auto iters = geometries.equal_range(const_cast<Geometry*>(geometry));
    for (auto it = iters.first; it != iters.second; ++it) {
        const auto idx = *it;
        // Pointer compare for geometry uniqueness is ok, since the SceneContext
        // enforces that we can't create two SceneObjects with the same name.
        const std::string& part = parts[idx];
        if (part == partName) {
            return idx;
        } else if (part == "") {
            defaultAssignmentId = idx;
        }
    }

    // Return the default assignment.
    return defaultAssignmentId;
}

bool
TraceSet::contains(const Geometry* geometry) const
{
    const auto& geometries = get(sGeometriesKey);

    // We have to do a const cast here, because we have a container of
    // SceneObject*, but equal_range takes a const T&. The compiler does not
    // like us trying to take a const pointer reference. It's complicated and
    // annoying.
    const auto iters = geometries.equal_range(const_cast<Geometry*>(geometry));
    return iters.first != iters.second;
}

TraceSet::GeometryIterator
TraceSet::begin(const Geometry* geometry) const
{
    const auto& geometries = get(sGeometriesKey);

    // Using the IndexableArray::equal_range function allows us to cull out a
    // lot of the objects before we iterate over them.
    const auto p = geometries.equal_range(const_cast<Geometry*>(geometry));
    return GeometryIterator(p.first,
                            p.first, p.second,
                            detail::makeContainerWrapper(geometries),
                            geometry);
}

TraceSet::GeometryIterator
TraceSet::end(const Geometry* geometry) const
{
    const auto& geometries = get(sGeometriesKey);

    // Using the IndexableArray::equal_range function allows us to cull out a
    // lot of the objects before we iterate over them.
    const auto p = geometries.equal_range(const_cast<Geometry*>(geometry));
    return GeometryIterator(p.second,
                            p.first, p.second,
                            detail::makeContainerWrapper(geometries),
                            geometry);
}

} // namespace rdl2
} // namspace scene_rdl2

