// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "py_scene_rdl2_helpers.h"

#include <algorithm>
#include <iterator>
#include <sstream>
#include <vector>

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
#include <scene_rdl2/scene/rdl2/VolumeShader.h>

#include <scene_rdl2/common/math/Vec2.h>
#include <scene_rdl2/common/math/Vec3.h>
#include <scene_rdl2/common/math/Vec4.h>
#include <scene_rdl2/common/math/Col3.h>
#include <scene_rdl2/common/math/Col4.h>
#include <scene_rdl2/common/math/Mat3.h>
#include <scene_rdl2/common/math/Mat4.h>

#ifdef bool
#undef bool
#endif

#ifdef true
#undef true
#endif

#ifdef false
#undef false
#endif

namespace py_scene_rdl2
{

std::string
getSceneObjectTypeName(scene_rdl2::rdl2::SceneObject* sceneObject)
{
    std::ostringstream oss;
    oss << "GENERIC (SceneObject)";

    if (sceneObject->isA<scene_rdl2::rdl2::Node>()) {
        oss << " | NODE";

        if (sceneObject->isA<scene_rdl2::rdl2::Camera>()) {
            oss << " | CAMERA";
        }
        else if (sceneObject->isA<scene_rdl2::rdl2::EnvMap>()) {
            oss << " | ENVMAP";
        }
        else if (sceneObject->isA<scene_rdl2::rdl2::Geometry>()) {
            oss << " | GEOMETRY";
        }
        else if (sceneObject->isA<scene_rdl2::rdl2::Light>()) {
            oss << " | LIGHT";
        }
        else if (sceneObject->isA<scene_rdl2::rdl2::LightFilter>()) {
            oss << " | LIGHT_FILTER";
        }
        else {
            oss << " | UNKNOWN";
        }
    }
    else if (sceneObject->isA<scene_rdl2::rdl2::GeometrySet>()) {
        oss << " | GEOMETRYSET";
    }
    else if (sceneObject->isA<scene_rdl2::rdl2::Layer>()) {
        oss << " | LAYER";
    }
    else if (sceneObject->isA<scene_rdl2::rdl2::LightSet>()) {
        oss << " | LIGHTSET";
    }
    else if (sceneObject->isA<scene_rdl2::rdl2::Metadata>()) {
        oss << " | METADATA";
    }
    else if (sceneObject->isA<scene_rdl2::rdl2::RenderOutput>()) {
        oss << " | RENDEROUTPUT";
    }
    else if (sceneObject->isA<scene_rdl2::rdl2::UserData>()) {
        oss << " | USERDATA";
    }
    else if (sceneObject->isA<scene_rdl2::rdl2::Shader>()) {
        oss << " | SHADER";

        if (sceneObject->isA<scene_rdl2::rdl2::Map>()) {
            oss << " | MAP";
        }
        else if (sceneObject->isA<scene_rdl2::rdl2::RootShader>()) {
            oss << " | ROOTSHADER";

            if (sceneObject->isA<scene_rdl2::rdl2::Material>()) {
                oss << " | MATERIAL";
            }
            else if (sceneObject->isA<scene_rdl2::rdl2::VolumeShader>()) {
                oss << " | VOLUMESHADER";
            }
            else if (sceneObject->isA<scene_rdl2::rdl2::Displacement>()) {
                oss << " | DISPLACEMENT";
            }
            else {
                oss << " | UNKNOWN";
            }
        }
        else {
            oss << " | UNKNOWN";
        }
    }
    else {
        oss << " | UNKNOWN";
    }

    return oss.str();
}

bp::object
getAttributeValueByName(scene_rdl2::rdl2::SceneObject& sceneObject, const std::string& attrName)
{
    const scene_rdl2::rdl2::SceneClass& sc = sceneObject.getSceneClass();
    const scene_rdl2::rdl2::Attribute* attr = sc.getAttribute(attrName);

    if (checkType(attr, scene_rdl2::rdl2::AttributeType::TYPE_BOOL)) {
        return extractPrimitiveAttrValueAsPyObj<scene_rdl2::rdl2::Bool>(sceneObject, sc, attrName);
    }
    else if (checkType(attr, scene_rdl2::rdl2::AttributeType::TYPE_INT)) {
        return extractPrimitiveAttrValueAsPyObj<scene_rdl2::rdl2::Int>(sceneObject, sc, attrName);
    }
    else if (checkType(attr, scene_rdl2::rdl2::AttributeType::TYPE_LONG)) {
        return extractPrimitiveAttrValueAsPyObj<scene_rdl2::rdl2::Long>(sceneObject, sc, attrName);
    }
    else if (checkType(attr, scene_rdl2::rdl2::AttributeType::TYPE_FLOAT)) {
        return extractPrimitiveAttrValueAsPyObj<scene_rdl2::rdl2::Float>(sceneObject, sc, attrName);
    }
    else if (checkType(attr, scene_rdl2::rdl2::AttributeType::TYPE_DOUBLE)) {
        return extractPrimitiveAttrValueAsPyObj<scene_rdl2::rdl2::Double>(sceneObject, sc, attrName);
    }
    else if (checkType(attr, scene_rdl2::rdl2::AttributeType::TYPE_STRING)) {
        return extractPrimitiveAttrValueAsPyObj<scene_rdl2::rdl2::String>(sceneObject, sc, attrName);
    }
    else if (checkType(attr, scene_rdl2::rdl2::AttributeType::TYPE_RGB)) {
        return extractAttrValueAsPyObj<scene_rdl2::rdl2::Rgb>(sceneObject, sc, attrName);
    }
    else if (checkType(attr, scene_rdl2::rdl2::AttributeType::TYPE_RGBA)) {
        return extractAttrValueAsPyObj<scene_rdl2::rdl2::Rgba>(sceneObject, sc, attrName);
    }
    else if (checkType(attr, scene_rdl2::rdl2::AttributeType::TYPE_VEC2F)) {
        return extractAttrValueAsPyObj<scene_rdl2::rdl2::Vec2f>(sceneObject, sc, attrName);
    }
    else if (checkType(attr, scene_rdl2::rdl2::AttributeType::TYPE_VEC2D)) {
        return extractAttrValueAsPyObj<scene_rdl2::rdl2::Vec2d>(sceneObject, sc, attrName);
    }
    else if (checkType(attr, scene_rdl2::rdl2::AttributeType::TYPE_VEC3F)) {
        return extractAttrValueAsPyObj<scene_rdl2::rdl2::Vec3f>(sceneObject, sc, attrName);
    }
    else if (checkType(attr, scene_rdl2::rdl2::AttributeType::TYPE_VEC3D)) {
        return extractAttrValueAsPyObj<scene_rdl2::rdl2::Vec3d>(sceneObject, sc, attrName);
    }
    else if (checkType(attr, scene_rdl2::rdl2::AttributeType::TYPE_VEC4F)) {
        return extractAttrValueAsPyObj<scene_rdl2::rdl2::Vec4f>(sceneObject, sc, attrName);
    }
    else if (checkType(attr, scene_rdl2::rdl2::AttributeType::TYPE_VEC4D)) {
        return extractAttrValueAsPyObj<scene_rdl2::rdl2::Vec4d>(sceneObject, sc, attrName);
    }
    else if (checkType(attr, scene_rdl2::rdl2::AttributeType::TYPE_MAT4F)) {
        return extractAttrValueAsPyObj<scene_rdl2::rdl2::Mat4f>(sceneObject, sc, attrName);
    }
    else if (checkType(attr, scene_rdl2::rdl2::AttributeType::TYPE_MAT4D)) {
        return extractAttrValueAsPyObj<scene_rdl2::rdl2::Mat4d>(sceneObject, sc, attrName);
    }

    // Array types
    else if (checkType(attr, scene_rdl2::rdl2::AttributeType::TYPE_BOOL_VECTOR)) {

        return bp::object(
                BoolVectorWrapper(sceneObject.get<scene_rdl2::rdl2::BoolVector>(
                        sc.getAttributeKey<scene_rdl2::rdl2::BoolVector>(attrName))));
    }
    else if (checkType(attr, scene_rdl2::rdl2::AttributeType::TYPE_INT_VECTOR)) {
        return extractVectorAttrValueAsPyObj<scene_rdl2::rdl2::Int>(sceneObject, sc, attrName);
    }
    else if (checkType(attr, scene_rdl2::rdl2::AttributeType::TYPE_LONG_VECTOR)) {
        return extractVectorAttrValueAsPyObj<scene_rdl2::rdl2::Long>(sceneObject, sc, attrName);
    }
    else if (checkType(attr, scene_rdl2::rdl2::AttributeType::TYPE_FLOAT_VECTOR)) {
        return extractVectorAttrValueAsPyObj<scene_rdl2::rdl2::Float>(sceneObject, sc, attrName);
    }
    else if (checkType(attr, scene_rdl2::rdl2::AttributeType::TYPE_DOUBLE_VECTOR)) {
        return extractVectorAttrValueAsPyObj<scene_rdl2::rdl2::Double>(sceneObject, sc, attrName);
    }
    else if (checkType(attr, scene_rdl2::rdl2::AttributeType::TYPE_STRING_VECTOR)) {
        return extractVectorAttrValueAsPyObj<scene_rdl2::rdl2::String>(sceneObject, sc, attrName);
    }
    else if (checkType(attr, scene_rdl2::rdl2::AttributeType::TYPE_RGB_VECTOR)) {
        return extractVectorAttrValueAsPyObj<scene_rdl2::rdl2::Rgb>(sceneObject, sc, attrName);
    }
    else if (checkType(attr, scene_rdl2::rdl2::AttributeType::TYPE_RGBA_VECTOR)) {
        return extractVectorAttrValueAsPyObj<scene_rdl2::rdl2::Rgba>(sceneObject, sc, attrName);
    }
    else if (checkType(attr, scene_rdl2::rdl2::AttributeType::TYPE_VEC2F_VECTOR)) {
        return extractVectorAttrValueAsPyObj<scene_rdl2::rdl2::Vec2f>(sceneObject, sc, attrName);
    }
    else if (checkType(attr, scene_rdl2::rdl2::AttributeType::TYPE_VEC2D_VECTOR)) {
        return extractVectorAttrValueAsPyObj<scene_rdl2::rdl2::Vec2d>(sceneObject, sc, attrName);
    }
    else if (checkType(attr, scene_rdl2::rdl2::AttributeType::TYPE_VEC3F_VECTOR)) {
        return extractVectorAttrValueAsPyObj<scene_rdl2::rdl2::Vec3f>(sceneObject, sc, attrName);
    }
    else if (checkType(attr, scene_rdl2::rdl2::AttributeType::TYPE_VEC3D_VECTOR)) {
        return extractVectorAttrValueAsPyObj<scene_rdl2::rdl2::Vec3d>(sceneObject, sc, attrName);
    }
    else if (checkType(attr, scene_rdl2::rdl2::AttributeType::TYPE_VEC4F_VECTOR)) {
        return extractVectorAttrValueAsPyObj<scene_rdl2::rdl2::Vec4f>(sceneObject, sc, attrName);
    }
    else if (checkType(attr, scene_rdl2::rdl2::AttributeType::TYPE_VEC4D_VECTOR)) {
        return extractVectorAttrValueAsPyObj<scene_rdl2::rdl2::Vec4d>(sceneObject, sc, attrName);
    }
    else if (checkType(attr, scene_rdl2::rdl2::AttributeType::TYPE_MAT4F_VECTOR)) {
        return extractVectorAttrValueAsPyObj<scene_rdl2::rdl2::Mat4f>(sceneObject, sc, attrName);
    }
    else if (checkType(attr, scene_rdl2::rdl2::AttributeType::TYPE_MAT4D_VECTOR)) {
        return extractVectorAttrValueAsPyObj<scene_rdl2::rdl2::Mat4d>(sceneObject, sc, attrName);
    }
    else if (checkType(attr, scene_rdl2::rdl2::AttributeType::TYPE_SCENE_OBJECT_VECTOR)) {
        return bp::object{ SceneObjectVectorWrapper(
                            sceneObject.get<scene_rdl2::rdl2::SceneObjectVector>(
                                    sc.getAttributeKey<scene_rdl2::rdl2::SceneObjectVector>(attrName))) };
    }

    return bp::object{ };
}

std::string
getAttrTypeName(scene_rdl2::rdl2::AttributeType attrType)
{
    if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_BOOL) {
        return { "Bool" };
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_INT) {
        return { "Int" };
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_LONG) {
        return { "Long" };
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_FLOAT) {
        return { "Float" };
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_DOUBLE) {
        return { "Double" };
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_STRING) {
        return { "String" };
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_RGB) {
        return { "Rgb" };
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_RGBA) {
        return { "Rgba" };
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_VEC2F) {
        return { "Vec2f" };
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_VEC2D) {
        return { "Vec2d" };
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_VEC3F) {
        return { "Vec3f" };
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_VEC3D) {
        return { "Vec3d" };
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_VEC4F) {
        return { "Vec4f" };
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_VEC4D) {
        return { "Vec4d" };
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_MAT4F) {
        return { "Mat4f" };
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_MAT4D) {
        return { "Mat4d" };
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_SCENE_OBJECT) {
        return { "SceneObject" };
    }

