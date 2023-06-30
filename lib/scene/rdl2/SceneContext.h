// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "Camera.h"
#include "SceneObject.h"
#include "SceneContext.h"
#include "SceneVariables.h"
#include "Types.h"

#include <scene_rdl2/render/util/Alloc.h>
#include <scene_rdl2/common/platform/Platform.h>
#include <tbb/concurrent_hash_map.h>

#include <string>

namespace scene_rdl2 {
namespace rdl2 {

/**
 * The SceneContext represents all the data for a specific scene in RDL. This
 * includes all the objects in the scene (SceneObjects) as well as their types
 * (SceneClasses). It provides some basic functionality for creating
 * SceneClasses and SceneObjects, iterating through them, finding them by their
 * unique name, and retreving them so you can query or update their attributes.
 *
 * Once the data has been loaded or updated, the rendering libraries should be
 * given a const reference to the SceneContext. RDL makes heavy use of its const
 * correct API to indicate which methods and objects are read-only and thread
 * safe. If you stick to the const API and don't violate its integrity with
 * any const_casts, it should be completely safe to traverse the SceneContext
 * and all its objects from multiple threads concurrently.
 *
 * Don't worry about calling createSceneClass() or createSceneObject() multiple
 * times by accident. They both have "create if it does not exist" semantics,
 * and are effectively no-ops if the class or object exists. In the case of
 * createSceneObject, it will return the existing object.
 *
 * Thread Safety:
 *  - The only points of synchronization in the SceneContext are the SceneClass
 *      and SceneObject hash maps, which are tbb::concurrent_hash_maps. These
 *      control access to the hash map with reader/writer locks, so it should
 *      only be slow in the presence of writers. Even then, it's only slow while
 *      we're inserting the SceneClass or SceneObject into the hash table.
 *      Once the insertion is finished, the lock is released and you can continue
 *      updating the object without holding the lock.
 *  - SceneClasses and SceneObjects do not synchronize access to themselves,
 *      so writing to these objects must only happen in a single thread. They
 *      are completely self contained, though, so you are free to write to
 *      different SceneClasses or SceneObjects in different threads concurrently.
 */
class SceneContext
{
private:
    typedef tbb::concurrent_hash_map<std::string, SceneClass*> SceneClassMap;
    typedef SceneClassMap::value_type SceneClassMapItem;
    typedef tbb::concurrent_hash_map<std::string, SceneObject*> SceneObjectMap;
    typedef SceneObjectMap::value_type SceneObjectMapItem;

public:
    // need access to underlaying container for random access to make
    // code parallel.
    typedef std::vector<Geometry*> GeometryVector;

    typedef std::vector<GeometrySet *> GeometrySetVector;
    typedef SceneClassMap::const_iterator SceneClassConstIterator;
    typedef SceneObjectMap::const_iterator SceneObjectConstIterator;
    typedef GeometryVector::const_iterator GeometryConstIterator;
    typedef GeometrySetVector::const_iterator GeometrySetConstIterator;
    typedef std::function<void(SceneObject *)> SceneObjectCallback;
    typedef std::vector<const RenderOutput *> RenderOutputVector;
    typedef std::vector<Camera*>::const_iterator CameraConstIterator;

    /// Construct a new SceneContext.
    SceneContext();

    /// Destroy a SceneContext and all of its data.
    ~SceneContext();

    // ---------- CONST (READ-ONLY) API ---------------------------------

    /// Retrieves the DSO path this SceneContext is using to locate DSO SceneClasses.
    /// This fetches the value of the SceneVariable "dso path", which is sourced in 
    /// the following order:
    /// 1 - If -dso_path was passed on command line, this overrides anything from below
    /// 2 - If RDL2_DSO_PATH environment variable is set, this overrides anything from below
    /// 3 - If neither of the above are set, defaults to searching for "raas_render" executable
    /// and building path to "rdl2dso" based on location of executable.
    finline const std::string getDsoPath() const;

    std::unordered_map<std::string, size_t> getDsoCounts() const;

