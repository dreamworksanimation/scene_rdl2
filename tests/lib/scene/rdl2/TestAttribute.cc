// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "TestAttribute.h"

#include <scene_rdl2/scene/rdl2/Attribute.h>
#include <scene_rdl2/scene/rdl2/Types.h>

#include <scene_rdl2/common/except/exceptions.h>

#include <cppunit/extensions/HelperMacros.h>

#include <cstddef>
#include <string>
#include <stdint.h>

namespace scene_rdl2 {
namespace rdl2 {
namespace unittest {

void
TestAttribute::setUp()
{
    mConstant.reset(new Attribute("constant", TYPE_BOOL, FLAGS_NONE, 0, 32));
    mBindable.reset(new Attribute("bindable", TYPE_INT, FLAGS_BINDABLE, 1, 64));
    mBlurrable.reset(new Attribute("blurrable", TYPE_FLOAT, FLAGS_BLURRABLE, 2, 128));
    mBoth.reset(new Attribute("both", TYPE_DOUBLE, FLAGS_BINDABLE | FLAGS_BLURRABLE, 3, 256));
    mEnumerable.reset(new Attribute("enumerable", TYPE_INT, FLAGS_ENUMERABLE, 4, 512));
    mFilename.reset(new Attribute("filename", TYPE_STRING, FLAGS_FILENAME, 5, 1024));
}

void
TestAttribute::tearDown()
{
}

void
TestAttribute::testConstructBlurrable()
{
    // Test types that are blurrable. They should not throw.
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_INT, FLAGS_BLURRABLE, 0, 42);
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_LONG, FLAGS_BLURRABLE, 0, 42);
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_FLOAT, FLAGS_BLURRABLE, 0, 42);
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_DOUBLE, FLAGS_BLURRABLE, 0, 42);
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_RGB, FLAGS_BLURRABLE, 0, 42);
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_RGBA, FLAGS_BLURRABLE, 0, 42);
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_VEC2F, FLAGS_BLURRABLE, 0, 42);
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_VEC2D, FLAGS_BLURRABLE, 0, 42);
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_VEC3F, FLAGS_BLURRABLE, 0, 42);
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_VEC3D, FLAGS_BLURRABLE, 0, 42);
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_MAT4F, FLAGS_BLURRABLE, 0, 42);
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_MAT4D, FLAGS_BLURRABLE, 0, 42);
    );

    // Test types that are not blurrable. They should throw.
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_BOOL, FLAGS_BLURRABLE, 0, 42);
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_STRING, FLAGS_BLURRABLE, 0, 42);
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_SCENE_OBJECT, FLAGS_BLURRABLE, 0, 42);
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_BOOL_VECTOR, FLAGS_BLURRABLE, 0, 42);
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_INT_VECTOR, FLAGS_BLURRABLE, 0, 42);
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_LONG_VECTOR, FLAGS_BLURRABLE, 0, 42);
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_FLOAT_VECTOR, FLAGS_BLURRABLE, 0, 42);
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_DOUBLE_VECTOR, FLAGS_BLURRABLE, 0, 42);
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_STRING_VECTOR, FLAGS_BLURRABLE, 0, 42);
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_RGB_VECTOR, FLAGS_BLURRABLE, 0, 42);
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_RGBA_VECTOR, FLAGS_BLURRABLE, 0, 42);
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_VEC2F_VECTOR, FLAGS_BLURRABLE, 0, 42);
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_VEC2D_VECTOR, FLAGS_BLURRABLE, 0, 42);
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_VEC3F_VECTOR, FLAGS_BLURRABLE, 0, 42);
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_VEC3D_VECTOR, FLAGS_BLURRABLE, 0, 42);
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_MAT4F_VECTOR, FLAGS_BLURRABLE, 0, 42);
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_MAT4D_VECTOR, FLAGS_BLURRABLE, 0, 42);
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_SCENE_OBJECT_VECTOR, FLAGS_BLURRABLE, 0, 42);
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_SCENE_OBJECT_INDEXABLE, FLAGS_BLURRABLE, 0, 42);
    , except::TypeError);
}