    // Vector types
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_BOOL_VECTOR) {
        return { "BoolVector" };
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_INT_VECTOR) {
        return { "IntVector" };
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_LONG_VECTOR) {
        return { "LongVector" };
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_FLOAT_VECTOR) {
        return { "FloatVector" };
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_DOUBLE_VECTOR) {
        return { "DoubleVector" };
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_STRING_VECTOR) {
        return { "StringVector" };
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_RGB_VECTOR) {
        return { "RgbVector" };
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_RGBA_VECTOR) {
        return { "RgbaVector" };
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_VEC2F_VECTOR) {
        return { "Vec2fVector" };
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_VEC2D_VECTOR) {
        return { "Vec2dVector" };
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_VEC3F_VECTOR) {
        return { "Vec3fVector" };
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_VEC3D_VECTOR) {
        return { "Vec3dVector" };
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_VEC4F_VECTOR) {
        return { "Vec4fVector" };
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_VEC4D_VECTOR) {
        return { "Vec4dVector" };
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_MAT4F_VECTOR) {
        return { "Mat4fVector" };
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_MAT4D_VECTOR) {
        return { "Mat4dVector" };
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_SCENE_OBJECT_VECTOR) {
        return { "SceneObjectVector" };
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_SCENE_OBJECT_INDEXABLE) {
        return { "SceneObjectIndexable" };
    }

    return { "UNKNOWN" };
}

