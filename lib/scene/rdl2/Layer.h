// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "TraceSet.h"

#include "Displacement.h"
#include "Types.h"
#include "UpdateHelper.h"
#include "VolumeShader.h"

#include <scene_rdl2/common/except/exceptions.h>

#include <cstdint>
#include <string>
#include <utility>
#include <unordered_map>
#include <unordered_set>

namespace scene_rdl2 {
namespace rdl2 {

struct LayerAssignment {
    LayerAssignment() : mMaterial(nullptr),
                        mLightSet(nullptr),
                        mDisplacement(nullptr),
                        mVolumeShader(nullptr),
                        mLightFilterSet(nullptr),
                        mShadowSet(nullptr),
                        mShadowReceiverSet(nullptr)
    {}

    Material* mMaterial;
    LightSet* mLightSet;
    Displacement* mDisplacement;
    VolumeShader* mVolumeShader;
    LightFilterSet* mLightFilterSet;
    ShadowSet* mShadowSet;
    ShadowReceiverSet* mShadowReceiverSet;
};

/**
 * The Layer is a subclass of the TraceSet. It stores material and light
 * assignments to parts on a Geometry. Each assignment is made up of the
 * following tuple:
 *      (Geometry*, String, Material*, LightSet*, Displacement*, VolumeShader*)
 * 
 * The Geometry and part name String uniquely identify a particular assignment,
 * while the Material and LightSet are the values of the assignment.
 *
 * When the assign() method is called, it returns a 32-bit unsigned integer.
 * This is the assignment ID. It is unique for a particular Geometry/Part pair.
 * It can be used to quickly and efficiently look up the assigned Material
 * and LightSet.
 *
 * If you only have the Geometry and part name String, you can still look up
 * assignments, but it will be much slower than using an assignment ID. If you
 * lose the assignment ID, you can get it again by using the assignmentId()
 * method. Make sure to save it this time, though, because finding the
 * assignment ID is what's slow, so find the ID and looking up the assignment
 * is just as a slow as directly looking up the assignment by Geometry/Part.
 *
 * Calling the assign() method again with an existing Geometry/Part pair will
 * result in a reassignment of the Material and LightSet to that assignment.
 * It will return the same assignment ID as the previous assignment that was
 * there before.
 */
class Layer : public TraceSet
{
public:
    typedef TraceSet Parent;
    typedef std::pair<const Material*, const LightSet*> MaterialLightSetPair;
    typedef std::unordered_set<RootShader *> RootShaderSet;
    typedef std::unordered_map<Geometry *, int> GeometryIndexMap;
    typedef std::unordered_set<const Geometry *> GeometrySet;
    typedef std::unordered_set<Material *> MaterialSet;
    typedef std::unordered_set<Displacement *> DisplacementSet;
    typedef std::unordered_map<Geometry *, RootShaderSet> GeometryToRootShadersMap;
    typedef std::unordered_set<VolumeShader *> VolumeShaderSet;
    typedef std::unordered_set<const LightSet *> LightSetSet;

    typedef
    FilterIndexIterator<detail::ContainerWrapper<SceneObjectVector>,
        IndexIterator>
        DisplacementIterator;

    typedef
    FilterIndexIterator<detail::ContainerWrapper<SceneObjectVector>,
        IndexIterator>
        VolumeShaderIterator;

    typedef
    FilterIndexIterator<detail::ContainerWrapper<SceneObjectVector>,
        IndexIterator>
        RootShaderIterator;

    typedef
    FilterIndexIterator<detail::ContainerWrapper<SceneObjectVector>,
        IndexIterator>
        LightSetIterator;

    Layer(const SceneClass& sceneClass, const std::string& name);
    static SceneObjectInterface declare(SceneClass& sceneClass);

    /**
     * Makes a new assignment in the layer, or reassigns the Material and
     * LightSet of a previous assignment. The Geometry and part name form a
     * unique key, to which a single Material and LightSet is assigned.
     * This method assigns a NULL displacement and volume shader to the part,
     * and should be deprecated once all code has been updated to use
     * part-based displacement.
     *
     * @param   geometry    The Geometry on which the part lives.
     * @param   partName    The name of the part with the assignment.
     * @param   material    The Material to assign to the part.
     * @param   lightSet    The set of lights to assign to the part.
     * @return  The assignment ID that can be used for fast lookups.
     */
    int32_t assign(Geometry* geometry, const String& partName,
                   Material* material, LightSet* lightSet);