void
TestAttribute::testConstructWithDefault()
{
    // Correct construction should not throw.
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_BOOL, FLAGS_NONE, 0, 42, false);
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_INT, FLAGS_NONE, 0, 42, Int(0));
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_LONG, FLAGS_NONE, 0, 42, Long(0));
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_FLOAT, FLAGS_NONE, 0, 42, 0.0f);
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_DOUBLE, FLAGS_NONE, 0, 42, 0.0);
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_STRING, FLAGS_NONE, 0, 42, String(""));
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_RGB, FLAGS_NONE, 0, 42, Rgb(0.0f, 0.0f, 0.0f));
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_RGBA, FLAGS_NONE, 0, 42, Rgba(0.0f, 0.0f, 0.0f, 0.0f));
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_VEC2F, FLAGS_NONE, 0, 42, Vec2f(0.0f, 0.0f));
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_VEC2D, FLAGS_NONE, 0, 42, Vec2d(0.0, 0.0));
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_VEC3F, FLAGS_NONE, 0, 42, Vec3f(0.0f, 0.0f, 0.0f));
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_VEC3D, FLAGS_NONE, 0, 42, Vec3d(0.0, 0.0, 0.0));
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_VEC4F, FLAGS_NONE, 0, 42, Vec4f(0.0f, 0.0f, 0.0f, 0.0f));
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_VEC4D, FLAGS_NONE, 0, 42, Vec4d(0.0, 0.0, 0.0, 0.0));
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_MAT4F, FLAGS_NONE, 0, 42, Mat4f(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f));
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_MAT4D, FLAGS_NONE, 0, 42, Mat4d(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0));
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_SCENE_OBJECT, FLAGS_NONE, 0, 42, static_cast<SceneObject*>(nullptr));
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_BOOL_VECTOR, FLAGS_NONE, 0, 42, BoolVector());
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_INT_VECTOR, FLAGS_NONE, 0, 42, IntVector());
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_LONG_VECTOR, FLAGS_NONE, 0, 42, LongVector());
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_FLOAT_VECTOR, FLAGS_NONE, 0, 42, FloatVector());
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_DOUBLE_VECTOR, FLAGS_NONE, 0, 42, DoubleVector());
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_STRING_VECTOR, FLAGS_NONE, 0, 42, StringVector());
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_RGB_VECTOR, FLAGS_NONE, 0, 42, RgbVector());
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_RGBA_VECTOR, FLAGS_NONE, 0, 42, RgbaVector());
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_VEC2F_VECTOR, FLAGS_NONE, 0, 42, Vec2fVector());
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_VEC2D_VECTOR, FLAGS_NONE, 0, 42, Vec2dVector());
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_VEC3F_VECTOR, FLAGS_NONE, 0, 42, Vec3fVector());
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_VEC3D_VECTOR, FLAGS_NONE, 0, 42, Vec3dVector());
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_VEC4F_VECTOR, FLAGS_NONE, 0, 42, Vec4fVector());
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_VEC4D_VECTOR, FLAGS_NONE, 0, 42, Vec4dVector());
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_MAT4F_VECTOR, FLAGS_NONE, 0, 42, Mat4fVector());
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_MAT4D_VECTOR, FLAGS_NONE, 0, 42, Mat4dVector());
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_SCENE_OBJECT_VECTOR, FLAGS_NONE, 0, 42, SceneObjectVector());
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_SCENE_OBJECT_INDEXABLE, FLAGS_NONE, 0, 42, SceneObjectIndexable());
    );

    // Construction of blurrables should throw (or not) according to the type.
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_BOOL, FLAGS_BLURRABLE, 0, 42, false);
    , except::TypeError);
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_INT, FLAGS_BLURRABLE, 0, 42, Int(0));
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_LONG, FLAGS_BLURRABLE, 0, 42, Long(0));
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_FLOAT, FLAGS_BLURRABLE, 0, 42, 0.0f);
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_DOUBLE, FLAGS_BLURRABLE, 0, 42, 0.0);
    );
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_STRING, FLAGS_BLURRABLE, 0, 42, String(""));
    , except::TypeError);
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_RGB, FLAGS_BLURRABLE, 0, 42, Rgb(0.0f, 0.0f, 0.0f));
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_RGBA, FLAGS_BLURRABLE, 0, 42, Rgba(0.0f, 0.0f, 0.0f, 0.0f));
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_VEC2F, FLAGS_BLURRABLE, 0, 42, Vec2f(0.0f, 0.0f));
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_VEC2D, FLAGS_BLURRABLE, 0, 42, Vec2d(0.0, 0.0));
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_VEC3F, FLAGS_BLURRABLE, 0, 42, Vec3f(0.0f, 0.0f, 0.0f));
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_VEC3D, FLAGS_BLURRABLE, 0, 42, Vec3d(0.0, 0.0, 0.0));
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_VEC4F, FLAGS_BLURRABLE, 0, 42, Vec4f(0.0f, 0.0f, 0.0f, 0.0f));
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_VEC4D, FLAGS_BLURRABLE, 0, 42, Vec4d(0.0, 0.0, 0.0, 0.0));
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_MAT4F, FLAGS_BLURRABLE, 0, 42, Mat4f(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f));
    );
    CPPUNIT_ASSERT_NO_THROW(
        Attribute attr("attr", TYPE_MAT4D, FLAGS_BLURRABLE, 0, 42, Mat4d(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0));
    );
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_SCENE_OBJECT, FLAGS_BLURRABLE, 0, 42, static_cast<SceneObject*>(nullptr));
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_BOOL_VECTOR, FLAGS_BLURRABLE, 0, 42, BoolVector());
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_INT_VECTOR, FLAGS_BLURRABLE, 0, 42, IntVector());
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_LONG_VECTOR, FLAGS_BLURRABLE, 0, 42, LongVector());
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_FLOAT_VECTOR, FLAGS_BLURRABLE, 0, 42, FloatVector());
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_DOUBLE_VECTOR, FLAGS_BLURRABLE, 0, 42, DoubleVector());
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_STRING_VECTOR, FLAGS_BLURRABLE, 0, 42, StringVector());
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_RGB_VECTOR, FLAGS_BLURRABLE, 0, 42, RgbVector());
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_RGBA_VECTOR, FLAGS_BLURRABLE, 0, 42, RgbaVector());
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_VEC2F_VECTOR, FLAGS_BLURRABLE, 0, 42, Vec2fVector());
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_VEC2D_VECTOR, FLAGS_BLURRABLE, 0, 42, Vec2dVector());
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_VEC3F_VECTOR, FLAGS_BLURRABLE, 0, 42, Vec3fVector());
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_VEC3D_VECTOR, FLAGS_BLURRABLE, 0, 42, Vec3dVector());
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_VEC4F_VECTOR, FLAGS_BLURRABLE, 0, 42, Vec4fVector());
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_VEC4D_VECTOR, FLAGS_BLURRABLE, 0, 42, Vec4dVector());
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_MAT4F_VECTOR, FLAGS_BLURRABLE, 0, 42, Mat4fVector());
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_MAT4D_VECTOR, FLAGS_BLURRABLE, 0, 42, Mat4dVector());
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_SCENE_OBJECT_VECTOR, FLAGS_BLURRABLE, 0, 42, SceneObjectVector());
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_SCENE_OBJECT_INDEXABLE, FLAGS_BLURRABLE, 0, 42, SceneObjectIndexable());
    , except::TypeError);

    // Mismatched attribute types and default value types should throw.
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_UNKNOWN, FLAGS_NONE, 0, 42, false);
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_UNKNOWN, FLAGS_NONE, 0, 42, Int(0));
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_UNKNOWN, FLAGS_NONE, 0, 42, Long(0));
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_UNKNOWN, FLAGS_NONE, 0, 42, 0.0f);
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_UNKNOWN, FLAGS_NONE, 0, 42, 0.0);
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_UNKNOWN, FLAGS_NONE, 0, 42, String(""));
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_UNKNOWN, FLAGS_NONE, 0, 42, Rgb(0.0f, 0.0f, 0.0f));
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_UNKNOWN, FLAGS_NONE, 0, 42, Rgba(0.0f, 0.0f, 0.0f, 0.0f));
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_UNKNOWN, FLAGS_NONE, 0, 42, Vec2f(0.0f, 0.0f));
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_UNKNOWN, FLAGS_NONE, 0, 42, Vec2d(0.0, 0.0));
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_UNKNOWN, FLAGS_NONE, 0, 42, Vec3f(0.0f, 0.0f, 0.0f));
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_UNKNOWN, FLAGS_NONE, 0, 42, Vec3d(0.0, 0.0, 0.0));
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_UNKNOWN, FLAGS_NONE, 0, 42, Vec4f(0.0f, 0.0f, 0.0f, 0.0f));
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_UNKNOWN, FLAGS_NONE, 0, 42, Vec4d(0.0, 0.0, 0.0, 0.0));
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_UNKNOWN, FLAGS_NONE, 0, 42, Mat4f(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f));
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_UNKNOWN, FLAGS_NONE, 0, 42, Mat4d(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0));
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_UNKNOWN, FLAGS_NONE, 0, 42, static_cast<SceneObject*>(nullptr));
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_UNKNOWN, FLAGS_NONE, 0, 42, BoolVector());
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_UNKNOWN, FLAGS_NONE, 0, 42, IntVector());
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_UNKNOWN, FLAGS_NONE, 0, 42, LongVector());
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_UNKNOWN, FLAGS_NONE, 0, 42, FloatVector());
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_UNKNOWN, FLAGS_NONE, 0, 42, DoubleVector());
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_UNKNOWN, FLAGS_NONE, 0, 42, String());
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_UNKNOWN, FLAGS_NONE, 0, 42, RgbVector());
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_UNKNOWN, FLAGS_NONE, 0, 42, RgbVector());
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_UNKNOWN, FLAGS_NONE, 0, 42, Vec2fVector());
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_UNKNOWN, FLAGS_NONE, 0, 42, Vec2dVector());
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_UNKNOWN, FLAGS_NONE, 0, 42, Vec3fVector());
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_UNKNOWN, FLAGS_NONE, 0, 42, Vec3dVector());
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_UNKNOWN, FLAGS_NONE, 0, 42, Vec4fVector());
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_UNKNOWN, FLAGS_NONE, 0, 42, Vec4dVector());
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_UNKNOWN, FLAGS_NONE, 0, 42, Mat4fVector());
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_UNKNOWN, FLAGS_NONE, 0, 42, Mat4dVector());
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        Attribute attr("attr", TYPE_UNKNOWN, FLAGS_NONE, 0, 42, SceneObjectVector());
    , except::TypeError);
}