//-------------------------------------------------------------------------------------
//
// Set primitive types
//
//-------------------------------------------------------------------------------------

template <typename T>
void
internal_setPrimitiveAttrValue(scene_rdl2::rdl2::SceneObject& sceneObject,
                                 const scene_rdl2::rdl2::SceneClass& sc,
                                 const std::string& attrName,
                                 bp::object& pyValue)
{
    static_assert(
            (std::is_same<T, scene_rdl2::rdl2::Bool>::value ||
             std::is_same<T, scene_rdl2::rdl2::Int>::value ||
             std::is_same<T, scene_rdl2::rdl2::Long>::value ||
             std::is_same<T, scene_rdl2::rdl2::Float>::value ||
             std::is_same<T, scene_rdl2::rdl2::Double>::value ||
             std::is_same<T, scene_rdl2::rdl2::String>::value),
             "internal_setPrimitiveAttrValue<T> can only handle primitive "
             "types (bool, int, long, float, double) and strings.");

    scene_rdl2::rdl2::AttributeKey<T> attrKey = sc.getAttributeKey<T>(attrName);
    bool isValid = true;

    // Extract value from boost::python::object
    T value = static_cast<T>(bp::extract<T>(pyValue));

    // Set the value (NOTE: needs an UpdateGuard)
    if (isValid) {
        scene_rdl2::rdl2::SceneObject::UpdateGuard updateGuard(&sceneObject);
        sceneObject.set(attrKey, value);
    }
}

