// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "boost_python.h"
#include "py_scene_rdl2.h"
#include "py_scene_rdl2_helpers.h"

// scene_rdl2
#include <scene_rdl2/scene/rdl2/Geometry.h>
#include <scene_rdl2/scene/rdl2/GeometrySet.h>
#include <scene_rdl2/scene/rdl2/Layer.h>
#include <scene_rdl2/scene/rdl2/Proxies.h>
#include <scene_rdl2/scene/rdl2/SceneContext.h>
using namespace scene_rdl2;

namespace py_scene_rdl2
{

    //------------------------------------
    // rdl2::SceneContext
    //------------------------------------

    bp::list
    PySceneContext_getSceneClassNames(rdl2::SceneContext& self)
    {
        bp::list sceneClassNames;
        for (auto iter = self.beginSceneClass(); iter != self.endSceneClass(); ++iter) {
            sceneClassNames.append(iter->first);
        }

        return sceneClassNames;
    }

    bp::list
    PySceneContext_getSceneObjectNames(rdl2::SceneContext& self)
    {
        bp::list sceneObjNames;
        for (auto iter = self.beginSceneObject(); iter != self.endSceneObject(); ++iter) {
            sceneObjNames.append(iter->first);
        }

        return sceneObjNames;
    }

    bp::dict
    PySceneContext_getSceneObjectNamesAndTypes(rdl2::SceneContext& self)
    {
        bp::dict sceneObjNamesAndTypes;
        for (auto iter = self.beginSceneObject(); iter != self.endSceneObject(); ++iter) {
            sceneObjNamesAndTypes[iter->first] = getSceneObjectTypeName(
                    const_cast<rdl2::SceneObject*>(iter->second));
        }

        return sceneObjNamesAndTypes;
    }

    void
    PySceneContext_setRender2World(rdl2::SceneContext& self, rdl2::Mat4d& render2World)
    {
        self.setRender2World(&render2World);
    }

    rdl2::Geometry*
    PySceneContext_getGeometryAt(rdl2::SceneContext& self, std::size_t index)
    {
        auto iter = self.beginGeometry();
        const std::size_t geomVectSize =
                std::distance(iter, self.endGeometry());

        if (index >= geomVectSize) {
            return nullptr;
        }

        std::advance(iter, index);
        return *iter;
    }

    std::size_t
    PySceneContext_getGeometryListSize(rdl2::SceneContext& self)
    {
        return std::distance(self.beginGeometry(), self.endGeometry());
    }

    rdl2::GeometrySet*
    PySceneContext_getGeometrySetAt(rdl2::SceneContext& self, std::size_t index)
    {
        auto iter = self.beginGeometrySet();
        const std::size_t geomSetVectSize =
                std::distance(iter, self.endGeometrySet());

        if (index >= geomSetVectSize) {
            return nullptr;
        }

        std::advance(iter, index);
        return *iter;
    }

    std::size_t
    PySceneContext_getGeometrySetListSize(rdl2::SceneContext& self)
    {
        return std::distance(self.beginGeometrySet(), self.endGeometrySet());
    }

    bp::list
    PySceneContext_getGeometrySetIndicesForLayer(rdl2::SceneContext& self, rdl2::Layer* layer)
    {
        if (layer == nullptr) {
            return {};
        }

        bp::list res;
        const std::vector<rdl2::GeometrySet*> geomSetVec = self.getGeometrySetsForLayer(layer);

        std::size_t counter = 0;
        for (auto iter = self.beginGeometrySet(); iter != self.endGeometrySet(); ++iter) {
            if (std::find(geomSetVec.cbegin(), geomSetVec.cend(), *iter) != geomSetVec.cend()) {
                res.append(counter);
            }

            ++counter;
        }

        return res;
    }

