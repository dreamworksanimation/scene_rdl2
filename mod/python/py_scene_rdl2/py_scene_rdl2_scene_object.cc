// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "boost_python.h"
#include "py_scene_rdl2.h"
#include "py_scene_rdl2_helpers.h"

// scene_rdl2
#include <scene_rdl2/scene/rdl2/SceneObject.h>
#include <scene_rdl2/scene/rdl2/Camera.h>
#include <scene_rdl2/scene/rdl2/Displacement.h>
#include <scene_rdl2/scene/rdl2/EnvMap.h>
#include <scene_rdl2/scene/rdl2/Geometry.h>
#include <scene_rdl2/scene/rdl2/GeometrySet.h>
#include <scene_rdl2/scene/rdl2/Layer.h>
#include <scene_rdl2/scene/rdl2/Light.h>
#include <scene_rdl2/scene/rdl2/LightFilter.h>
#include <scene_rdl2/scene/rdl2/LightSet.h>
#include <scene_rdl2/scene/rdl2/Map.h>
#include <scene_rdl2/scene/rdl2/Material.h>
#include <scene_rdl2/scene/rdl2/Metadata.h>
#include <scene_rdl2/scene/rdl2/Node.h>
#include <scene_rdl2/scene/rdl2/RenderOutput.h>
#include <scene_rdl2/scene/rdl2/RootShader.h>
#include <scene_rdl2/scene/rdl2/Shader.h>
#include <scene_rdl2/scene/rdl2/UserData.h>
#include <scene_rdl2/scene/rdl2/VolumeShader.h>

#include <scene_rdl2/scene/rdl2/Proxies.h>
using namespace scene_rdl2;

namespace py_scene_rdl2
{
    //------------------------------------
    // rdl2::SceneObject
    //------------------------------------

    rdl2::Displacement*
    PySceneObject_toDisplacement(rdl2::SceneObject& self)
    {
        return self.isA<rdl2::Displacement>() ? static_cast<rdl2::Displacement*>(&self) : nullptr;
    }

    rdl2::Map*
    PySceneObject_toMap(rdl2::SceneObject& self)
    {
        return self.isA<rdl2::Map>() ? static_cast<rdl2::Map*>(&self) : nullptr;
    }

    rdl2::Metadata*
    PySceneObject_toMetadata(rdl2::SceneObject& self)
    {
        return self.isA<rdl2::Metadata>() ? static_cast<rdl2::Metadata*>(&self) : nullptr;
    }

    rdl2::UserData*
    PySceneObject_toUserData(rdl2::SceneObject& self)
    {
        return self.isA<rdl2::UserData>() ? static_cast<rdl2::UserData*>(&self) : nullptr;
    }

    rdl2::RenderOutput*
    PySceneObject_toRenderOutput(rdl2::SceneObject& self)
    {
        return self.isA<rdl2::RenderOutput>() ? static_cast<rdl2::RenderOutput*>(&self) : nullptr;
    }

    rdl2::Node*
    PySceneObject_toNode(rdl2::SceneObject& self)
    {
        return self.isA<rdl2::Node>() ? static_cast<rdl2::Node*>(&self) : nullptr;
    }

    rdl2::EnvMap*
    PySceneObject_toEnvMap(rdl2::SceneObject& self)
    {
        return self.isA<rdl2::EnvMap>() ? static_cast<rdl2::EnvMap*>(&self) : nullptr;
    }

    rdl2::Material*
    PySceneObject_toMaterial(rdl2::SceneObject& self)
    {
        return self.isA<rdl2::Material>() ? static_cast<rdl2::Material*>(&self) : nullptr;
    }

    rdl2::Light*
    PySceneObject_toLight(rdl2::SceneObject& self)
    {
        return self.isA<rdl2::Light>() ? static_cast<rdl2::Light*>(&self) : nullptr;
    }