    /// Retrieves whether or not the SceneContext is currently in proxy mode.
    finline bool getProxyModeEnabled() const;

    /// Retrieves the SceneVariables object.
    finline const SceneVariables& getSceneVariables() const;

    /// Retrieves a SceneClass by its name.
    const SceneClass* getSceneClass(const std::string& name) const;

    /// Checks for existence of a SceneClass with the given name.
    finline bool sceneClassExists(const std::string& name) const;

    /// Returns a begin iterator to the SceneClasses.
    finline SceneClassConstIterator beginSceneClass() const;

    /// Returns an end iterator to the SceneClasses.
    finline SceneClassConstIterator endSceneClass() const;

    /// Retrieves a SceneObject by its name.
    const SceneObject* getSceneObject(const std::string& name) const;

    /// Checks for existence of a SceneObject with the given name.
    finline bool sceneObjectExists(const std::string& name) const;

    /// Returns a begin iterator to the SceneObjects.
    finline SceneObjectConstIterator beginSceneObject() const;

    /// Returns an end iterator to the SceneObjects.
    finline SceneObjectConstIterator endSceneObject() const;

    finline GeometryConstIterator beginGeometry() const;
    finline GeometryConstIterator endGeometry() const;
    finline GeometrySetConstIterator beginGeometrySet() const;
    finline GeometrySetConstIterator endGeometrySet() const;

    // Returns the primary camera, or nullptr if no cameras in the context.
    const rdl2::Camera* getPrimaryCamera() const;

    // Gets all the cameras in the context.  The primary camera is the
    // first one.
    std::vector<const rdl2::Camera*> getCameras() const;

    // Gets all cameras with render outputs.  The primary camera is the
    // first one.
    std::vector<const rdl2::Camera*> getActiveCameras() const;

    // Gets the global dicing camera, or nullptr
    const rdl2::Camera* getDicingCamera() const;

    /// Return the render to world transform, if set.  nullptr if not.
    finline const Mat4d* getRender2World() const;

    finline bool getCheckpointActive() const;
    finline bool getResumableOutput() const;
    finline bool getResumeRender() const;

    // ---------- NON-CONST (WRITE) API ---------------------------------

    /**
     * Sets the DSO path in SceneVariables that SceneContext will use when looking 
     * for DSOs that define SceneClasses. The string is a colon separated list of 
     * paths, much like the $PATH shell variable.
     *
     * @param   dsoPath     A colon separated list of paths to search for DSOs.
     */
    finline void setDsoPath(const std::string& dsoPath);

    /**
     * Sets whether or not the SceneContext is in proxy mode.
     *
     * When in proxy mode, new SceneClasses will be created such that any
     * objects of that SceneClass will be proxies, not the actual objects. This
     * is useful if you want to use RDL to read and write SceneContexts, but
     * don't want to drag in the dependencies of all the DSOs you're using.
     * DSOs should never have declare() functions that drag in extra
     * dependencies.
     * 
     * Proxy objects can use get() and set() on their attributes as expected,
     * but you CANNOT safely downcast them to their proxied type. Thus, you
     * CANNOT use member function interfaces on proxy objects, including the
     * virtual functions defined by their parent classes. For example, you
     * CANNOT call createProcedural() on a GeometryProxy object. You can get()
     * and set() attributes and that's about it.
     *
     * It's also important to remember that proxy mode only affects *new*
     * SceneClasses that are created. Objects created from that SceneClass will
     * always be created as proxies or not depending on whether the context was
     * in proxy mode when the SceneClass was created. In general, you probably
     * want the context to always be in proxy mode or never be in proxy mode.
     *
     * @param   enabled     True to enable proxy mode, false to disable.
     */
    finline void setProxyModeEnabled(bool enabled);

    /// Retrieves a mutable reference to the SceneVariables object.
    finline SceneVariables& getSceneVariables();

    /// Retrieves a mutable SceneObject by its name.
    SceneObject* getSceneObject(const std::string& name);

    /// Sets the render to world transform
    void setRender2World(const Mat4d *render2World);