//-------------------------------------------------------------------------------------
//
// Set non-primitive types
//
//-------------------------------------------------------------------------------------

template <typename T>
void
internal_setVecAttrValue(scene_rdl2::rdl2::SceneObject& sceneObject,
                         const scene_rdl2::rdl2::SceneClass& sc,
                         const std::string& attrName,
                         bp::object& pyValue)
{
    static_assert(
        (std::is_same<T, scene_rdl2::math::Vec2i>::value ||
         std::is_same<T, scene_rdl2::rdl2::Vec2f>::value ||
         std::is_same<T, scene_rdl2::rdl2::Vec2d>::value ||
         std::is_same<T, scene_rdl2::rdl2::Vec3f>::value ||
         std::is_same<T, scene_rdl2::rdl2::Vec3d>::value ||
         std::is_same<T, scene_rdl2::rdl2::Vec4f>::value ||
         std::is_same<T, scene_rdl2::rdl2::Vec4d>::value ||
         std::is_same<T, scene_rdl2::rdl2::Rgb>::value ||
         std::is_same<T, scene_rdl2::rdl2::Rgba>::value),
         "internal_setVecAttrValue<T> cannot handle type T.");

    constexpr std::size_t T_size = getElementCount<T>();

    bool isValid = true;
    scene_rdl2::rdl2::AttributeKey<T> attrKey = sc.getAttributeKey<T>(attrName);
    T value { };

    // Extract value from boost::python::object
    if (PyList_CheckExact(pyValue.ptr()) == true) {
        bp::list pyList = bp::extract<bp::list>(pyValue);
        for (std::size_t idx = 0; idx < T_size; ++idx) {
            value[idx] = bp::extract<typename T::Scalar>(pyList[idx]);
        }
    }
    else if (PyTuple_CheckExact(pyValue.ptr()) == true) {
        bp::tuple pyTuple = bp::extract<bp::tuple>(pyValue);
        for (std::size_t idx = 0; idx < T_size; ++idx) {
            value[idx] = bp::extract<typename T::Scalar>(pyTuple[idx]);
        }
    }

    // Set the value (NOTE: needs an UpdateGuard)
    if (isValid) {
        scene_rdl2::rdl2::SceneObject::UpdateGuard updateGuard(&sceneObject);
        sceneObject.set(attrKey, value);
    }
}

//-------------------------------------------------------------------------------------
//
// Set matrix types
//
//-------------------------------------------------------------------------------------

template <typename T>
void
internal_setMatrixAttrValue(scene_rdl2::rdl2::SceneObject& sceneObject,
                                 const scene_rdl2::rdl2::SceneClass& sc,
                                 const std::string& attrName,
                                 bp::object& pyValue)
{
    static_assert(
        (std::is_same<T, scene_rdl2::math::Mat3f>::value ||
         std::is_same<T, scene_rdl2::math::Mat3d>::value ||
         std::is_same<T, scene_rdl2::rdl2::Mat4f>::value ||
         std::is_same<T, scene_rdl2::rdl2::Mat4d>::value),
          "internal_setMatrixVectorAttrValue<T> can only handle matrices");

    constexpr std::size_t dimension = getMatrixDimension<T>();

    bool isValid = true;
    scene_rdl2::rdl2::AttributeKey<T> attrKey = sc.getAttributeKey<T>(attrName);
    T value { };

    // Extract value from boost::python::object
    if (PyList_CheckExact(pyValue.ptr()) == true) {
        bp::list pyList = bp::extract<bp::list>(pyValue);
        for (std::size_t row = 0, idx = 0; row < dimension; ++row) {
            for (std::size_t col = 0; col < dimension; ++col) {
                value[row][col] =
                        bp::extract<typename T::Vector::Scalar>(pyList[idx]);
                ++idx;
            }
        }
    }
    else if (PyTuple_CheckExact(pyValue.ptr()) == true) {
        bp::tuple pyTuple = bp::extract<bp::tuple>(pyValue);
        for (std::size_t row = 0, idx = 0; row < dimension; ++row) {
            for (std::size_t col = 0; col < dimension; ++col) {
                value[row][col] =
                        bp::extract<typename T::Vector::Scalar>(pyTuple[idx]);
                ++idx;
            }
        }
    }

    // Set the value (NOTE: needs an UpdateGuard)
    if (isValid) {
        scene_rdl2::rdl2::SceneObject::UpdateGuard updateGuard(&sceneObject);
        sceneObject.set(attrKey, value);
    }
}