void
TestAttribute::testGetName()
{
    CPPUNIT_ASSERT_EQUAL(std::string("constant"), mConstant->getName());
    CPPUNIT_ASSERT_EQUAL(std::string("bindable"), mBindable->getName());
    CPPUNIT_ASSERT_EQUAL(std::string("blurrable"), mBlurrable->getName());
    CPPUNIT_ASSERT_EQUAL(std::string("both"), mBoth->getName());
}

void
TestAttribute::testGetType()
{
    CPPUNIT_ASSERT_EQUAL(TYPE_BOOL, mConstant->getType());
    CPPUNIT_ASSERT_EQUAL(TYPE_INT, mBindable->getType());
    CPPUNIT_ASSERT_EQUAL(TYPE_FLOAT, mBlurrable->getType());
    CPPUNIT_ASSERT_EQUAL(TYPE_DOUBLE, mBoth->getType());
}

void
TestAttribute::testGetFlags()
{
    CPPUNIT_ASSERT(mConstant->getFlags() == FLAGS_NONE);
    CPPUNIT_ASSERT(mBindable->getFlags() == FLAGS_BINDABLE);
    CPPUNIT_ASSERT(mBlurrable->getFlags() == FLAGS_BLURRABLE);
    CPPUNIT_ASSERT(mBoth->getFlags() == (FLAGS_BINDABLE | FLAGS_BLURRABLE));
    CPPUNIT_ASSERT(mEnumerable->getFlags() == FLAGS_ENUMERABLE);
    CPPUNIT_ASSERT(mFilename->getFlags() == FLAGS_FILENAME);
}

