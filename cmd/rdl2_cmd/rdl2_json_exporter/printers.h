// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <scene_rdl2/scene/rdl2/rdl2.h>
#include <json/value.h>

template <typename T>
Json::Value
getJsonForValue(const T value)
{
    return Json::Value(value);
}

template<>
Json::Value
getJsonForValue<scene_rdl2::rdl2::Long>(const scene_rdl2::rdl2::Long l)
{
    // Not sure why this template specialization is necessary. Without it
    // the compiler complains that it doesn't know which constructor to use,
    // but the choice seems like it should be pretty clear.
    return Json::Value(static_cast<Json::Int64>(l));
}

template <>
Json::Value
getJsonForValue<scene_rdl2::rdl2::Rgb>(const scene_rdl2::rdl2::Rgb rgb)
{
    Json::Value arrayValue;
    arrayValue.append(Json::Value(rgb.r));
    arrayValue.append(Json::Value(rgb.g));
    arrayValue.append(Json::Value(rgb.b));
    return arrayValue;
}

template <>
Json::Value
getJsonForValue<scene_rdl2::rdl2::Rgba>(const scene_rdl2::rdl2::Rgba rgb)
{
    Json::Value arrayValue;
    arrayValue.append(Json::Value(rgb.r));
    arrayValue.append(Json::Value(rgb.g));
    arrayValue.append(Json::Value(rgb.b));
    arrayValue.append(Json::Value(rgb.a));
    return arrayValue;
}

template<>
Json::Value
getJsonForValue<scene_rdl2::rdl2::Vec2f>(const scene_rdl2::rdl2::Vec2f vec)
{
    Json::Value arrayValue;
    arrayValue.append(Json::Value(vec.x));
    arrayValue.append(Json::Value(vec.y));
    return arrayValue;
}

template<>
Json::Value
getJsonForValue<scene_rdl2::rdl2::Vec2d>(const scene_rdl2::rdl2::Vec2d vec)
{
    Json::Value arrayValue;
    arrayValue.append(Json::Value(vec.x));
    arrayValue.append(Json::Value(vec.y));
    return arrayValue;
}

template<>
Json::Value
getJsonForValue<scene_rdl2::rdl2::Vec3f>(const scene_rdl2::rdl2::Vec3f vec)
{
    Json::Value arrayValue;
    arrayValue.append(Json::Value(vec.x));
    arrayValue.append(Json::Value(vec.y));
    arrayValue.append(Json::Value(vec.z));
    return arrayValue;
}

template<>
Json::Value
getJsonForValue<scene_rdl2::rdl2::Vec3d>(const scene_rdl2::rdl2::Vec3d vec)
{
    Json::Value arrayValue;
    arrayValue.append(Json::Value(vec.x));
    arrayValue.append(Json::Value(vec.y));
    arrayValue.append(Json::Value(vec.z));
    return arrayValue;
}

template<>
Json::Value
getJsonForValue<scene_rdl2::rdl2::Mat4f::Vector>(const scene_rdl2::rdl2::Mat4f::Vector vec)
{
    Json::Value arrayValue;
    arrayValue.append(Json::Value(vec.x));
    arrayValue.append(Json::Value(vec.y));
    arrayValue.append(Json::Value(vec.z));
    arrayValue.append(Json::Value(vec.w));
    return arrayValue;
}

template<>
Json::Value
getJsonForValue<scene_rdl2::rdl2::Mat4d::Vector>(const scene_rdl2::rdl2::Mat4d::Vector vec)
{
    Json::Value arrayValue;
    arrayValue.append(Json::Value(vec.x));
    arrayValue.append(Json::Value(vec.y));
    arrayValue.append(Json::Value(vec.z));
    arrayValue.append(Json::Value(vec.w));
    return arrayValue;
}

template<>
Json::Value
getJsonForValue<scene_rdl2::rdl2::Mat4f>(const scene_rdl2::rdl2::Mat4f mat)
{
    Json::Value arrayValue;
    arrayValue.append(getJsonForValue(mat.row0()));
    arrayValue.append(getJsonForValue(mat.row1()));
    arrayValue.append(getJsonForValue(mat.row2()));
    arrayValue.append(getJsonForValue(mat.row3()));
    return arrayValue;
}

template<>
Json::Value
getJsonForValue<scene_rdl2::rdl2::Mat4d>(const scene_rdl2::rdl2::Mat4d mat)
{
    Json::Value arrayValue;
    arrayValue.append(getJsonForValue(mat.row0()));
    arrayValue.append(getJsonForValue(mat.row1()));
    arrayValue.append(getJsonForValue(mat.row2()));
    arrayValue.append(getJsonForValue(mat.row3()));
    return arrayValue;
}

// This is a little confusing, but, because pointer is not part of the typeid,
// I can't do a template specialization that takes a pointer. And I can't pass
// the object by reference because it might be NULL. So, I'm using function
// overloading instead.
Json::Value
getJsonForValue(const scene_rdl2::rdl2::SceneObject* obj)
{
    if (obj) return Json::Value(obj->getName());
    return Json::Value();
}

// From printers.cc
template <typename T>
Json::Value
outputDefaultHelper(const scene_rdl2::rdl2::Attribute& attr)
{
    return getJsonForValue(attr.getDefaultValue<T>());
}

template <typename T>
Json::Value
outputDefaultVectorHelper(const scene_rdl2::rdl2::Attribute& attr)
{
    Json::Value arrayValue(Json::arrayValue);
    auto vec = attr.getDefaultValue<T>();
    for (const auto& value : vec) {
        arrayValue.append(getJsonForValue(value));
    }

    return arrayValue;
}