    /**
     * Makes a new assignment in the layer, or reassigns the Material,
     * LightSet, Displacement and VolumeShader of a previous assignment. The
     * Geometry and part name form a unique key, to which a single Material,
     * LightSet, Displacement and VolumeShader is assigned.
     *
     * @param   geometry      The Geometry on which the part lives.
     * @param   partName      The name of the part with the assignment.
     * @param   material      The Material to assign to the part.
     * @param   lightSet      The set of lights to assign to the part.
     * @param   displacement  The Displacement to assign to the part.
     * @param   volumeShader  The VolumeShader to assign to the part.
     * @return  The assignment ID that can be used for fast lookups.
     */
    int32_t assign(Geometry* geometry, const String& partName,
                   Material* material, LightSet* lightSet,
                   Displacement* displacement, VolumeShader* volumeShader);

    /**
     * Makes a new assignment in the layer, or reassigns the LayerAssignment
     * of a previous assignment. The Geometry and part name form a unique key.
     * The LayerAssignment struct is extensible, and contains all SceneObjects
     * that can be validly assigned to a Layer.
     *
     * @param   geometry        The Geometry on which the part lives.
     * @param   partName        The name of the part with the assignment.
     * @param   layerAssignment The collection of SceneObjects to assign to the part.
     * @return  The assignment ID that can be used for fast lookups.
     */
    int32_t assign(Geometry* geometry, const String& partName, const LayerAssignment& layerAssignment);

    /**
     * Given a valid assignment ID, this will return a std::pair containing the
     * Material and LightSet assignments which are set in the Layer. If the
     * assignmentId is invalid, except::IndexError is thrown.
     *
     * @param   assignmentId    The assignment ID to look up assignments for.
     * @return  A std::pair of the Material* and LightSet* assignments.
     * @throw   except::IndexError  If the assignmentId is invalid.
     */
    MaterialLightSetPair lookup(int32_t assignmentId) const;

    /**
     * Given a valid assignment ID, this will return the Material which is
     * set in the Layer. If the assignmentId is invalid, except::IndexError
     * is thrown.
     *
     * @param   assignmentId    The assignment ID to look up assignments for.
     * @return  The Material* assignment.
     * @throw   except::IndexError  If the assignmentId is invalid.
     */
    const Material* lookupMaterial(int32_t assignmentId) const;

    /**
     * Given a valid assignment ID, this will return the LightSet which is
     * set in the Layer.
     *
     * @param   assignmentId    The assignment ID to look up assignments for.
     * @return  The LightSet* assignment.
     */
    const LightSet* lookupLightSet(int32_t assignmentId) const;

    /**
     * Given a valid assignment ID, this will return the Displacement which is
     * set in the Layer. If the assignmentId is invalid, except::IndexError
     * is thrown.
     *
     * @param   assignmentId    The assignment ID to look up assignments for.
     * @return  The Displacement* assignment.
     * @throw   except::IndexError  If the assignmentId is invalid.
     */
    const Displacement* lookupDisplacement(int32_t assignmentId) const;

    /**
     * Given a valid assignment ID, this will return the VolumeShader which is
     * set in the Layer. If the assignmentId is invalid, except::IndexError
     * is thrown.
     *
     * @param   assignmentId    The assignment ID to look up assignments for.
     * @return  The VolumeShader* assignment.
     * @throw   except::IndexError  If the assignmentId is invalid.
     */
    const VolumeShader* lookupVolumeShader(int32_t assignmentId) const;

    /**
     * Given a valid assignment ID, this will return the LightFilterSet which is
     * set in the Layer.
     *
     * @param   assignmentId    The ID of the requested layer assignment.
     * @return  The LightFilterSet* assignment.
     */
    const LightFilterSet* lookupLightFilterSet(int32_t assignmentId) const;

    /**
     * Given a valid assignment ID, this will return the ShadowSet which is
     * set in the Layer.
     *
     * @param   assignmentId    The ID of the requested layer assignment.
     * @return  The ShadowSet* assignment.
     */
    const ShadowSet* lookupShadowSet(int32_t assignmentId) const;

    /**
     * Given a valid assignment ID, this will return the 
     * ShadowReceiverSet which is set in the Layer. 
     *
     * @param   assignmentId    The ID of the requested layer assignment.
     * @return  The ShadowReceiverSet* assignment.
     */
    const ShadowReceiverSet* lookupShadowReceiverSet(int32_t assignmentId) const;