//-------------------------------------------------------------------------------------
//
// Special case: SceneObject*
//
//-------------------------------------------------------------------------------------

void
internal_setSceneObjectAttrValue(scene_rdl2::rdl2::SceneObject& sceneObject,
                                        const scene_rdl2::rdl2::SceneClass& sc,
                                        const std::string& attrName,
                                        bp::object& pyValue)
{
    scene_rdl2::rdl2::AttributeKey<scene_rdl2::rdl2::SceneObject*> attrKey =
            sc.getAttributeKey<scene_rdl2::rdl2::SceneObject*>(attrName);

    // Extract value from boost::python::object
    scene_rdl2::rdl2::SceneObject* value = bp::extract<scene_rdl2::rdl2::SceneObject*>(pyValue);

    // Set the value (NOTE: needs an UpdateGuard)
    {
        scene_rdl2::rdl2::SceneObject::UpdateGuard updateGuard(&sceneObject);
        sceneObject.set(attrKey, value);
    }
}

//-------------------------------------------------------------------------------------
//
// Special case: vector SceneObject*
//
//-------------------------------------------------------------------------------------

void
internal_setSceneObjectVectorAttrValue(scene_rdl2::rdl2::SceneObject& sceneObject,
                                       const scene_rdl2::rdl2::SceneClass& sc,
                                       const std::string& attrName,
                                       bp::object& pyValue)
{
    scene_rdl2::rdl2::AttributeKey<std::vector<scene_rdl2::rdl2::SceneObject*>> attrKey =
            sc.getAttributeKey<std::vector<scene_rdl2::rdl2::SceneObject*>>(attrName);

    std::vector<scene_rdl2::rdl2::SceneObject*> value;

    // Extract value from boost::python::object
    if (PyList_CheckExact(pyValue.ptr()) == true) {
        bp::list pyList = bp::extract<bp::list>(pyValue);
        value = conversions::PySceneObjectContainerToStdVector(pyList);
    }
    else if (PyTuple_CheckExact(pyValue.ptr()) == true) {
        bp::tuple pyTuple = bp::extract<bp::tuple>(pyValue);
        value = conversions::PySceneObjectContainerToStdVector(pyTuple);
    }
    else {
        throw std::runtime_error("in internal_setSceneObjectVectorAttrValue<T>, "
                "Python object passed in must be either a list or a tuple.");
    }

    // Set the value (NOTE: needs an UpdateGuard)
    {
        scene_rdl2::rdl2::SceneObject::UpdateGuard updateGuard(&sceneObject);
        sceneObject.set(attrKey, value);
    }
}

//-------------------------------------------------------------------------------------
//
// Special case for rdl2::BoolVector
//
//-------------------------------------------------------------------------------------

void
internal_setBoolVectorAttrValue(scene_rdl2::rdl2::SceneObject& sceneObject,
                                const scene_rdl2::rdl2::SceneClass& sc,
                                const std::string& attrName,
                                bp::object& pyValue)
{
    scene_rdl2::rdl2::AttributeKey<std::deque<scene_rdl2::rdl2::Bool>> attrKey =
            sc.getAttributeKey<std::deque<scene_rdl2::rdl2::Bool>>(attrName);

    bool isValid = true;
    std::deque<scene_rdl2::rdl2::Bool> value;

    // Extract value from boost::python::object
    if (PyList_CheckExact(pyValue.ptr()) == true) {
        bp::list pyList = bp::extract<bp::list>(pyValue);
        value = conversions::PyContainerToStdDeque<scene_rdl2::rdl2::Bool>(pyList);
    }
    else if (PyTuple_CheckExact(pyValue.ptr()) == true) {
        bp::tuple pyTuple = bp::extract<bp::tuple>(pyValue);
        value = conversions::PyContainerToStdDeque<scene_rdl2::rdl2::Bool>(pyTuple);
    }

    // Set the value (NOTE: needs an UpdateGuard)
    if (isValid) {
        scene_rdl2::rdl2::SceneObject::UpdateGuard updateGuard(&sceneObject);
        sceneObject.set(attrKey, value);
    }
}

//-------------------------------------------------------------------------------------
//
// Set arrays of primitives (int, long, float, double, string)
//
//-------------------------------------------------------------------------------------

