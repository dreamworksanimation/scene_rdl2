// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "boost_python.h"
#include "py_scene_rdl2.h"

// scene_rdl2
#include <scene_rdl2/scene/rdl2/Layer.h>
using namespace scene_rdl2;

namespace py_scene_rdl2
{

    //------------------------------------
    // rdl2::Camera
    //------------------------------------

    void
    registerLayerPyBinding()
    {
        using PyLayerClass_t = bp::class_<rdl2::Layer,
                                          std::shared_ptr<rdl2::Layer>,
                                          bp::bases<rdl2::SceneObject>,
                                          boost::noncopyable>;

        PyLayerClass_t("Layer", bp::no_init)
            .def(bp::init<const rdl2::SceneClass&, const std::string&>( (bp::arg("sceneClass"), bp::arg("name")) ))

            .def("declare",
                 &rdl2::Layer::declare,
                 bp::arg("sceneClass"))
            .staticmethod("declare")

            .def("getSceneClass",
                 &rdl2::Layer::getSceneClass,
                 bp::return_internal_reference<>(),
                 "Retrieves a the SceneClass to which this SceneObject belongs.")

            .def("getName",
                 &rdl2::Layer::getName,
                 bp::return_value_policy<bp::copy_const_reference>(),
                 "Retrieves the name of this SceneObject.")

            .def("getType",
                 &rdl2::Layer::getType,
                 "Retrieves the object type bitmask. This value may not be one of the enum "
                 "options, but rather a bitwise combination of them, so you'll need to use "
                 "bitwise operators to check for a specific interface.")

            .def("resetToDefault",
                 ( void (rdl2::Layer::*) (const std::string&) ) &rdl2::Layer::resetToDefault,
                 bp::arg("name"),
                 "Convenience function to reset an attribute value to its default value by name rather "
                 "than by AttributeKey. If no default value is supplied by the SceneClass, a "
                 "reasonable default is supplied for you (0, empty string, null, etc.)"
                 "\n"
                 "Inputs:    name    The name of an attribute which you want to reset to its default value.")

            .def("resetAllToDefault",
                 &rdl2::Layer::resetAllToDefault,
                 "Resets all attributes in the SceneObject to their default values. If no default value "
                 "is supplied for an attribute by the SceneClass, a reasonable default is supplied for "
                 "you (0, empty string, null, etc.)")

            .def("getAssignmentCount",
                 &rdl2::Layer::getAssignmentCount,
                 "Returns the number of assignments made in this layer so far.")

            .def("getAssignmentId",
                 &rdl2::Layer::getAssignmentId,
                 ( bp::arg("geometry"), bp::arg("partName") ),
                 "Given a Geometry and part name on that Geometry, this will return the assignment ID "
                 "for that assignment, which can be used for fast assignment lookups. For efficiency, "
                 "you should save this value to use for multiple lookups. If no assignment is found, "
                 "-1 is returned. \n"
                 "\n"
                 "Inputs:    geometry    The Geometry on which the part lives. \n"
                 "           partName    The name of the part with the assignment. \n"
                 "Returns the assignment ID that can be used for fast lookups.")

            .def("contains",
                 &rdl2::Layer::contains,
                 bp::arg("geometry"),
                 "Given a Geometry, this will return whether or not the layer contains said geometry. \n"
                 "\n"
                 "Input:    geometry    The Geometry to check to see if it exists in the layer. \n"
                 "Returns whether the geometry exists in they layer or not.")

            .def("clear",
                 &rdl2::Layer::clear,
                 "Completely empties the Layer so that it doesn't contain anything.")
            ;
    }

} // namespace py_scene_rdl2