    /**
     * Given a Geometry and a part name on that Geometry, this will return a
     * std::pair containing the Material and LightSet assignments which are
     * set in the Layer. If there is no assignment with the given Geometry/Part
     * pair, except::KeyError is thrown.
     *
     * For efficiency, prefer using the lookup function that takes an assignment
     * id.
     *
     * @param   geometry    The Geometry on which the part lives.
     * @param   partName    The name of the part with the assignment.
     * @return  A std::pair of the Material* and LightSet* assignments.
     * @throw   except::IndexError  If the Geometry/Part pair has no assignment.
     */
    MaterialLightSetPair lookup(const Geometry* geometry, const String& partName) const;

    /**
     * Call updatePrep on all assigned SceneObjects. Call updatePrepFast on 
     * the Layer itself
     * Should only be called after all UpdateGuards.
     */
    void updatePrepAssignments(UpdateHelper& sceneObjects, int depth, Camera* camera);

    /**
     * Clears the cached primitive attributes on all the surface shaders in the
     * layer. Possible TODO: once there are caches for other root shaders, clear
     * those as well.
     */
    void clearShaderGraphPrimAttributeCache() const;

    /**
     * Resets the update masks on the layer.
     * Should be called after all applyUpdates() and before the next set
     * of SceneObject::UpdateGuards. Otherwise, the next applyUpdates() won't
     * actually apply updates.
     */
    void resetAssignmentUpdates();

    /**
     * Returns the set of RootShaders where anything in the binding tree of the
     * RootShader has be changed. These RootShaders need new primitive-attribute
     * tables. Only call after updatePrepAssignments().
     *
     * @return  Set of RootShaders needing new primitive-attribute tables
     */
    const RootShaderSet& getChangedRootShaders() { return mChangedRootShaders; }

    /**
     * Adds all the RootShaders referenced by the Layer, either directly or
     * indirectly, to the provided set of RootShaders. Takes an argument to
     * avoid unnecessary copies. Typically used during initialization, when all
     * RootShaders have been updated.
     *
     * @param   rootShaders   Set of RootShaders to be added to
     */
    void getAllRootShaders(RootShaderSet& rootShaders);

    /**
     * Adds all Materials in the Layer. Typically used during
     * initialization, when all Materials have been updated.
     *
     * @param   materials   Set of all Materials to be added
     */
    void getAllMaterials(MaterialSet& materials);

    /**
     * Adds all LightSets in the Layer.
     *
     * @param   lightSets   Set of all light sets to be added
     */
    void getAllLightSets(LightSetSet& lightsets) const;

    /**
     * Adds all Geometries in the Layer.
     *
     * @param   geometries   Set of all geometries to be added
     */
    void getAllGeometries(GeometrySet& geometries) const;

    /**
     * Indicates whether any LightSets in the Layer have changed or whether any
     * lights in a light set have changed. Only call after updatePrepAssignments().
     *
     * @return  True if any LightSets or their Lights were updated
     */
    bool lightSetsChanged() const { return mLightSetsChanged; }
    
    /**
     * Indicates whether any LightFilterSets in the Layer have changed or whether any
     * lightfilters in a lightfilter set have changed. Only call after updatePrepAssignments().
     *
     * @return  True if any LightFilterSets or their LightFilters were updated
     */
    bool lightFilterSetsChanged() const { return mLightFilterSetsChanged; }

    /**
     * Indicates whether any ShadowSets in the Layer have changed.
     * @return  True if any ShadowSets were updated
     */
    bool shadowSetsChanged() const { return mShadowSetsChanged; }

    /**
     * Indicates whether any ShadowReceiverSets in the Layer have 
     * changed. 
     * @return  True if any ShadowReceiverSets were updated
     */
    bool shadowReceiverSetsChanged() const { return mShadowReceiverSetsChanged; }

    /**
     * Extends the provided map with all the Geometry and RootShader assignments
     * in the Layer. Accelerates the determination of which RootShaders are
     * assigned to a given Geometry. Only call after all relevant UpdateGuards.
     * Takes an argument to avoid copies and minimize scope of extra memory
     * usage.
     *
     * @param   g2s     Map of Geometry to a set of assigned RootShaders
     */
    void getAllGeometryToRootShaders(GeometryToRootShadersMap& g2s);

