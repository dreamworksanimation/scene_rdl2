// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "boost_python.h"
#include "py_scene_rdl2.h"
#include "py_scene_rdl2_helpers.h"

// scene_rdl2
#include <scene_rdl2/scene/rdl2/Camera.h>
#include <scene_rdl2/scene/rdl2/SceneClass.h>
#include <scene_rdl2/scene/rdl2/SceneVariables.h>
using namespace scene_rdl2;

namespace py_scene_rdl2
{

    //------------------------------------
    // rdl2::SceneVariables
    //------------------------------------

    math::Vec2i
    PySceneVariables_getDebugPixel(rdl2::SceneVariables& self)
    {
        math::Vec2i res;
        self.getDebugPixel(res);

        return res;
    }

    bp::list
    PySceneVariables_getDebugRaysPrimaryRange(rdl2::SceneVariables& self)
    {
        int start = 0;
        int end = 0;

        self.getDebugRaysPrimaryRange(start, end);

        bp::list res;

        res.append(start);
        res.append(end);

        return res;
    }

    bp::list
    PySceneVariables_getDebugRaysDepthRange(rdl2::SceneVariables& self)
    {
        int start = 0;
        int end = 0;

        self.getDebugRaysDepthRange(start, end);

        bp::list res;

        res.append(start);
        res.append(end);

        return res;
    }

    math::HalfOpenViewport
    PySceneVariables_getSubViewport(rdl2::SceneVariables& self)
    {
        math::HalfOpenViewport vp;
        self.getSubViewport(vp);
        return vp;
    }

    void
    registerSceneVariablesPyBinding()
    {
        using PySceneVariablesClass_t = bp::class_<rdl2::SceneVariables,
                                                   std::shared_ptr<rdl2::SceneVariables>,
                                                   bp::bases<rdl2::SceneObject>,
                                                   boost::noncopyable>;

        PySceneVariablesClass_t("SceneVariables", bp::no_init)
            .def(bp::init<const rdl2::SceneClass&, const std::string&>(
                    (bp::arg("sceneClass"), bp::arg("name")) ))

            .def("declare",
                 &rdl2::SceneVariables::declare,
                 bp::arg("sceneClass"))
            .staticmethod("declare")

            .def("getSceneClass",
                 &rdl2::SceneVariables::getSceneClass,
                 bp::return_internal_reference<>(),
                 "Retrieves a the SceneClass to which this SceneObject belongs.")

            .def("getName",
                 &rdl2::SceneVariables::getName,
                 bp::return_value_policy<bp::copy_const_reference>(),
                 "Retrieves the name of this SceneObject.")

            .def("getType",
                 &rdl2::SceneVariables::getType,
                 "Retrieves the object type bitmask. This value may not be one of the enum "
                 "options, but rather a bitwise combination of them, so you'll need to use "
                 "bitwise operators to check for a specific interface.")

            .def("resetToDefault",
                 ( void (rdl2::SceneVariables::*) (const std::string&) ) &rdl2::SceneVariables::resetToDefault,
                 bp::arg("name"),
                 "Convenience function to reset an attribute value to its default value by name rather "
                 "than by AttributeKey. If no default value is supplied by the SceneClass, a "
                 "reasonable default is supplied for you (0, empty string, null, etc.)"
                 "\n"
                 "Inputs:    name    The name of an attribute which you want to reset to its default value.")

            .def("resetAllToDefault",
                 &rdl2::SceneVariables::resetAllToDefault,
                 "Resets all attributes in the SceneObject to their default values. If no default value "
                 "is supplied for an attribute by the SceneClass, a reasonable default is supplied for "
                 "you (0, empty string, null, etc.)")

            .def("update",
                 &rdl2::SceneVariables::update,
                 "update() is called automatically before rendering starts by RDL, whenever the "
                 "attributes or bindings of an object have changed (on this object or any of its "
                 "object-attributes or bindings). You should not have to manually call this "
                 "function on a scene object."
                 "\n"
                 "The update() method is to notify a derived class that the object has changed. "
                 "This method can be reimplemented by derived (DSO) object types to react to changes "
                 "in this objects attributes. This can be used for verifying that attribute data "
                 "is valid, or rebuilding cached data from attribute source data."
                 "\n"
                 "There may be changes to multiple attributes or bindings per single update() call. "
                 "You can find out which attributes or bindings changed using the hasChanged(AttributeKey) "
                 "and hasBindingChanged(AttributeKey) functions for the attributes in question."
                 "\n"
                 "When this function is called on a given object, you are guaranteed that it has already "
                 "been called on its dependencies, the tree (more accurately the directed acyclic graph) "
                 "of objects connected through attributes and bindings to this object. You have no guarantees, "
                 "however, about objects outside of this tree (graph). Though all objects may be accessible "
                 "through the SceneContext via the SceneClass, it is only safe to query the objects in "
                 "the dependency tree of the current object."
                 "\n"
                 "Note that currently update() will not be called on any additional objects based on "
                 "changes to SceneVariables."
                 "\n"
                 "RDL does not track attribute value history, so it cannot tell you what the "
                 "previous value was.")

            .def("getRezedWidth",
                 &rdl2::SceneVariables::getRezedWidth,
                 "Retrieves the frame width (AFTER applying the resolution divisor and viewport), "
                 "in pixels. This is probably what you want.")

            .def("getRezedHeight",
                 &rdl2::SceneVariables::getRezedHeight,
                 "Retrieves the frame height (AFTER applying the resolution divisor and viewport), "
                 "in pixels. This is probably what you want.")

            .def("getMachineId",
                 &rdl2::SceneVariables::getMachineId,
                 "Get the machine ID. Machine IDs must be >= 0 and < numMachines.")

            .def("getNumMachines",
                 &rdl2::SceneVariables::getNumMachines,
                 "Get the number of machines in the cluster. If not rendering in a cluster, this is 1.")

            .def("getDebugPixel",
                 &PySceneVariables_getDebugPixel,
                 "Get the pixel to debug. The get call also returns whether or not the debug pixel was set "
                 "(false by default). The debug pixel is expressed in rezed / frame-viewport coordinates "
                 "(see getFrameViewport()). The debug pixel is initialized to an invalid value. If it has "
                 "not been set to something else, the getter will return false. Therefore, the return boolean "
                 "should be checked by the caller.")

            .def("getDebugRaysPrimaryRange",
                 &PySceneVariables_getDebugRaysPrimaryRange,
                 "Returns a list containing two integers: start and end ray to debug, inclusive. The debug rays "
                 "primary range is initialized to an invalid value. If it has not been set to something else, "
                 "the getter will return false. Therefore, the return boolean should be checked by the caller.")

            .def("getDebugRaysPrimaryRange",
                 &PySceneVariables_getDebugRaysDepthRange,
                 "Get start and end ray depth debug, inclusive. The debug rays depth range is initialized to "
                 "an invalid value. If it has not been set to something else, the getter will return false. "
                 "Therefore, the return boolean should be checked by the caller.")

            .def("getSubViewport",
                 &PySceneVariables_getSubViewport,
                 "Get sub-viewport. We don't render pixels outside of this viewport. Max x and y coordinates "
                 "are inclusive, i.e. we render them. The sub-viewport is expressed in rezed / frame-viewport "
                 "coordinates (see getFrameViewport()). The subviewport is initialized to an invalid value. If "
                 "it has not been set to something else, the getter will return false. Therefore, the return "
                 "boolean should be checked by the caller.")

            .def("disableSubViewport", &rdl2::SceneVariables::disableSubViewport, "Disable sub-viewport.")
            ;
    }

} // namespace py_scene_rdl2