    rdl2::LightSet*
    PySceneObject_toLightSet(rdl2::SceneObject& self)
    {
        return self.isA<rdl2::LightSet>() ? static_cast<rdl2::LightSet*>(&self) : nullptr;
    }

    rdl2::LightFilter*
    PySceneObject_toLightFilter(rdl2::SceneObject& self)
    {
        return self.isA<rdl2::LightFilter>() ? static_cast<rdl2::LightFilter*>(&self) : nullptr;
    }

    rdl2::Geometry*
    PySceneObject_toGeometry(rdl2::SceneObject& self)
    {
        return self.isA<rdl2::Geometry>() ? static_cast<rdl2::Geometry*>(&self) : nullptr;
    }

    rdl2::Camera*
    PySceneObject_toCamera(rdl2::SceneObject& self)
    {
        return self.isA<rdl2::Camera>() ? static_cast<rdl2::Camera*>(&self) : nullptr;
    }

    rdl2::Layer*
    PySceneObject_toLayer(rdl2::SceneObject& self)
    {
        return self.isA<rdl2::Layer>() ? static_cast<rdl2::Layer*>(&self) : nullptr;
    }

    rdl2::GeometrySet*
    PySceneObject_toGeometrySet(rdl2::SceneObject& self)
    {
        return self.isA<rdl2::GeometrySet>() ? static_cast<rdl2::GeometrySet*>(&self) : nullptr;
    }

    bp::list
    PySceneObject_getAttributeGroupNames(rdl2::SceneObject& self)
    {
        const rdl2::SceneClass& sc = self.getSceneClass();
        return getAttributeGroupNames(const_cast<rdl2::SceneClass&>(sc));
    }

    const rdl2::Attribute*
    PySceneObject_getAttributeFromGroup(rdl2::SceneObject& self, const std::string& groupName, int i)
    {
        const rdl2::SceneClass& sc = self.getSceneClass();
        return getAttributeFromGroup(const_cast<rdl2::SceneClass&>(sc), groupName, i);
    }

    std::size_t
    PySceneObject_getAttributeGroupSize(rdl2::SceneObject& self, const std::string& groupName)
    {
        const rdl2::SceneClass& sc = self.getSceneClass();
        return getAttributeGroupSize(const_cast<rdl2::SceneClass&>(sc), groupName);
    }

    std::size_t
    PySceneObject_getAttributeCount(rdl2::SceneObject& self)
    {
        const rdl2::SceneClass& sc = self.getSceneClass();
        return getAttributeCount(const_cast<rdl2::SceneClass&>(sc));
    }

    bp::list
    PySceneObject_getAttributeNames(rdl2::SceneObject& self)
    {
        const rdl2::SceneClass& sc = self.getSceneClass();
        return getAttributeNames(const_cast<rdl2::SceneClass&>(sc));
    }

    bp::dict
    PySceneObject_getAttributeNamesAndIndices(rdl2::SceneObject& self)
    {
        const rdl2::SceneClass& sc = self.getSceneClass();
        return getAttributeNamesAndIndices(const_cast<rdl2::SceneClass&>(sc));
    }

    const rdl2::Attribute*
    PySceneObject_getAttributeAt(rdl2::SceneObject& self, int index)
    {
        const rdl2::SceneClass& sc = self.getSceneClass();
        return getAttributeAt(const_cast<rdl2::SceneClass&>(sc), index);
    }

    bp::dict
    PySceneObject_getAttributeNamesAndTypes(rdl2::SceneObject& self)
    {
        const rdl2::SceneClass& sc = self.getSceneClass();
        return getAttributeNamesAndTypes(const_cast<rdl2::SceneClass&>(sc));
    }