void
TestAttribute::testGetDefaultValue()
{
    Attribute boolAttr("bool", TYPE_BOOL, FLAGS_NONE, 0, 42, true);
    CPPUNIT_ASSERT(boolAttr.getDefaultValue<Bool>() == true);
    CPPUNIT_ASSERT_THROW(boolAttr.getDefaultValue<Int>(), except::TypeError);

    Attribute intAttr("int", TYPE_INT, FLAGS_NONE, 0, 42, Int(100));
    CPPUNIT_ASSERT(intAttr.getDefaultValue<Int>() == Int(100));
    CPPUNIT_ASSERT_THROW(intAttr.getDefaultValue<Bool>(), except::TypeError);

    Attribute longAttr("long", TYPE_LONG, FLAGS_NONE, 0, 42, Long(101));
    CPPUNIT_ASSERT(longAttr.getDefaultValue<Long>() == Long(101));
    CPPUNIT_ASSERT_THROW(longAttr.getDefaultValue<Bool>(), except::TypeError);

    Attribute floatAttr("float", TYPE_FLOAT, FLAGS_NONE, 0, 42, 1.0f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f, floatAttr.getDefaultValue<Float>(), 0.0001f);
    CPPUNIT_ASSERT_THROW(floatAttr.getDefaultValue<Bool>(), except::TypeError);

    Attribute doubleAttr("double", TYPE_DOUBLE, FLAGS_NONE, 0, 42, 2.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, doubleAttr.getDefaultValue<Double>(), 0.0001);
    CPPUNIT_ASSERT_THROW(doubleAttr.getDefaultValue<Bool>(), except::TypeError);

    Attribute stringAttr("string", TYPE_STRING, FLAGS_NONE, 0, 42, String("wat"));
    CPPUNIT_ASSERT(stringAttr.getDefaultValue<String>() == String("wat"));
    CPPUNIT_ASSERT_THROW(stringAttr.getDefaultValue<Bool>(), except::TypeError);

    Attribute rgbAttr("rgb", TYPE_RGB, FLAGS_NONE, 0, 42, Rgb(0.1f, 0.2f, 0.3f));
    CPPUNIT_ASSERT(rgbAttr.getDefaultValue<Rgb>() == Rgb(0.1f, 0.2f, 0.3f));
    CPPUNIT_ASSERT_THROW(rgbAttr.getDefaultValue<Bool>(), except::TypeError);

    Attribute rgbaAttr("rgba", TYPE_RGBA, FLAGS_NONE, 0, 42, Rgba(0.1f, 0.2f, 0.3f, 0.4f));
    CPPUNIT_ASSERT(rgbaAttr.getDefaultValue<Rgba>() == Rgba(0.1f, 0.2f, 0.3f, 0.4f));
    CPPUNIT_ASSERT_THROW(rgbaAttr.getDefaultValue<Bool>(), except::TypeError);

    Attribute vec2fAttr("vec2f", TYPE_VEC2F, FLAGS_NONE, 0, 42, Vec2f(1.0f, 2.0f));
    CPPUNIT_ASSERT(vec2fAttr.getDefaultValue<Vec2f>() == Vec2f(1.0f, 2.0f));
    CPPUNIT_ASSERT_THROW(vec2fAttr.getDefaultValue<Bool>(), except::TypeError);

    Attribute vec2dAttr("vec2d", TYPE_VEC2D, FLAGS_NONE, 0, 42, Vec2d(1.0, 2.0));
    CPPUNIT_ASSERT(vec2dAttr.getDefaultValue<Vec2d>() == Vec2d(1.0, 2.0));
    CPPUNIT_ASSERT_THROW(vec2dAttr.getDefaultValue<Bool>(), except::TypeError);

    Attribute vec3fAttr("vec3f", TYPE_VEC3F, FLAGS_NONE, 0, 42, Vec3f(1.0f, 2.0f, 3.0f));
    CPPUNIT_ASSERT(vec3fAttr.getDefaultValue<Vec3f>() == Vec3f(1.0f, 2.0f, 3.0f));
    CPPUNIT_ASSERT_THROW(vec3fAttr.getDefaultValue<Bool>(), except::TypeError);

    Attribute vec3dAttr("vec3d", TYPE_VEC3D, FLAGS_NONE, 0, 42, Vec3d(1.0, 2.0, 3.0));
    CPPUNIT_ASSERT(vec3dAttr.getDefaultValue<Vec3d>() == Vec3d(1.0, 2.0, 3.0));
    CPPUNIT_ASSERT_THROW(vec3dAttr.getDefaultValue<Bool>(), except::TypeError);

    Attribute vec4fAttr("vec4f", TYPE_VEC4F, FLAGS_NONE, 0, 42, Vec4f(1.0f, 2.0f, 3.0f, 4.0f));
    CPPUNIT_ASSERT(vec4fAttr.getDefaultValue<Vec4f>() == Vec4f(1.0f, 2.0f, 3.0f, 4.0f));
    CPPUNIT_ASSERT_THROW(vec4fAttr.getDefaultValue<Bool>(), except::TypeError);

    Attribute vec4dAttr("vec4d", TYPE_VEC4D, FLAGS_NONE, 0, 42, Vec4d(1.0, 2.0, 3.0, 4.0));
    CPPUNIT_ASSERT(vec4dAttr.getDefaultValue<Vec4d>() == Vec4d(1.0, 2.0, 3.0, 4.0));
    CPPUNIT_ASSERT_THROW(vec4dAttr.getDefaultValue<Bool>(), except::TypeError);

    Attribute mat4fAttr("mat4f", TYPE_MAT4F, FLAGS_NONE, 0, 42, Mat4f(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f));
    CPPUNIT_ASSERT(mat4fAttr.getDefaultValue<Mat4f>() == Mat4f(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f));
    CPPUNIT_ASSERT_THROW(mat4fAttr.getDefaultValue<Bool>(), except::TypeError);

    Attribute mat4dAttr("mat4d", TYPE_MAT4D, FLAGS_NONE, 0, 42, Mat4d(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0));
    CPPUNIT_ASSERT(mat4dAttr.getDefaultValue<Mat4d>() == Mat4d(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0));
    CPPUNIT_ASSERT_THROW(mat4dAttr.getDefaultValue<Bool>(), except::TypeError);

    Attribute sceneObjectAttr("scene object", TYPE_SCENE_OBJECT, FLAGS_NONE, 0, 42, (SceneObject*)0xdeadbeef);
    CPPUNIT_ASSERT(sceneObjectAttr.getDefaultValue<SceneObject*>() == (SceneObject*)0xdeadbeef);
    CPPUNIT_ASSERT_THROW(sceneObjectAttr.getDefaultValue<Bool>(), except::TypeError);

    BoolVector boolVec;
    boolVec.push_back(true);
    boolVec.push_back(false);
    Attribute boolVectorAttr("bool vector", TYPE_BOOL_VECTOR, FLAGS_NONE, 0, 42, boolVec);
    CPPUNIT_ASSERT(boolVectorAttr.getDefaultValue<BoolVector>() == boolVec);
    CPPUNIT_ASSERT_THROW(boolVectorAttr.getDefaultValue<Int>(), except::TypeError);

    IntVector intVec;
    intVec.push_back(Int(100));
    intVec.push_back(Int(101));
    Attribute intVectorAttr("int vector", TYPE_INT_VECTOR, FLAGS_NONE, 0, 42, intVec);
    CPPUNIT_ASSERT(intVectorAttr.getDefaultValue<IntVector>() == intVec);
    CPPUNIT_ASSERT_THROW(intVectorAttr.getDefaultValue<Bool>(), except::TypeError);

    LongVector longVec;
    longVec.push_back(Long(102));
    longVec.push_back(Long(103));
    Attribute longVectorAttr("long vector", TYPE_LONG_VECTOR, FLAGS_NONE, 0, 42, longVec);
    CPPUNIT_ASSERT(longVectorAttr.getDefaultValue<LongVector>() == longVec);
    CPPUNIT_ASSERT_THROW(longVectorAttr.getDefaultValue<Bool>(), except::TypeError);

    FloatVector floatVec;
    floatVec.push_back(1.0f);
    floatVec.push_back(2.0f);
    Attribute floatVectorAttr("float vector", TYPE_FLOAT_VECTOR, FLAGS_NONE, 0, 42, floatVec);
    CPPUNIT_ASSERT(floatVectorAttr.getDefaultValue<FloatVector>() == floatVec);
    CPPUNIT_ASSERT_THROW(floatVectorAttr.getDefaultValue<Bool>(), except::TypeError);

    DoubleVector doubleVec;
    doubleVec.push_back(3.0);
    doubleVec.push_back(4.0);
    Attribute doubleVectorAttr("double vector", TYPE_DOUBLE_VECTOR, FLAGS_NONE, 0, 42, doubleVec);
    CPPUNIT_ASSERT(doubleVectorAttr.getDefaultValue<DoubleVector>() == doubleVec);
    CPPUNIT_ASSERT_THROW(doubleVectorAttr.getDefaultValue<Bool>(), except::TypeError);

    StringVector stringVec;
    stringVec.push_back("a");
    stringVec.push_back("b");
    Attribute stringVectorAttr("string vector", TYPE_STRING_VECTOR, FLAGS_NONE, 0, 42, stringVec);
    CPPUNIT_ASSERT(stringVectorAttr.getDefaultValue<StringVector>() == stringVec);
    CPPUNIT_ASSERT_THROW(stringVectorAttr.getDefaultValue<Bool>(), except::TypeError);

    RgbVector rgbVec;
    rgbVec.push_back(Rgb(0.1f, 0.2f, 0.3f));
    rgbVec.push_back(Rgb(0.4f, 0.5f, 0.6f));
    Attribute rgbVectorAttr("rgb vector", TYPE_RGB_VECTOR, FLAGS_NONE, 0, 42, rgbVec);
    CPPUNIT_ASSERT(rgbVectorAttr.getDefaultValue<RgbVector>() == rgbVec);
    CPPUNIT_ASSERT_THROW(rgbVectorAttr.getDefaultValue<Bool>(), except::TypeError);

    RgbaVector rgbaVec;
    rgbaVec.push_back(Rgba(0.1f, 0.2f, 0.3f, 0.4f));
    rgbaVec.push_back(Rgba(0.5f, 0.6f, 0.7f, 0.8f));
    Attribute rgbaVectorAttr("rgba vector", TYPE_RGBA_VECTOR, FLAGS_NONE, 0, 42, rgbaVec);
    CPPUNIT_ASSERT(rgbaVectorAttr.getDefaultValue<RgbaVector>() == rgbaVec);
    CPPUNIT_ASSERT_THROW(rgbaVectorAttr.getDefaultValue<Bool>(), except::TypeError);

    Vec2fVector vec2fVec;
    vec2fVec.push_back(Vec2f(1.0f, 2.0f));
    vec2fVec.push_back(Vec2f(3.0f, 4.0f));
    Attribute vec2fVectorAttr("vec2f vector", TYPE_VEC2F_VECTOR, FLAGS_NONE, 0, 42, vec2fVec);
    CPPUNIT_ASSERT(vec2fVectorAttr.getDefaultValue<Vec2fVector>() == vec2fVec);
    CPPUNIT_ASSERT_THROW(vec2fVectorAttr.getDefaultValue<Bool>(), except::TypeError);

    Vec2dVector vec2dVec;
    vec2dVec.push_back(Vec2d(1.0, 2.0));
    vec2dVec.push_back(Vec2d(3.0, 4.0));
    Attribute vec2dVectorAttr("vec2d vector", TYPE_VEC2D_VECTOR, FLAGS_NONE, 0, 42, vec2dVec);
    CPPUNIT_ASSERT(vec2dVectorAttr.getDefaultValue<Vec2dVector>() == vec2dVec);
    CPPUNIT_ASSERT_THROW(vec2dVectorAttr.getDefaultValue<Bool>(), except::TypeError);

    Vec3fVector vec3fVec;
    vec3fVec.push_back(Vec3f(1.0f, 2.0f, 3.0f));
    vec3fVec.push_back(Vec3f(4.0f, 5.0f, 6.0f));
    Attribute vec3fVectorAttr("vec3f vector", TYPE_VEC3F_VECTOR, FLAGS_NONE, 0, 42, vec3fVec);
    CPPUNIT_ASSERT(vec3fVectorAttr.getDefaultValue<Vec3fVector>() == vec3fVec);
    CPPUNIT_ASSERT_THROW(vec3fVectorAttr.getDefaultValue<Bool>(), except::TypeError);

    Vec3dVector vec3dVec;
    vec3dVec.push_back(Vec3d(1.0, 2.0, 3.0));
    vec3dVec.push_back(Vec3d(4.0, 5.0, 6.0));
    Attribute vec3dVectorAttr("vec3d vector", TYPE_VEC3D_VECTOR, FLAGS_NONE, 0, 42, vec3dVec);
    CPPUNIT_ASSERT(vec3dVectorAttr.getDefaultValue<Vec3dVector>() == vec3dVec);
    CPPUNIT_ASSERT_THROW(vec3dVectorAttr.getDefaultValue<Bool>(), except::TypeError);

    Vec4fVector vec4fVec;
    vec4fVec.push_back(Vec4f(1.0f, 2.0f, 3.0f, 4.0f));
    vec4fVec.push_back(Vec4f(5.0f, 6.0f, 7.0f, 8.0f));
    Attribute vec4fVectorAttr("vec4f vector", TYPE_VEC4F_VECTOR, FLAGS_NONE, 0, 42, vec4fVec);
    CPPUNIT_ASSERT(vec4fVectorAttr.getDefaultValue<Vec4fVector>() == vec4fVec);
    CPPUNIT_ASSERT_THROW(vec4fVectorAttr.getDefaultValue<Bool>(), except::TypeError);

    Vec4dVector vec4dVec;
    vec4dVec.push_back(Vec4d(1.0, 2.0, 3.0, 4.0));
    vec4dVec.push_back(Vec4d(5.0, 6.0, 7.0, 8.0));
    Attribute vec4dVectorAttr("vec4d vector", TYPE_VEC4D_VECTOR, FLAGS_NONE, 0, 42, vec4dVec);
    CPPUNIT_ASSERT(vec4dVectorAttr.getDefaultValue<Vec4dVector>() == vec4dVec);
    CPPUNIT_ASSERT_THROW(vec4dVectorAttr.getDefaultValue<Bool>(), except::TypeError);

    Mat4fVector mat4fVec;
    mat4fVec.push_back(Mat4f(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f));
    mat4fVec.push_back(Mat4f(17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f, 24.0f, 25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f, 32.0f));
    Attribute mat4fVectorAttr("mat4f vector", TYPE_MAT4F_VECTOR, FLAGS_NONE, 0, 42, mat4fVec);
    CPPUNIT_ASSERT(mat4fVectorAttr.getDefaultValue<Mat4fVector>() == mat4fVec);
    CPPUNIT_ASSERT_THROW(mat4fVectorAttr.getDefaultValue<Bool>(), except::TypeError);

    Mat4dVector mat4dVec;
    mat4dVec.push_back(Mat4d(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0));
    mat4dVec.push_back(Mat4d(17.0, 18.0, 19.0, 20.0, 21.0, 22.0, 23.0, 24.0, 25.0, 26.0, 27.0, 28.0, 29.0, 30.0, 31.0, 32.0));
    Attribute mat4dVectorAttr("mat4d vector", TYPE_MAT4D_VECTOR, FLAGS_NONE, 0, 42, mat4dVec);
    CPPUNIT_ASSERT(mat4dVectorAttr.getDefaultValue<Mat4dVector>() == mat4dVec);
    CPPUNIT_ASSERT_THROW(mat4dVectorAttr.getDefaultValue<Bool>(), except::TypeError);

    SceneObjectVector sceneObjectVec;
    sceneObjectVec.push_back((SceneObject*)0xdeadbeef);
    sceneObjectVec.push_back((SceneObject*)0xc001d00d);
    Attribute sceneObjectVectorAttr("scene object vector", TYPE_SCENE_OBJECT_VECTOR, FLAGS_NONE, 0, 42, sceneObjectVec);
    CPPUNIT_ASSERT(sceneObjectVectorAttr.getDefaultValue<SceneObjectVector>() == sceneObjectVec);
    CPPUNIT_ASSERT_THROW(sceneObjectVectorAttr.getDefaultValue<Bool>(), except::TypeError);

    SceneObjectIndexable sceneObjectIdx;
    sceneObjectIdx.push_back((SceneObject*)0xdeadbeef);
    sceneObjectIdx.push_back((SceneObject*)0xc001d00d);
    Attribute sceneObjectIndexableAttr("scene object indexable", TYPE_SCENE_OBJECT_INDEXABLE, FLAGS_NONE, 0, 42, sceneObjectIdx);
    CPPUNIT_ASSERT(sceneObjectIndexableAttr.getDefaultValue<SceneObjectIndexable>() == sceneObjectIdx);
    CPPUNIT_ASSERT_THROW(sceneObjectIndexableAttr.getDefaultValue<Bool>(), except::TypeError);
}

