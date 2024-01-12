// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "boost_python.h"
#include "py_scene_rdl2.h"

// scene_rdl2
#include <scene_rdl2/scene/rdl2/Geometry.h>
#include <scene_rdl2/scene/rdl2/GeometrySet.h>
using namespace scene_rdl2;

namespace py_scene_rdl2
{

    //------------------------------------
    // rdl2::GeometrySet
    //------------------------------------

    bp::list
    PyGeometrySet_getGeometries(rdl2::GeometrySet& self)
    {
        bp::list geomSet;

        const rdl2::SceneObjectIndexable& indexableArrayRef = self.getGeometries();

        for (rdl2::SceneObject* geom : indexableArrayRef) {
            geomSet.append(geom);
        }

        return geomSet;
    }

    void
    PyGeometrySet_add(rdl2::GeometrySet& self, rdl2::Geometry& geometry)
    {
        // *** NOTE ***
        // Make sure ref count is incremented, check before, during, and after call
        self.add(&geometry);
    }

    void
    PyGeometrySet_remove(rdl2::GeometrySet& self, rdl2::Geometry& geometry)
    {
        self.remove(&geometry);
    }

    void
    registerGeometrySetPyBinding()
    {
        using PyGeometrySetClass_t = bp::class_<rdl2::GeometrySet,
                                             std::shared_ptr<rdl2::GeometrySet>,
                                             bp::bases<rdl2::SceneObject>,
                                             boost::noncopyable>;

        PyGeometrySetClass_t("GeometrySet", bp::no_init)
            .def(bp::init<const rdl2::SceneClass&, const std::string&>(
                    (bp::arg("sceneClass"), bp::arg("name")) ))

            .def("declare",
                 &rdl2::GeometrySet::declare,
                 bp::arg("sceneClass"))
            .staticmethod("declare")

            .def("getSceneClass",
                 &rdl2::GeometrySet::getSceneClass,
                 bp::return_internal_reference<>(),
                 "Retrieves a the SceneClass to which this SceneObject belongs.")

            .def("getName",
                 &rdl2::GeometrySet::getName,
                 bp::return_value_policy<bp::copy_const_reference>(),
                 "Retrieves the name of this SceneObject.")

            .def("getType",
                 &rdl2::GeometrySet::getType,
                 "Retrieves the object type bitmask. This value may not be one of the enum "
                 "options, but rather a bitwise combination of them, so you'll need to use "
                 "bitwise operators to check for a specific interface.")

            .def("resetToDefault",
                 ( void (rdl2::GeometrySet::*) (const std::string&) ) &rdl2::GeometrySet::resetToDefault,
                 bp::arg("name"),
                 "Convenience function to reset an attribute value to its default value by name rather "
                 "than by AttributeKey. If no default value is supplied by the SceneClass, a "
                 "reasonable default is supplied for you (0, empty string, null, etc.)"
                 "\n"
                 "Inputs:    name    The name of an attribute which you want to reset to its default value.")

            .def("resetAllToDefault",
                 &rdl2::GeometrySet::resetAllToDefault,
                 "Resets all attributes in the SceneObject to their default values. If no default value "
                 "is supplied for an attribute by the SceneClass, a reasonable default is supplied for "
                 "you (0, empty string, null, etc.)")

            .def("isStatic",
                 &rdl2::GeometrySet::isStatic,
                 "Returns true if all Geometry objects in the set are themselves static.")

            .def("getGeometries",
                 &PyGeometrySet_getGeometries,
                 "Retrieves the set of unique Geometry in this GeometrySet.")

            .def("add",
                 &PyGeometrySet_add,
                 bp::arg("geometry"),
                 "Adds the given Geometry to the GeometrySet, if it is not already "
                 "a member of the set. If it is already a member of the set, this does nothing. \n"
                 "\n"
                 "** Attention Python Users ** \n"
                 "Currently do not assume using this method is safe; this may or may not properly "
                 "increment the Geometry object's ref count. \n"
                 "\n"
                 "Input:    geometry    The Geometry to add to the GeometrySet.")

            .def("remove",
                 &PyGeometrySet_remove,
                 bp::arg("geometry"),
                 "Removes the given Geometry from the GeometrySet, if it is already a member of "
                 "the set. If it is not a member of the set, this does nothing. \n"
                 "\n"
                 "Input:    geometry    The Geometry to remove from the GeometrySet.")

            .def("contains",
                 &rdl2::GeometrySet::contains,
                 bp::arg("geometry"),
                 "Returns true if the given Geometry is a member of the GeometrySet. There's no need to "
                 "call this before calling add() or remove(), as they will gracefully handle those edge cases. \n"
                 "\n"
                 "Inputs:    geometry    The Geometry to check for membership. \n"
                 "Returns True if the geometry is a member of the GeometrySet.")

            .def("clear",
                 &rdl2::GeometrySet::clear,
                 "Completely empties the GeometrySet so that it doesn't contain anything.")
            ;
    }

} // namespace py_scene_rdl2