template <typename T>
void
internal_setPrimitiveVectorAttrValue(scene_rdl2::rdl2::SceneObject& sceneObject,
                                     const scene_rdl2::rdl2::SceneClass& sc,
                                     const std::string& attrName,
                                     bp::object& pyValue)
{
    static_assert(
            (std::is_same<T, scene_rdl2::rdl2::Int>::value ||
             std::is_same<T, scene_rdl2::rdl2::Long>::value ||
             std::is_same<T, scene_rdl2::rdl2::Float>::value ||
             std::is_same<T, scene_rdl2::rdl2::Double>::value ||
             std::is_same<T, scene_rdl2::rdl2::String>::value),
             "internal_setPrimitiveVectorAttrValue<T> can only handle primitive "
             "types (int, long, float, double) and strings.");

    scene_rdl2::rdl2::AttributeKey<std::vector<T>> attrKey =
            sc.getAttributeKey<std::vector<T>>(attrName);

    bool isValid = true;
    std::vector<T> value;

    // Extract value from Python object
    if (PyList_CheckExact(pyValue.ptr()) == true) {
        bp::list pyList = bp::extract<bp::list>(pyValue);
        value = conversions::PyPrimitiveContainerToStdVector<T>(pyList);
    }
    else if (PyTuple_CheckExact(pyValue.ptr()) == true) {
        bp::tuple pyTuple = bp::extract<bp::tuple>(pyValue);
        value = conversions::PyPrimitiveContainerToStdVector<T>(pyTuple);
    }
    else {
        throw std::runtime_error("in internal_setPrimitiveVectorAttrValue<T>, "
                "Python object passed in must be either a list or a tuple.");
    }

    // Set the value (NOTE: needs an UpdateGuard)
    if (isValid) {
        scene_rdl2::rdl2::SceneObject::UpdateGuard updateGuard(&sceneObject);
        sceneObject.set(attrKey, value);
    }
}

//-------------------------------------------------------------------------------------
//
// Set arrays of non-primitives (types other than bool, int, long, float, double, string, and SceneObject*)
//  * EXCLUDING MATRICES! *
//
//-------------------------------------------------------------------------------------

template <typename T>
void
internal_setVecVectorAttrValue(scene_rdl2::rdl2::SceneObject& sceneObject,
                               const scene_rdl2::rdl2::SceneClass& sc,
                               const std::string& attrName,
                               bp::object& pyValue)
{
    static_assert(
            (std::is_same<T, scene_rdl2::math::Vec2i>::value ||
             std::is_same<T, scene_rdl2::rdl2::Vec2f>::value ||
             std::is_same<T, scene_rdl2::rdl2::Vec2d>::value ||

             std::is_same<T, scene_rdl2::rdl2::Vec3f>::value ||
             std::is_same<T, scene_rdl2::rdl2::Vec3d>::value ||
             std::is_same<T, scene_rdl2::rdl2::Rgb>::value ||

             std::is_same<T, scene_rdl2::rdl2::Vec4f>::value ||
             std::is_same<T, scene_rdl2::rdl2::Vec4d>::value ||
             std::is_same<T, scene_rdl2::rdl2::Rgba>::value),
             "internal_setVecVectorAttrValue<T> cannot handle type T.");

    scene_rdl2::rdl2::AttributeKey<std::vector<T>> attrKey =
            sc.getAttributeKey<std::vector<T>>(attrName);

    bool isValid = true;
    std::vector<T> value;

    // Extract value from Python object
    if (PyList_CheckExact(pyValue.ptr()) == true) {
        bp::list pyList = bp::extract<bp::list>(pyValue);
        value = conversions::PyVecContainerToStdVector<T>(pyList);
    }
    else if (PyTuple_CheckExact(pyValue.ptr()) == true) {
        bp::tuple pyTuple = bp::extract<bp::tuple>(pyValue);
        value = conversions::PyVecContainerToStdVector<T>(pyTuple);
    }
    else {
        throw std::runtime_error("in internal_setVecVectorAttrValue<T>, "
                "Python object passed in must be either a list or a tuple.");
    }

    // Set the value (NOTE: needs an UpdateGuard)
    if (isValid) {
        scene_rdl2::rdl2::SceneObject::UpdateGuard updateGuard(&sceneObject);
        sceneObject.set(attrKey, value);
    }
}

//-------------------------------------------------------------------------------------
//
// Set arrays of matrices
//
//-------------------------------------------------------------------------------------

template <typename T>
void
internal_setMatrixVectorAttrValue(scene_rdl2::rdl2::SceneObject& sceneObject,
                                  const scene_rdl2::rdl2::SceneClass& sc,
                                  const std::string& attrName,
                                  bp::object& pyValue)
{
    static_assert(
            (std::is_same<T, scene_rdl2::math::Mat3f>::value ||
             std::is_same<T, scene_rdl2::math::Mat3d>::value ||
             std::is_same<T, scene_rdl2::rdl2::Mat4f>::value ||
             std::is_same<T, scene_rdl2::rdl2::Mat4d>::value),
              "internal_setMatrixVectorAttrValue<T> can only handle matrices");

    scene_rdl2::rdl2::AttributeKey<std::vector<T>> attrKey =
            sc.getAttributeKey<std::vector<T>>(attrName);

    bool isValid = true;
    std::vector<T> value;

    // Extract value from Python object
    if (PyList_CheckExact(pyValue.ptr()) == true) {
        bp::list pyList = bp::extract<bp::list>(pyValue);
        value = conversions::PyMatrixContainerToStdVector<T>(pyList);
    }
    else if (PyTuple_CheckExact(pyValue.ptr()) == true) {
        bp::tuple pyTuple = bp::extract<bp::tuple>(pyValue);
        value = conversions::PyMatrixContainerToStdVector<T>(pyTuple);
    }
    else {
        throw std::runtime_error("in internal_setMatrixVectorAttrValue<T>, "
                "Python object passed in must be either a list or a tuple.");
    }

    // Set the value (NOTE: needs an UpdateGuard)
    if (isValid) {
        scene_rdl2::rdl2::SceneObject::UpdateGuard updateGuard(&sceneObject);
        sceneObject.set(attrKey, value);
    }
}