Json::Value
outputDefault(const scene_rdl2::rdl2::Attribute& attr)
{
    switch (attr.getType()) {
    case scene_rdl2::rdl2::TYPE_BOOL:
        return outputDefaultHelper<scene_rdl2::rdl2::Bool>(attr);

    case scene_rdl2::rdl2::TYPE_INT:
        return outputDefaultHelper<scene_rdl2::rdl2::Int>(attr);

    case scene_rdl2::rdl2::TYPE_LONG:
        return outputDefaultHelper<scene_rdl2::rdl2::Long>(attr);

    case scene_rdl2::rdl2::TYPE_FLOAT:
        return outputDefaultHelper<scene_rdl2::rdl2::Float>(attr);

    case scene_rdl2::rdl2::TYPE_DOUBLE:
        return outputDefaultHelper<scene_rdl2::rdl2::Double>(attr);

    case scene_rdl2::rdl2::TYPE_STRING:
        return outputDefaultHelper<scene_rdl2::rdl2::String>(attr);

    case scene_rdl2::rdl2::TYPE_RGB:
        return outputDefaultHelper<scene_rdl2::rdl2::Rgb>(attr);

    case scene_rdl2::rdl2::TYPE_RGBA:
        return outputDefaultHelper<scene_rdl2::rdl2::Rgba>(attr);

    case scene_rdl2::rdl2::TYPE_VEC2F:
        return outputDefaultHelper<scene_rdl2::rdl2::Vec2f>(attr);

    case scene_rdl2::rdl2::TYPE_VEC2D:
        return outputDefaultHelper<scene_rdl2::rdl2::Vec2d>(attr);

    case scene_rdl2::rdl2::TYPE_VEC3F:
        return outputDefaultHelper<scene_rdl2::rdl2::Vec3f>(attr);

    case scene_rdl2::rdl2::TYPE_VEC3D:
        return outputDefaultHelper<scene_rdl2::rdl2::Vec3d>(attr);

    case scene_rdl2::rdl2::TYPE_MAT4F:
        return outputDefaultHelper<scene_rdl2::rdl2::Mat4f>(attr);

    case scene_rdl2::rdl2::TYPE_MAT4D:
        return outputDefaultHelper<scene_rdl2::rdl2::Mat4d>(attr);

    case scene_rdl2::rdl2::TYPE_SCENE_OBJECT:
        return outputDefaultHelper<scene_rdl2::rdl2::SceneObject*>(attr);

    case scene_rdl2::rdl2::TYPE_BOOL_VECTOR:
        return outputDefaultVectorHelper<scene_rdl2::rdl2::BoolVector>(attr);

    case scene_rdl2::rdl2::TYPE_INT_VECTOR:
        return outputDefaultVectorHelper<scene_rdl2::rdl2::IntVector>(attr);

    case scene_rdl2::rdl2::TYPE_LONG_VECTOR:
        return outputDefaultVectorHelper<scene_rdl2::rdl2::LongVector>(attr);

    case scene_rdl2::rdl2::TYPE_FLOAT_VECTOR:
        return outputDefaultVectorHelper<scene_rdl2::rdl2::FloatVector>(attr);

    case scene_rdl2::rdl2::TYPE_DOUBLE_VECTOR:
        return outputDefaultVectorHelper<scene_rdl2::rdl2::DoubleVector>(attr);

    case scene_rdl2::rdl2::TYPE_STRING_VECTOR:
        return outputDefaultVectorHelper<scene_rdl2::rdl2::StringVector>(attr);

    case scene_rdl2::rdl2::TYPE_RGB_VECTOR:
        return outputDefaultVectorHelper<scene_rdl2::rdl2::RgbVector>(attr);

    case scene_rdl2::rdl2::TYPE_RGBA_VECTOR:
        return outputDefaultVectorHelper<scene_rdl2::rdl2::RgbaVector>(attr);

    case scene_rdl2::rdl2::TYPE_VEC2F_VECTOR:
        return outputDefaultVectorHelper<scene_rdl2::rdl2::Vec2fVector>(attr);

    case scene_rdl2::rdl2::TYPE_VEC2D_VECTOR:
        return outputDefaultVectorHelper<scene_rdl2::rdl2::Vec2dVector>(attr);

    case scene_rdl2::rdl2::TYPE_VEC3F_VECTOR:
        return outputDefaultVectorHelper<scene_rdl2::rdl2::Vec3fVector>(attr);

    case scene_rdl2::rdl2::TYPE_VEC3D_VECTOR:
        return outputDefaultVectorHelper<scene_rdl2::rdl2::Vec3dVector>(attr);

    case scene_rdl2::rdl2::TYPE_MAT4F_VECTOR:
        return outputDefaultVectorHelper<scene_rdl2::rdl2::Mat4fVector>(attr);

    case scene_rdl2::rdl2::TYPE_MAT4D_VECTOR:
        return outputDefaultVectorHelper<scene_rdl2::rdl2::Mat4dVector>(attr);

    case scene_rdl2::rdl2::TYPE_SCENE_OBJECT_VECTOR:
        return outputDefaultVectorHelper<scene_rdl2::rdl2::SceneObjectVector>(attr);

    case scene_rdl2::rdl2::TYPE_SCENE_OBJECT_INDEXABLE:
        return outputDefaultVectorHelper<scene_rdl2::rdl2::SceneObjectIndexable>(attr);

    default:
        return Json::Value();
    }
}


