// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "SceneContext.h"

#include "Camera.h"
#include "Dso.h"
#include "DsoFinder.h"
#include "Geometry.h"
#include "GeometrySet.h"
#include "UpdateHelper.h"
#include "Layer.h"
#include "LightFilterSet.h"
#include "LightSet.h"
#include "Map.h"
#include "NormalMap.h"
#include "Material.h"
#include "Metadata.h"
#include "ObjectFactory.h"
#include "RenderOutput.h"
#include "SceneClass.h"
#include "SceneObject.h"
#include "SceneVariables.h"
#include "TraceSet.h"
#include "UserData.h"

#include <scene_rdl2/common/platform/Platform.h>
#include <scene_rdl2/common/except/exceptions.h>
#include <scene_rdl2/render/util/Strings.h>
#include <scene_rdl2/render/logging/logging.h>

#include <tbb/concurrent_hash_map.h>
#include <tbb/mutex.h>
#include <tbb/parallel_for_each.h>

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <dirent.h>
#include <errno.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace scene_rdl2 {

using logging::Logger;
namespace rdl2 {

namespace {

void
verifyMatchingSceneClass(const std::string& className, const SceneObject* obj)
{
    MNRY_ASSERT(obj);
    const std::string& existingClassName = obj->getSceneClass().getName();
    if (className != existingClassName) {
        throw except::TypeError(util::buildString("Cannot create new"
                " SceneObject of SceneClass '", className, "' because '",
                obj->getName(), "' of SceneClass '", existingClassName,
                "' already exists."));
    }
}

} // namespace

template <typename T>
void
SceneContext::createBuiltInSceneClass(const std::string& className)
{
    SceneClassMap::accessor writer;
    if (mSceneClasses.insert(writer, className)) {
        SceneClass* sc = new SceneClass(this, className,
                ObjectFactory::createBuiltInFactory<T>());
        sc->declare();
        sc->setComplete();
        writer->second = sc;
    }
}

SceneContext::SceneContext() :
    mProxyModeEnabled(false),
    mSceneVariables(nullptr),
    mRender2World(nullptr),
    mDsoPath(DsoFinder::find())
{
    // Create SceneClasses for builtin types. If you add any new built in
    // types, you must add an explicit instantiation of this function template
    // in ObjectFactory.cc, otherwise it will fail to link.

    createBuiltInSceneClass<GeometrySet>("GeometrySet");
    createBuiltInSceneClass<Joint>("Joint");
    createBuiltInSceneClass<TraceSet>("TraceSet");
    createBuiltInSceneClass<Layer>("Layer");
    createBuiltInSceneClass<LightFilterSet>("LightFilterSet");
    createBuiltInSceneClass<LightSet>("LightSet");
    createBuiltInSceneClass<RenderOutput>("RenderOutput");
    createBuiltInSceneClass<SceneVariables>("SceneVariables");
    createBuiltInSceneClass<ShadowSet>("ShadowSet");
    createBuiltInSceneClass<ShadowReceiverSet>("ShadowReceiverSet");
    createBuiltInSceneClass<UserData>("UserData");
    createBuiltInSceneClass<Metadata>("Metadata");

    // Create the singleton __SceneVariables__ object.
    mSceneVariables = static_cast<SceneVariables*>(
            createSceneObject("SceneVariables", "__SceneVariables__"));

    // Initialize the fast time rescaling coefficients
    computeTimeRescalingCoeffs(0.0f, 0.0f, mSceneVariables->get(SceneVariables::sMotionSteps));
}

SceneContext::~SceneContext()
{
    // Delete all scene objects.
    for (SceneObjectMap::iterator objIter = mSceneObjects.begin();
            objIter != mSceneObjects.end(); ++objIter) {
        // Call any on-delete callbacks first
        for (auto cb : mDeleteCallbacks) {
            cb(objIter->second);
        }
        
        Geometry *pGeom = dynamic_cast<Geometry *>(objIter->second);
        if (pGeom)
            pGeom->unloadProcedural();
            
        delete objIter->second;
        objIter->second = nullptr;
    }

    // Delete all scene classes.
    for (SceneClassMap::iterator classIter = mSceneClasses.begin();
            classIter != mSceneClasses.end(); ++classIter) {
        delete classIter->second;
        classIter->second = nullptr;
    }
}

const SceneClass*
SceneContext::getSceneClass(const std::string& name) const
{
    SceneClassMap::const_accessor reader;
    if (!mSceneClasses.find(reader, name)) {
        std::stringstream errMsg;
        errMsg << "No SceneClass named '" << name << "' in the SceneContext.";
        throw except::KeyError(errMsg.str());
    }

    return reader->second;
}

const SceneObject*
SceneContext::getSceneObject(const std::string& name) const
{
    SceneObjectMap::const_accessor reader;
    if (!mSceneObjects.find(reader, name)) {
        std::stringstream errMsg;
        errMsg << "No SceneObject named '" << name << "' in the SceneContext.";
        throw except::KeyError(errMsg.str());
    }

    return reader->second;
}

SceneObject*
SceneContext::getSceneObject(const std::string& name)
{
    SceneObjectMap::const_accessor reader;
    if (!mSceneObjects.find(reader, name)) {
        std::stringstream errMsg;
        errMsg << "No SceneObject named '" << name << "' in the SceneContext.";
        throw except::KeyError(errMsg.str());
    }

    return reader->second;
}

const rdl2::Camera*
SceneContext::getPrimaryCamera() const
{
    // Prevent possible race condition with mCameras.push_back() in createSceneObject().
    tbb::mutex::scoped_lock lock(mCreateSceneObjectMutex);

    if (mCameras.size() == 0) {
        return nullptr;
    }

    // The primary camera is the first element in the output cameras vector
    if (mSceneVariables->get(SceneVariables::sCamera)) {
        // Use the camera specified by the scene variable
        return mSceneVariables->get(SceneVariables::sCamera)->asA<Camera>();
    } else {
        // Else just the first camera created in the context
        return mCameras[0];
    }
}

std::vector<const rdl2::Camera*>
SceneContext::getCameras() const
{
    // Prevent possible race condition with mCameras.push_back() in createSceneObject().
    tbb::mutex::scoped_lock lock(mCreateSceneObjectMutex);

    std::vector<const rdl2::Camera*> cameras;

    if (mCameras.size() == 0) {
        return cameras;
    }

    // The primary camera is the first element in the output cameras vector
    if (mSceneVariables->get(SceneVariables::sCamera)) {
        // Use the camera specified by the scene variable
        cameras.push_back(mSceneVariables->get(SceneVariables::sCamera)->asA<Camera>());
    } else {
        // Else just the first camera created in the context
        cameras.push_back(mCameras[0]);
    }

    // Populate the rest of the cameras, skipping the primary
    for (size_t i = 0; i < mCameras.size(); i++) {
        if (cameras[0] != mCameras[i]) {
            cameras.push_back(mCameras[i]);
        }
    }

    return cameras;
}

std::vector<const rdl2::Camera*>
SceneContext::getActiveCameras(void) const
{
    // Prevent possible race condition with mCameras.push_back() in createSceneObject().
    tbb::mutex::scoped_lock lock(mCreateSceneObjectMutex);

    std::vector<const rdl2::Camera*> cameras;

    if (mCameras.size() == 0) {
        return cameras;
    }

    // The primary camera is the first element in the output cameras vector
    if (mSceneVariables->get(SceneVariables::sCamera)) {
        // Use the camera specified by the scene variable
        cameras.push_back(mSceneVariables->get(SceneVariables::sCamera)->asA<Camera>());
    } else {
        // Else just the first camera created in the context
        cameras.push_back(mCameras[0]);
    }

    // Populate the rest of the cameras, skipping the primary
    const rdl2::SceneContext::RenderOutputVector &outputs = getAllRenderOutputs();
    for (size_t i = 0; i < mCameras.size(); i++) {
        if (cameras[0] != mCameras[i]) {
            // Check if this camera is referenced by a render output, skip those
            // that aren't.
            for (size_t j = 0; j < outputs.size(); j++) {
                if (outputs[j]->getCamera() == mCameras[i]) {
                    cameras.push_back(mCameras[i]);
                    break;
                }
            }
        }
    }

    return cameras;
}

const Camera*
SceneContext::getDicingCamera() const {
    SceneObject* so = mSceneVariables->get(SceneVariables::sDicingCamera);
    if (so) {
        Camera* dicingCamera = so->asA<Camera>();
        return dicingCamera;
    }
    return nullptr;
}

SceneClass*
SceneContext::createSceneClass(const std::string& className)
{
    if (className.empty()) {
        throw except::ValueError("Cannot create a SceneClass with an empty class name.");
    }

    // First, do a quick check for existence. If the class already exists,
    // multiple readers can do this simultaneously.

    { // Begin reader lock scope.
        SceneClassMap::const_accessor reader;
        
        if (mSceneClasses.find(reader, className)) {
            // Found it.
            return reader->second;
        }
    } // End reader lock scope.

    // WARNING: THIS CODE IS CRITICAL TO THREAD SAFETY!
    //
    // It's tempting to think we don't need to check for existence again after
    // this point, but remember that a read lock is not exclusive. That means
    // if a class does not exist, multiple threads could make it here and try
    // to create the same class at the same time.
    //
    // Acquiring a write lock here will give us exclusive access, which we can
    // then use to check for existence again. We then have both exclusive
    // access to the container AND we know whether it exists. If it does exist
    // in our exclusive test, that means another thread beat us to the
    // exclusive writer lock, created the SceneClass, and we should just return
    // what we found.
    //
    // It's also important to remember that this exclusive lock is how we are
    // able to guarantee thread safety to DSO declare() function. That is only
    // guaranteed because we acquire this exclusive lock before calling it.

    // Writer lock in scope until the end of the function.
    SceneClassMap::accessor writer;

    // If insert returns true, the item is new.
    if (mSceneClasses.insert(writer, className)) {
        // This tbb writer lock is per bucket and not per container, so anything
        // outside the mSceneClasses container needs to be thread safe by its own
        // means not depending on the writer lock

        std::unique_ptr<SceneClass> sc;

        // It really does not exist yet. Let's try to create it.
        try {
            std::string dsoPath = getDsoPath();
            if (mProxyModeEnabled) {
                sc.reset(new SceneClass(this, className,
                    ObjectFactory::createProxyFactory(className, dsoPath)));
            } else {
                sc.reset(new SceneClass(this, className,
                    ObjectFactory::createDsoFactory(className, dsoPath)));
            }
            sc->declare();
            sc->setComplete();
        } catch (...) {
            // Something went wrong when creating the SceneClass. Roll back
            // our insertion by erasing the key.
            mSceneClasses.erase(writer);

            // Rethrow whatever caused the problem.
            throw;
        }

        // If no exceptions were thrown, we should have a valid SceneClass and
        // it should be safe to go ahead with the insert.
        MNRY_ASSERT(sc, "SceneClass should never be invalid prior to insertion.");
        writer->second = sc.release();
    }

    // Regardless of whether we created the SceneClass just now because it was
    // a new entry in the container, or it wasn't new because another thread
    // beat us to the writer lock, we want to return it.
    return writer->second;
}

SceneObject*
SceneContext::createSceneObject(const std::string& className,
                                const std::string& objectName)
{
    if (className.empty()) {
        throw except::ValueError("Cannot create a SceneObject with an empty class name.");
    } else if (objectName.empty()) {
        throw except::ValueError("Cannot create a SceneObject with an empty object name.");
    }

    // Enforce the singleton-ness of the __SceneVariables__ object.
    if (className == "SceneVariables" && objectName != "__SceneVariables__") {
        return mSceneVariables;
    }

    // Do a quick check for existence. If the class already exists, multiple
    // readers can do this simultaneously.

    { // Begin reader lock scope.
        SceneObjectMap::const_accessor reader;
  
        if (mSceneObjects.find(reader, objectName)) {
            // Found it.
            verifyMatchingSceneClass(className, reader->second);
            return reader->second;
        }
    } // End reader lock scope.

    // WARNING: THIS CODE IS CRITICAL TO THREAD SAFETY!
    //
    // It's tempting to think we don't need to check for existence again after
    // this point, but remember that a read lock is not exclusive. That means
    // if the object does not exist, multiple threads could make it here and
    // try to create the same object at the same time.
    //
    // Acquiring a write lock here will give us exclusive access, which we can
    // then use to check for existence again. We then have both exclusive
    // access to the container AND we know whether it exists. If it does exist
    // in our exclusive test, that means another thread beat us to the
    // exclusive writer lock, created the SceneObject, and we should just return
    // what we found.
    //
    // It's also important to remember that this exclusive lock is how we are
    // able to guarantee thread safety to DSO create() function. That is only
    // guaranteed because we acquire this exclusive lock before calling it.

    // Get (or create, if necessary) the SceneClass first.
    SceneClass* sc = createSceneClass(className);

    // Writer lock in scope until the end of the function.
    SceneObjectMap::accessor writer;

    // If insert returns true, the item is new.
    if (mSceneObjects.insert(writer, objectName)) {
        SceneObject* obj = nullptr;

        // It really does not exist yet. Let's try to create it.
        try {
            // The createObject() function should not leak if it fails.
            obj = sc->createObject(objectName);
        } catch (...) {
            // Something went wrong when creating the SceneObject. Roll back
            // our insertion by erasing the key.
            mSceneObjects.erase(writer);

            // Rethrow whatever caused the problem.
            throw;
        }

        // If no exceptions were thrown, we should have a valid SceneObject and
        // it should be safe to go ahead with the insert.
        MNRY_ASSERT(obj, "SceneObject should never be invalid prior to insertion.");
        writer->second = obj;

        // The containers that are below are not thread safe versions and are not protected
        // by the mSceneObjects write lock since tbb locks per bucket and not per container.
        // mCreateSceneObjectMutex is a SceneContext class mutex that protects non thread safe
        // write access to them and also to the camera. Any other non thread safe code in this
        // method should also be protected by it. Write and read access at the same time is NOT allowed
        // and will not since it has performance implications for the read only calls of the renderer.
        // The api exposes the containers and iterators but they should never be used even for reading
        // if other thread is performing modifications to them by calling this function.

        // Do any type-specific setup.
        if (obj->isA<Geometry>()) {
            tbb::mutex::scoped_lock lock(mCreateSceneObjectMutex);
            mGeometries.push_back(obj->asA<Geometry>());
        } else if (obj->isA<GeometrySet>()) {
            tbb::mutex::scoped_lock lock(mCreateSceneObjectMutex);
            mGeometrySets.push_back(obj->asA<GeometrySet>());
        } else if (obj->isA<Camera>()) {
            tbb::mutex::scoped_lock lock(mCreateSceneObjectMutex);
            mCameras.push_back(obj->asA<Camera>());
        } else if (obj->isA<RenderOutput>()) {
            tbb::mutex::scoped_lock lock(mCreateSceneObjectMutex);
            mRenderOutputs.push_back(obj->asA<RenderOutput>());
        } 

        // Call any on-creation callbacks
        for (auto cb : mCreateCallbacks) {
            cb(obj);
        }
    } else {
        // We didn't win the insertion race, so verify that the SceneClass
        // matches.
        verifyMatchingSceneClass(className, writer->second);
    }

    // Regardless of whether we created the SceneObject just now because it was
    // a new entry in the container, or it wasn't new because another thread
    // beat us to the writer lock, we want to return it.
    return writer->second;
}

void
SceneContext::applyUpdates(Layer * const layer)
{
    // Now that the scene variables and the camera are available, we can update the
    // coefficients in the scene context that hold information about the shutter interval and
    // motion steps.
    // Note that the camera's shutter times are ignored if motion blur is disabled (either in
    // the scene vars or in the camera itself), in order to remain consistent with the way
    // geometry objects behave.

    const Camera* primaryCamera = getPrimaryCamera();

    float shutterOpen  = 0.0f;
    float shutterClose = 0.0f;
    if (primaryCamera && mSceneVariables->get(SceneVariables::sEnableMotionBlur)) {
        shutterOpen  = primaryCamera->get(Camera::sMbShutterOpenKey);
        shutterClose = primaryCamera->get(Camera::sMbShutterCloseKey);
    }
    computeTimeRescalingCoeffs(shutterOpen, shutterClose, mSceneVariables->get(SceneVariables::sMotionSteps));

    // cache primitive attributes contained in the shader network of all materials.
    // This must be done before any updates to SceneObjects.
    if (layer) {
        Layer::MaterialSet materials;
        layer->getAllMaterials(materials);
        for (const Material* m : materials) {
            if (m) {
                m->cacheShaderGraphPrimAttributes();
            }
        }
    }

    // SceneVariables need to be updated first
    mSceneVariables->updatePrep(mSceneObjectUpdateGraph, 0);

    // Cameras needs to be updated second
    for (size_t i = 0; i < mCameras.size(); i++) {
        mCameras[i]->updatePrep(mSceneObjectUpdateGraph, 0);
    }

    // Flag shaders that are in the update graph, as we will need to re-build the associated attribute tables.
    // Also flag the associated geometry, since a change in the attribute table might require a geometry update. Also
    // in this step, we will flag the LightSets, LightFilterSets, ShadowSets, and ShadowReceiverSets that will need 
    // updating in the preFrame() function.
    if (layer) {
        Camera* camera = const_cast<Camera*>(primaryCamera);
        layer->updatePrepAssignments(mSceneObjectUpdateGraph, 0, camera);
    }

    for (GeometrySet * geometrySet : mGeometrySets) {
        // This is an optimization to avoid calling a full blown updatePrep(),
        // which would require an unnecessary full loop over geometries again.
        geometrySet->updatePrepFast(mSceneObjectUpdateGraph, 0);
    }

    // need to update display filters
    for (auto& kv : mSceneObjects) {
        SceneObject* so = kv.second;
        if(so->isA<DisplayFilter>()) {
            so->updatePrep(mSceneObjectUpdateGraph, 0);
        }
    }

    // Update all leaves
    int s = mSceneObjectUpdateGraph.size();
    if (s == 0){
        Logger::info("There is no leaf scene object need to be updated");
    } else if (s == 1) {
        Logger::info("Updating 1 leaf scene object...");
    } else {
        Logger::info("Updating ", s, " leaf scene objects...");
    } 

    tbb::parallel_for_each(mSceneObjectUpdateGraph.cbegin(), mSceneObjectUpdateGraph.cend(),
            [&] (SceneObject* const obj)
    {
        obj->debug("Updating");
        obj->update();
    });

    // Update the objects from bottom up
    for (int i = mSceneObjectUpdateGraph.getMaxDepth()-1; i >= 0; --i) {
        int s = mSceneObjectUpdateGraph.size(i);
        if (s == 0){
            Logger::info("There is no scene object need to be updated at level ", i);
        } else if (s == 1) {
            Logger::info("Updating 1 scene object at level ", i, "...");
        } else {
            Logger::info("Updating ", s, " scene objects at level ", i, "...");
        }

        tbb::parallel_for_each(mSceneObjectUpdateGraph.cbegin(i), mSceneObjectUpdateGraph.cend(i),
                [&] (SceneObject* const obj)
        {
            obj->debug("Updating");
            obj->update();
        });
    }

    // Changes in a shader's requested primitive attributes require updating
    // the geometry.
    // Also, changes in the volumeShader requires updating the geometry because
    // volume shaders are baked into the geometry.
    if (layer) {
        const auto& changedOrDeformedGeometries = layer->getChangedOrDeformedGeometries();
        // If there are no changed or deformed geometries, do nothing.
        if (changedOrDeformedGeometries.empty()) return;

        for (auto& geomPair : changedOrDeformedGeometries) {
            Geometry* geom = geomPair.first;
            int index = geomPair.second;

            // If a shader requests new primitive attributes from the geometry, then
            // we need to update the geometry.
            const Material * material = layer->lookupMaterial(index);
            if (material && material->haveShaderGraphPrimAttributesChanged()) {
                geom->requestUpdate();
            }
            // If a volume shader needs updating, then we need to update the geometry
            const VolumeShader * volumeShader = layer->lookupVolumeShader(index);
            if (volumeShader && volumeShader->updateBakeRequired()) {
                geom->requestUpdate();
            }
        }
    }
}

void
SceneContext::applyUpdatesToMeshLightLayer(Layer * const layer)
{
    // Be sure to call applyUpdates before this function -- it will flag the scene
    // objects that need updating, while this function handles the layer updates

    if (!layer) {
        return;
    }

    // cache primitive attributes contained in the shader network of all materials.
    // This must be done before any updates to SceneObjects.
    Layer::MaterialSet materials;
    layer->getAllMaterials(materials);
    for (const Material* m : materials) {
        if (m) {
            m->cacheShaderGraphPrimAttributes();
        }
    }

    // Flag shaders that are in the update graph, as we will need to re-build the associated attribute tables.
    // Also flag the associated geometry, since a change in the attribute table might require a geometry update. Also
    // in this step, we will flag the LightSets, LightFilterSets, ShadowSets, and ShadowReceiverSets that will need 
    // updating in the preFrame() function.
    layer->updatePrepAssignments(mSceneObjectUpdateGraph, 0, nullptr);

    // Changes in a shader's requested primitive attributes require updating
    // the geometry.
    // Also, changes in the volumeShader requires updating the geometry because
    // volume shaders are baked into the geometry.

    const auto& changedOrDeformedGeometries = layer->getChangedOrDeformedGeometries();
    // If there are no changed or deformed geometries, do nothing.
    if (changedOrDeformedGeometries.empty()) return;

    for (auto& geomPair : changedOrDeformedGeometries) {
        Geometry* geom = geomPair.first;
        int index = geomPair.second;

        // If a shader requests new primitive attributes from the geometry, then
        // we need to update the geometry.
        const Material * const material = layer->lookupMaterial(index);
        if (material && material->haveShaderGraphPrimAttributesChanged()) {
            geom->requestUpdate();
        }
    }
}

std::unordered_map<std::string, size_t>
SceneContext::getDsoCounts() const
{
    std::unordered_map<std::string, size_t> dsoCounts;
    for (int i = mSceneObjectUpdateGraph.getMaxDepth()-1; i >= 0; --i) {
        for (auto it = mSceneObjectUpdateGraph.cbegin(i); it != mSceneObjectUpdateGraph.cend(i); ++it) {
            const SceneObject* obj = *it;
            if (obj->isA<Material>() ||
                obj->isA<Map>() ||
                obj->isA<NormalMap>() ||
                obj->isA<VolumeShader>() ||
                obj->isA<DisplayFilter>() ||
                obj->isA<Geometry>() ||
                obj->isA<Displacement>() ||
                obj->isA<LightFilter>() ||
                obj->isA<Light>()) {

                const std::string& className = obj->getSceneClass().getName();
                if (dsoCounts.find(className) == dsoCounts.end()) {
                    dsoCounts.insert(std::make_pair(className, 1));
                } else {
                    ++dsoCounts[className];
                }
            }
        }
    }
    return dsoCounts;
}

void
SceneContext::resetUpdates(Layer * const layer)
{
    for (auto pair : mSceneObjects) pair.second->resetUpdate();
    if (layer) layer->resetAssignmentUpdates();
    mSceneObjectUpdateGraph.clear();
}

void
SceneContext::getUpdatedOrDeformedGeometrySets(Layer * const layer,
                                     GeometrySetVector & updatedSets)
{
    const Layer::GeometryIndexMap & updatedOrDeformedGeometries =
                                                  layer->getChangedOrDeformedGeometries();
    for (GeometrySet * const set : mGeometrySets) {
        if (set->updatePrepApplied()) {
            updatedSets.push_back(set);
            continue;
        }
        for (SceneObject * const object : set->getGeometries()) {
            if (updatedOrDeformedGeometries.count(object->asA<Geometry>()) != 0) {
                updatedSets.push_back(set);
                break;
            }
        }
    }
}

void
SceneContext::commitAllChanges()
{
    std::for_each(mSceneObjects.begin(), mSceneObjects.end(),
    [](const SceneObjectMapItem& item) {
        item.second->commitChanges();
    });
}

void
SceneContext::loadAllSceneClasses()
{
    std::string remaining(getDsoPath());
    while (!remaining.empty()) {
        // Grab the next path entry.
        std::size_t colonPos = remaining.find_first_of(':');
        std::string directory = remaining.substr(0, colonPos);

        // Grab the contents of each directory.
        DIR* directoryPtr = opendir(directory.c_str());
        if (directoryPtr != nullptr) {
            struct dirent* entryPtr;
            while ((entryPtr = readdir(directoryPtr))) {
                std::string fileName(entryPtr->d_name);

                // If the file is a valid DSO, then create a SceneClass from it.
                if (Dso::isValidDso(directory + '/' + fileName, mProxyModeEnabled)) {
                    // Class name is the file name without ".so" (or
                    // ".so.proxy" in proxy mode).
                    std::string className;
                    if (mProxyModeEnabled) {
                        className = fileName.substr(0, fileName.size() - 9);
                    } else {
                        className = fileName.substr(0, fileName.size() - 3);
                    }

                    try {
                        createSceneClass(className);
                    } catch (...) {
                        // Swallow exceptions here. If something was wrong with
                        // the declare() function, just move on to the next
                        // SceneClass.
                    }
                }
            }
        }
        closedir(directoryPtr);

        // Move to the next path entry.
        if (colonPos != std::string::npos) {
            remaining = remaining.substr(colonPos + 1, std::string::npos);
        } else {
            remaining = "";
        }
    }
}

void
SceneContext::computeTimeRescalingCoeffs(float shutterOpen, float shutterClose, const std::vector<float> &motionSteps)
{
    // See declaration of TimeRescalingCoeffs in Types.h for details.

    tbb::mutex::scoped_lock lock(mTimeRescalingCoeffsMutex);

    MNRY_ASSERT_REQUIRE(motionSteps.size() >= 1 && motionSteps.size() <= 2);
    if (motionSteps.size() == 1  ||  motionSteps[0] == motionSteps[1]) {
        // Handle extraordinary case where we don't have 2 distinct motion steps
        // (denominator of the coefficients would be 0).
        mTimeRescalingCoeffs.mScale = 0.0f;
        mTimeRescalingCoeffs.mOffset = 0.0f;
    } else {
        float oneOverDenom = 1.0f / (motionSteps[1] - motionSteps[0]);
        mTimeRescalingCoeffs.mScale  = (shutterClose - shutterOpen   ) * oneOverDenom;
        mTimeRescalingCoeffs.mOffset = (shutterOpen  - motionSteps[0]) * oneOverDenom;
    }
}

void
SceneContext::setRender2World(const Mat4d *render2World)
{
    mRender2World = render2World;
}

SceneContext::GeometrySetVector SceneContext::getGeometrySetsForLayer(const Layer* layer)
{
    GeometrySetVector ret;

    // We have to iterate over the geometry pointers stored in the GeometrySet
    // to see if the geometry is in the layer. This is in a separate function so
    // that we can break out of our outer loop if we find it, because we only
    // want to add the geometry set once.
    auto geomIter = [&ret, layer] (GeometrySet* p) {
        for (const auto& sceneobj : p->getGeometries()) {
            const auto geom = sceneobj->asA<Geometry>();
            if (geom && layer->contains(geom)) {
                ret.push_back(p);
                return;
            }
        }
    };

    for (const auto gs : mGeometrySets) {
        geomIter(gs);
    }

    return ret;
}

} // namespace rdl2
} // namespace scene_rdl2