void
TestAttribute::testIndex()
{
    CPPUNIT_ASSERT(mConstant->mIndex == 0);
    CPPUNIT_ASSERT(mBindable->mIndex == 1);
    CPPUNIT_ASSERT(mBlurrable->mIndex == 2);
    CPPUNIT_ASSERT(mBoth->mIndex == 3);
}

void
TestAttribute::testOffset()
{
    CPPUNIT_ASSERT(mConstant->mOffset == 32);
    CPPUNIT_ASSERT(mBindable->mOffset == 64);
    CPPUNIT_ASSERT(mBlurrable->mOffset == 128);
    CPPUNIT_ASSERT(mBoth->mOffset == 256);
}

void
TestAttribute::testIsBindable()
{
    CPPUNIT_ASSERT_EQUAL(false, mConstant->isBindable());
    CPPUNIT_ASSERT_EQUAL(true, mBindable->isBindable());
    CPPUNIT_ASSERT_EQUAL(false, mBlurrable->isBindable());
    CPPUNIT_ASSERT_EQUAL(true, mBoth->isBindable());
    CPPUNIT_ASSERT_EQUAL(false, mEnumerable->isBindable());
    CPPUNIT_ASSERT_EQUAL(false, mFilename->isBindable());
}

void
TestAttribute::testIsBlurrable()
{
    CPPUNIT_ASSERT_EQUAL(false, mConstant->isBlurrable());
    CPPUNIT_ASSERT_EQUAL(false, mBindable->isBlurrable());
    CPPUNIT_ASSERT_EQUAL(true, mBlurrable->isBlurrable());
    CPPUNIT_ASSERT_EQUAL(true, mBoth->isBlurrable());
    CPPUNIT_ASSERT_EQUAL(false, mEnumerable->isBlurrable());
    CPPUNIT_ASSERT_EQUAL(false, mFilename->isBlurrable());
}