    void
    registerSceneContextPyBinding()
    {
        const std::string rdl2SceneContextDocstring =
                 "The SceneContext represents all the data for a specific scene in RDL. This "
                 "includes all the objects in the scene (SceneObjects) as well as their types "
                 "(SceneClasses). It provides some basic functionality for creating "
                 "SceneClasses and SceneObjects, iterating through them, finding them by their "
                 "unique name, and retreving them so you can query or update their attributes.\n"
                 "Once the data has been loaded or updated, the rendering libraries should be "
                 "given a immutable reference to the SceneContext. RDL makes heavy use of its const "
                 "correct API to indicate which methods and objects are read-only and thread "
                 "safe. If you stick to the const API and don't violate its integrity, it should "
                 "be completely safe to traverse the SceneContext and all its objects from multiple"
                 " threads concurrently.\n"
                 "Don't worry about calling createSceneClass() or createSceneObject() multiple "
                 "times by accident. They both have 'create if it does not exist' semantics, "
                 "and are effectively no-ops if the class or object exists. In the case of "
                 "createSceneObject, it will return the existing object.\n"
                 "Thread Safety:\n"
                 "  - The only points of synchronization in the SceneContext are the SceneClass "
                 "and SceneObject hash maps. These control access to the hash map with reader/writer "
                 "locks, so it should only be slow in the presence of writers. Even then, it's only "
                 "slow while we're inserting the SceneClass or SceneObject into the hash table."
                 " Once the insertion is finished, the lock is released and you can continue updating"
                 " the object without holding the lock.\n"
                 "  - SceneClasses and SceneObjects do not synchronize access to themselves, so "
                 "writing to these objects must only happen in a single thread. They are completely "
                 "self contained, though, so you are free to write to different SceneClasses or "
                 "SceneObjects in different threads concurrently.";

        using PySceneContextClass_t = bp::class_<rdl2::SceneContext,
                                                 std::shared_ptr<rdl2::SceneContext>,
                                                 boost::noncopyable>;

        PySceneContextClass_t("SceneContext", rdl2SceneContextDocstring.c_str())
            .def("getDsoPath",
                 &rdl2::SceneContext::getDsoPath,
                 "Retrieves the DSO path this SceneContext is using to locate DSO SceneClasses. "
                 "This fetches the value of the SceneVariable 'dso path', which is sourced in "
                 "the following order:"
                 "\n  1 - If -dso_path was passed on command line, this overrides anything from below"
                 "\n  2 - If RDL2_DSO_PATH environment variable is set, this overrides anything from below"
                 "\n  3 - If neither of the above are set, defaults to searching for 'raas_render' executable "
                 "and building path to 'rdl2dso' based on location of executable.")

            .def("getProxyModeEnabled",
                 &rdl2::SceneContext::getProxyModeEnabled,
                 "Retrieves whether or not the SceneContext is currently in proxy mode.")

            .def("setDsoPath",
                 &rdl2::SceneContext::setDsoPath,
                 bp::arg("dsoPath"),
                 "Sets the DSO path in SceneVariables that SceneContext will use when looking "
                 "for DSOs that define SceneClasses. The string is a colon separated list of "
                 "paths, much like the $PATH shell variable.\n"
                 "Input:    dsoPath    A colon separated list of paths to search for DSOs.")

            .def("setProxyModeEnabled",
                 &rdl2::SceneContext::setProxyModeEnabled,
                 bp::arg("enabled"),
                 "Sets whether or not the SceneContext is in proxy mode.\n"
                 "When in proxy mode, new SceneClasses will be created such that any "
                 "objects of that SceneClass will be proxies, not the actual objects. This "
                 "is useful if you want to use RDL to read and write SceneContexts, but "
                 "don't want to drag in the dependencies of all the DSOs you're using.\n"
                 "It's important to remember that proxy mode only affects *new* "
                 "SceneClasses that are created. Objects created from that SceneClass will "
                 "always be created as proxies or not depending on whether the context was "
                 "in proxy mode when the SceneClass was created. In general, you probably "
                 "want the context to always be in proxy mode or never be in proxy mode.\n"
                 "Input    enabled    True to enable proxy mode, false to disable.")

            // .def("setActiveCamera",
            //      (void (rdl2::SceneContext::*) (const std::string&)) &rdl2::SceneContext::setActiveCamera,
            //      bp::arg("name"),
            //      "Sets the active camera by name.")

            // .def("setActiveCamera",
            //      (void (rdl2::SceneContext::*) (rdl2::Camera*)) &rdl2::SceneContext::setActiveCamera,
            //      bp::arg("camera"),
            //      "Sets the active camera by a Camera object direclty.")

            .def("getPrimaryCamera",
                 &rdl2::SceneContext::getPrimaryCamera,
                 bp::return_internal_reference<>(),
                 "Returns the primary camera, if one is set. Otherwise, returns None.")

            .def("setRender2World",
                 &PySceneContext_setRender2World,
                 bp::arg("render2World"),
                 "Sets the render to world transform.")

            .def("getRender2World",
                 &rdl2::SceneContext::getRender2World,
                 bp::return_internal_reference<>(),
                 "Returns the render to world transform, if set, None if not.")

            .def("commitAllChanges",
                 &rdl2::SceneContext::commitAllChanges,
                 "Clears all flags on all attributes of all objects that are tracking "
                 "what has changed. This effectively puts the SceneContext in its 'base' "
                 "state, where nothing has changed.")

            .def("loadAllSceneClasses",
                 &rdl2::SceneContext::loadAllSceneClasses,
                 "Searches every directory in the DSO path looking for '.so' files and "
                 "attempts to load them as RDL DSOs. Files that are not successfully "
                 "opened as RDL DSOs are ignored. This can be used to fill up the SceneClass "
                 "map with all the available SceneClasses, and then iterate over them "
                 "exploring their attributes and attribute metadata.")

            .def("sceneObjectExists",
                 &rdl2::SceneContext::sceneObjectExists,
                 bp::arg("name"),
                 "Checks for existence of a SceneObject with the given name.")

            .def("getSceneObjectNames",
                 &PySceneContext_getSceneObjectNames,
                 "(Python only) Returns the list of all SceneObject names; to get to a specific SceneObject, "
                 "find its name in the returned list then use SceneClass.getSceneObject(name).")

            .def("getSceneObjectNamesAndTypes",
                 &PySceneContext_getSceneObjectNamesAndTypes,
                 "(Python only) Returns a dictionary of SceneObjects and their types (name : type); "
                 "to get to a specific SceneObject, find its name in the returned list then use "
                 "SceneClass.getSceneObject(name).")

            .def("getSceneObject",
                 static_cast<rdl2::SceneObject* (rdl2::SceneContext::*) (const std::string&)>(&rdl2::SceneContext::getSceneObject),
                 bp::arg("name"),
                 bp::return_internal_reference<>(),
                 "Retrieves a mutable SceneObject by its name.")

            .def("sceneClassExists",
                 &rdl2::SceneContext::sceneClassExists,
                 bp::arg("name"),
                 "Checks for existence of a SceneClass with the given name.")

            .def("getSceneClassNames",
                 &PySceneContext_getSceneClassNames,
                 "(Python only) Returns the list of all SceneClass names; to get to a specific SceneClass, "
                 "find its name in the returned list then use SceneClass.getSceneClass(name).")

            .def("getSceneClass",
                 static_cast<const rdl2::SceneClass* (rdl2::SceneContext::*) (const std::string&) const>(&rdl2::SceneContext::getSceneClass),
                 bp::arg("name"),
                 bp::return_internal_reference<>(),
                 "Retrieves a SceneClass by its name..")

            .def("getSceneVariables",
                 static_cast<rdl2::SceneVariables& (rdl2::SceneContext::*) ()>(&rdl2::SceneContext::getSceneVariables),
                 bp::return_internal_reference<>(),
                 "Retrieves a mutable reference to the SceneVariables object.")

            .def("createSceneClass",
                 &rdl2::SceneContext::createSceneClass,
                 bp::arg("className"),
                 bp::return_internal_reference<>(),
                 "Creates a SceneClass of the given name. \n"
                 "\n"
                 "If the class already exists, nothing happens, and the existing SceneClass will be returned. "
                 "If the class does not exist, the SceneContext will search the DSO path for a file named the "
                 "same as the class name with a '.so' extension and attempt to open it as an RDL DSO. If all "
                 "goes well, the SceneClass will be created and returned. \n"
                 "\n"
                 "The SceneContext owns the returned pointer, and will free it when the context is destroyed. \n"
                 "\n"
                 "Inputs:    className    The name of the SceneClass to create and load. \n"
                 "Returns the new SceneClass or the existing SceneClass (if it already existed).")

            .def("createSceneObject",
                 &rdl2::SceneContext::createSceneObject,
                 ( bp::arg("className"), bp::arg("objectName") ),
                 bp::return_internal_reference<>(),
                 "Create a SceneObject from the given SceneClass name with the given object name. \n"
                 "\n"
                 "If the object already exists, nothing happens, and the existing object will be returned. "
                 "If the object does not exist, it will be created and the new object will be returned. \n"
                 "\n"
                 "The SceneContext owns the returned pointer, and will free it when the context is destroyed. \n"
                 "\n"
                 "Inputs:    className     The name of the SceneClass that this object will be created from. \n"
                 "           objectName    The name of the object. Must be unique. \n"
                 "Returns the new SceneObject or the existing SceneObject (if the name already existed).")

            .def("getGeometryListSize",
                 &PySceneContext_getGeometryListSize,
                 "(Python Only) Returns the number of Geometry objects held by this SceneContext.")

            .def("getGeometryAt",
                 &PySceneContext_getGeometryAt,
                 bp::arg("index"),
                 bp::return_internal_reference<>(),
                 "(Python Only) Returns the Geometry object located at index 'index' in the list of "
                 "Geometry objects.")

            .def("getGeometrySetListSize",
                 &PySceneContext_getGeometrySetListSize,
                 "(Python Only) Returns the number of GeometrySet objects held by this SceneContext.")

            .def("getGeometrySetAt",
                 &PySceneContext_getGeometrySetAt,
                 bp::arg("index"),
                 bp::return_internal_reference<>(),
                 "(Python Only) Returns the GeometrySet object located at index 'index' in the list "
                 "of GeometrySet objects.")

            .def("getGeometrySetIndicesForLayer",
                 &PySceneContext_getGeometrySetIndicesForLayer,
                 bp::arg("layer"),
                 "(Python Only) Returns a list of indices to GeometrySet objects that have geometry in the specified layer. \n"
                 "You can use SceneContext.getGeometrySetAt(index) to retrieve a specific GeometrySet.")
                 ;

    }

} // namespace py_scene_rdl2