//---------------------------------------------------------------

void
extractAndSetAttributeValue(scene_rdl2::rdl2::SceneObject& sceneObject,
                            const std::string& attrName,
                            bp::object& pyValue)
{
    //-------------------------------------------
    // Find the attribute, find type, and get AttributeKey

    const scene_rdl2::rdl2::SceneClass& sc = sceneObject.getSceneClass();
    const scene_rdl2::rdl2::Attribute* attr = sc.getAttribute(attrName);
    const scene_rdl2::rdl2::AttributeType attrType = attr->getType();

    //
    // Un-comment for debug:
    //
    //const std::string objType = bp::extract<std::string>(
    //        pyValue.attr("__class__").attr("__name__"));

    //-------------------------------------------
    // First, deal with non-array types

    if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_BOOL) {

        internal_setPrimitiveAttrValue<scene_rdl2::rdl2::Bool>(
                sceneObject,
                sc,
                attrName,
                pyValue);

    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_INT) {
        internal_setPrimitiveAttrValue<scene_rdl2::rdl2::Int>(
                sceneObject,
                sc,
                attrName,
                pyValue);
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_LONG) {
        internal_setPrimitiveAttrValue<scene_rdl2::rdl2::Long>(
                sceneObject,
                sc,
                attrName,
                pyValue);
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_FLOAT) {
        internal_setPrimitiveAttrValue<scene_rdl2::rdl2::Float>(
                sceneObject,
                sc,
                attrName,
                pyValue);
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_DOUBLE) {
        internal_setPrimitiveAttrValue<scene_rdl2::rdl2::Double>(
                sceneObject,
                sc,
                attrName,
                pyValue);
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_STRING) {
        internal_setPrimitiveAttrValue<scene_rdl2::rdl2::String>(
                sceneObject,
                sc,
                attrName,
                pyValue);
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_RGB) {
        internal_setVecAttrValue<scene_rdl2::rdl2::Rgb>(
                sceneObject,
                sc,
                attrName,
                pyValue);
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_RGBA) {
        internal_setVecAttrValue<scene_rdl2::rdl2::Rgba>(
                sceneObject,
                sc,
                attrName,
                pyValue);
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_VEC2F) {
        internal_setVecAttrValue<scene_rdl2::rdl2::Vec2f>(
                sceneObject,
                sc,
                attrName,
                pyValue);
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_VEC2D) {
        internal_setVecAttrValue<scene_rdl2::rdl2::Vec2d>(
                sceneObject,
                sc,
                attrName,
                pyValue);
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_VEC3F) {
        internal_setVecAttrValue<scene_rdl2::rdl2::Vec3f>(
                sceneObject,
                sc,
                attrName,
                pyValue);
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_VEC3D) {
        internal_setVecAttrValue<scene_rdl2::rdl2::Vec3d>(
                sceneObject,
                sc,
                attrName,
                pyValue);
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_VEC4F) {
        internal_setVecAttrValue<scene_rdl2::rdl2::Vec4f>(
                sceneObject,
                sc,
                attrName,
                pyValue);
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_VEC4D) {
        internal_setVecAttrValue<scene_rdl2::rdl2::Vec4d>(
                sceneObject,
                sc,
                attrName,
                pyValue);
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_MAT4F) {
        internal_setMatrixAttrValue<scene_rdl2::rdl2::Mat4f>(
                sceneObject,
                sc,
                attrName,
                pyValue);
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_MAT4D) {
        internal_setMatrixAttrValue<scene_rdl2::rdl2::Mat4d>(
                sceneObject,
                sc,
                attrName,
                pyValue);
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_SCENE_OBJECT) {
        internal_setSceneObjectAttrValue(
                sceneObject,
                sc,
                attrName,
                pyValue);
    }

    //-------------------------------------------
    // Vector types

    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_BOOL_VECTOR) {
        internal_setBoolVectorAttrValue(
                sceneObject,
                sc,
                attrName,
                pyValue);
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_INT_VECTOR) {
        internal_setPrimitiveVectorAttrValue<scene_rdl2::rdl2::Int>(
                sceneObject,
                sc,
                attrName,
                pyValue);
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_LONG_VECTOR) {
        internal_setPrimitiveVectorAttrValue<scene_rdl2::rdl2::Long>(
                sceneObject,
                sc,
                attrName,
                pyValue);
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_FLOAT_VECTOR) {
        internal_setPrimitiveVectorAttrValue<scene_rdl2::rdl2::Float>(
                sceneObject,
                sc,
                attrName,
                pyValue);
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_DOUBLE_VECTOR) {
        internal_setPrimitiveVectorAttrValue<scene_rdl2::rdl2::Double>(
                sceneObject,
                sc,
                attrName,
                pyValue);
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_STRING_VECTOR) {
        internal_setPrimitiveVectorAttrValue<scene_rdl2::rdl2::String>(
                sceneObject,
                sc,
                attrName,
                pyValue);
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_RGB_VECTOR) {
        internal_setVecVectorAttrValue<scene_rdl2::rdl2::Rgb>(
                sceneObject,
                sc,
                attrName,
                pyValue);
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_RGBA_VECTOR) {
        internal_setVecVectorAttrValue<scene_rdl2::rdl2::Rgba>(
                sceneObject,
                sc,
                attrName,
                pyValue);
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_VEC2F_VECTOR) {
        internal_setVecVectorAttrValue<scene_rdl2::rdl2::Vec2f>(
                sceneObject,
                sc,
                attrName,
                pyValue);
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_VEC2D_VECTOR) {
        internal_setVecVectorAttrValue<scene_rdl2::rdl2::Vec2d>(
                sceneObject,
                sc,
                attrName,
                pyValue);
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_VEC3F_VECTOR) {
        internal_setVecVectorAttrValue<scene_rdl2::rdl2::Vec3f>(
                sceneObject,
                sc,
                attrName,
                pyValue);
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_VEC3D_VECTOR) {
        internal_setVecVectorAttrValue<scene_rdl2::rdl2::Vec3d>(
                sceneObject,
                sc,
                attrName,
                pyValue);
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_VEC4F_VECTOR) {
        internal_setVecVectorAttrValue<scene_rdl2::rdl2::Vec4f>(
                sceneObject,
                sc,
                attrName,
                pyValue);
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_VEC4D_VECTOR) {
        internal_setVecVectorAttrValue<scene_rdl2::rdl2::Vec4d>(
                sceneObject,
                sc,
                attrName,
                pyValue);
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_MAT4F_VECTOR) {
        internal_setMatrixVectorAttrValue<scene_rdl2::rdl2::Mat4f>(
                sceneObject,
                sc,
                attrName,
                pyValue);
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_MAT4D_VECTOR) {
        internal_setMatrixVectorAttrValue<scene_rdl2::rdl2::Mat4d>(
                sceneObject,
                sc,
                attrName,
                pyValue);
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_SCENE_OBJECT_VECTOR) {
        internal_setSceneObjectVectorAttrValue(
                sceneObject,
                sc,
                attrName,
                pyValue);
    }
    else if (attrType == scene_rdl2::rdl2::AttributeType::TYPE_SCENE_OBJECT_INDEXABLE) {
        throw std::runtime_error("TEMP DEBUG: SceneObject.set() : scene_rdl2::rdl2::AttributeType::TYPE_SCENE_OBJECT_INDEXABLE not supported.");
    }
    else {
        throw std::runtime_error("TEMP DEBUG: Object of unknown type passed to SceneObject.set()");
    }
}

