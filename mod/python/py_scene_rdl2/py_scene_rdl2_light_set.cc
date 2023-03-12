// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "boost_python.h"
#include "py_scene_rdl2.h"
#include "py_scene_rdl2_helpers.h"

// scene_rdl2
#include <scene_rdl2/scene/rdl2/LightSet.h>
using namespace scene_rdl2;

namespace py_scene_rdl2
{
    SceneObjectVectorWrapper
    PyLightSet_getLights(rdl2::LightSet& self)
    {
        return SceneObjectVectorWrapper{ self.getLights() };
    }

    //------------------------------------
    // rdl2::LightSet
    //------------------------------------

    void
    registerLightSetPyBinding()
    {
        bp::class_<rdl2::LightSet,
                   std::shared_ptr<rdl2::LightSet>,
                   bp::bases<rdl2::SceneObject>,
                   boost::noncopyable>("LightSet", bp::no_init)

            .def(bp::init<const rdl2::SceneClass&, const std::string&>( (bp::arg("sceneClass"), bp::arg("name")) ))

            .def("getLights",
                 &PyLightSet_getLights,
                 "Retrieves the set of unique Lights in this LightSet.")

            .def("add",
                 &rdl2::LightSet::add,
                 bp::arg("light"),
                 "Adds the given Light to the LightSet, if it is not already a member of "
                 "the set. If it is already a member of the set, this does nothing. \n"
                 "\n"
                 "Inputs:    light    The Light to add to the LightSet.")

            .def("remove",
                 &rdl2::LightSet::remove,
                 bp::arg("light"),
                 "Removes the given Light from the LightSet, if it is already a member of "
                 " the set. If it is not a member of the set, this does nothing.\n"
                 "\n"
                 "Inputs:    light    The Light to remove from the LightSet.")

            .def("declare",
                 &rdl2::LightSet::declare,
                 bp::arg("sceneClass"))
            .staticmethod("declare")

            .def("getSceneClass",
                 &rdl2::LightSet::getSceneClass,
                 bp::return_internal_reference<>(),
                 "Retrieves a the SceneClass to which this SceneObject belongs.")

            .def("getName",
                 &rdl2::LightSet::getName,
                 bp::return_value_policy<bp::copy_const_reference>(),
                 "Retrieves the name of this SceneObject.")

            .def("getType",
                 &rdl2::LightSet::getType,
                 "Retrieves the object type bitmask. This value may not be one of the enum "
                 "options, but rather a bitwise combination of them, so you'll need to use "
                 "bitwise operators to check for a specific interface.")

            .def("resetToDefault",
                 ( void (rdl2::LightSet::*) (const std::string&) ) &rdl2::LightSet::resetToDefault,
                 bp::arg("name"),
                 "Convenience function to reset an attribute value to its default value by name rather "
                 "than by AttributeKey. If no default value is supplied by the SceneClass, a "
                 "reasonable default is supplied for you (0, empty string, null, etc.)"
                 "\n"
                 "Inputs:    name    The name of an attribute which you want to reset to its default value.")

            .def("resetAllToDefault",
                 &rdl2::LightSet::resetAllToDefault,
                 "Resets all attributes in the SceneObject to their default values. If no default value "
                 "is supplied for an attribute by the SceneClass, a reasonable default is supplied for "
                 "you (0, empty string, null, etc.)")

            .def("contains",
                 &rdl2::LightSet::contains,
                 bp::arg("light"),

                 "Returns true if the given Light is a member of the LightSet. There's "
                 "no need to call this before calling add() or remove(), as they will "
                 "gracefully handle those edge cases. \n"
                 "\n"
                 "Input:     light    The Light to check for membership. \n"
                 "Returns    True if the light is a member of the LightSet."

                 "Given a Geometry, this will return whether or not the layer contains said geometry. \n"
                 "\n"
                 "Input:    geometry    The Geometry to check to see if it exists in the layer. \n"
                 "Returns whether the geometry exists in they layer or not.")

            .def("clear",
                 &rdl2::LightSet::clear,
                 "Completely empties the LightSet so that it doesn't contain anything.")
            ;
    }

} // namespace py_scene_rdl2

