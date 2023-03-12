// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "boost_python.h"
#include "py_scene_rdl2.h"

#include <scene_rdl2/scene/rdl2/Types.h>
using namespace scene_rdl2;

namespace py_scene_rdl2
{
    //------------------------------------
    // Enums
    //------------------------------------

    void
    registerSceneRdl2EnumsPyBinding()
    {
        bp::enum_<rdl2::AttributeType>("AttributeType",
                                       "Runtime values for all the attribute types we support. These are "
                                       "used as a fallback when we can't do compile time type checking.")
                .value("UNKNOWN_VEC",  rdl2::AttributeType::TYPE_UNKNOWN)
                .value("BOOL",         rdl2::AttributeType::TYPE_BOOL)
                .value("INT",          rdl2::AttributeType::TYPE_INT)
                .value("LONG",         rdl2::AttributeType::TYPE_LONG)
                .value("FLOAT",        rdl2::AttributeType::TYPE_FLOAT)
                .value("DOUBLE",       rdl2::AttributeType::TYPE_DOUBLE)
                .value("STRING",       rdl2::AttributeType::TYPE_STRING)
                .value("RGB",          rdl2::AttributeType::TYPE_RGB)
                .value("RGBA",         rdl2::AttributeType::TYPE_RGBA)
                .value("VEC2F",        rdl2::AttributeType::TYPE_VEC2F)
                .value("VEC2D",        rdl2::AttributeType::TYPE_VEC2D)
                .value("VEC3F",        rdl2::AttributeType::TYPE_VEC3F)
                .value("VEC3D",        rdl2::AttributeType::TYPE_VEC3D)
                .value("VEC4F",        rdl2::AttributeType::TYPE_VEC4F)
                .value("VEC4D",        rdl2::AttributeType::TYPE_VEC4D)
                .value("MAT4F",        rdl2::AttributeType::TYPE_MAT4F)
                .value("MAT4D",        rdl2::AttributeType::TYPE_MAT4D)
                .value("SCENE_OBJECT", rdl2::AttributeType::TYPE_SCENE_OBJECT)

                .value("BOOL_VECTOR",            rdl2::AttributeType::TYPE_BOOL_VECTOR)
                .value("INT_VECTOR",             rdl2::AttributeType::TYPE_INT_VECTOR)
                .value("LONG_VECTOR",            rdl2::AttributeType::TYPE_LONG_VECTOR)
                .value("FLOAT_VECTOR",           rdl2::AttributeType::TYPE_FLOAT_VECTOR)
                .value("DOUBLE_VECTOR",          rdl2::AttributeType::TYPE_DOUBLE_VECTOR)
                .value("STRING_VECTOR",          rdl2::AttributeType::TYPE_STRING_VECTOR)
                .value("RGB_VECTOR",             rdl2::AttributeType::TYPE_RGB_VECTOR)
                .value("RGBA_VECTOR",            rdl2::AttributeType::TYPE_RGBA_VECTOR)
                .value("VEC2F_VECTOR",           rdl2::AttributeType::TYPE_VEC2F_VECTOR)
                .value("VEC2D_VECTOR",           rdl2::AttributeType::TYPE_VEC2D_VECTOR)
                .value("VEC3F_VECTOR",           rdl2::AttributeType::TYPE_VEC3F_VECTOR)
                .value("VEC3D_VECTOR",           rdl2::AttributeType::TYPE_VEC3D_VECTOR)
                .value("VEC4F_VECTOR",           rdl2::AttributeType::TYPE_VEC4F_VECTOR)
                .value("VEC4D_VECTOR",           rdl2::AttributeType::TYPE_VEC4D_VECTOR)
                .value("MAT4F_VECTOR",           rdl2::AttributeType::TYPE_MAT4F_VECTOR)
                .value("MAT4D_VECTOR",           rdl2::AttributeType::TYPE_MAT4D_VECTOR)
                .value("SCENE_OBJECT_VECTOR",    rdl2::AttributeType::TYPE_SCENE_OBJECT_VECTOR)
                .value("SCENE_OBJECT_INDEXABLE", rdl2::AttributeType::TYPE_SCENE_OBJECT_INDEXABLE);

        bp::enum_<rdl2::SceneObjectInterface>("SceneObjectInterface",
                                              "NOTE: *BIT MASKS* representing various SceneObject hierarchy interfaces.")
                .value("GENERIC", rdl2::SceneObjectInterface::INTERFACE_GENERIC)
                .value("GEOMETRYSET", rdl2::SceneObjectInterface::INTERFACE_GEOMETRYSET)
                .value("LAYER", rdl2::SceneObjectInterface::INTERFACE_LAYER)
                .value("LIGHTSET", rdl2::SceneObjectInterface::INTERFACE_LIGHTSET)
                .value("NODE", rdl2::SceneObjectInterface::INTERFACE_NODE)
                .value("CAMERA", rdl2::SceneObjectInterface::INTERFACE_CAMERA)
                .value("ENVMAP", rdl2::SceneObjectInterface::INTERFACE_ENVMAP)
                .value("GEOMETRY", rdl2::SceneObjectInterface::INTERFACE_GEOMETRY)
                .value("LIGHT", rdl2::SceneObjectInterface::INTERFACE_LIGHT)
                .value("SHADER", rdl2::SceneObjectInterface::INTERFACE_SHADER)
                .value("DISPLACEMENT", rdl2::SceneObjectInterface::INTERFACE_DISPLACEMENT)
                .value("MAP", rdl2::SceneObjectInterface::INTERFACE_MAP)
                .value("ROOTSHADER", rdl2::SceneObjectInterface::INTERFACE_ROOTSHADER)
                .value("MATERIAL", rdl2::SceneObjectInterface::INTERFACE_MATERIAL)
                .value("VOLUMESHADER", rdl2::SceneObjectInterface::INTERFACE_VOLUMESHADER)
                .value("RENDEROUTPUT", rdl2::SceneObjectInterface::INTERFACE_RENDEROUTPUT)
                .value("USERDATA", rdl2::SceneObjectInterface::INTERFACE_USERDATA)
                .value("DWABASELAYERABLE", rdl2::SceneObjectInterface::INTERFACE_DWABASELAYERABLE)
                .value("METADATA", rdl2::SceneObjectInterface::INTERFACE_METADATA)
                .value("LIGHTFILTER", rdl2::SceneObjectInterface::INTERFACE_LIGHTFILTER)
                .value("TRACESET", rdl2::SceneObjectInterface::INTERFACE_TRACESET)
                .value("JOINT", rdl2::SceneObjectInterface::INTERFACE_JOINT)
                .value("LIGHTFILTERSET", rdl2::SceneObjectInterface::INTERFACE_LIGHTFILTERSET);

        bp::enum_<rdl2::AttributeFlags>("AttributeFlags",
                "Defines bitflags that affect the behavior of attributes. \n"
                 "\n"
                 "The 'bindable' flag indicates that an attribute may have a binding registered in addition "
                 "to having a value. Client code must decide what to do with the bound object. RDL does "
                 "not know how to 'evaluate' these bindings.\n"
                 "\n"
                 "The 'blurrable' flag indicates that an attribute has multiple values, one at each "
                 "timestep defined by the AttributeTimestep enum.\n"
                 "\n"
                 "The 'enumerable' flag indicates that an attribute can only take on a fixed number of "
                 "defined values.")
                .value("NONE", rdl2::AttributeFlags::FLAGS_NONE)
                .value("BINDABLE", rdl2::AttributeFlags::FLAGS_BINDABLE)
                .value("BLURRABLE", rdl2::AttributeFlags::FLAGS_BLURRABLE)
                .value("ENUMERABLE", rdl2::AttributeFlags::FLAGS_ENUMERABLE)
                .value("FILENAME", rdl2::AttributeFlags::FLAGS_FILENAME);
    }
} // namespace py_scene_rdl2