    /**
     * Creates a SceneClass of the given name.
     *
     * If the class already exists, nothing happens, and the existing
     * SceneClass will be returned. If the class does not exist, the
     * SceneContext will search the DSO path for a file named the same as the
     * class name with a ".so" extension and attempt to open it as an RDL DSO.
     * If all goes well, the SceneClass will be created and returned.
     *
     * The SceneContext owns the returned pointer, and will free it when the
     * context is destroyed.
     *
     * @param   className   The name of the SceneClass to create and load.
     * @return  The new SceneClass or the existing SceneClass (if it already
     *          existed).
     */
    SceneClass* createSceneClass(const std::string& className);

    /**
     * Create a SceneObject from the given SceneClass name with the given
     * object name.
     * 
     * If the object already exists, nothing happens, and the existing object
     * will be returned. If the object does not exist, it will be created and
     * the new object will be returned.
     *
     * The SceneContext owns the returned pointer, and will free it when the
     * context is destroyed.
     *
     * @param   className   The name of the SceneClass that this object will
     *                      be created from.
     * @param   objectName  The name of the object. Must be unique.
     * @return  The new SceneObject or the existing SceneObject (if the name
     *          already existed).
     */
    SceneObject* createSceneObject(const std::string& className, const std::string& objectName);

    /**
     * Calls update() on any of the following that are modified: SceneVariables,
     * the active Camera, the supplied Layer, and assigned SceneObjects and
     * SceneObject attributes in the Layer. Should only be called after
     * all SceneObject::UpdateGuards.
     *
     * @param   layer           The active Layer
     */
    void applyUpdates(Layer * layer);

    /**
     * Updates the MeshLightLayer after a call to applyUpdates(). Flags the MeshLight shaders that have been
     * updated in applyUpdates so that we can update the corresponding attribute tables. Also flags the related geometry
     * since a change in the attribute table could require a geometry update. Note that we can't call applyUpdates, 
     * resetUpdates(), or this function until the flagged values have been accessed in loadGeometries() and preFrame().
     *
     * @param layer The MeshLightLayer
    */
    void applyUpdatesToMeshLightLayer(Layer * layer);

    /**
     * Resets the update masks on all the SceneObjects in this SceneContext.
     * Should be called after all updatePrep() and update(), and before the 
     * next set of SceneObject::UpdateGuards. Otherwise, the next prepUpdate()
     * won't be tracked correctly.
     *
     * @param   layer   The active Layer
     */
    void resetUpdates(Layer * layer);
    
    /**
     * Used for iterating through all the GeometrySets, such as during
     * initialization.
     *
     * @return  A vector of all the GeometrySets in the scene
     */
    const GeometrySetVector & getAllGeometrySets() { return mGeometrySets; }

    /**
     * Gets all GeometrySets that have geometry in the specified layer.
     *
     * @return  A vector of all the GeometrySets with geometry in the specified
     * layer.
     */
    GeometrySetVector getGeometrySetsForLayer(const Layer* layer);

    /**
     * Add to the provided vector all the GeometrySets that have changed
     * including updates to attribute, bindings, and/or geometry deformation. Used
     * for updating a BVH with just the GeometrySets that have changed. Takes a
     * vector argument to avoid copies and minimize scope of extra memory usage.
     * Only call after Layer::applyAssignedUpdates().
     *
     * @param   layer       Active Layer
     * @param   updatedSets Vector to which to add updated GeometrySets
     */
    void getUpdatedOrDeformedGeometrySets(Layer * layer, GeometrySetVector & updatedSets);

    /**
     * Clears all flags on all attributes of all objects that are tracking
     * what has changed. This effectively puts the SceneContext in its "base"
     * state, where nothing has changed.
     */
    void commitAllChanges();

    /**
     * Searches every directory in the DSO path looking for ".so" files and
     * attempts to load them as RDL DSOs. Files that are not successfully
     * opened as RDL DSOs are ignored. This can be used to fill up the SceneClass
     * map with all the available SceneClasses, and then iterate over them
     * exploring their attributes and attribute metadata.
     */
    void loadAllSceneClasses();