void
TestAttribute::testIsEnumerable()
{
    CPPUNIT_ASSERT_EQUAL(false, mConstant->isEnumerable());
    CPPUNIT_ASSERT_EQUAL(false, mBindable->isEnumerable());
    CPPUNIT_ASSERT_EQUAL(false, mBlurrable->isEnumerable());
    CPPUNIT_ASSERT_EQUAL(false, mBoth->isEnumerable());
    CPPUNIT_ASSERT_EQUAL(true, mEnumerable->isEnumerable());
    CPPUNIT_ASSERT_EQUAL(false, mFilename->isEnumerable());
}

void
TestAttribute::testIsFilename()
{
    CPPUNIT_ASSERT_EQUAL(false, mConstant->isFilename());
    CPPUNIT_ASSERT_EQUAL(false, mBindable->isFilename());
    CPPUNIT_ASSERT_EQUAL(false, mBlurrable->isFilename());
    CPPUNIT_ASSERT_EQUAL(false, mBoth->isFilename());
    CPPUNIT_ASSERT_EQUAL(false, mEnumerable->isFilename());
    CPPUNIT_ASSERT_EQUAL(true, mFilename->isFilename());
}

void
TestAttribute::testSetMetadata()
{
    CPPUNIT_ASSERT_NO_THROW(mConstant->setMetadata("description", "A constant attribute."));
    CPPUNIT_ASSERT_NO_THROW(mConstant->setMetadata("min", "0.0"));
    CPPUNIT_ASSERT_NO_THROW(mConstant->setMetadata("max", "1.0"));

    CPPUNIT_ASSERT_NO_THROW(mBindable->setMetadata("description", "A bindable attribute."));
    CPPUNIT_ASSERT_NO_THROW(mBindable->setMetadata("min", "0.0"));
    CPPUNIT_ASSERT_NO_THROW(mBindable->setMetadata("max", "1.0"));

    CPPUNIT_ASSERT_NO_THROW(mBlurrable->setMetadata("description", "A blurrable attribute."));
    CPPUNIT_ASSERT_NO_THROW(mBlurrable->setMetadata("min", "0.0"));
    CPPUNIT_ASSERT_NO_THROW(mBlurrable->setMetadata("max", "1.0"));

    CPPUNIT_ASSERT_NO_THROW(mBoth->setMetadata("description", "A bindable and blurrable attribute."));
    CPPUNIT_ASSERT_NO_THROW(mBoth->setMetadata("min", "0.0"));
    CPPUNIT_ASSERT_NO_THROW(mBoth->setMetadata("max", "1.0"));
}

void
TestAttribute::testGetMetadata()
{
    testSetMetadata();

    CPPUNIT_ASSERT_EQUAL(std::string("A constant attribute."), mConstant->getMetadata("description"));
    CPPUNIT_ASSERT_EQUAL(std::string("0.0"), mConstant->getMetadata("min"));
    CPPUNIT_ASSERT_EQUAL(std::string("1.0"), mConstant->getMetadata("max"));
    CPPUNIT_ASSERT_THROW(mConstant->getMetadata("default"), except::KeyError);

    CPPUNIT_ASSERT_EQUAL(std::string("A bindable attribute."), mBindable->getMetadata("description"));
    CPPUNIT_ASSERT_EQUAL(std::string("0.0"), mBindable->getMetadata("min"));
    CPPUNIT_ASSERT_EQUAL(std::string("1.0"), mBindable->getMetadata("max"));
    CPPUNIT_ASSERT_THROW(mBindable->getMetadata("default"), except::KeyError);

    CPPUNIT_ASSERT_EQUAL(std::string("A blurrable attribute."), mBlurrable->getMetadata("description"));
    CPPUNIT_ASSERT_EQUAL(std::string("0.0"), mBlurrable->getMetadata("min"));
    CPPUNIT_ASSERT_EQUAL(std::string("1.0"), mBlurrable->getMetadata("max"));
    CPPUNIT_ASSERT_THROW(mBlurrable->getMetadata("default"), except::KeyError);

    CPPUNIT_ASSERT_EQUAL(std::string("A bindable and blurrable attribute."), mBoth->getMetadata("description"));
    CPPUNIT_ASSERT_EQUAL(std::string("0.0"), mBoth->getMetadata("min"));
    CPPUNIT_ASSERT_EQUAL(std::string("1.0"), mBoth->getMetadata("max"));
    CPPUNIT_ASSERT_THROW(mBoth->getMetadata("default"), except::KeyError);
}

