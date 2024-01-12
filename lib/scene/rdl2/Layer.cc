// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "Layer.h"

#include "AttributeKey.h"
#include "Geometry.h"
#include "Light.h"
#include "LightFilterSet.h"
#include "LightSet.h"
#include "Material.h"
#include "SceneClass.h"
#include "SceneObject.h"
#include "ShadowReceiverSet.h"
#include "ShadowSet.h"
#include "Types.h"

#include <scene_rdl2/common/except/exceptions.h>
#include <scene_rdl2/render/util/Strings.h>

#include <cstddef>
#include <sstream>
#include <string>
#include <utility>
#include <stdint.h>

namespace {

// convenience function that checks for the existence of a procedural
// and then calls its deformed() method, returning false if it
// does not exist.
bool
isDeformed(const scene_rdl2::rdl2::Geometry *geom)
{
    return geom->getProcedural() ? geom->deformed() : false;
}

}

namespace scene_rdl2 {
namespace rdl2 {

AttributeKey<SceneObjectVector> Layer::sSurfaceShadersKey;
AttributeKey<SceneObjectVector> Layer::sLightSetsKey;
AttributeKey<SceneObjectVector> Layer::sDisplacementsKey;
AttributeKey<SceneObjectVector> Layer::sVolumeShadersKey;
AttributeKey<SceneObjectVector> Layer::sLightFilterSetsKey;
AttributeKey<SceneObjectVector> Layer::sShadowSetsKey;
AttributeKey<SceneObjectVector> Layer::sShadowReceiverSetsKey;

Layer::Layer(const SceneClass& sceneClass, const std::string& name) :
    Parent(sceneClass, name),
    mLightSetsChanged(false),
    mLightFilterSetsChanged(false),
    mShadowSetsChanged(false),
    mShadowReceiverSetsChanged(false)
{
    // Add the Layer interface.
    mType |= INTERFACE_LAYER;
}

SceneObjectInterface
Layer::declare(SceneClass& sceneClass)
{
    auto interface = Parent::declare(sceneClass);

    // This call overrides the comments for the corresponding attribute in TraceSet, from which Layer inherits
    sceneClass.setMetadata(sGeometriesKey, "comment",
        "The geometry objects included in the layer, each of which must be included in the GeometrySet.\n"
        "    Note: this attribute is typically not set directly, but by using a Lua table for defining "
        "layer entries. For example,\n"
        "      Layer(\"/myLayer/\") { \n"
        "        {myGeometry1, \"\", ...}\n"
        "        {myGeometry2, \"\", ...}\n"
        "      }");

    // This call overrides the comments for the corresponding attribute in TraceSet, from which Layer inherits
    sceneClass.setMetadata(sPartsKey, "comment",
        "For each geometry object in the layer, the list of names of the parts of that geometry to be included.\n"
        "    To include all parts of a geometry object without needing to name them explicitly, "
        "use the empty string, \"\".\n"
        "    Note: this attribute is typically not set directly, but by using a Lua table for defining "
        "layer entries. For example,\n"
        "      Layer(\"/myLayer/\") { \n"
        "        {myGeometry, {\"part1\", \"part2\"}, ...}\n"
        "      }\n"
        "    If the list contains only one entry (either the empty string or a single part name), the braces can "
        "optionally be omitted. For example,\n"
        "      Layer(\"/myLayer/\") { \n"
        "        {myGeometry, \"part1\", ...}\n"
        "      }");

    sSurfaceShadersKey = sceneClass.declareAttribute<SceneObjectVector>("surface_shaders", FLAGS_NONE,
        INTERFACE_MATERIAL, { "surface shaders" });
    sceneClass.setMetadata(sSurfaceShadersKey, "label", "surface shaders");
    sceneClass.setMetadata(sSurfaceShadersKey, rdl2::SceneClass::sComment, "The materials "
        "assigned to geometry objects in the layer, or to their specified parts.\n"
        "    Note: this attribute is typically not set directly, but by using a Lua table for defining "
        "layer entries. For example,\n"
        "      Layer(\"/myLayer/\") { \n"
        "        {myGeometry, \"\", myMaterial, ...}\n"
        "      }");

    sLightSetsKey = sceneClass.declareAttribute<SceneObjectVector>("lightsets", FLAGS_NONE, INTERFACE_LIGHTSET);
    sceneClass.setMetadata(sLightSetsKey, rdl2::SceneClass::sComment, "The light sets "
        "assigned to geometry objects in the layer, or to their specified parts.\n"
        "    Note: this attribute is typically not set directly, but by using a Lua table for defining "
        "layer entries. For example,\n"
        "      Layer(\"/myLayer/\") { \n"
        "        {myGeometry, \"\", myLightSet, ...}\n"
        "      }");

    sDisplacementsKey = sceneClass.declareAttribute<SceneObjectVector>("displacements", FLAGS_NONE, INTERFACE_DISPLACEMENT);
    sceneClass.setMetadata(sDisplacementsKey, rdl2::SceneClass::sComment, "The displacement shaders "
        "assigned to geometry objects in the layer, or to their specified parts.\n"
        "    Note: this attribute is typically not set directly, but by using a Lua table for defining "
        "layer entries. For example,\n"
        "      Layer(\"/myLayer/\") { \n"
        "        {myGeometry, \"\", myDisplacement, ...}\n"
        "      }");

    sVolumeShadersKey = sceneClass.declareAttribute<SceneObjectVector>("volume_shaders", FLAGS_NONE, INTERFACE_VOLUMESHADER, { "volume shaders" });
    sceneClass.setMetadata(sVolumeShadersKey, "label", "volume shaders");
    sceneClass.setMetadata(sVolumeShadersKey, rdl2::SceneClass::sComment, "The volume shaders "
        "assigned to geometry objects in the layer, or to their specified parts.\n"
        "    Note: this attribute is typically not set directly, but by using a Lua table for defining "
        "layer entries. For example,\n"
        "      Layer(\"/myLayer/\") { \n"
        "        {myGeometry, \"\", myVolumeShader, ...}\n"
        "      }");

    sLightFilterSetsKey = sceneClass.declareAttribute<SceneObjectVector>("lightfiltersets", FLAGS_NONE,
        INTERFACE_LIGHTFILTERSET);
    sceneClass.setMetadata(sLightFilterSetsKey, rdl2::SceneClass::sComment, "The light filter sets "
        "assigned to geometry objects in the layer, or to their specified parts.\n"
        "    Note: this attribute is typically not set directly, but by using a Lua table for defining "
        "layer entries. For example,\n"
        "      Layer(\"/myLayer/\") { \n"
        "        {myGeometry, \"\", myLightFilterSet, ...}\n"
        "      }");

    sShadowSetsKey = sceneClass.declareAttribute<SceneObjectVector>("shadowsets", FLAGS_NONE, INTERFACE_SHADOWSET);
    sceneClass.setMetadata(sShadowSetsKey, rdl2::SceneClass::sComment, "The shadow sets "
        "assigned to geometry objects in the layer, or to their specified parts.\n"
        "    Note: this attribute is typically not set directly, but by using a Lua table for defining "
        "layer entries. For example,\n"
        "      Layer(\"/myLayer/\") { \n"
        "        {myGeometry, \"\", myShadowSet, ...}\n"
        "      }");

    sShadowReceiverSetsKey = sceneClass.declareAttribute<SceneObjectVector>("shadowreceiversets", FLAGS_NONE,
        INTERFACE_SHADOWRECEIVERSET);
    sceneClass.setMetadata(sShadowReceiverSetsKey, rdl2::SceneClass::sComment, "The shadow receiver sets "
        "assigned to geometry objects in the layer, or to their specified parts.\n"
        "    Note: this attribute is typically not set directly, but by using a Lua table for defining "
        "layer entries. For example,\n"
        "      Layer(\"/myLayer/\") { \n"
        "        {myGeometry, \"\", myShadowReceiverSet, ...}\n"
        "      }");

    return interface | INTERFACE_LAYER;
}

void
Layer::dirtyAssignments() {
    // Manually turn on the set flags, the update flags, and dirty flag since we
    // didn't go through the set() method.
    mAttributeUpdateMask.set(sGeometriesKey.mIndex, true);
    mAttributeUpdateMask.set(sPartsKey.mIndex, true);
    mAttributeUpdateMask.set(sSurfaceShadersKey.mIndex, true);
    mAttributeUpdateMask.set(sLightSetsKey.mIndex, true);
    mAttributeUpdateMask.set(sDisplacementsKey.mIndex, true);
    mAttributeUpdateMask.set(sVolumeShadersKey.mIndex, true);
    mAttributeUpdateMask.set(sLightFilterSetsKey.mIndex, true);
    mAttributeUpdateMask.set(sShadowSetsKey.mIndex, true);
    mAttributeUpdateMask.set(sShadowReceiverSetsKey.mIndex, true);
    mAttributeSetMask.set(sGeometriesKey.mIndex, true);
    mAttributeSetMask.set(sPartsKey.mIndex, true);
    mAttributeSetMask.set(sSurfaceShadersKey.mIndex, true);
    mAttributeSetMask.set(sLightSetsKey.mIndex, true);
    mAttributeSetMask.set(sLightFilterSetsKey.mIndex, true);
    mAttributeSetMask.set(sDisplacementsKey.mIndex, true);
    mAttributeSetMask.set(sVolumeShadersKey.mIndex, true);
    mAttributeSetMask.set(sShadowSetsKey.mIndex, true);
    mAttributeSetMask.set(sShadowReceiverSetsKey.mIndex, true);
    mDirty = true;
}

int32_t
Layer::assign(Geometry* geometry, const String& partName,
              Material* material, LightSet* lightSet)
{
    LayerAssignment layerAssignment;
    layerAssignment.mMaterial = material;
    layerAssignment.mLightSet = lightSet;
    return assign(geometry, partName, layerAssignment);
}

int32_t
Layer::assign(Geometry* geometry, const String& partName,
              Material* material, LightSet* lightSet,
              Displacement* displacement, VolumeShader* volumeShader)
{
    LayerAssignment layerAssignment;
    layerAssignment.mMaterial = material;
    layerAssignment.mLightSet = lightSet;
    layerAssignment.mDisplacement = displacement;
    layerAssignment.mVolumeShader = volumeShader;
    return assign(geometry, partName, layerAssignment);
}

int32_t
Layer::assign(Geometry* geometry, const String& partName, const LayerAssignment& layerAssignment)
{
    if (!mUpdateActive) {
        std::stringstream errMsg;
        errMsg << "Can only make assignment ('" << geometry->getName() <<
            "', '" << partName << "') in Layer '" << mName << "' between"
            " beginUpdate() and endUpdate() calls.";
        throw except::RuntimeError(errMsg.str());
    }

    // Assign the geometry and part
    // For geometry with a volume shader, we ignore the parts which causes
    // moonray to just use the entire geometry.  Individual parts are generally
    // not closed shapes which causes problems with the volume integrator
    // because MoonRay treats parts as individual pieces of geometry.  Skipping the
    // part list uses the entire geometry as one "welded together" piece.
    // It is OK to repeatedly call TraceSet::assign() with the same geometry and empty
    // part because it is smart enough to lookup the existing idx before attempting
    // to add a new entry.
    int32_t idx = layerAssignment.mVolumeShader ? TraceSet::assign(geometry, "") : 
                                                  TraceSet::assign(geometry, partName);

    // Get mutable references to the attribute vectors.
    auto& surfaceShaders = getMutable(sSurfaceShadersKey);
    auto& lightSets = getMutable(sLightSetsKey);
    auto& displacements = getMutable(sDisplacementsKey);
    auto& volumeShaders = getMutable(sVolumeShadersKey);
    auto& lightFilterSets = getMutable(sLightFilterSetsKey);
    auto& shadowSets = getMutable(sShadowSetsKey);
    auto& shadowReceiverSets = getMutable(sShadowReceiverSetsKey);

    if (idx < static_cast<int32_t>(surfaceShaders.size())) {
        // assignment is for existing geometry / part pair
        bool shouldDirtyAssignments = false;
        if (surfaceShaders[idx] != layerAssignment.mMaterial) {
            surfaceShaders[idx] = layerAssignment.mMaterial;
            shouldDirtyAssignments = true;
        }
        if (lightSets[idx] != layerAssignment.mLightSet) {
            lightSets[idx] = layerAssignment.mLightSet;
            shouldDirtyAssignments = true;
        }
        if (displacements[idx] != layerAssignment.mDisplacement) {
            displacements[idx] = layerAssignment.mDisplacement;
            shouldDirtyAssignments = true;
        }
        if (volumeShaders[idx] != layerAssignment.mVolumeShader) {
            volumeShaders[idx] = layerAssignment.mVolumeShader;
            shouldDirtyAssignments = true;
        }
        if (lightFilterSets[idx] != layerAssignment.mLightFilterSet) {
            lightFilterSets[idx] = layerAssignment.mLightFilterSet;
            shouldDirtyAssignments = true;
        }
        if (shadowSets[idx] != layerAssignment.mShadowSet) {
            shadowSets[idx] = layerAssignment.mShadowSet;
            shouldDirtyAssignments = true;
        }
        if (shadowReceiverSets[idx] != layerAssignment.mShadowReceiverSet) {
            shadowReceiverSets[idx] = layerAssignment.mShadowReceiverSet;
            shouldDirtyAssignments = true;
        }

        // IMPORTANT: Binary reader requires this attributes serialized. It can not call this
        // method if the data is not present
        if (shouldDirtyAssignments) {
            dirtyAssignments();
        }
    } else {
        // assignment is for new geometry / part pair
        dirtyAssignments();

        // Assignment doesn't exist yet, so create it.
        surfaceShaders.push_back(layerAssignment.mMaterial);
        lightSets.push_back(layerAssignment.mLightSet);
        displacements.push_back(layerAssignment.mDisplacement);
        volumeShaders.push_back(layerAssignment.mVolumeShader);
        lightFilterSets.push_back(layerAssignment.mLightFilterSet);
        shadowSets.push_back(layerAssignment.mShadowSet);
        shadowReceiverSets.push_back(layerAssignment.mShadowReceiverSet);

        MNRY_ASSERT(surfaceShaders.size() == idx + 1);
    }

    return idx;
}

const Material*
Layer::lookupMaterial(int32_t assignmentId) const
{
    const auto& surfaceShaders = get(sSurfaceShadersKey);

    // Sanity check.
    if (assignmentId < 0 || std::size_t(assignmentId) >= surfaceShaders.size()) {
        std::stringstream errMsg;
        errMsg << "Assignment ID '" << assignmentId << "' on layer '" <<
            getName() << "' is out of range (contains " << surfaceShaders.size() <<
            " assignments).";
        throw except::IndexError(errMsg.str());
    }

    if (surfaceShaders[assignmentId]) {
        return surfaceShaders[assignmentId]->asA<Material>();
    } else {
        return nullptr;
    }
}

const LightSet*
Layer::lookupLightSet(int32_t assignmentId) const
{
    const auto& lightSets = get(sLightSetsKey);

    // Sanity check.
    MNRY_ASSERT(std::size_t(assignmentId) < lightSets.size());

    if (lightSets[assignmentId]) {
        return lightSets[assignmentId]->asA<LightSet>();
    } else {
        return nullptr;
    }
}

const Displacement*
Layer::lookupDisplacement(int32_t assignmentId) const
{
    const auto& displacements = get(sDisplacementsKey);

    // Sanity check.
    if (assignmentId < 0 || std::size_t(assignmentId) >= displacements.size()) {
        std::stringstream errMsg;
        errMsg << "Assignment ID '" << assignmentId << "' on layer '" <<
            getName() << "' is out of range (contains " << displacements.size() <<
            " assignments).";
        throw except::IndexError(errMsg.str());
    }

    if (displacements[assignmentId]) {
        return displacements[assignmentId]->asA<Displacement>();
    } else {
        return nullptr;
    }
}

const VolumeShader*
Layer::lookupVolumeShader(int32_t assignmentId) const
{
    const auto& volumeShaders = get(sVolumeShadersKey);

    // Sanity check.
    if (assignmentId < 0 || std::size_t(assignmentId) >= volumeShaders.size()) {
        std::stringstream errMsg;
        errMsg << "Assignment ID '" << assignmentId << "' on layer '" <<
            getName() << "' is out of range (contains " << volumeShaders.size() <<
            " assignments).";
        throw except::IndexError(errMsg.str());
    }

    if (volumeShaders[assignmentId]) {
        return volumeShaders[assignmentId]->asA<VolumeShader>();
    } else {
        return nullptr;
    }
}

const LightFilterSet*
Layer::lookupLightFilterSet(int32_t assignmentId) const
{
    const auto& lightFilterSets = get(sLightFilterSetsKey);

    // Sanity check.
    MNRY_ASSERT(std::size_t(assignmentId) < lightFilterSets.size());

    if (lightFilterSets[assignmentId]) {
        return lightFilterSets[assignmentId]->asA<LightFilterSet>();
    } else {
        return nullptr;
    }
}

const ShadowSet*
Layer::lookupShadowSet(int32_t assignmentId) const
{
    const auto& shadowSets = get(sShadowSetsKey);

    // Sanity check.
    MNRY_ASSERT(std::size_t(assignmentId) < shadowSets.size());

    if (shadowSets[assignmentId]) {
        return shadowSets[assignmentId]->asA<ShadowSet>();
    } else {
        return nullptr;
    }
}

const ShadowReceiverSet*
Layer::lookupShadowReceiverSet(int32_t assignmentId) const
{
    const auto& shadowReceiverSets = get(sShadowReceiverSetsKey);

    // Sanity check.
    MNRY_ASSERT(std::size_t(assignmentId) < shadowReceiverSets.size());

    if (shadowReceiverSets[assignmentId]) {
        return shadowReceiverSets[assignmentId]->asA<scene_rdl2::rdl2::ShadowReceiverSet>();
    } else {
        return nullptr;
    }
}

Layer::MaterialLightSetPair
Layer::lookup(int32_t assignmentId) const
{
    const auto& surfaceShaders = get(sSurfaceShadersKey);
    const auto& lightSets = get(sLightSetsKey);

    // Sanity check.
    if (assignmentId < 0 || std::size_t(assignmentId) >= surfaceShaders.size()) {
        std::stringstream errMsg;
        errMsg << "Assignment ID '" << assignmentId << "' on layer '" <<
            getName() << "' is out of range (contains " << surfaceShaders.size() <<
            " assignments).";
        throw except::IndexError(errMsg.str());
    }

    if (surfaceShaders[assignmentId]) {
        return MaterialLightSetPair(surfaceShaders[assignmentId]->asA<Material>(),
                                    lightSets[assignmentId]->asA<LightSet>());
    } else {
        return MaterialLightSetPair(nullptr,
                                    lightSets[assignmentId]->asA<LightSet>());
    }
}

Layer::MaterialLightSetPair
Layer::lookup(const Geometry* geometry, const String& partName) const
{
    return lookup(getAssignmentId(geometry, partName));
}

void
Layer::updatePrepAssignments(UpdateHelper& sceneObjects, int depth, Camera* camera)
{
    MNRY_ASSERT_REQUIRE(!mUpdateActive);
    
    mLightSetsChanged = hasChanged(sLightSetsKey);
    mLightFilterSetsChanged = hasChanged(sLightFilterSetsKey);
    mShadowSetsChanged = hasChanged(sShadowSetsKey);
    mShadowReceiverSetsChanged = hasChanged(sShadowReceiverSetsKey);

    Geometry* cameraMediumGeometry = camera ? camera->getMediumGeometry() : nullptr;

    // After previous update, resetAssignmentUpdates should have been called
    // to clean up these tables.
    MNRY_ASSERT(mChangedRootShaders.empty());
    MNRY_ASSERT(mChangedOrDeformedGeometries.empty());

    // Loop through all of the shaders in the scene and check if they are in the update graph. If so, flag so that 
    // we can update the primitive attribute tables in renderPrep(). Also flag the associated geometry for reload in
    // renderPrep(). 
    bool changed = false;    
    const auto& surfaceShaders = get(sSurfaceShadersKey);
    const auto& geometries = get(sGeometriesKey);
    const auto& displacements = get(sDisplacementsKey);
    const auto& volumeShaders = get(sVolumeShadersKey);
    for (size_t i = 0; i < surfaceShaders.size(); ++i) {
        Geometry * const geometry = geometries[i]->asA<Geometry>();

        // For IOR tracking purposes -- check if the geometry matches the geometry attached to the camera. If so, flag 
        // it so that (in updatePriorityAssignments) we can check for intersection with the geometry and set the 
        // initial IOR on the primary ray
        if (geometry == cameraMediumGeometry) {
            cameraMediumGeometry->setContainsCamera();
            if (!camera->getMediumMaterial()) {
                scene_rdl2::logging::Logger::warn("You must also attach to the Camera the \"medium_material\" you wish"
                                                  " to be applied to the medium_geometry.");
            }
        }

        if (surfaceShaders[i] != nullptr) {
            Material * const material = surfaceShaders[i]->asA<Material>();
            if (material && material->updatePrep(sceneObjects, depth + 1)) { // true if object is in update graph
                mChangedRootShaders.insert(material);
                // Geometries depend on materials because material request primitive
                // attributes from the geometry. That means if a material changes
                // it might request a new primitive attribute from the geometry
                // and so the geometry would need to be reloaded and retessellated.
                // At this point we do not know which primitive attributes the material
                // requests, that occurs during the update calls, so we add this
                // geometry to the list of changed or deformed geometries just in case.
                mChangedOrDeformedGeometries[geometry] = i;
                changed = true;
            }
        }
        if (volumeShaders[i] != nullptr) {
            VolumeShader * const volumeShader = volumeShaders[i]->asA<VolumeShader>();
            if (volumeShader && volumeShader->updatePrep(sceneObjects, depth + 1)) { // true if object is in update graph
                mChangedRootShaders.insert(volumeShader);
                // Geometries depend on volumeShaders because we bake the maps into the geometry itself
                mChangedOrDeformedGeometries[geometry] = i;
                changed = true;
            }
        }
        if (geometry) {
            if (isDeformed(geometry)) {
                mChangedOrDeformedGeometries[geometry] = i;
                changed = true;
            } else if (geometry->updatePrep(sceneObjects, depth + 1)) {
                // true if the dirtied attributes involve geometry change
                if (geometry->requiresGeometryUpdate(sceneObjects, depth + 1)) {
                    mChangedOrDeformedGeometries[geometry] = i;
                }
                changed = true;
            }
        }
        if (displacements[i] != nullptr) {
            Displacement * const displacement = displacements[i]->asA<Displacement>();
            if (displacement && displacement->updatePrep(sceneObjects, depth + 1)) { // true if object is in update graph
                mChangedRootShaders.insert(displacement);
                mChangedOrDeformedGeometries[geometry] = i;
                // geometry must re-tessellate even though no attrs or bindings have changed
                geometry->requestUpdate();
                changed = true;
            }
        }
    }
    // Flag LightSets, LightFilterSets, ShadowSets, and ShadowReceiverSets that need to be updated in preFrame()
    for (SceneObject * const lightSetObj : get(sLightSetsKey)) {
        if (lightSetObj) {
            LightSet * lightSet = lightSetObj->asA<LightSet>();
            if (lightSet->updatePrepLight(sceneObjects, depth  + 1)) {
                mLightSetsChanged = true;
                changed = true;
            }

            for (SceneObject * const light : lightSet->getLights()) {
                if (light->hasChanged(Light::sLightFiltersKey)) {
                    mLightFilterSetsChanged = true;
                    changed = true;
                }
            }
        }
    }

    if (!mLightFilterSetsChanged) {
        for (SceneObject * const lightFilterSet : get(sLightFilterSetsKey)) {
            if (lightFilterSet && lightFilterSet->asA<LightFilterSet>()->
                updatePrepLightFilter(sceneObjects, depth  + 1)) {
                mLightFilterSetsChanged = true;
                changed = true;
            }
        }
    }

    for (SceneObject * const shadowSetObj : get(sShadowSetsKey)) {
        if (shadowSetObj) {
            ShadowSet * shadowSet = shadowSetObj->asA<ShadowSet>();
            if (shadowSet->haveLightsChanged()) {
                mShadowSetsChanged = true;
                changed = true;
            }
        }
    }

    for (SceneObject * const shadowReceiverSetObj : get(sShadowReceiverSetsKey)) {
        if (shadowReceiverSetObj) {
            const scene_rdl2::rdl2::ShadowReceiverSet * shadowReceiverSet =
                shadowReceiverSetObj->asA<scene_rdl2::rdl2::ShadowReceiverSet>();
            if (shadowReceiverSet->haveGeometriesChanged()) {
                mShadowReceiverSetsChanged = true;
                changed = true;
            }
        }
    }

    // This is an optimization to avoid calling a full blown updatePrep(),
    // which would require an unnecessary full loop over root shaders, geometries
    // and lightSets again.
    updatePrepFast(changed, false, sceneObjects, depth);
}

void
Layer::clearShaderGraphPrimAttributeCache() const
{
    const auto& surfaceShaders = get(sSurfaceShadersKey);
    for (const SceneObject* s : surfaceShaders) {
        if (s && s->isA<RootShader>()) {
            s->asA<RootShader>()->clearShaderGraphCachedPrimAttributes();
        }
    }
}

bool
Layer::updatePrepFast(bool attributeTreeChanged,
                      bool bindingTreeChanged,
                      UpdateHelper& sceneObjects,
                      int depth)
{
    MNRY_ASSERT_REQUIRE(!mUpdateActive);
    // early out
    if (mUpdatePrepApplied &&
            (sceneObjects.getDepth(this) >= depth || sceneObjects.isLeaf(this))) {
        return updateRequired();
    }
    mUpdatePrepApplied = true;

    mAttributeTreeChanged = attributeTreeChanged || mAttributeUpdateMask.any();
    mBindingTreeChanged = bindingTreeChanged || mBindingUpdateMask.any();

    if (mAttributeTreeChanged || mBindingTreeChanged) {
        sceneObjects.insert(this, depth);
    }
    return updateRequired();
}

void
Layer::resetDeformedGeometries()
{
    // reset the modified flag for rdl geometries
    for (auto iter = mChangedOrDeformedGeometries.begin();
         iter != mChangedOrDeformedGeometries.end(); ++iter) {
        if (isDeformed(iter->first)) {
            iter->first->resetDeformed();
        }
    }
    mChangedOrDeformedGeometries.clear();
}

void
Layer::resetAssignmentUpdates()
{
    clearShaderGraphPrimAttributeCache();
    mLightSetsChanged = false;
    mChangedRootShaders.clear();
    resetDeformedGeometries();
}

static void
addRootShaderToSet(RootShader *s, Layer::RootShaderSet &rootShaders)
{
    // add this rootshader to rootshaders and any rootshader
    // that it directly references - recurse.
    auto res = rootShaders.insert(s);
    if (!res.second) return; // s was already in root shader

    const auto &sc = s->getSceneClass();
    for (auto it = sc.beginAttributes(); it != sc.endAttributes(); ++it) {
        if ((*it)->getType() == TYPE_SCENE_OBJECT) {
            SceneObject *o = s->get<SceneObject *>(AttributeKey<SceneObject *>(**it));
            if (o && o->isA<RootShader>()) {
                addRootShaderToSet(o->asA<RootShader>(), rootShaders);
            }
        }
    }
}

void
Layer::getAllRootShaders(RootShaderSet& rootShaders)
{
    for (SceneObject * const object : get(sSurfaceShadersKey)) {
        if (object) {
            addRootShaderToSet(object->asA<RootShader>(),rootShaders);
        }
    }
    for (SceneObject * const object : get(sVolumeShadersKey)) {
        if (object) {
            addRootShaderToSet(object->asA<RootShader>(),rootShaders);
        }
    }
    for (SceneObject * const object : get(sDisplacementsKey)) {
        if (object) {
            addRootShaderToSet(object->asA<RootShader>(),rootShaders);
        }
    }
}

void
Layer::getAllMaterials(MaterialSet& materials)
{
    for (SceneObject * const object : get(sSurfaceShadersKey)) {
        if (object) {
            materials.insert(object->asA<Material>());
        }
    }
}

void
Layer::getAllLightSets(LightSetSet& lightSets) const
{
    for (SceneObject * const object : get(sLightSetsKey)) {
        if (object) {
            lightSets.insert(object->asA<LightSet>());
        }
    }
}

void
Layer::getAllGeometries(GeometrySet& geometries) const
{
    for (SceneObject * const object : get(sGeometriesKey)) {
        if (object) {
            geometries.insert(object->asA<Geometry>());
        }
    }
}

void
Layer::getAllGeometryToRootShaders(GeometryToRootShadersMap & g2s)
{
    const auto& geometries = get(sGeometriesKey);
    const auto& surfaceShaders = get(sSurfaceShadersKey);
    const auto& displacements = get(sDisplacementsKey);
    const auto& volumeShaders = get(sVolumeShadersKey);
    for (size_t i = 0; i < geometries.size(); ++i) {
        Geometry * const geometry = geometries[i]->asA<Geometry>();
        if (surfaceShaders[i]) {
            RootShader * const rootShader = surfaceShaders[i]->asA<RootShader>();
            g2s[geometry].insert(rootShader);
        }
        if (displacements[i]) {
            RootShader * const rootShader = displacements[i]->asA<RootShader>();
            g2s[geometry].insert(rootShader);
        }
        if (volumeShaders[i]) {
            RootShader * const rootShader = volumeShaders[i]->asA<RootShader>();
            g2s[geometry].insert(rootShader);
        }
        if (g2s.find(geometry) == g2s.end()) {
            g2s.emplace(geometry, RootShaderSet());
        }
    }
}

void
Layer::getChangedGeometryToRootShaders(GeometryToRootShadersMap & g2s)
{
    // If nothing is modified, do nothing.
    // This check includes geometries with attribute, binding, and/or deform changes.
    if (mChangedOrDeformedGeometries.empty()) return;
    
    const auto& surfaceShaders = get(sSurfaceShadersKey);
    const auto& displacements = get(sDisplacementsKey);
    const auto& volumeShaders = get(sVolumeShadersKey);
    for (auto iter = mChangedOrDeformedGeometries.begin();
         iter != mChangedOrDeformedGeometries.end(); ++iter) {
        // check for geometry which has attribute or binding updated
        // or a request to update by the shader
        // ignore those with only geometry data deformed.
        Geometry* geom = iter->first;
        int index = iter->second;
        // Even if a geometry is added to mChangedOrDeformedGeometries, it still might
        // not need to be updated. The three reasons why a geometry needs to be updated is if
        // 1) An attribute that requires a geometry update changes. Note that if an attribute
        //    changes that does not require a geometry update, special care is taken to
        //    set the mAttributeTreeChanged flag to false.
        // 2) An attribute binding changes
        // 3) A shader requests that the geometry is updated.
        // 3 is a special case. If any change is made to a geometry's assigned material,
        // that geometry is added to mChangedOrDeformedGeometries. After this happens,
        // we check if the material requests the geometry update. See SceneContext::applyUpdates
        // and Layer::updatePrepAssignment for more details.
        if (geom->updateRequired()) {
            if (surfaceShaders[index]) {
                RootShader * const rootShader = surfaceShaders[index]->asA<RootShader>();
                g2s[geom].insert(rootShader);
            }
            if (displacements[index]) {
                RootShader * const rootShader = displacements[index]->asA<RootShader>();
                g2s[geom].insert(rootShader);
            }
            if (volumeShaders[index]) {
                RootShader * const rootShader = volumeShaders[index]->asA<RootShader>();
                g2s[geom].insert(rootShader);
            }
            if (g2s.find(geom) == g2s.end()) {
                g2s.emplace(geom, RootShaderSet());
            }
        }
    }
}

void
Layer::clear()
{
    if (!mUpdateActive) {
        throw except::RuntimeError(util::buildString("Layer '", mName,
            "' can only be cleared between beginUpdate() and endUpdate() calls."));
    }

    clearShaderGraphPrimAttributeCache();

    // Get mutable references to the attribute vectors.
    auto& geometries = getMutable(sGeometriesKey);
    auto& parts = getMutable(sPartsKey);
    auto& surfaceShaders = getMutable(sSurfaceShadersKey);
    auto& lightSets = getMutable(sLightSetsKey);
    auto& displacements = getMutable(sDisplacementsKey);
    auto& volumeShaders = getMutable(sVolumeShadersKey);
    auto& lightFilterSets = getMutable(sLightFilterSetsKey);
    auto& shadowSets = getMutable(sShadowSetsKey);
    auto& shadowReceiverSets = getMutable(sShadowReceiverSetsKey);

    geometries.clear();
    parts.clear();
    surfaceShaders.clear();
    lightSets.clear();
    displacements.clear();
    volumeShaders.clear();
    lightFilterSets.clear();
    shadowSets.clear();
    shadowReceiverSets.clear();

    // Manually turn on the set flags, the update flags, and dirty flag since we
    // didn't go through the set() method.
    mAttributeUpdateMask.set(sGeometriesKey.mIndex, true);
    mAttributeUpdateMask.set(sPartsKey.mIndex, true);
    mAttributeUpdateMask.set(sSurfaceShadersKey.mIndex, true);
    mAttributeUpdateMask.set(sLightSetsKey.mIndex, true);
    mAttributeUpdateMask.set(sDisplacementsKey.mIndex, true);
    mAttributeUpdateMask.set(sVolumeShadersKey.mIndex, true);
    mAttributeUpdateMask.set(sLightFilterSetsKey.mIndex, true);
    mAttributeUpdateMask.set(sShadowSetsKey.mIndex, true);
    mAttributeUpdateMask.set(sShadowReceiverSetsKey.mIndex, true);
    mAttributeSetMask.set(sGeometriesKey.mIndex, true);
    mAttributeSetMask.set(sPartsKey.mIndex, true);
    mAttributeSetMask.set(sSurfaceShadersKey.mIndex, true);
    mAttributeSetMask.set(sLightSetsKey.mIndex, true);
    mAttributeSetMask.set(sDisplacementsKey.mIndex, true);
    mAttributeSetMask.set(sVolumeShadersKey.mIndex, true);
    mAttributeSetMask.set(sLightFilterSetsKey.mIndex, true);
    mAttributeSetMask.set(sShadowSetsKey.mIndex, true);
    mAttributeSetMask.set(sShadowReceiverSetsKey.mIndex, true);
    mDirty = true;
    
    mLightSetsChanged = true;
    mChangedRootShaders.clear();
    resetDeformedGeometries();
}

Layer::DisplacementIterator
Layer::begin(const Displacement* displacement) const
{
    const auto& displacements = get(sDisplacementsKey);
    return DisplacementIterator(IndexIterator(0),
                                IndexIterator(0), IndexIterator(displacements.size()),
                                detail::makeContainerWrapper(displacements),
                                displacement);
}

Layer::DisplacementIterator
Layer::end(const Displacement* displacement) const
{
    const auto& displacements = get(sDisplacementsKey);
    return DisplacementIterator(IndexIterator(displacements.size()),
                                IndexIterator(0), IndexIterator(displacements.size()),
                                detail::makeContainerWrapper(displacements),
                                displacement);
}

Layer::VolumeShaderIterator
Layer::begin(const VolumeShader* volumeShader) const
{
    const auto& volumeShaders = get(sVolumeShadersKey);
    return VolumeShaderIterator(IndexIterator(0),
                                IndexIterator(0), IndexIterator(volumeShaders.size()),
                                detail::makeContainerWrapper(volumeShaders),
                                volumeShader);
}

Layer::VolumeShaderIterator
Layer::end(const VolumeShader* volumeShader) const
{
    const auto& volumeShaders = get(sVolumeShadersKey);
    return VolumeShaderIterator(IndexIterator(volumeShaders.size()),
                                IndexIterator(0), IndexIterator(volumeShaders.size()),
                                detail::makeContainerWrapper(volumeShaders),
                                volumeShader);
}

Layer::RootShaderIterator
Layer::begin(const RootShader* rootShader) const
{
    const auto& surfaceShaders = get(sSurfaceShadersKey);
    return RootShaderIterator(IndexIterator(0),
                            IndexIterator(0), IndexIterator(surfaceShaders.size()),
                            detail::makeContainerWrapper(surfaceShaders),
                            rootShader);
}

Layer::RootShaderIterator
Layer::end(const RootShader* rootShader) const
{
    const auto& surfaceShaders = get(sSurfaceShadersKey);
    return RootShaderIterator(IndexIterator(surfaceShaders.size()),
                            IndexIterator(0), IndexIterator(surfaceShaders.size()),
                            detail::makeContainerWrapper(surfaceShaders),
                            rootShader);
}

Layer::LightSetIterator
Layer::begin(const LightSet* lightSet) const
{
    const auto& lightSets = get(sLightSetsKey);
    return LightSetIterator(IndexIterator(0),
                            IndexIterator(0), IndexIterator(lightSets.size()),
                            detail::makeContainerWrapper(lightSets),
                            lightSet);
}

Layer::LightSetIterator
Layer::end(const LightSet* lightSet) const
{
    const auto& lightSets = get(sLightSetsKey);
    return LightSetIterator(IndexIterator(lightSets.size()),
                            IndexIterator(0), IndexIterator(lightSets.size()),
                            detail::makeContainerWrapper(lightSets),
                            lightSet);
}

} // namespace rdl2
} // namespace scene_rdl2