    void setFatalShadeFunc(ShadeFunc f) {mFatalShadeFunc = f;}
    ShadeFunc getFatalShadeFunc() const {return mFatalShadeFunc;}
    void setFatalSampleFunc(SampleFunc f) {mFatalSampleFunc = f;}
    SampleFunc getFatalSampleFunc() const {return mFatalSampleFunc;}
    void setFatalSampleNormalFunc(SampleNormalFunc f) {mFatalSampleNormalFunc = f;}
    SampleNormalFunc getFatalSampleNormalFunc() const {return mFatalSampleNormalFunc;}
    void setFatalPresenceFunc(PresenceFunc f) {mFatalPresenceFunc = f;}
    PresenceFunc getFatalPresenceFunc() const {return mFatalPresenceFunc;}
    void setFatalIorFunc(IorFunc f) {mFatalIorFunc = f;}
    IorFunc getFatalIorFunc() const {return mFatalIorFunc;}
    void setFatalPreventLightCullingFunc(PreventLightCullingFunc f) {mFatalPreventLightCullingFunc = f;}
    PreventLightCullingFunc getFatalPreventLightCullingFunc() const {return mFatalPreventLightCullingFunc;}


    /**
     * These callbacks will be called after each SceneObject in this context is created.
     */
    void addCreateSceneObjectCallback(SceneObjectCallback cb) { mCreateCallbacks.push_back(cb); }

    /**
     * These callbacks will be called before each SceneObject in this context is deleted.
     */
    void addDeleteSceneObjectCallback(SceneObjectCallback cb) { mDeleteCallbacks.push_back(cb); }

    /**
     * Get all render outputs in the scene
     */
    const RenderOutputVector &getAllRenderOutputs() const { return mRenderOutputs; }

private:
    // Non-copyable.
    SceneContext(const SceneContext&);
    const SceneContext& operator=(const SceneContext&);

    // Creates a built in SceneClass that is always provided for free by RDL.
    // The template parameter is the built in type to instantiante, the
    // className is the name by which that SceneClass will be known.
    template <typename T>
    void createBuiltInSceneClass(const std::string& className);

    // Computes the fast time rescaling coefficients for use by interpolated get().
    // No interpolated gets should be happening on other threads while these are updated.
    void computeTimeRescalingCoeffs(float shutterOpen, float shutterClose, const std::vector<float> &motionSteps);

    // Precomputed coefficients for fast time rescaling, which is used by the
    // interpolated get(). For more information, see the declaration of
    // TimeRescalingCoeffs in Types.h.
    TimeRescalingCoeffs mTimeRescalingCoeffs;

    // If we're in proxy mode, new scene classes will be created with a proxy
    // object factory instead of a DSO factory.
    bool mProxyModeEnabled;

    // The map of SceneClass names to SceneClasses. It owns all the SceneClass
    // pointers it contains and is responsible for destroying them.
    SceneClassMap mSceneClasses;

    // The map of SceneObject names to SceneObjects. It owns all the SceneObject
    // pointers it contains and is responsible for destroying them.
    SceneObjectMap mSceneObjects;

    // Quick access to the SceneVariables singleton object. This is just an
    // observational pointer. The owner of the SceneVariables object is the
    // SceneObject map.
    SceneVariables* mSceneVariables;

    GeometryVector mGeometries;
    GeometrySetVector mGeometrySets;

    // We also need a mutex to protect them, since the two SceneObjects could
    // be updated concurrently in different threads. HOWEVER, this does NOT
    // protect against the value of mTimeRescalingCoeffs being *read* while
    // it's being updated. In other words, doing an interpolated get() against
    // ANY object while the camera's shutter interval or SceneVariables motion
    // steps are being updated is a recipe for threading errors.
    std::mutex mTimeRescalingCoeffsMutex;

    // All cameras in the rdl context (including the primary camera).
    // This is in creation order.  The primary camera can't be assumed to be
    // the first one.  Use getPrimaryCamera() if you need the primary camera.
    std::vector<Camera*> mCameras;