void
TestAttribute::testMetadataExists()
{
    testSetMetadata();

    CPPUNIT_ASSERT(mConstant->metadataExists("description"));
    CPPUNIT_ASSERT(mConstant->metadataExists("min"));
    CPPUNIT_ASSERT(mConstant->metadataExists("max"));
    CPPUNIT_ASSERT(!mConstant->metadataExists("pizza"));

    CPPUNIT_ASSERT(mBindable->metadataExists("description"));
    CPPUNIT_ASSERT(mBindable->metadataExists("min"));
    CPPUNIT_ASSERT(mBindable->metadataExists("max"));
    CPPUNIT_ASSERT(!mBindable->metadataExists("pizza"));

    CPPUNIT_ASSERT(mBlurrable->metadataExists("description"));
    CPPUNIT_ASSERT(mBlurrable->metadataExists("min"));
    CPPUNIT_ASSERT(mBlurrable->metadataExists("max"));
    CPPUNIT_ASSERT(!mBlurrable->metadataExists("pizza"));

    CPPUNIT_ASSERT(mBoth->metadataExists("description"));
    CPPUNIT_ASSERT(mBoth->metadataExists("min"));
    CPPUNIT_ASSERT(mBoth->metadataExists("max"));
    CPPUNIT_ASSERT(!mBoth->metadataExists("pizza"));
}

void
TestAttribute::testIterateMetadata()
{
    testSetMetadata();
    
    Attribute::MetadataConstIterator constantIter;
    CPPUNIT_ASSERT_NO_THROW(constantIter = mConstant->beginMetadata());
    Attribute::MetadataConstIterator constantEnd;
    CPPUNIT_ASSERT_NO_THROW(constantEnd = mConstant->endMetadata());
    CPPUNIT_ASSERT_EQUAL(std::string("description"), constantIter->first);
    CPPUNIT_ASSERT_EQUAL(std::string("A constant attribute."), constantIter->second);
    ++constantIter;
    CPPUNIT_ASSERT_EQUAL(std::string("max"), constantIter->first);
    CPPUNIT_ASSERT_EQUAL(std::string("1.0"), constantIter->second);
    ++constantIter;
    CPPUNIT_ASSERT_EQUAL(std::string("min"), constantIter->first);
    CPPUNIT_ASSERT_EQUAL(std::string("0.0"), constantIter->second);
    ++constantIter;
    CPPUNIT_ASSERT(constantEnd == constantIter);

    Attribute::MetadataConstIterator bindableIter;
    CPPUNIT_ASSERT_NO_THROW(bindableIter = mBindable->beginMetadata());
    Attribute::MetadataConstIterator bindableEnd;
    CPPUNIT_ASSERT_NO_THROW(bindableEnd = mBindable->endMetadata());
    CPPUNIT_ASSERT_EQUAL(std::string("description"), bindableIter->first);
    CPPUNIT_ASSERT_EQUAL(std::string("A bindable attribute."), bindableIter->second);
    ++bindableIter;
    CPPUNIT_ASSERT_EQUAL(std::string("max"), bindableIter->first);
    CPPUNIT_ASSERT_EQUAL(std::string("1.0"), bindableIter->second);
    ++bindableIter;
    CPPUNIT_ASSERT_EQUAL(std::string("min"), bindableIter->first);
    CPPUNIT_ASSERT_EQUAL(std::string("0.0"), bindableIter->second);
    ++bindableIter;
    CPPUNIT_ASSERT(bindableEnd == bindableIter);

    Attribute::MetadataConstIterator blurrableIter;
    CPPUNIT_ASSERT_NO_THROW(blurrableIter = mBlurrable->beginMetadata());
    Attribute::MetadataConstIterator blurrableEnd;
    CPPUNIT_ASSERT_NO_THROW(blurrableEnd = mBlurrable->endMetadata());
    CPPUNIT_ASSERT_EQUAL(std::string("description"), blurrableIter->first);
    CPPUNIT_ASSERT_EQUAL(std::string("A blurrable attribute."), blurrableIter->second);
    ++blurrableIter;
    CPPUNIT_ASSERT_EQUAL(std::string("max"), blurrableIter->first);
    CPPUNIT_ASSERT_EQUAL(std::string("1.0"), blurrableIter->second);
    ++blurrableIter;
    CPPUNIT_ASSERT_EQUAL(std::string("min"), blurrableIter->first);
    CPPUNIT_ASSERT_EQUAL(std::string("0.0"), blurrableIter->second);
    ++blurrableIter;
    CPPUNIT_ASSERT(blurrableEnd == blurrableIter);

    Attribute::MetadataConstIterator bothIter;
    CPPUNIT_ASSERT_NO_THROW(bothIter = mBoth->beginMetadata());
    Attribute::MetadataConstIterator bothEnd;
    CPPUNIT_ASSERT_NO_THROW(bothEnd = mBoth->endMetadata());
    CPPUNIT_ASSERT_EQUAL(std::string("description"), bothIter->first);
    CPPUNIT_ASSERT_EQUAL(std::string("A bindable and blurrable attribute."), bothIter->second);
    ++bothIter;
    CPPUNIT_ASSERT_EQUAL(std::string("max"), bothIter->first);
    CPPUNIT_ASSERT_EQUAL(std::string("1.0"), bothIter->second);
    ++bothIter;
    CPPUNIT_ASSERT_EQUAL(std::string("min"), bothIter->first);
    CPPUNIT_ASSERT_EQUAL(std::string("0.0"), bothIter->second);
    ++bothIter;
    CPPUNIT_ASSERT(bothEnd == bothIter);
}

