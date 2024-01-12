// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include <scene_rdl2/scene/rdl2/rdl2.h>

using namespace scene_rdl2;

RDL2_DSO_ATTR_DECLARE

    rdl2::AttributeKey<rdl2::Bool> attrBool;
    rdl2::AttributeKey<rdl2::Int> attrInt;
    rdl2::AttributeKey<rdl2::Long> attrLong;
    rdl2::AttributeKey<rdl2::Float> attrFloat;
    rdl2::AttributeKey<rdl2::Double> attrDouble;
    rdl2::AttributeKey<rdl2::String> attrString;
    rdl2::AttributeKey<rdl2::Rgb> attrRgb;
    rdl2::AttributeKey<rdl2::Rgba> attrRgba;
    rdl2::AttributeKey<rdl2::Vec2f> attrVec2f;
    rdl2::AttributeKey<rdl2::Vec2d> attrVec2d;
    rdl2::AttributeKey<rdl2::Vec3f> attrVec3f;
    rdl2::AttributeKey<rdl2::Vec3d> attrVec3d;
    rdl2::AttributeKey<rdl2::Vec4f> attrVec4f;
    rdl2::AttributeKey<rdl2::Vec4d> attrVec4d;
    rdl2::AttributeKey<rdl2::Mat4f> attrMat4f;
    rdl2::AttributeKey<rdl2::Mat4d> attrMat4d;
    rdl2::AttributeKey<rdl2::SceneObject*> attrSceneObject;
    rdl2::AttributeKey<rdl2::BoolVector> attrBoolVector;
    rdl2::AttributeKey<rdl2::IntVector> attrIntVector;
    rdl2::AttributeKey<rdl2::LongVector> attrLongVector;
    rdl2::AttributeKey<rdl2::FloatVector> attrFloatVector;
    rdl2::AttributeKey<rdl2::DoubleVector> attrDoubleVector;
    rdl2::AttributeKey<rdl2::StringVector> attrStringVector;
    rdl2::AttributeKey<rdl2::RgbVector> attrRgbVector;
    rdl2::AttributeKey<rdl2::RgbaVector> attrRgbaVector;
    rdl2::AttributeKey<rdl2::Vec2fVector> attrVec2fVector;
    rdl2::AttributeKey<rdl2::Vec2dVector> attrVec2dVector;
    rdl2::AttributeKey<rdl2::Vec3fVector> attrVec3fVector;
    rdl2::AttributeKey<rdl2::Vec3dVector> attrVec3dVector;
    rdl2::AttributeKey<rdl2::Vec4fVector> attrVec4fVector;
    rdl2::AttributeKey<rdl2::Vec4dVector> attrVec4dVector;
    rdl2::AttributeKey<rdl2::Mat4fVector> attrMat4fVector;
    rdl2::AttributeKey<rdl2::Mat4dVector> attrMat4dVector;
    rdl2::AttributeKey<rdl2::SceneObjectVector> attrSceneObjectVector;