    // The render to world transform, which may not be the same as the active
    // camera transform
    const Mat4d* mRender2World;
    

    // Functions to be used for shading and sampling in case of fatal errors at update
    ShadeFunc mFatalShadeFunc;
    SampleFunc mFatalSampleFunc;
    SampleNormalFunc mFatalSampleNormalFunc;
    PresenceFunc mFatalPresenceFunc;
    IorFunc mFatalIorFunc;
    PreventLightCullingFunc mFatalPreventLightCullingFunc;

    // Callbacks
    std::vector<SceneObjectCallback> mCreateCallbacks;
    std::vector<SceneObjectCallback> mDeleteCallbacks;

    // Classes requiring access for fast time rescaling coefficients.
    friend class SceneObject;
    friend class SceneVariables;
    friend class Camera;

    // Mutex to sync write access to thread unsafe vectors like mGeometries only in
    // conditioning time. Those vectors will remain lock free for reading and reading / writing
    // at the same time is not allowed or protected in any way
    mutable std::mutex mCreateSceneObjectMutex;

    RenderOutputVector mRenderOutputs;
    std::string mDsoPath;

    // DAG of scene objects to update. It is a member variable of SceneContext so that we can call
    // updatePrep on multiple scene objects, or on the same scene object multiple times, without
    // the possibility of redundantly updating the same scene object multiple times.
    UpdateHelper mSceneObjectUpdateGraph;
};

const std::string
SceneContext::getDsoPath() const
{
    return mDsoPath;
}

void
SceneContext::setDsoPath(const std::string& dsoPath)
{
    mDsoPath = dsoPath;
}

bool
SceneContext::getProxyModeEnabled() const
{
    return mProxyModeEnabled;
}

void
SceneContext::setProxyModeEnabled(bool enabled)
{
    mProxyModeEnabled = enabled;
}

const SceneVariables&
SceneContext::getSceneVariables() const
{
    return *mSceneVariables;
}

SceneVariables&
SceneContext::getSceneVariables()
{
    return *mSceneVariables;
}

bool
SceneContext::sceneClassExists(const std::string& name) const
{
    SceneClassMap::const_accessor reader;
    return mSceneClasses.find(reader, name);
}

SceneContext::SceneClassConstIterator
SceneContext::beginSceneClass() const
{
    return mSceneClasses.begin();
}

SceneContext::SceneClassConstIterator
SceneContext::endSceneClass() const
{
    return mSceneClasses.end();
}

bool
SceneContext::sceneObjectExists(const std::string& name) const
{
    SceneObjectMap::const_accessor reader;
    return mSceneObjects.find(reader, name);
}

SceneContext::SceneObjectConstIterator
SceneContext::beginSceneObject() const
{
    return mSceneObjects.begin();
}

SceneContext::SceneObjectConstIterator
SceneContext::endSceneObject() const
{
    return mSceneObjects.end();
}

SceneContext::GeometryConstIterator
SceneContext::beginGeometry() const
{
    return mGeometries.begin();
}

SceneContext::GeometryConstIterator
SceneContext::endGeometry() const
{
    return mGeometries.end();
}

SceneContext::GeometrySetConstIterator
SceneContext::beginGeometrySet() const
{
    return mGeometrySets.begin();
}

SceneContext::GeometrySetConstIterator
SceneContext::endGeometrySet() const
{
    return mGeometrySets.end();
}

const Mat4d*
SceneContext::getRender2World() const
{
    return mRender2World;
}

bool
SceneContext::getCheckpointActive() const
{
    return getSceneVariables().get(rdl2::SceneVariables::sCheckpointActive);
}

bool
SceneContext::getResumableOutput() const
{
    return getSceneVariables().get(rdl2::SceneVariables::sResumableOutput);
}

bool
SceneContext::getResumeRender() const
{
    return getSceneVariables().get(rdl2::SceneVariables::sResumeRender);
}

} // namespace rdl2
} // namespace scene_rdl2