void
TestAttribute::testAttributeType()
{
    // Test valid types.
    CPPUNIT_ASSERT(attributeType<Bool>() == TYPE_BOOL);
    CPPUNIT_ASSERT(attributeType<Int>() == TYPE_INT);
    CPPUNIT_ASSERT(attributeType<Long>() == TYPE_LONG);
    CPPUNIT_ASSERT(attributeType<Float>() == TYPE_FLOAT);
    CPPUNIT_ASSERT(attributeType<Double>() == TYPE_DOUBLE);
    CPPUNIT_ASSERT(attributeType<String>() == TYPE_STRING);
    CPPUNIT_ASSERT(attributeType<Rgb>() == TYPE_RGB);
    CPPUNIT_ASSERT(attributeType<Rgba>() == TYPE_RGBA);
    CPPUNIT_ASSERT(attributeType<Vec2f>() == TYPE_VEC2F);
    CPPUNIT_ASSERT(attributeType<Vec2d>() == TYPE_VEC2D);
    CPPUNIT_ASSERT(attributeType<Vec3f>() == TYPE_VEC3F);
    CPPUNIT_ASSERT(attributeType<Vec3d>() == TYPE_VEC3D);
    CPPUNIT_ASSERT(attributeType<Vec4f>() == TYPE_VEC4F);
    CPPUNIT_ASSERT(attributeType<Vec4d>() == TYPE_VEC4D);
    CPPUNIT_ASSERT(attributeType<Mat4f>() == TYPE_MAT4F);
    CPPUNIT_ASSERT(attributeType<Mat4d>() == TYPE_MAT4D);
    CPPUNIT_ASSERT(attributeType<SceneObject*>() == TYPE_SCENE_OBJECT);
    CPPUNIT_ASSERT(attributeType<BoolVector>() == TYPE_BOOL_VECTOR);
    CPPUNIT_ASSERT(attributeType<IntVector>() == TYPE_INT_VECTOR);
    CPPUNIT_ASSERT(attributeType<LongVector>() == TYPE_LONG_VECTOR);
    CPPUNIT_ASSERT(attributeType<FloatVector>() == TYPE_FLOAT_VECTOR);
    CPPUNIT_ASSERT(attributeType<DoubleVector>() == TYPE_DOUBLE_VECTOR);
    CPPUNIT_ASSERT(attributeType<StringVector>() == TYPE_STRING_VECTOR);
    CPPUNIT_ASSERT(attributeType<RgbVector>() == TYPE_RGB_VECTOR);
    CPPUNIT_ASSERT(attributeType<RgbaVector>() == TYPE_RGBA_VECTOR);
    CPPUNIT_ASSERT(attributeType<Vec2fVector>() == TYPE_VEC2F_VECTOR);
    CPPUNIT_ASSERT(attributeType<Vec2dVector>() == TYPE_VEC2D_VECTOR);
    CPPUNIT_ASSERT(attributeType<Vec3fVector>() == TYPE_VEC3F_VECTOR);
    CPPUNIT_ASSERT(attributeType<Vec3dVector>() == TYPE_VEC3D_VECTOR);
    CPPUNIT_ASSERT(attributeType<Vec4fVector>() == TYPE_VEC4F_VECTOR);
    CPPUNIT_ASSERT(attributeType<Vec4dVector>() == TYPE_VEC4D_VECTOR);
    CPPUNIT_ASSERT(attributeType<Mat4fVector>() == TYPE_MAT4F_VECTOR);
    CPPUNIT_ASSERT(attributeType<Mat4dVector>() == TYPE_MAT4D_VECTOR);
    CPPUNIT_ASSERT(attributeType<SceneObjectVector>() == TYPE_SCENE_OBJECT_VECTOR);
    CPPUNIT_ASSERT(attributeType<SceneObjectIndexable>() == TYPE_SCENE_OBJECT_INDEXABLE);

    // Test an invalid type.
    CPPUNIT_ASSERT(attributeType<char>() == TYPE_UNKNOWN);
}

void
TestAttribute::testSetEnumValue()
{
    CPPUNIT_ASSERT_NO_THROW(mEnumerable->setEnumValue(0, "zero"));
    CPPUNIT_ASSERT_NO_THROW(mEnumerable->setEnumValue(1, "one"));
    CPPUNIT_ASSERT_NO_THROW(mEnumerable->setEnumValue(2, "two"));

    CPPUNIT_ASSERT_THROW(mConstant->setEnumValue(0, "thing"), except::TypeError);
}

void
TestAttribute::testGetEnumDescription()
{
    testSetEnumValue();

    CPPUNIT_ASSERT_EQUAL(std::string("zero"), mEnumerable->getEnumDescription(0));
    CPPUNIT_ASSERT_EQUAL(std::string("one"), mEnumerable->getEnumDescription(1));
    CPPUNIT_ASSERT_EQUAL(std::string("two"), mEnumerable->getEnumDescription(2));

    CPPUNIT_ASSERT_THROW(mEnumerable->getEnumDescription(3), except::KeyError);
    CPPUNIT_ASSERT_THROW(mConstant->getEnumDescription(0), except::TypeError);
}

void
TestAttribute::testIsValidEnumValue()
{
    testSetEnumValue();

    CPPUNIT_ASSERT(mEnumerable->isValidEnumValue(0));
    CPPUNIT_ASSERT(mEnumerable->isValidEnumValue(1));
    CPPUNIT_ASSERT(mEnumerable->isValidEnumValue(2));
    CPPUNIT_ASSERT(!mEnumerable->isValidEnumValue(3));

    CPPUNIT_ASSERT_THROW(mConstant->isValidEnumValue(0), except::TypeError);
}

void
TestAttribute::testIterateEnumValues()
{
    testSetEnumValue();
    
    Attribute::EnumValueConstIterator enumIter;
    CPPUNIT_ASSERT_NO_THROW(enumIter = mEnumerable->beginEnumValues());
    Attribute::EnumValueConstIterator enumEnd;
    CPPUNIT_ASSERT_NO_THROW(enumEnd = mEnumerable->endEnumValues());
    CPPUNIT_ASSERT_EQUAL(0, enumIter->first);
    CPPUNIT_ASSERT_EQUAL(std::string("zero"), enumIter->second);
    ++enumIter;
    CPPUNIT_ASSERT_EQUAL(1, enumIter->first);
    CPPUNIT_ASSERT_EQUAL(std::string("one"), enumIter->second);
    ++enumIter;
    CPPUNIT_ASSERT_EQUAL(2, enumIter->first);
    CPPUNIT_ASSERT_EQUAL(std::string("two"), enumIter->second);
    ++enumIter;
    CPPUNIT_ASSERT(enumEnd == enumIter);

    Attribute::EnumValueConstIterator constantIter;
    CPPUNIT_ASSERT_NO_THROW(constantIter = mConstant->beginEnumValues());
    Attribute::EnumValueConstIterator constantEnd;
    CPPUNIT_ASSERT_NO_THROW(constantEnd = mConstant->endEnumValues());
    CPPUNIT_ASSERT(constantIter == constantEnd);
}

} // namespace unittest
} // namespace rdl2
} // namespace scene_rdl2

