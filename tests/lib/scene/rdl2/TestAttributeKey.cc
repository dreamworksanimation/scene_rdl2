// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "TestAttributeKey.h"

#include <scene_rdl2/scene/rdl2/Attribute.h>
#include <scene_rdl2/scene/rdl2/AttributeKey.h>
#include <scene_rdl2/scene/rdl2/Types.h>

#include <cppunit/extensions/HelperMacros.h>

namespace scene_rdl2 {
namespace rdl2 {
namespace unittest {

void
TestAttributeKey::setUp()
{
    mAttribute.reset(new Attribute("awesome", TYPE_FLOAT, FLAGS_BLURRABLE, 12, 34, INTERFACE_CAMERA));
    mKey.reset(new AttributeKey<Float>(*mAttribute));
}

void
TestAttributeKey::tearDown()
{
}

void
TestAttributeKey::testIndex()
{
    CPPUNIT_ASSERT(mKey->mIndex == 12);
}

void
TestAttributeKey::testOffset()
{
    CPPUNIT_ASSERT(mKey->mOffset == 34);
}

void
TestAttributeKey::testFlags()
{
    CPPUNIT_ASSERT(mKey->mFlags == FLAGS_BLURRABLE);
}

void
TestAttributeKey::testObjectType()
{
    CPPUNIT_ASSERT(mKey->mObjectType == INTERFACE_CAMERA);
}

void
TestAttributeKey::testEquality()
{
    {
        Attribute attr("bool", TYPE_BOOL, FLAGS_NONE, 0, 42);
        AttributeKey<Bool> key(attr);
        Attribute sameAttr("same bool", TYPE_BOOL, FLAGS_NONE, 0, 42);
        AttributeKey<Bool> sameKey(attr);
        Attribute otherAttr("other bool", TYPE_BOOL, FLAGS_NONE, 1, 42);
        AttributeKey<Bool> otherKey(otherAttr);
        CPPUNIT_ASSERT(key == sameKey);
        CPPUNIT_ASSERT(!(key == otherKey));
        CPPUNIT_ASSERT(!(key != sameKey));
        CPPUNIT_ASSERT(key != otherKey);
    }
    {
        Attribute attr("int", TYPE_INT, FLAGS_NONE, 0, 42);
        AttributeKey<Int> key(attr);
        Attribute sameAttr("same int", TYPE_INT, FLAGS_NONE, 0, 42);
        AttributeKey<Int> sameKey(attr);
        Attribute otherAttr("other int", TYPE_INT, FLAGS_NONE, 1, 42);
        AttributeKey<Int> otherKey(otherAttr);
        CPPUNIT_ASSERT(key == sameKey);
        CPPUNIT_ASSERT(!(key == otherKey));
        CPPUNIT_ASSERT(!(key != sameKey));
        CPPUNIT_ASSERT(key != otherKey);
    }
    {
        Attribute attr("long", TYPE_LONG, FLAGS_NONE, 0, 42);
        AttributeKey<Long> key(attr);
        Attribute sameAttr("same long", TYPE_LONG, FLAGS_NONE, 0, 42);
        AttributeKey<Long> sameKey(attr);
        Attribute otherAttr("other long", TYPE_LONG, FLAGS_NONE, 1, 42);
        AttributeKey<Long> otherKey(otherAttr);
        CPPUNIT_ASSERT(key == sameKey);
        CPPUNIT_ASSERT(!(key == otherKey));
        CPPUNIT_ASSERT(!(key != sameKey));
        CPPUNIT_ASSERT(key != otherKey);
    }
    {
        Attribute attr("float", TYPE_FLOAT, FLAGS_NONE, 0, 42);
        AttributeKey<Float> key(attr);
        Attribute sameAttr("same float", TYPE_FLOAT, FLAGS_NONE, 0, 42);
        AttributeKey<Float> sameKey(attr);
        Attribute otherAttr("other float", TYPE_FLOAT, FLAGS_NONE, 1, 42);
        AttributeKey<Float> otherKey(otherAttr);
        CPPUNIT_ASSERT(key == sameKey);
        CPPUNIT_ASSERT(!(key == otherKey));
        CPPUNIT_ASSERT(!(key != sameKey));
        CPPUNIT_ASSERT(key != otherKey);
    }
    {
        Attribute attr("double", TYPE_DOUBLE, FLAGS_NONE, 0, 42);
        AttributeKey<Double> key(attr);
        Attribute sameAttr("same double", TYPE_DOUBLE, FLAGS_NONE, 0, 42);
        AttributeKey<Double> sameKey(attr);
        Attribute otherAttr("other double", TYPE_DOUBLE, FLAGS_NONE, 1, 42);
        AttributeKey<Double> otherKey(otherAttr);
        CPPUNIT_ASSERT(key == sameKey);
        CPPUNIT_ASSERT(!(key == otherKey));
        CPPUNIT_ASSERT(!(key != sameKey));
        CPPUNIT_ASSERT(key != otherKey);
    }
    {
        Attribute attr("string", TYPE_STRING, FLAGS_NONE, 0, 42);
        AttributeKey<String> key(attr);
        Attribute sameAttr("same string", TYPE_STRING, FLAGS_NONE, 0, 42);
        AttributeKey<String> sameKey(attr);
        Attribute otherAttr("other string", TYPE_STRING, FLAGS_NONE, 1, 42);
        AttributeKey<String> otherKey(otherAttr);
        CPPUNIT_ASSERT(key == sameKey);
        CPPUNIT_ASSERT(!(key == otherKey));
        CPPUNIT_ASSERT(!(key != sameKey));
        CPPUNIT_ASSERT(key != otherKey);
    }
    {
        Attribute attr("rgb", TYPE_RGB, FLAGS_NONE, 0, 42);
        AttributeKey<Rgb> key(attr);
        Attribute sameAttr("same rgb", TYPE_RGB, FLAGS_NONE, 0, 42);
        AttributeKey<Rgb> sameKey(attr);
        Attribute otherAttr("other rgb", TYPE_RGB, FLAGS_NONE, 1, 42);
        AttributeKey<Rgb> otherKey(otherAttr);
        CPPUNIT_ASSERT(key == sameKey);
        CPPUNIT_ASSERT(!(key == otherKey));
        CPPUNIT_ASSERT(!(key != sameKey));
        CPPUNIT_ASSERT(key != otherKey);
    }
    {
        Attribute attr("rgba", TYPE_RGBA, FLAGS_NONE, 0, 42);
        AttributeKey<Rgba> key(attr);
        Attribute sameAttr("same rgba", TYPE_RGBA, FLAGS_NONE, 0, 42);
        AttributeKey<Rgba> sameKey(attr);
        Attribute otherAttr("other rgba", TYPE_RGBA, FLAGS_NONE, 1, 42);
        AttributeKey<Rgba> otherKey(otherAttr);
        CPPUNIT_ASSERT(key == sameKey);
        CPPUNIT_ASSERT(!(key == otherKey));
        CPPUNIT_ASSERT(!(key != sameKey));
        CPPUNIT_ASSERT(key != otherKey);
    }
    {
        Attribute attr("vec2f", TYPE_VEC2F, FLAGS_NONE, 0, 42);
        AttributeKey<Vec2f> key(attr);
        Attribute sameAttr("same vec2f", TYPE_VEC2F, FLAGS_NONE, 0, 42);
        AttributeKey<Vec2f> sameKey(attr);
        Attribute otherAttr("other vec2f", TYPE_VEC2F, FLAGS_NONE, 1, 42);
        AttributeKey<Vec2f> otherKey(otherAttr);
        CPPUNIT_ASSERT(key == sameKey);
        CPPUNIT_ASSERT(!(key == otherKey));
        CPPUNIT_ASSERT(!(key != sameKey));
        CPPUNIT_ASSERT(key != otherKey);
    }
    {
        Attribute attr("vec2d", TYPE_VEC2D, FLAGS_NONE, 0, 42);
        AttributeKey<Vec2d> key(attr);
        Attribute sameAttr("same vec2d", TYPE_VEC2D, FLAGS_NONE, 0, 42);
        AttributeKey<Vec2d> sameKey(attr);
        Attribute otherAttr("other vec2d", TYPE_VEC2D, FLAGS_NONE, 1, 42);
        AttributeKey<Vec2d> otherKey(otherAttr);
        CPPUNIT_ASSERT(key == sameKey);
        CPPUNIT_ASSERT(!(key == otherKey));
        CPPUNIT_ASSERT(!(key != sameKey));
        CPPUNIT_ASSERT(key != otherKey);
    }
    {
        Attribute attr("vec3f", TYPE_VEC3F, FLAGS_NONE, 0, 42);
        AttributeKey<Vec3f> key(attr);
        Attribute sameAttr("same vec3f", TYPE_VEC3F, FLAGS_NONE, 0, 42);
        AttributeKey<Vec3f> sameKey(attr);
        Attribute otherAttr("other vec3f", TYPE_VEC3F, FLAGS_NONE, 1, 42);
        AttributeKey<Vec3f> otherKey(otherAttr);
        CPPUNIT_ASSERT(key == sameKey);
        CPPUNIT_ASSERT(!(key == otherKey));
        CPPUNIT_ASSERT(!(key != sameKey));
        CPPUNIT_ASSERT(key != otherKey);
    }
    {
        Attribute attr("vec3d", TYPE_VEC3D, FLAGS_NONE, 0, 42);
        AttributeKey<Vec3d> key(attr);
        Attribute sameAttr("same vec3d", TYPE_VEC3D, FLAGS_NONE, 0, 42);
        AttributeKey<Vec3d> sameKey(attr);
        Attribute otherAttr("other vec3d", TYPE_VEC3D, FLAGS_NONE, 1, 42);
        AttributeKey<Vec3d> otherKey(otherAttr);
        CPPUNIT_ASSERT(key == sameKey);
        CPPUNIT_ASSERT(!(key == otherKey));
        CPPUNIT_ASSERT(!(key != sameKey));
        CPPUNIT_ASSERT(key != otherKey);
    }
    {
        Attribute attr("vec4f", TYPE_VEC4F, FLAGS_NONE, 0, 42);
        AttributeKey<Vec4f> key(attr);
        Attribute sameAttr("same vec4f", TYPE_VEC4F, FLAGS_NONE, 0, 42);
        AttributeKey<Vec4f> sameKey(attr);
        Attribute otherAttr("other vec4f", TYPE_VEC4F, FLAGS_NONE, 1, 42);
        AttributeKey<Vec4f> otherKey(otherAttr);
        CPPUNIT_ASSERT(key == sameKey);
        CPPUNIT_ASSERT(!(key == otherKey));
        CPPUNIT_ASSERT(!(key != sameKey));
        CPPUNIT_ASSERT(key != otherKey);
    }
    {
        Attribute attr("vec4d", TYPE_VEC4D, FLAGS_NONE, 0, 42);
        AttributeKey<Vec4d> key(attr);
        Attribute sameAttr("same vec4d", TYPE_VEC4D, FLAGS_NONE, 0, 42);
        AttributeKey<Vec4d> sameKey(attr);
        Attribute otherAttr("other vec4d", TYPE_VEC4D, FLAGS_NONE, 1, 42);
        AttributeKey<Vec4d> otherKey(otherAttr);
        CPPUNIT_ASSERT(key == sameKey);
        CPPUNIT_ASSERT(!(key == otherKey));
        CPPUNIT_ASSERT(!(key != sameKey));
        CPPUNIT_ASSERT(key != otherKey);
    }
    {
        Attribute attr("mat4f", TYPE_MAT4F, FLAGS_NONE, 0, 42);
        AttributeKey<Mat4f> key(attr);
        Attribute sameAttr("same mat4f", TYPE_MAT4F, FLAGS_NONE, 0, 42);
        AttributeKey<Mat4f> sameKey(attr);
        Attribute otherAttr("other mat4f", TYPE_MAT4F, FLAGS_NONE, 1, 42);
        AttributeKey<Mat4f> otherKey(otherAttr);
        CPPUNIT_ASSERT(key == sameKey);
        CPPUNIT_ASSERT(!(key == otherKey));
        CPPUNIT_ASSERT(!(key != sameKey));
        CPPUNIT_ASSERT(key != otherKey);
    }
    {
        Attribute attr("mat4d", TYPE_MAT4D, FLAGS_NONE, 0, 42);
        AttributeKey<Mat4d> key(attr);
        Attribute sameAttr("same mat4d", TYPE_MAT4D, FLAGS_NONE, 0, 42);
        AttributeKey<Mat4d> sameKey(attr);
        Attribute otherAttr("other mat4d", TYPE_MAT4D, FLAGS_NONE, 1, 42);
        AttributeKey<Mat4d> otherKey(otherAttr);
        CPPUNIT_ASSERT(key == sameKey);
        CPPUNIT_ASSERT(!(key == otherKey));
        CPPUNIT_ASSERT(!(key != sameKey));
        CPPUNIT_ASSERT(key != otherKey);
    }
    {
        Attribute attr("scene object", TYPE_SCENE_OBJECT, FLAGS_NONE, 0, 42);
        AttributeKey<SceneObject*> key(attr);
        Attribute sameAttr("same scene object", TYPE_SCENE_OBJECT, FLAGS_NONE, 0, 42);
        AttributeKey<SceneObject*> sameKey(attr);
        Attribute otherAttr("other scene object", TYPE_SCENE_OBJECT, FLAGS_NONE, 1, 42);
        AttributeKey<SceneObject*> otherKey(otherAttr);
        CPPUNIT_ASSERT(key == sameKey);
        CPPUNIT_ASSERT(!(key == otherKey));
        CPPUNIT_ASSERT(!(key != sameKey));
        CPPUNIT_ASSERT(key != otherKey);
    }
    {
        Attribute attr("bool vector", TYPE_BOOL_VECTOR, FLAGS_NONE, 0, 42);
        AttributeKey<BoolVector> key(attr);
        Attribute sameAttr("same bool vector", TYPE_BOOL_VECTOR, FLAGS_NONE, 0, 42);
        AttributeKey<BoolVector> sameKey(attr);
        Attribute otherAttr("other bool vector", TYPE_BOOL_VECTOR, FLAGS_NONE, 1, 42);
        AttributeKey<BoolVector> otherKey(otherAttr);
        CPPUNIT_ASSERT(key == sameKey);
        CPPUNIT_ASSERT(!(key == otherKey));
        CPPUNIT_ASSERT(!(key != sameKey));
        CPPUNIT_ASSERT(key != otherKey);
    }
    {
        Attribute attr("int vector", TYPE_INT_VECTOR, FLAGS_NONE, 0, 42);
        AttributeKey<IntVector> key(attr);
        Attribute sameAttr("same int vector", TYPE_INT_VECTOR, FLAGS_NONE, 0, 42);
        AttributeKey<IntVector> sameKey(attr);
        Attribute otherAttr("other int vector", TYPE_INT_VECTOR, FLAGS_NONE, 1, 42);
        AttributeKey<IntVector> otherKey(otherAttr);
        CPPUNIT_ASSERT(key == sameKey);
        CPPUNIT_ASSERT(!(key == otherKey));
        CPPUNIT_ASSERT(!(key != sameKey));
        CPPUNIT_ASSERT(key != otherKey);
    }
    {
        Attribute attr("long vector", TYPE_LONG_VECTOR, FLAGS_NONE, 0, 42);
        AttributeKey<LongVector> key(attr);
        Attribute sameAttr("same long vector", TYPE_LONG_VECTOR, FLAGS_NONE, 0, 42);
        AttributeKey<LongVector> sameKey(attr);
        Attribute otherAttr("other long vector", TYPE_LONG_VECTOR, FLAGS_NONE, 1, 42);
        AttributeKey<LongVector> otherKey(otherAttr);
        CPPUNIT_ASSERT(key == sameKey);
        CPPUNIT_ASSERT(!(key == otherKey));
        CPPUNIT_ASSERT(!(key != sameKey));
        CPPUNIT_ASSERT(key != otherKey);
    }
    {
        Attribute attr("float vector", TYPE_FLOAT_VECTOR, FLAGS_NONE, 0, 42);
        AttributeKey<FloatVector> key(attr);
        Attribute sameAttr("same float vector", TYPE_FLOAT_VECTOR, FLAGS_NONE, 0, 42);
        AttributeKey<FloatVector> sameKey(attr);
        Attribute otherAttr("other float vector", TYPE_FLOAT_VECTOR, FLAGS_NONE, 1, 42);
        AttributeKey<FloatVector> otherKey(otherAttr);
        CPPUNIT_ASSERT(key == sameKey);
        CPPUNIT_ASSERT(!(key == otherKey));
        CPPUNIT_ASSERT(!(key != sameKey));
        CPPUNIT_ASSERT(key != otherKey);
    }
    {
        Attribute attr("double vector", TYPE_DOUBLE_VECTOR, FLAGS_NONE, 0, 42);
        AttributeKey<DoubleVector> key(attr);
        Attribute sameAttr("same double vector", TYPE_DOUBLE_VECTOR, FLAGS_NONE, 0, 42);
        AttributeKey<DoubleVector> sameKey(attr);
        Attribute otherAttr("other double vector", TYPE_DOUBLE_VECTOR, FLAGS_NONE, 1, 42);
        AttributeKey<DoubleVector> otherKey(otherAttr);
        CPPUNIT_ASSERT(key == sameKey);
        CPPUNIT_ASSERT(!(key == otherKey));
        CPPUNIT_ASSERT(!(key != sameKey));
        CPPUNIT_ASSERT(key != otherKey);
    }
    {
        Attribute attr("string vector", TYPE_STRING_VECTOR, FLAGS_NONE, 0, 42);
        AttributeKey<StringVector> key(attr);
        Attribute sameAttr("same string vector", TYPE_STRING_VECTOR, FLAGS_NONE, 0, 42);
        AttributeKey<StringVector> sameKey(attr);
        Attribute otherAttr("other string vector", TYPE_STRING_VECTOR, FLAGS_NONE, 1, 42);
        AttributeKey<StringVector> otherKey(otherAttr);
        CPPUNIT_ASSERT(key == sameKey);
        CPPUNIT_ASSERT(!(key == otherKey));
        CPPUNIT_ASSERT(!(key != sameKey));
        CPPUNIT_ASSERT(key != otherKey);
    }
    {
        Attribute attr("rgb vector", TYPE_RGB_VECTOR, FLAGS_NONE, 0, 42);
        AttributeKey<RgbVector> key(attr);
        Attribute sameAttr("same rgb vector", TYPE_RGB_VECTOR, FLAGS_NONE, 0, 42);
        AttributeKey<RgbVector> sameKey(attr);
        Attribute otherAttr("other rgb vector", TYPE_RGB_VECTOR, FLAGS_NONE, 1, 42);
        AttributeKey<RgbVector> otherKey(otherAttr);
        CPPUNIT_ASSERT(key == sameKey);
        CPPUNIT_ASSERT(!(key == otherKey));
        CPPUNIT_ASSERT(!(key != sameKey));
        CPPUNIT_ASSERT(key != otherKey);
    }
    {
        Attribute attr("rgba vector", TYPE_RGBA_VECTOR, FLAGS_NONE, 0, 42);
        AttributeKey<RgbaVector> key(attr);
        Attribute sameAttr("same rgba vector", TYPE_RGBA_VECTOR, FLAGS_NONE, 0, 42);
        AttributeKey<RgbaVector> sameKey(attr);
        Attribute otherAttr("other rgba vector", TYPE_RGBA_VECTOR, FLAGS_NONE, 1, 42);
        AttributeKey<RgbaVector> otherKey(otherAttr);
        CPPUNIT_ASSERT(key == sameKey);
        CPPUNIT_ASSERT(!(key == otherKey));
        CPPUNIT_ASSERT(!(key != sameKey));
        CPPUNIT_ASSERT(key != otherKey);
    }
    {
        Attribute attr("vec2f vector", TYPE_VEC2F_VECTOR, FLAGS_NONE, 0, 42);
        AttributeKey<Vec2fVector> key(attr);
        Attribute sameAttr("same vec2f vector", TYPE_VEC2F_VECTOR, FLAGS_NONE, 0, 42);
        AttributeKey<Vec2fVector> sameKey(attr);
        Attribute otherAttr("other vec2f vector", TYPE_VEC2F_VECTOR, FLAGS_NONE, 1, 42);
        AttributeKey<Vec2fVector> otherKey(otherAttr);
        CPPUNIT_ASSERT(key == sameKey);
        CPPUNIT_ASSERT(!(key == otherKey));
        CPPUNIT_ASSERT(!(key != sameKey));
        CPPUNIT_ASSERT(key != otherKey);
    }
    {
        Attribute attr("vec2d vector", TYPE_VEC2D_VECTOR, FLAGS_NONE, 0, 42);
        AttributeKey<Vec2dVector> key(attr);
        Attribute sameAttr("same vec2d vector", TYPE_VEC2D_VECTOR, FLAGS_NONE, 0, 42);
        AttributeKey<Vec2dVector> sameKey(attr);
        Attribute otherAttr("other vec2d vector", TYPE_VEC2D_VECTOR, FLAGS_NONE, 1, 42);
        AttributeKey<Vec2dVector> otherKey(otherAttr);
        CPPUNIT_ASSERT(key == sameKey);
        CPPUNIT_ASSERT(!(key == otherKey));
        CPPUNIT_ASSERT(!(key != sameKey));
        CPPUNIT_ASSERT(key != otherKey);
    }
    {
        Attribute attr("vec3f vector", TYPE_VEC3F_VECTOR, FLAGS_NONE, 0, 42);
        AttributeKey<Vec3fVector> key(attr);
        Attribute sameAttr("same vec3f vector", TYPE_VEC3F_VECTOR, FLAGS_NONE, 0, 42);
        AttributeKey<Vec3fVector> sameKey(attr);
        Attribute otherAttr("other vec3f vector", TYPE_VEC3F_VECTOR, FLAGS_NONE, 1, 42);
        AttributeKey<Vec3fVector> otherKey(otherAttr);
        CPPUNIT_ASSERT(key == sameKey);
        CPPUNIT_ASSERT(!(key == otherKey));
        CPPUNIT_ASSERT(!(key != sameKey));
        CPPUNIT_ASSERT(key != otherKey);
    }
    {
        Attribute attr("vec3d vector", TYPE_VEC3D_VECTOR, FLAGS_NONE, 0, 42);
        AttributeKey<Vec3dVector> key(attr);
        Attribute sameAttr("same vec3d vector", TYPE_VEC3D_VECTOR, FLAGS_NONE, 0, 42);
        AttributeKey<Vec3dVector> sameKey(attr);
        Attribute otherAttr("other vec3d vector", TYPE_VEC3D_VECTOR, FLAGS_NONE, 1, 42);
        AttributeKey<Vec3dVector> otherKey(otherAttr);
        CPPUNIT_ASSERT(key == sameKey);
        CPPUNIT_ASSERT(!(key == otherKey));
        CPPUNIT_ASSERT(!(key != sameKey));
        CPPUNIT_ASSERT(key != otherKey);
    }
    {
        Attribute attr("mat4f vector", TYPE_MAT4F_VECTOR, FLAGS_NONE, 0, 42);
        AttributeKey<Mat4fVector> key(attr);
        Attribute sameAttr("same mat4f vector", TYPE_MAT4F_VECTOR, FLAGS_NONE, 0, 42);
        AttributeKey<Mat4fVector> sameKey(attr);
        Attribute otherAttr("other mat4f vector", TYPE_MAT4F_VECTOR, FLAGS_NONE, 1, 42);
        AttributeKey<Mat4fVector> otherKey(otherAttr);
        CPPUNIT_ASSERT(key == sameKey);
        CPPUNIT_ASSERT(!(key == otherKey));
        CPPUNIT_ASSERT(!(key != sameKey));
        CPPUNIT_ASSERT(key != otherKey);
    }
    {
        Attribute attr("mat4d vector", TYPE_MAT4D_VECTOR, FLAGS_NONE, 0, 42);
        AttributeKey<Mat4dVector> key(attr);
        Attribute sameAttr("same mat4d vector", TYPE_MAT4D_VECTOR, FLAGS_NONE, 0, 42);
        AttributeKey<Mat4dVector> sameKey(attr);
        Attribute otherAttr("other mat4d vector", TYPE_MAT4D_VECTOR, FLAGS_NONE, 1, 42);
        AttributeKey<Mat4dVector> otherKey(otherAttr);
        CPPUNIT_ASSERT(key == sameKey);
        CPPUNIT_ASSERT(!(key == otherKey));
        CPPUNIT_ASSERT(!(key != sameKey));
        CPPUNIT_ASSERT(key != otherKey);
    }
    {
        Attribute attr("scene object vector", TYPE_SCENE_OBJECT_VECTOR, FLAGS_NONE, 0, 42);
        AttributeKey<SceneObjectVector> key(attr);
        Attribute sameAttr("same scene object vector", TYPE_SCENE_OBJECT_VECTOR, FLAGS_NONE, 0, 42);
        AttributeKey<SceneObjectVector> sameKey(attr);
        Attribute otherAttr("other scene object vector", TYPE_SCENE_OBJECT_VECTOR, FLAGS_NONE, 1, 42);
        AttributeKey<SceneObjectVector> otherKey(otherAttr);
        CPPUNIT_ASSERT(key == sameKey);
        CPPUNIT_ASSERT(!(key == otherKey));
        CPPUNIT_ASSERT(!(key != sameKey));
        CPPUNIT_ASSERT(key != otherKey);
    }
    {
        Attribute attr("scene object indexable", TYPE_SCENE_OBJECT_INDEXABLE, FLAGS_NONE, 0, 42);
        AttributeKey<SceneObjectIndexable> key(attr);
        Attribute sameAttr("same scene object indexable", TYPE_SCENE_OBJECT_INDEXABLE, FLAGS_NONE, 0, 42);
        AttributeKey<SceneObjectIndexable> sameKey(attr);
        Attribute otherAttr("other scene object indexable", TYPE_SCENE_OBJECT_INDEXABLE, FLAGS_NONE, 1, 42);
        AttributeKey<SceneObjectIndexable> otherKey(otherAttr);
        CPPUNIT_ASSERT(key == sameKey);
        CPPUNIT_ASSERT(!(key == otherKey));
        CPPUNIT_ASSERT(!(key != sameKey));
        CPPUNIT_ASSERT(key != otherKey);
    }

    // Default constructed AttributeKeys are not valid, and should not be
    // equal to anything, including other invalid AttributeKeys.
    {
        Attribute attr("bool", TYPE_BOOL, FLAGS_NONE, 0, 42);
        AttributeKey<Bool> validKey(attr);
        AttributeKey<Bool> invalidKey;
        AttributeKey<Bool> otherInvalidKey;
        CPPUNIT_ASSERT(!(validKey == invalidKey));
        CPPUNIT_ASSERT(validKey != invalidKey);
        CPPUNIT_ASSERT(!(invalidKey == otherInvalidKey));
        CPPUNIT_ASSERT(invalidKey != otherInvalidKey);
    }
}

void
TestAttributeKey::testTypes()
{
    Attribute unknownAttr("unknown", TYPE_UNKNOWN, FLAGS_NONE, 0, 42);

    {
        Attribute attr("bool", TYPE_BOOL, FLAGS_NONE, 0, 42);
        CPPUNIT_ASSERT_NO_THROW(
            AttributeKey<Bool> key(attr);
        );
        CPPUNIT_ASSERT_THROW(
            AttributeKey<Bool> key(unknownAttr);
        , except::TypeError);
    }
    {
        Attribute attr("int", TYPE_INT, FLAGS_NONE, 0, 42);
        CPPUNIT_ASSERT_NO_THROW(
            AttributeKey<Int> key(attr);
        );
        CPPUNIT_ASSERT_THROW(
            AttributeKey<Int> key(unknownAttr);
        , except::TypeError);
    }
    {
        Attribute attr("long", TYPE_LONG, FLAGS_NONE, 0, 42);
        CPPUNIT_ASSERT_NO_THROW(
            AttributeKey<Long> key(attr);
        );
        CPPUNIT_ASSERT_THROW(
            AttributeKey<Long> key(unknownAttr);
        , except::TypeError);
    }
    {
        Attribute attr("float", TYPE_FLOAT, FLAGS_NONE, 0, 42);
        CPPUNIT_ASSERT_NO_THROW(
            AttributeKey<Float> key(attr);
        );
        CPPUNIT_ASSERT_THROW(
            AttributeKey<Float> key(unknownAttr);
        , except::TypeError);
    }
    {
        Attribute attr("double", TYPE_DOUBLE, FLAGS_NONE, 0, 42);
        CPPUNIT_ASSERT_NO_THROW(
            AttributeKey<Double> key(attr);
        );
        CPPUNIT_ASSERT_THROW(
            AttributeKey<Double> key(unknownAttr);
        , except::TypeError);
    }
    {
        Attribute attr("string", TYPE_STRING, FLAGS_NONE, 0, 42);
        CPPUNIT_ASSERT_NO_THROW(
            AttributeKey<String> key(attr);
        );
        CPPUNIT_ASSERT_THROW(
            AttributeKey<String> key(unknownAttr);
        , except::TypeError);
    }
    {
        Attribute attr("rgb", TYPE_RGB, FLAGS_NONE, 0, 42);
        CPPUNIT_ASSERT_NO_THROW(
            AttributeKey<Rgb> key(attr);
        );
        CPPUNIT_ASSERT_THROW(
            AttributeKey<Rgb> key(unknownAttr);
        , except::TypeError);
    }
    {
        Attribute attr("rgba", TYPE_RGBA, FLAGS_NONE, 0, 42);
        CPPUNIT_ASSERT_NO_THROW(
            AttributeKey<Rgba> key(attr);
        );
        CPPUNIT_ASSERT_THROW(
            AttributeKey<Rgba> key(unknownAttr);
        , except::TypeError);
    }
    {
        Attribute attr("vec2f", TYPE_VEC2F, FLAGS_NONE, 0, 42);
        CPPUNIT_ASSERT_NO_THROW(
            AttributeKey<Vec2f> key(attr);
        );
        CPPUNIT_ASSERT_THROW(
            AttributeKey<Vec2f> key(unknownAttr);
        , except::TypeError);
    }
    {
        Attribute attr("vec2d", TYPE_VEC2D, FLAGS_NONE, 0, 42);
        CPPUNIT_ASSERT_NO_THROW(
            AttributeKey<Vec2d> key(attr);
        );
        CPPUNIT_ASSERT_THROW(
            AttributeKey<Vec2d> key(unknownAttr);
        , except::TypeError);
    }
    {
        Attribute attr("vec3f", TYPE_VEC3F, FLAGS_NONE, 0, 42);
        CPPUNIT_ASSERT_NO_THROW(
            AttributeKey<Vec3f> key(attr);
        );
        CPPUNIT_ASSERT_THROW(
            AttributeKey<Vec3f> key(unknownAttr);
        , except::TypeError);
    }
    {
        Attribute attr("vec3d", TYPE_VEC3D, FLAGS_NONE, 0, 42);
        CPPUNIT_ASSERT_NO_THROW(
            AttributeKey<Vec3d> key(attr);
        );
        CPPUNIT_ASSERT_THROW(
            AttributeKey<Vec3d> key(unknownAttr);
        , except::TypeError);
    }
    {
        Attribute attr("mat4f", TYPE_MAT4F, FLAGS_NONE, 0, 42);
        CPPUNIT_ASSERT_NO_THROW(
            AttributeKey<Mat4f> key(attr);
        );
        CPPUNIT_ASSERT_THROW(
            AttributeKey<Mat4f> key(unknownAttr);
        , except::TypeError);
    }
    {
        Attribute attr("mat4d", TYPE_MAT4D, FLAGS_NONE, 0, 42);
        CPPUNIT_ASSERT_NO_THROW(
            AttributeKey<Mat4d> key(attr);
        );
        CPPUNIT_ASSERT_THROW(
            AttributeKey<Mat4d> key(unknownAttr);
        , except::TypeError);
    }
    {
        Attribute attr("scene object", TYPE_SCENE_OBJECT, FLAGS_NONE, 0, 42);
        CPPUNIT_ASSERT_NO_THROW(
            AttributeKey<SceneObject*> key(attr);
        );
        CPPUNIT_ASSERT_THROW(
            AttributeKey<SceneObject*> key(unknownAttr);
        , except::TypeError);
    }
    {
        Attribute attr("bool vector", TYPE_BOOL_VECTOR, FLAGS_NONE, 0, 42);
        CPPUNIT_ASSERT_NO_THROW(
            AttributeKey<BoolVector> key(attr);
        );
        CPPUNIT_ASSERT_THROW(
            AttributeKey<BoolVector> key(unknownAttr);
        , except::TypeError);
    }
    {
        Attribute attr("int vector", TYPE_INT_VECTOR, FLAGS_NONE, 0, 42);
        CPPUNIT_ASSERT_NO_THROW(
            AttributeKey<IntVector> key(attr);
        );
        CPPUNIT_ASSERT_THROW(
            AttributeKey<IntVector> key(unknownAttr);
        , except::TypeError);
    }
    {
        Attribute attr("long vector", TYPE_LONG_VECTOR, FLAGS_NONE, 0, 42);
        CPPUNIT_ASSERT_NO_THROW(
            AttributeKey<LongVector> key(attr);
        );
        CPPUNIT_ASSERT_THROW(
            AttributeKey<LongVector> key(unknownAttr);
        , except::TypeError);
    }
    {
        Attribute attr("float vector", TYPE_FLOAT_VECTOR, FLAGS_NONE, 0, 42);
        CPPUNIT_ASSERT_NO_THROW(
            AttributeKey<FloatVector> key(attr);
        );
        CPPUNIT_ASSERT_THROW(
            AttributeKey<FloatVector> key(unknownAttr);
        , except::TypeError);
    }
    {
        Attribute attr("double vector", TYPE_DOUBLE_VECTOR, FLAGS_NONE, 0, 42);
        CPPUNIT_ASSERT_NO_THROW(
            AttributeKey<DoubleVector> key(attr);
        );
        CPPUNIT_ASSERT_THROW(
            AttributeKey<DoubleVector> key(unknownAttr);
        , except::TypeError);
    }
    {
        Attribute attr("string vector", TYPE_STRING_VECTOR, FLAGS_NONE, 0, 42);
        CPPUNIT_ASSERT_NO_THROW(
            AttributeKey<StringVector> key(attr);
        );
        CPPUNIT_ASSERT_THROW(
            AttributeKey<StringVector> key(unknownAttr);
        , except::TypeError);
    }
    {
        Attribute attr("rgb vector", TYPE_RGB_VECTOR, FLAGS_NONE, 0, 42);
        CPPUNIT_ASSERT_NO_THROW(
            AttributeKey<RgbVector> key(attr);
        );
        CPPUNIT_ASSERT_THROW(
            AttributeKey<RgbVector> key(unknownAttr);
        , except::TypeError);
    }
    {
        Attribute attr("rgba vector", TYPE_RGBA_VECTOR, FLAGS_NONE, 0, 42);
        CPPUNIT_ASSERT_NO_THROW(
            AttributeKey<RgbaVector> key(attr);
        );
        CPPUNIT_ASSERT_THROW(
            AttributeKey<RgbaVector> key(unknownAttr);
        , except::TypeError);
    }
    {
        Attribute attr("vec2f vector", TYPE_VEC2F_VECTOR, FLAGS_NONE, 0, 42);
        CPPUNIT_ASSERT_NO_THROW(
            AttributeKey<Vec2fVector> key(attr);
        );
        CPPUNIT_ASSERT_THROW(
            AttributeKey<Vec2fVector> key(unknownAttr);
        , except::TypeError);
    }
    {
        Attribute attr("vec2d vector", TYPE_VEC2D_VECTOR, FLAGS_NONE, 0, 42);
        CPPUNIT_ASSERT_NO_THROW(
            AttributeKey<Vec2dVector> key(attr);
        );
        CPPUNIT_ASSERT_THROW(
            AttributeKey<Vec2dVector> key(unknownAttr);
        , except::TypeError);
    }
    {
        Attribute attr("vec3f vector", TYPE_VEC3F_VECTOR, FLAGS_NONE, 0, 42);
        CPPUNIT_ASSERT_NO_THROW(
            AttributeKey<Vec3fVector> key(attr);
        );
        CPPUNIT_ASSERT_THROW(
            AttributeKey<Vec3fVector> key(unknownAttr);
        , except::TypeError);
    }
    {
        Attribute attr("vec3d vector", TYPE_VEC3D_VECTOR, FLAGS_NONE, 0, 42);
        CPPUNIT_ASSERT_NO_THROW(
            AttributeKey<Vec3dVector> key(attr);
        );
        CPPUNIT_ASSERT_THROW(
            AttributeKey<Vec3dVector> key(unknownAttr);
        , except::TypeError);
    }
    {
        Attribute attr("mat4f vector", TYPE_MAT4F_VECTOR, FLAGS_NONE, 0, 42);
        CPPUNIT_ASSERT_NO_THROW(
            AttributeKey<Mat4fVector> key(attr);
        );
        CPPUNIT_ASSERT_THROW(
            AttributeKey<Mat4fVector> key(unknownAttr);
        , except::TypeError);
    }
    {
        Attribute attr("mat4d vector", TYPE_MAT4D_VECTOR, FLAGS_NONE, 0, 42);
        CPPUNIT_ASSERT_NO_THROW(
            AttributeKey<Mat4dVector> key(attr);
        );
        CPPUNIT_ASSERT_THROW(
            AttributeKey<Mat4dVector> key(unknownAttr);
        , except::TypeError);
    }
    {
        Attribute attr("scene object vector", TYPE_SCENE_OBJECT_VECTOR, FLAGS_NONE, 0, 42);
        CPPUNIT_ASSERT_NO_THROW(
            AttributeKey<SceneObjectVector> key(attr);
        );
        CPPUNIT_ASSERT_THROW(
            AttributeKey<SceneObjectVector> key(unknownAttr);
        , except::TypeError);
    }
    {
        Attribute attr("scene object indexable", TYPE_SCENE_OBJECT_INDEXABLE, FLAGS_NONE, 0, 42);
        CPPUNIT_ASSERT_NO_THROW(
            AttributeKey<SceneObjectIndexable> key(attr);
        );
        CPPUNIT_ASSERT_THROW(
            AttributeKey<SceneObjectIndexable> key(unknownAttr);
        , except::TypeError);
    }
}

void
TestAttributeKey::testIsValid()
{
    Attribute attr("bool", TYPE_BOOL, FLAGS_NONE, 0, 42);
    AttributeKey<Bool> validKey(attr);
    AttributeKey<Bool> invalidKey;
    CPPUNIT_ASSERT(validKey.isValid());
    CPPUNIT_ASSERT(!invalidKey.isValid());
}

void
TestAttributeKey::testIsBindable()
{
    Attribute bindableAttr("bindable float", TYPE_FLOAT, FLAGS_BINDABLE, 0, 42);
    AttributeKey<Float> bindableKey(bindableAttr);
    CPPUNIT_ASSERT(bindableKey.isBindable());

    Attribute simpleAttr("simple float", TYPE_FLOAT, FLAGS_NONE, 1, 64);
    AttributeKey<Float> simpleKey(simpleAttr);
    CPPUNIT_ASSERT(!simpleKey.isBindable());
}

void
TestAttributeKey::testIsBlurrable()
{
    Attribute blurrableAttr("blurrable float", TYPE_FLOAT, FLAGS_BLURRABLE, 0, 42);
    AttributeKey<Float> blurrableKey(blurrableAttr);
    CPPUNIT_ASSERT(blurrableKey.isBlurrable());

    Attribute simpleAttr("simple float", TYPE_FLOAT, FLAGS_NONE, 1, 64);
    AttributeKey<Float> simpleKey(simpleAttr);
    CPPUNIT_ASSERT(!simpleKey.isBlurrable());
}

void
TestAttributeKey::testIsEnumerable()
{
    Attribute enumerableAttr("enumerable int", TYPE_INT, FLAGS_ENUMERABLE, 0, 42);
    AttributeKey<Int> enumerableKey(enumerableAttr);
    CPPUNIT_ASSERT(enumerableKey.isEnumerable());

    Attribute simpleAttr("simple int", TYPE_INT, FLAGS_NONE, 1, 64);
    AttributeKey<Int> simpleKey(simpleAttr);
    CPPUNIT_ASSERT(!simpleKey.isEnumerable());
}

void
TestAttributeKey::testIsFilename()
{
    Attribute filenameAttr("filename string", TYPE_STRING, FLAGS_FILENAME, 0, 42);
    AttributeKey<String> filenameKey(filenameAttr);
    CPPUNIT_ASSERT(filenameKey.isFilename());

    Attribute simpleAttr("simple string", TYPE_STRING, FLAGS_NONE, 1, 64);
    AttributeKey<String> simpleKey(simpleAttr);
    CPPUNIT_ASSERT(!simpleKey.isFilename());
}

} // namespace unittest
} // namespace rdl2
} // namespace scene_rdl2