bp::dict
getAttributeNamesAndTypes(scene_rdl2::rdl2::SceneClass& sceneClass)
{
    bp::dict attrInfo;
    for (auto iter = sceneClass.beginAttributes(); iter != sceneClass.endAttributes(); ++iter) {
        const std::string attrName = (*iter)->getName();
        attrInfo[attrName] = getAttrTypeName(*iter);
    }

    return attrInfo;
}

bp::list
getAttributeGroupNames(scene_rdl2::rdl2::SceneClass& sceneClass)
{
    bp::list groupNamesList;

    for (auto iter = sceneClass.beginGroups(); iter != sceneClass.endGroups(); ++iter) {
        groupNamesList.append(*iter);
    }

    return groupNamesList;
}

const scene_rdl2::rdl2::Attribute*
getAttributeFromGroup(scene_rdl2::rdl2::SceneClass& sceneClass, const std::string& groupName, unsigned int i)
{
    const std::vector<const scene_rdl2::rdl2::Attribute*> attrGrpVec = sceneClass.getAttributeGroup(groupName);

    if (attrGrpVec.empty() || i >= attrGrpVec.size()) {
        return nullptr;
    }

    return attrGrpVec[i];
}

std::size_t
getAttributeGroupSize(scene_rdl2::rdl2::SceneClass& sceneClass, const std::string& groupName)
{
    return sceneClass.getAttributeGroup(groupName).size();
}

std::size_t
getAttributeCount(scene_rdl2::rdl2::SceneClass& sceneClass)
{
    return std::distance(sceneClass.beginAttributes(), sceneClass.endAttributes());
}

bp::list
getAttributeNames(scene_rdl2::rdl2::SceneClass& sceneClass)
{
    bp::list attrNames;

    for (auto iter = sceneClass.beginAttributes(); iter != sceneClass.endAttributes(); ++iter) {
        attrNames.append(std::string((*iter)->getName()));
    }

    return attrNames;
}

bp::dict
getAttributeNamesAndIndices(scene_rdl2::rdl2::SceneClass& sceneClass)
{
    bp::dict attrNames;

    std::size_t idx = 0;
    for (auto iter = sceneClass.beginAttributes(); iter != sceneClass.endAttributes(); ++iter, ++idx) {
        attrNames[std::string((*iter)->getName())] = idx;
    }

    return attrNames;
}

const scene_rdl2::rdl2::Attribute*
getAttributeAt(scene_rdl2::rdl2::SceneClass& sceneClass, unsigned int index)
{
    auto iter = sceneClass.beginAttributes();
    const std::size_t attrCount = std::distance(iter, sceneClass.endAttributes());
    if (index >= attrCount) {
        return nullptr;
    }

    std::advance(iter, index);
    return *iter;
}

} // namespace py_scene_rdl2