    void
    registerSceneObjectPyBinding()
    {
        using PySceneObjectClass_t = bp::class_<rdl2::SceneObject,
                                                std::shared_ptr<rdl2::SceneObject>,
                                                boost::noncopyable>;

        PySceneObjectClass_t("SceneObject", bp::no_init)

            .def("getSceneClass",
                 &rdl2::SceneObject::getSceneClass,
                 bp::return_internal_reference<>(),
                 "Retrieves a the SceneClass to which this SceneObject belongs.")

            .def("getName",
                 &rdl2::SceneObject::getName,
                 bp::return_value_policy<bp::copy_const_reference>(),
                 "Retrieves the name of this SceneObject.")

            .def("getType",
                 &rdl2::SceneObject::getType,
                 "Retrieves the object type bitmask. This value may not be one of the enum "
                 "options, but rather a bitwise combination of them, so you'll need to use "
                 "bitwise operators to check for a specific interface.")

            .def("getTypeName",
                 &getSceneObjectTypeName,
                 "Retrieves the object type name as a string.")

            .def("resetToDefault",
                 ( void (rdl2::SceneObject::*) (const std::string&) ) &rdl2::SceneObject::resetToDefault,
                 bp::arg("name"),
                 "Convenience function to reset an attribute value to its default value by name rather "
                 "than by AttributeKey. If no default value is supplied by the SceneClass, a "
                 "reasonable default is supplied for you (0, empty string, null, etc.)"
                 "\n"
                 "Inputs:    name    The name of an attribute which you want to reset to its default value.")

            .def("resetAllToDefault",
                 &rdl2::SceneObject::resetAllToDefault,
                 "Resets all attributes in the SceneObject to their default values. If no default value "
                 "is supplied for an attribute by the SceneClass, a reasonable default is supplied for "
                 "you (0, empty string, null, etc.)")

            //------------------------------------------------
            // Get information on class Attributes

            .def("getAttributeGroupNames",
                 &PySceneObject_getAttributeGroupNames,
                 "WRITE HELP LATER")

            .def("getAttributeGroupSize",
                 &PySceneObject_getAttributeGroupSize,
                 bp::arg("groupName"),
                 "WRITE HELP LATER")

            .def("getAttributeFromGroup",
                 &PySceneObject_getAttributeFromGroup,
                 ( bp::arg("groupName"), bp::arg("index") ),
                 bp::return_internal_reference<>(),
                 "WRITE HELP LATER")

            .def("getAttributeCount",
                 &PySceneObject_getAttributeCount,
                 "WRITE HELP LATER")

            .def("getAttributeAt",
                 &PySceneObject_getAttributeAt,
                 bp::arg("index"),
                 bp::return_internal_reference<>(),
                 "WRITE HELP LATER")

            .def("getAttributeNamesAndIndices",
                 &PySceneObject_getAttributeNamesAndIndices,
                 "WRITE HELP LATER")

            .def("getAttributeNames",
                 &PySceneObject_getAttributeNames,
                 "WRITE HELP LATER")

            //------------------------------------------------
            // Get Attribute values

            .def("getAttributeNamesAndTypes",
                 &PySceneObject_getAttributeNamesAndTypes,
                 "(Python Only) Returns a dictionary containing all Attribute names and their rdl2 types.")

            .def("get",
                 &getAttributeValueByName,
                 bp::arg("attrName"),
                 "WRITE HELP LATER")

             //------------------------------------------------
             // Set Attribute values

            .def("set",
                 &extractAndSetAttributeValue,
                 (bp::arg("attrName"), bp::arg("attrValue")),
                 "WRITE HELP LATER")

            //------------------------------------------------
            // Downcasting to derived types:

            .def("toDisplacement",
                 &PySceneObject_toDisplacement,
                 bp::return_internal_reference<>(),
                 "Downcast SceneObject to scene_rdl2.Displacement, only if this object is of type "
                 "scene_rdl2.SceneObjectInterface.DISPLACEMENT (1024).")

            .def("toMap",
                 &PySceneObject_toMap,
                 bp::return_internal_reference<>(),
                 "Downcast SceneObject to scene_rdl2.Map, only if this object is of type "
                 "scene_rdl2.SceneObjectInterface.MAP (2048).")

            .def("toMetadata",
                 &PySceneObject_toMetadata,
                 bp::return_internal_reference<>(),
                 "Downcast SceneObject to scene_rdl2.Metadata, only if this object is of type "
                 "scene_rdl2.SceneObjectInterface.METADATA (262144).")

            .def("toUserData",
                 &PySceneObject_toUserData,
                 bp::return_internal_reference<>(),
                 "Downcast SceneObject to scene_rdl2.UserData, only if this object is of type "
                 "scene_rdl2.SceneObjectInterface.USERDATA (65536).")

            .def("toRenderOutput",
                 &PySceneObject_toRenderOutput,
                 bp::return_internal_reference<>(),
                 "Downcast SceneObject to scene_rdl2.RenderOutput, only if this object is of type "
                 "scene_rdl2.SceneObjectInterface.RENDEROUTPUT (32768).")

            .def("toNode",
                 &PySceneObject_toNode,
                 bp::return_internal_reference<>(),
                 "Downcast SceneObject to scene_rdl2.Node, only if this object is of type "
                 "scene_rdl2.SceneObjectInterface.NODE (16).")

            .def("toEnvMap",
                 &PySceneObject_toEnvMap,
                 bp::return_internal_reference<>(),
                 "Downcast SceneObject to scene_rdl2.EnvMap, only if this object is of type "
                 "scene_rdl2.SceneObjectInterface.ENVMAP (64).")

            .def("toMaterial",
                 &PySceneObject_toMaterial,
                 bp::return_internal_reference<>(),
                 "Downcast SceneObject to scene_rdl2.Material, only if this object is of type "
                 "scene_rdl2.SceneObjectInterface.MATERIAL (8192).")

            .def("toLight",
                 &PySceneObject_toLight,
                 bp::return_internal_reference<>(),
                 "Downcast SceneObject to scene_rdl2.Light, only if this object is of type "
                 "scene_rdl2.SceneObjectInterface.LIGHT (256).")

            .def("toLightSet",
                 &PySceneObject_toLightSet,
                 bp::return_internal_reference<>(),
                 "Downcast SceneObject to scene_rdl2.LightSet, only if this object is of type "
                 "scene_rdl2.SceneObjectInterface.LIGHTSET (8).")

            .def("toLightFilter",
                 &PySceneObject_toLightFilter,
                 bp::return_internal_reference<>(),
                 "Downcast SceneObject to scene_rdl2.LightFilter, only if this object is of type "
                 "scene_rdl2.SceneObjectInterface.LIGHT_FILTER (524288).")

            .def("toGeometry",
                 &PySceneObject_toGeometry,
                 bp::return_internal_reference<>(),
                 "Downcast SceneObject to scene_rdl2.Geometry, only if this object is of type "
                 "scene_rdl2.SceneObjectInterface.GEOMETRY (128).")

            .def("toCamera",
                 &PySceneObject_toCamera,
                 bp::return_internal_reference<>(),
                 "Downcast SceneObject to scene_rdl2.Camera, only if this object is of type "
                 "scene_rdl2.SceneObjectInterface.CAMERA (32).")

            .def("toLayer",
                 &PySceneObject_toLayer,
                 bp::return_internal_reference<>(),
                 "Downcast SceneObject to scene_rdl2.Layer, only if this object is of type "
                 "scene_rdl2.SceneObjectInterface.LAYER (4).")

            .def("toGeometrySet",
                 &PySceneObject_toGeometrySet,
                 bp::return_internal_reference<>(),
                 "Downcast SceneObject to scene_rdl2.GeometrySet, only if this object is of type "
                 "scene_rdl2.SceneObjectInterface.GEOMETRYSET (2).")
                 ;

        //
        bp::class_<scene_rdl2::rdl2::SceneObjectVector>(
                "SceneObjectVector", "Array of SceneObject references (std::vector<rdl2::SceneObject*>).")
            .def(bp::vector_indexing_suite<scene_rdl2::rdl2::SceneObjectVector>());
    }

} // namespace py_scene_rdl2