    /**
     * Returns the set of Geometries that has changed or deformed.
     * These Geometries may need re-tessellation and a BVH rebuild.
     * Only call after applyAssignmentUpdates() calls.
     *
     * @return  Set of Geometries needing regenerate and re-tessellation
     */
    const GeometryIndexMap& getChangedOrDeformedGeometries() const
    { return mChangedOrDeformedGeometries; }

    /**
     * Extends the provided map with the Geometry and RootShader assignments
     * in the Layer where each Geometry needs to have its procedural 
     * regenerated. They are the geometries which has attribute or binding
     * updated, but excludes those with just geometry deformations (does not
     * require procedural regenerated). Accelerates the determination of which
     * RootShaders are assigned to a given Geometry. Only call after applyAssignmentUpdates().
     * Takes an argument to avoid copies and minimize scope of extra memory
     * usage.
     *
     * @param   g2s     Map of Geometry to a set of assigned RootShaders
     */
    void getChangedGeometryToRootShaders(GeometryToRootShadersMap& g2s);

    /// Completely empties the Layer so that it doesn't contain anything.
    void clear();

    /// The iterators returned by these functions are a little different from
    /// standard iterators: when dereferenced, they don't return an object, they
    /// return an index. This index can then be used in the Layer to lookup
    /// whatever information is needed.
    ///
    /// Calls to begin() and end() must reference the same object. The object
    /// passed into these calls is then used to iterate over entries in the
    /// layer that match the passed in object.
    using TraceSet::begin;
    using TraceSet::end;
    DisplacementIterator begin(const Displacement* displacement) const;
    DisplacementIterator end(const Displacement* displacement) const;
    VolumeShaderIterator begin(const VolumeShader* volumeShader) const;
    VolumeShaderIterator end(const VolumeShader* volumeShader) const;
    RootShaderIterator begin(const RootShader* rootShader) const;
    RootShaderIterator end(const RootShader* rootShader) const;
    LightSetIterator begin(const LightSet* lightSet) const;
    LightSetIterator end(const LightSet* lightSet) const;

private:

    void dirtyAssignments();

    /// Clears the updated or deformed geometry map and resets the deformed
    /// status of the geometry.
    void resetDeformedGeometries();

    /**
     * This is called internally when needed. You should not have to call this
     * manually on a specific object (see SceneContext::applyUpdates())
     *
     * This is a non-recursive version of updatePrep(). This is needed to handle the
     * case for Layer and GeometrySet, for which we have already looped on
     * their dependencies (see SceneContext::applyUpdates() and
     * Layer::updatePrepAssignments() for details), but the Layer and GeometrySet
     * objects themselves still need to be prepared for update.
     *
     * @param   sceneObjets   UpdateHelper passed from SceneContext.cc
     * @return  True if the object itself or any of its dependencies has been
     *          changed since last call of resetUpdate() and this object
     *          needs update
     */
    bool updatePrepFast(bool attributeTreeChanged,
                        bool bindingTreeChanged,
                        UpdateHelper& sceneObjects,
                        int depth);

private:
    static AttributeKey<SceneObjectVector> sDisplacementsKey;
    static AttributeKey<SceneObjectVector> sVolumeShadersKey;
    static AttributeKey<SceneObjectVector> sSurfaceShadersKey;
    static AttributeKey<SceneObjectVector> sLightSetsKey;
    static AttributeKey<SceneObjectVector> sLightFilterSetsKey;
    static AttributeKey<SceneObjectVector> sShadowSetsKey;
    static AttributeKey<SceneObjectVector> sShadowReceiverSetsKey;

    bool mLightSetsChanged;
    bool mLightFilterSetsChanged;
    bool mShadowSetsChanged;
    bool mShadowReceiverSetsChanged;
    RootShaderSet mChangedRootShaders;
    VolumeShaderSet mChangedVolumeShaders;
    /// Stores geometry pointer and its index which has attributes, bindings, 
    /// or geometry data deformed.
    GeometryIndexMap mChangedOrDeformedGeometries;

    /// Classes requiring access for serialization.
    friend class AsciiWriter;
};

template <>
inline const Layer*
SceneObject::asA() const
{
    return isA<Layer>() ? static_cast<const Layer*>(this) : nullptr;
}

template <>
inline Layer*
SceneObject::asA()
{
    return isA<Layer>() ? static_cast<Layer*>(this) : nullptr;
}

} // namespace rdl2
} // namespace scene_rdl2

