// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "boost_python.h"
#include "py_scene_rdl2.h"

// scene_rdl2
#include <scene_rdl2/scene/rdl2/Camera.h>
#include <scene_rdl2/scene/rdl2/Node.h>
using namespace scene_rdl2;

namespace py_scene_rdl2
{

    //------------------------------------
    // rdl2::Camera
    //------------------------------------

    void
    registerCameraPyBinding()
    {
        using PyCameraClass_t = bp::class_<rdl2::Camera,
                                           std::shared_ptr<rdl2::Camera>,
                                           bp::bases<rdl2::Node>,
                                           boost::noncopyable>;

        PyCameraClass_t("Camera", bp::no_init)
            .def(bp::init<const rdl2::SceneClass&, const std::string&>( (bp::arg("sceneClass"), bp::arg("name")) ))

            .def("declare",
                 &rdl2::Camera::declare,
                 bp::arg("sceneClass"))
            .staticmethod("declare")

            .def("getSceneClass",
                 &rdl2::Camera::getSceneClass,
                 bp::return_internal_reference<>(),
                 "Retrieves a the SceneClass to which this SceneObject belongs.")

            .def("getName",
                 &rdl2::Camera::getName,
                 bp::return_value_policy<bp::copy_const_reference>(),
                 "Retrieves the name of this SceneObject.")

            .def("getType",
                 &rdl2::Camera::getType,
                 "Retrieves the object type bitmask. This value may not be one of the enum "
                 "options, but rather a bitwise combination of them, so you'll need to use "
                 "bitwise operators to check for a specific interface.")

            .def("resetToDefault",
                 ( void (rdl2::Camera::*) (const std::string&) ) &rdl2::Camera::resetToDefault,
                 bp::arg("name"),
                 "Convenience function to reset an attribute value to its default value by name rather "
                 "than by AttributeKey. If no default value is supplied by the SceneClass, a "
                 "reasonable default is supplied for you (0, empty string, null, etc.)"
                 "\n"
                 "Inputs:    name    The name of an attribute which you want to reset to its default value.")

            .def("resetAllToDefault",
                 &rdl2::Camera::resetAllToDefault,
                 "Resets all attributes in the SceneObject to their default values. If no default value "
                 "is supplied for an attribute by the SceneClass, a reasonable default is supplied for "
                 "you (0, empty string, null, etc.)")

            .def("update",
                 &rdl2::Camera::update,
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

//            .def("isActive", &rdl2::Camera::isActive)

            .def("setNear", &rdl2::Camera::setNear, bp::arg("near"))

            .def("setFar", &rdl2::Camera::setFar, bp::arg("far"))
            ;
    }

} // namespace py_scene_rdl2

