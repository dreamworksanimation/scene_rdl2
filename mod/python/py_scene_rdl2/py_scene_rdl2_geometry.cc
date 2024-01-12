// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "boost_python.h"
#include "py_scene_rdl2.h"

// scene_rdl2
#include <scene_rdl2/scene/rdl2/Attribute.h>
#include <scene_rdl2/scene/rdl2/Proxies.h>
#include <scene_rdl2/scene/rdl2/Geometry.h>
#include <scene_rdl2/scene/rdl2/GeometrySet.h>
#include <scene_rdl2/scene/rdl2/SceneObject.h>
using namespace scene_rdl2;

namespace py_scene_rdl2
{
    void
    registerGeometryBasePyBinding()
    {
        using PyGeometryClass_t = bp::class_<PyGeometry,
                                             std::shared_ptr<PyGeometry>,
                                             bp::bases<rdl2::Node>,
                                             boost::noncopyable>;

        bp::scope GeometryScope = PyGeometryClass_t("Geometry", bp::no_init)
            .def(bp::init<const rdl2::SceneClass&, const std::string&>(
                    ( bp::arg("sceneClass"), bp::arg("name") )))

            .def("declare",
                 &PyGeometry::declare,
                 bp::arg("sceneClass"))
            .staticmethod("declare")

            .def("getSceneClass",
                 &PyGeometry::getSceneClass,
                 bp::return_internal_reference<>(),
                 "Retrieves a the SceneClass to which this SceneObject belongs.")

            .def("getName",
                 &PyGeometry::getName,
                 bp::return_value_policy<bp::copy_const_reference>(),
                 "Retrieves the name of this SceneObject.")

            .def("getType",
                 &PyGeometry::getType,
                 "Retrieves the object type bitmask. This value may not be one of the enum "
                 "options, but rather a bitwise combination of them, so you'll need to use "
                 "bitwise operators to check for a specific interface.")

            .def("resetToDefault",
                 ( void (PyGeometry::*) (const std::string&) ) &PyGeometry::resetToDefault,
                 bp::arg("name"),
                 "Convenience function to reset an attribute value to its default value by name rather "
                 "than by AttributeKey. If no default value is supplied by the SceneClass, a "
                 "reasonable default is supplied for you (0, empty string, null, etc.)"
                 "\n"
                 "Inputs:    name    The name of an attribute which you want to reset to its default value.")

            .def("resetAllToDefault",
                 &PyGeometry::resetAllToDefault,
                 "Resets all attributes in the SceneObject to their default values. If no default value "
                 "is supplied for an attribute by the SceneClass, a reasonable default is supplied for "
                 "you (0, empty string, null, etc.)")

            .def("loadProcedural",
                 &PyGeometry::loadProcedural,
                 "Invokes createProcedural() and captures the returned procedural.")

            .def("unloadProcedural",
                 &PyGeometry::unloadProcedural,
                 "Destroy the loaded procedural.")

            .def("setRender2Object",
                 &PyGeometry::setRender2Object,
                 bp::arg("render2Object"),
                 "Set the render to object transform cache, This should be set by the renderer "
                 "during geometry update or creation.")

            .def("getRender2Object",
                 &PyGeometry::getRender2Object,
                 "Returns the render2Object transform cache set by the renderer.")

            .def("isStatic",
                 &PyGeometry::isStatic,
                 "Convenience function for checking if the Geometry is static.")

            .def("getSideType",
                 &PyGeometry::getSideType,
                 "Returns the sidedness of the mesh.")

            .def("getVisibilityMask",
                 &PyGeometry::getVisibilityMask,
                 "Returns the mesh visibility Mask.")
            ;

        // Should go under scene_rdl2.Geometry
        bp::enum_<rdl2::Geometry::SideType>("GeometrySideType")
            .value("TWO_SIDED",          rdl2::Geometry::SideType::TWO_SIDED)
            .value("SINGLE_SIDED",       rdl2::Geometry::SideType::SINGLE_SIDED)
            .value("MESH_DEFAULT_SIDED", rdl2::Geometry::SideType::MESH_DEFAULT_SIDED);
    }
} // namespace py_scene_rdl2