RDL2_DSO_ATTR_DEFINE(rdl2::SceneObject)

    rdl2::BoolVector boolVec;
    boolVec.push_back(true);
    boolVec.push_back(false);

    rdl2::IntVector intVec;
    intVec.push_back(rdl2::Int(100));
    intVec.push_back(rdl2::Int(101));

    rdl2::LongVector longVec;
    longVec.push_back(rdl2::Long(102));
    longVec.push_back(rdl2::Long(103));

    rdl2::FloatVector floatVec;
    floatVec.push_back(1.0f);
    floatVec.push_back(2.0f);

    rdl2::DoubleVector doubleVec;
    doubleVec.push_back(3.0);
    doubleVec.push_back(4.0);

    rdl2::StringVector stringVec;
    stringVec.push_back("a");
    stringVec.push_back("b");

    rdl2::RgbVector rgbVec;
    rgbVec.push_back(rdl2::Rgb(0.1f, 0.2f, 0.3f));
    rgbVec.push_back(rdl2::Rgb(0.4f, 0.5f, 0.6f));

    rdl2::RgbaVector rgbaVec;
    rgbaVec.push_back(rdl2::Rgba(0.1f, 0.2f, 0.3f, 0.4f));
    rgbaVec.push_back(rdl2::Rgba(0.5f, 0.6f, 0.7f, 0.8f));

    rdl2::Vec2fVector vec2fVec;
    vec2fVec.push_back(rdl2::Vec2f(1.0f, 2.0f));
    vec2fVec.push_back(rdl2::Vec2f(3.0f, 4.0f));

    rdl2::Vec2dVector vec2dVec;
    vec2dVec.push_back(rdl2::Vec2d(1.0, 2.0));
    vec2dVec.push_back(rdl2::Vec2d(3.0, 4.0));

    rdl2::Vec3fVector vec3fVec;
    vec3fVec.push_back(rdl2::Vec3f(1.0f, 2.0f, 3.0f));
    vec3fVec.push_back(rdl2::Vec3f(4.0f, 5.0f, 6.0f));

    rdl2::Vec3dVector vec3dVec;
    vec3dVec.push_back(rdl2::Vec3d(1.0, 2.0, 3.0));
    vec3dVec.push_back(rdl2::Vec3d(4.0, 5.0, 6.0));

    rdl2::Vec4fVector vec4fVec;
    vec4fVec.push_back(rdl2::Vec4f(1.0f, 2.0f, 3.0f, 4.0f));
    vec4fVec.push_back(rdl2::Vec4f(5.0f, 6.0f, 7.0f, 8.0f));

    rdl2::Vec4dVector vec4dVec;
    vec4dVec.push_back(rdl2::Vec4d(1.0, 2.0, 3.0, 4.0));
    vec4dVec.push_back(rdl2::Vec4d(5.0, 6.0, 7.0, 8.0));

    rdl2::Mat4fVector mat4fVec;
    mat4fVec.push_back(rdl2::Mat4f(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f));
    mat4fVec.push_back(rdl2::Mat4f(17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f, 24.0f, 25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f, 32.0f));

    rdl2::Mat4dVector mat4dVec;
    mat4dVec.push_back(rdl2::Mat4d(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0));
    mat4dVec.push_back(rdl2::Mat4d(17.0, 18.0, 19.0, 20.0, 21.0, 22.0, 23.0, 24.0, 25.0, 26.0, 27.0, 28.0, 29.0, 30.0, 31.0, 32.0));

    attrBool =
        sceneClass.declareAttribute<rdl2::Bool>("bool", true);
    sceneClass.setMetadata(attrBool,
        "comment", "This is a boolean value.");

    attrInt =
        sceneClass.declareAttribute<rdl2::Int>("int", rdl2::Int(42), rdl2::FLAGS_BLURRABLE);
    sceneClass.setMetadata(attrInt,
        "comment", "This is a 32-bit signed integer value.");

    attrLong =
        sceneClass.declareAttribute<rdl2::Long>("long", rdl2::Long(43), rdl2::FLAGS_BLURRABLE);
    sceneClass.setMetadata(attrLong,
        "comment", "This is a 64-bit signed integer value.");

    attrFloat =
        sceneClass.declareAttribute<rdl2::Float>("float", 1.0f, rdl2::FLAGS_BLURRABLE);
    sceneClass.setMetadata(attrFloat,
        "comment", "This is a 32-bit floating point value.");

    attrDouble =
        sceneClass.declareAttribute<rdl2::Double>("double", 2.0, rdl2::FLAGS_BLURRABLE);
    sceneClass.setMetadata(attrDouble,
        "comment", "This is a 64-bit floating point value.");

    attrString =
        sceneClass.declareAttribute<rdl2::String>("string", "pizza", rdl2::FLAGS_BINDABLE);
    sceneClass.setMetadata(attrString,
        "comment", "This is a string value.");

    attrRgb =
        sceneClass.declareAttribute<rdl2::Rgb>("rgb", rdl2::Rgb(0.1f, 0.2f, 0.3f), rdl2::FLAGS_BLURRABLE);
    sceneClass.setMetadata(attrRgb,
        "comment", "This is a RGB color value.");

    attrRgba =
        sceneClass.declareAttribute<rdl2::Rgba>("rgba", rdl2::Rgba(0.1f, 0.2f, 0.3f, 0.4f), rdl2::FLAGS_BLURRABLE);
    sceneClass.setMetadata(attrRgba,
        "comment", "This is a RGBA color value.");

    attrVec2f =
        sceneClass.declareAttribute<rdl2::Vec2f>("vec2f", rdl2::Vec2f(1.0f, 2.0f), rdl2::FLAGS_BLURRABLE);
    sceneClass.setMetadata(attrVec2f,
        "comment", "This is a 2D vector of 32-bit floating point values.");

    attrVec2d =
        sceneClass.declareAttribute<rdl2::Vec2d>("vec2d", rdl2::Vec2d(2.0, 3.0), rdl2::FLAGS_BLURRABLE);
    sceneClass.setMetadata(attrVec2d,
        "comment", "This is a 2D vector of 64-bit floating point values.");

    attrVec3f =
        sceneClass.declareAttribute<rdl2::Vec3f>("vec3f", rdl2::Vec3f(1.0f, 2.0f, 3.0f), rdl2::FLAGS_BLURRABLE);
    sceneClass.setMetadata(attrVec3f,
        "comment", "This is a 3D vector of 32-bit floating point values.");

    attrVec3d =
        sceneClass.declareAttribute<rdl2::Vec3d>("vec3d", rdl2::Vec3d(2.0, 3.0, 4.0), rdl2::FLAGS_BLURRABLE);
    sceneClass.setMetadata(attrVec3d,
        "comment", "This is a 3D vector of 64-bit floating point values.");

    attrVec4f =
        sceneClass.declareAttribute<rdl2::Vec4f>("vec4f", rdl2::Vec4f(1.0f, 2.0f, 3.0f, 4.0f), rdl2::FLAGS_BLURRABLE);
    sceneClass.setMetadata(attrVec4f,
        "comment", "This is a 4D vector of 32-bit floating point values.");

    attrVec4d =
        sceneClass.declareAttribute<rdl2::Vec4d>("vec4d", rdl2::Vec4d(2.0, 3.0, 4.0, 5.0), rdl2::FLAGS_BLURRABLE);
    sceneClass.setMetadata(attrVec4d,
        "comment", "This is a 3D vector of 64-bit floating point values.");

    attrMat4f =
        sceneClass.declareAttribute<rdl2::Mat4f>("mat4f", rdl2::Mat4f(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f), rdl2::FLAGS_BLURRABLE);
    sceneClass.setMetadata(attrMat4f,
        "comment", "This is a 4x4 matrix of 32-bit floating point values.");

    attrMat4d =
        sceneClass.declareAttribute<rdl2::Mat4d>("mat4d", rdl2::Mat4d(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0), rdl2::FLAGS_BLURRABLE);
    sceneClass.setMetadata(attrMat4d,
        "comment", "This is a 4x4 matrix of 64-bit floating point values.");

    attrSceneObject =
        sceneClass.declareAttribute<rdl2::SceneObject*>("scene_object", nullptr, { "scene object" });
    sceneClass.setMetadata(attrSceneObject,
        "comment", "This is a pointer to a SceneObject.");

    attrBoolVector =
        sceneClass.declareAttribute<rdl2::BoolVector>("bool_vector", boolVec,
            rdl2::FLAGS_NONE, rdl2::INTERFACE_GENERIC, { "bool vector" });
    sceneClass.setMetadata(attrBoolVector,
        "comment", "This is a list boolean values.");

    attrIntVector =
        sceneClass.declareAttribute<rdl2::IntVector>("int_vector", intVec,
        rdl2::FLAGS_NONE, rdl2::INTERFACE_GENERIC, { "int vector" });
    sceneClass.setMetadata(attrIntVector,
        "comment", "This is a list of 32-bit signed integer values.");

    attrLongVector =
        sceneClass.declareAttribute<rdl2::LongVector>("long_vector", longVec, { "long vector" });
    sceneClass.setMetadata(attrLongVector,
        "comment", "This is a list of 64-bit signed integer values.");

    attrFloatVector =
        sceneClass.declareAttribute<rdl2::FloatVector>("float_vector", floatVec, { "float vector" });
    sceneClass.setMetadata(attrFloatVector,
        "comment", "This is a list of 32-bit floating point values.");

    attrDoubleVector =
        sceneClass.declareAttribute<rdl2::DoubleVector>("double_vector", doubleVec, { "double vector" });
    sceneClass.setMetadata(attrDoubleVector,
        "comment", "This is a list of 64-bit floating point values.");

    attrStringVector =
        sceneClass.declareAttribute<rdl2::StringVector>("string_vector", stringVec, { "string vector" });
    sceneClass.setMetadata(attrStringVector,
        "comment", "This is a list of string values.");

    attrRgbVector =
        sceneClass.declareAttribute<rdl2::RgbVector>("rgb_vector", rgbVec, { "rgb vector" });
    sceneClass.setMetadata(attrRgbVector,
        "comment", "This is a list of RGB color values.");

    attrRgbaVector =
        sceneClass.declareAttribute<rdl2::RgbaVector>("rgba_vector", rgbaVec, { "rgba vector" });
    sceneClass.setMetadata(attrRgbaVector,
        "comment", "This is a list of RGBA color values.");

    attrVec2fVector =
        sceneClass.declareAttribute<rdl2::Vec2fVector>("vec2f_vector", vec2fVec, { "vec2f vector" });
    sceneClass.setMetadata(attrVec2fVector,
        "comment", "This is a list of 2D vectors of 32-bit floating point values.");

    attrVec2dVector =
        sceneClass.declareAttribute<rdl2::Vec2dVector>("vec2d_vector", vec2dVec, { "vec2d vector" });
    sceneClass.setMetadata(attrVec2dVector,
        "comment", "This is a list of 2D vectors of 64-bit floating point values.");

    attrVec3fVector =
        sceneClass.declareAttribute<rdl2::Vec3fVector>("vec3f_vector", vec3fVec, { "vec3f vector" });
    sceneClass.setMetadata(attrVec3fVector,
        "comment", "This is a list of 3D vectors of 32-bit floating point values.");

    attrVec3dVector =
        sceneClass.declareAttribute<rdl2::Vec3dVector>("vec3d_vector", vec3dVec, { "vec3d vector" });
    sceneClass.setMetadata(attrVec3dVector,
        "comment", "This is a list of 3D vectors of 64-bit floating point values.");

    attrVec4fVector =
        sceneClass.declareAttribute<rdl2::Vec4fVector>("vec4f_vector", vec4fVec, { "vec4f vector" });
    sceneClass.setMetadata(attrVec4fVector,
        "comment", "This is a list of 4D vectors of 32-bit floating point values.");

    attrVec4dVector =
        sceneClass.declareAttribute<rdl2::Vec4dVector>("vec4d_vector", vec4dVec, { "vec4d vector" });
    sceneClass.setMetadata(attrVec4dVector,
        "comment", "This is a list of 4D vectors of 64-bit floating point values.");

    attrMat4fVector =
        sceneClass.declareAttribute<rdl2::Mat4fVector>("mat4f_vector", mat4fVec, { "mat4f vector" });
    sceneClass.setMetadata(attrMat4fVector,
        "comment", "This is a list of 4x4 matrices of 32-bit floating point values.");

    attrMat4dVector =
        sceneClass.declareAttribute<rdl2::Mat4dVector>("mat4d_vector", mat4dVec, { "mat4d vector" });
    sceneClass.setMetadata(attrMat4dVector,
        "comment", "This is a list of 4x4 matrices of 64-bit floating point values.");

    attrSceneObjectVector =
        sceneClass.declareAttribute<rdl2::SceneObjectVector>("scene_object_vector", { "scene object vector" });
    sceneClass.setMetadata(attrSceneObjectVector,
        "comment", "This is a list of pointers to SceneObjects.");

RDL2_DSO_ATTR_END

