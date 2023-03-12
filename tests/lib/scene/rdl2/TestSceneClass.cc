// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "TestSceneClass.h"

#include <scene_rdl2/scene/rdl2/Attribute.h>
#include <scene_rdl2/scene/rdl2/AttributeKey.h>
#include <scene_rdl2/scene/rdl2/SceneClass.h>
#include <scene_rdl2/scene/rdl2/Types.h>

#include <scene_rdl2/common/except/exceptions.h>

#include <cppunit/extensions/HelperMacros.h>

#include <cstddef>
#include <cstdlib>
#include <string>
#include <stdint.h>

#if __INTEL_COMPILER < 1600
#define WORKING_STRINGVECTOR_ATTRIBUTE_DEFAULT
#endif

namespace scene_rdl2 {
namespace rdl2 {
namespace unittest {

void
TestSceneClass::setUp()
{
    mBoolVec.clear();
    mBoolVec.push_back(true);
    mBoolVec.push_back(false);

    mBoolVec2.clear();
    mBoolVec2.push_back(false);
    mBoolVec2.push_back(true);

    mIntVec.clear();
    mIntVec.push_back(Int(100));
    mIntVec.push_back(Int(101));

    mIntVec2.clear();
    mIntVec2.push_back(Int(42));
    mIntVec2.push_back(Int(43));

    mLongVec.clear();
    mLongVec.push_back(Long(102));
    mLongVec.push_back(Long(103));

    mLongVec2.clear();
    mLongVec2.push_back(Long(44));
    mLongVec2.push_back(Long(45));

    mFloatVec.clear();
    mFloatVec.push_back(1.0f);
    mFloatVec.push_back(2.0f);

    mFloatVec2.clear();
    mFloatVec2.push_back(4.0f);
    mFloatVec2.push_back(5.0f);

    mDoubleVec.clear();
    mDoubleVec.push_back(3.0);
    mDoubleVec.push_back(4.0);

    mDoubleVec2.clear();
    mDoubleVec2.push_back(4.0);
    mDoubleVec2.push_back(5.0);

    mStringVec.clear();
    mStringVec.push_back("a");
    mStringVec.push_back("b");

    mStringVec2.clear();
    mStringVec2.push_back("c");
    mStringVec2.push_back("d");

    mRgbVec.clear();
    mRgbVec.push_back(Rgb(0.1f, 0.2f, 0.3f));
    mRgbVec.push_back(Rgb(0.4f, 0.5f, 0.6f));

    mRgbVec2.clear();
    mRgbVec2.push_back(Rgb(0.5f, 0.6f, 0.7f));
    mRgbVec2.push_back(Rgb(0.8f, 0.9f, 0.1f));

    mRgbaVec.clear();
    mRgbaVec.push_back(Rgba(0.1f, 0.2f, 0.3f, 0.4f));
    mRgbaVec.push_back(Rgba(0.5f, 0.6f, 0.7f, 0.8f));

    mRgbaVec2.clear();
    mRgbaVec2.push_back(Rgba(0.5f, 0.6f, 0.7f, 0.8f));
    mRgbaVec2.push_back(Rgba(0.9f, 0.1f, 0.2f, 0.3f));

    mVec2fVec.clear();
    mVec2fVec.push_back(Vec2f(1.0f, 2.0f));
    mVec2fVec.push_back(Vec2f(3.0f, 4.0f));

    mVec2fVec2.clear();
    mVec2fVec2.push_back(Vec2f(4.0f, 5.0f));
    mVec2fVec2.push_back(Vec2f(6.0f, 7.0f));

    mVec2dVec.clear();
    mVec2dVec.push_back(Vec2d(1.0, 2.0));
    mVec2dVec.push_back(Vec2d(3.0, 4.0));

    mVec2dVec2.clear();
    mVec2dVec2.push_back(Vec2d(4.0, 5.0));
    mVec2dVec2.push_back(Vec2d(6.0, 7.0));

    mVec3fVec.clear();
    mVec3fVec.push_back(Vec3f(1.0f, 2.0f, 3.0f));
    mVec3fVec.push_back(Vec3f(4.0f, 5.0f, 6.0f));

    mVec3fVec2.clear();
    mVec3fVec2.push_back(Vec3f(4.0f, 5.0f, 6.0f));
    mVec3fVec2.push_back(Vec3f(6.0f, 7.0f, 8.0f));

    mVec3dVec.clear();
    mVec3dVec.push_back(Vec3d(1.0, 2.0, 3.0));
    mVec3dVec.push_back(Vec3d(4.0, 5.0, 6.0));

    mVec3dVec2.clear();
    mVec3dVec2.push_back(Vec3d(1.0, 2.0, 3.0));
    mVec3dVec2.push_back(Vec3d(4.0, 5.0, 6.0));

    mVec4fVec.clear();
    mVec4fVec.push_back(Vec4f(1.0f, 2.0f, 3.0f, 4.0f));
    mVec4fVec.push_back(Vec4f(5.0f, 6.0f, 7.0f, 8.0f));

    mVec4fVec2.clear();
    mVec4fVec2.push_back(Vec4f(5.0f, 6.0f, 7.0f, 8.0f));
    mVec4fVec2.push_back(Vec4f(8.0f, 9.0f, 10.0f, 11.0f));

    mVec4dVec.clear();
    mVec4dVec.push_back(Vec4d(1.0, 2.0, 3.0, 4.0));
    mVec4dVec.push_back(Vec4d(5.0, 6.0, 7.0, 8.0));

    mVec4dVec2.clear();
    mVec4dVec2.push_back(Vec4d(1.0, 2.0, 3.0, 4.0));
    mVec4dVec2.push_back(Vec4d(5.0, 6.0, 7.0, 8.0));

    mMat4fVec.clear();
    mMat4fVec.push_back(Mat4f(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f));
    mMat4fVec.push_back(Mat4f(17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f, 24.0f, 25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f, 32.0f));

    mMat4fVec2.clear();
    mMat4fVec2.push_back(Mat4f(16.0f, 15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f));
    mMat4fVec2.push_back(Mat4f(32.0f, 31.0f, 30.0f, 29.0f, 28.0f, 27.0f, 26.0f, 25.0f, 24.0f, 23.0f, 22.0f, 21.0f, 20.0f, 19.0f, 18.0f, 17.0f));

    mMat4dVec.clear();
    mMat4dVec.push_back(Mat4d(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0));
    mMat4dVec.push_back(Mat4d(17.0, 18.0, 19.0, 20.0, 21.0, 22.0, 23.0, 24.0, 25.0, 26.0, 27.0, 28.0, 29.0, 30.0, 31.0, 32.0));

    mMat4dVec2.clear();
    mMat4dVec2.push_back(Mat4d(16.0, 15.0, 14.0, 13.0, 12.0, 11.0, 10.0, 9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0));
    mMat4dVec2.push_back(Mat4d(32.0, 31.0, 30.0, 29.0, 28.0, 27.0, 26.0, 25.0, 24.0, 23.0, 22.0, 21.0, 20.0, 19.0, 18.0, 17.0));

    mSceneObjectVec.clear();
    mSceneObjectVec.push_back((SceneObject*)0xdeadbeef);
    mSceneObjectVec.push_back((SceneObject*)0xc001d00d);

    mSceneObjectVec2.clear();
    mSceneObjectVec2.push_back((SceneObject*)0xbaadf00d);
    mSceneObjectVec2.push_back((SceneObject*)0xdeadc0de);
}

void
TestSceneClass::tearDown()
{
}

void
TestSceneClass::testGetName()
{
    SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

    CPPUNIT_ASSERT_EQUAL(std::string("ExampleObject"), sc.getName());
}

void
TestSceneClass::testDeclareSimple()
{
    SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

    // Make sure we can declare a simple (not bindable, not blurrable)
    // attribute of each attribute type, and that it doesn't throw.
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Bool> key = sc.declareAttribute<Bool>("bool");
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Int> key = sc.declareAttribute<Int>("int");
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Long> key = sc.declareAttribute<Long>("long");
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Float> key = sc.declareAttribute<Float>("float");
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Double> key = sc.declareAttribute<Double>("double");
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<String> key = sc.declareAttribute<String>("string");
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Rgb> key = sc.declareAttribute<Rgb>("rgb");
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Rgba> key = sc.declareAttribute<Rgba>("rgba");
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec2f> key = sc.declareAttribute<Vec2f>("vec2f");
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec2d> key = sc.declareAttribute<Vec2d>("vec2d");
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec3f> key = sc.declareAttribute<Vec3f>("vec3f");
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec3d> key = sc.declareAttribute<Vec3d>("vec3d");
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec4f> key = sc.declareAttribute<Vec4f>("vec4f");
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec4d> key = sc.declareAttribute<Vec4d>("vec4d");
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Mat4f> key = sc.declareAttribute<Mat4f>("mat4f");
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Mat4d> key = sc.declareAttribute<Mat4d>("mat4d");
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<SceneObject*> key = sc.declareAttribute<SceneObject*>("scene_object", { "scene object" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<BoolVector> key = sc.declareAttribute<BoolVector>("bool_vector", { "bool vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<IntVector> key = sc.declareAttribute<IntVector>("int_vector", { "int vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<LongVector> key = sc.declareAttribute<LongVector>("long_vector", { "long vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<FloatVector> key = sc.declareAttribute<FloatVector>("float_vector", { "float vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<DoubleVector> key = sc.declareAttribute<DoubleVector>("double_vector", { "double vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<StringVector> key = sc.declareAttribute<StringVector>("string_vector", { "string vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<RgbVector> key = sc.declareAttribute<RgbVector>("rgb_vector", { "rgb vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<RgbaVector> key = sc.declareAttribute<RgbaVector>("rgba_vector", { "rgba vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec2fVector> key = sc.declareAttribute<Vec2fVector>("vec2f_vector", { "vec2f vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec2dVector> key = sc.declareAttribute<Vec2dVector>("vec2d_vector", { "vec2d vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec3fVector> key = sc.declareAttribute<Vec3fVector>("vec3f_vector", { "vec3f vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec3dVector> key = sc.declareAttribute<Vec3dVector>("vec3d_vector", { "vec3d vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec4fVector> key = sc.declareAttribute<Vec4fVector>("vec4f_vector", { "vec4f vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec4dVector> key = sc.declareAttribute<Vec4dVector>("vec4d_vector", { "vec4d vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Mat4fVector> key = sc.declareAttribute<Mat4fVector>("mat4f_vector", { "mat4f vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Mat4dVector> key = sc.declareAttribute<Mat4dVector>("mat4d_vector", { "mat4d vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<SceneObjectVector> key = sc.declareAttribute<SceneObjectVector>("scene_object_vector", { "scene object vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    
    // Declaring attributes with existing names should throw.
    CPPUNIT_ASSERT_THROW(sc.declareAttribute<Bool>("bool"), except::KeyError);

    // Declaring attributes after setComplete() should throw.
    sc.setComplete();
    CPPUNIT_ASSERT_THROW(sc.declareAttribute<Bool>("bool_2"), except::RuntimeError);
}

void
TestSceneClass::testDeclareSimpleWithDefault()
{
    SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

    // Make sure we can declare a simple (not bindable, not blurrable)
    // attribute of each attribute type with a default, and that it doesn't throw.
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Bool> key = sc.declareAttribute<Bool>("bool", true);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Bool>() == true);
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Int> key = sc.declareAttribute<Int>("int", Int(100));
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Int>() == Int(100));
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Long> key = sc.declareAttribute<Long>("long", Long(101));
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Long>() == Long(101));
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Float> key = sc.declareAttribute<Float>("float", 1.0f);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f, attr->getDefaultValue<Float>(), 0.0001f);
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Double> key = sc.declareAttribute<Double>("double", 2.0);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, attr->getDefaultValue<Double>(), 0.0001);
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<String> key = sc.declareAttribute<String>("string", String("wat"));
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<String>() == String("wat"));
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Rgb> key = sc.declareAttribute<Rgb>("rgb", Rgb(0.1f, 0.2f, 0.3f));
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Rgb>() == Rgb(0.1f, 0.2f, 0.3f));
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Rgba> key = sc.declareAttribute<Rgba>("rgba", Rgba(0.1f, 0.2f, 0.3f, 0.4f));
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Rgba>() == Rgba(0.1f, 0.2f, 0.3f, 0.4f));
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec2f> key = sc.declareAttribute<Vec2f>("vec2f", Vec2f(1.0f, 2.0f));
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Vec2f>() == Vec2f(1.0f, 2.0f));
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec2d> key = sc.declareAttribute<Vec2d>("vec2d", Vec2d(1.0, 2.0));
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Vec2d>() == Vec2d(1.0, 2.0));
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec3f> key = sc.declareAttribute<Vec3f>("vec3f", Vec3f(1.0f, 2.0f, 3.0f));
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Vec3f>() == Vec3f(1.0f, 2.0f, 3.0f));
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec3d> key = sc.declareAttribute<Vec3d>("vec3d", Vec3d(1.0, 2.0, 3.0));
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Vec3d>() == Vec3d(1.0, 2.0, 3.0));
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec4f> key = sc.declareAttribute<Vec4f>("vec4f", Vec4f(1.0f, 2.0f, 3.0f, 4.0f));
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Vec4f>() == Vec4f(1.0f, 2.0f, 3.0f, 4.0f));
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec4d> key = sc.declareAttribute<Vec4d>("vec4d", Vec4d(1.0, 2.0, 3.0, 4.0));
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Vec4d>() == Vec4d(1.0, 2.0, 3.0, 4.0));
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Mat4f> key = sc.declareAttribute<Mat4f>("mat4f", Mat4f(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f));
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Mat4f>() == Mat4f(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f));
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Mat4d> key = sc.declareAttribute<Mat4d>("mat4d", Mat4d(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0));
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Mat4d>() == Mat4d(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0));
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<SceneObject*> key = sc.declareAttribute<SceneObject*>("scene_object", (SceneObject*)0xdeadbeef, { "scene object" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<SceneObject*>() == (SceneObject*)0xdeadbeef);
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<BoolVector> key = sc.declareAttribute<BoolVector>("bool_vector", mBoolVec, { "bool vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<BoolVector>() == mBoolVec);
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<IntVector> key = sc.declareAttribute<IntVector>("int_vector", mIntVec, { "int vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<IntVector>() == mIntVec);
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<LongVector> key = sc.declareAttribute<LongVector>("long_vector", mLongVec, { "long vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<LongVector>() == mLongVec);
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<FloatVector> key = sc.declareAttribute<FloatVector>("float_vector", mFloatVec, { "float vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<FloatVector>() == mFloatVec);
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<DoubleVector> key = sc.declareAttribute<DoubleVector>("double_vector", mDoubleVec, { "double vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<DoubleVector>() == mDoubleVec);
        );
    );
#ifdef WORKING_STRINGVECTOR_ATTRIBUTE_DEFAULT
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<StringVector> key = sc.declareAttribute<StringVector>("string_vector", mStringVec, { "string vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<StringVector>() == mStringVec);
        );
    );
#endif
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<RgbVector> key = sc.declareAttribute<RgbVector>("rgb_vector", mRgbVec, { "rgb vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<RgbVector>() == mRgbVec);
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<RgbaVector> key = sc.declareAttribute<RgbaVector>("rgba_vector", mRgbaVec, { "rgba vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<RgbaVector>() == mRgbaVec);
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec2fVector> key = sc.declareAttribute<Vec2fVector>("vec2f_vector", mVec2fVec, { "vec2f vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Vec2fVector>() == mVec2fVec);
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec2dVector> key = sc.declareAttribute<Vec2dVector>("vec2d_vector", mVec2dVec, { "vec2d vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Vec2dVector>() == mVec2dVec);
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec3fVector> key = sc.declareAttribute<Vec3fVector>("vec3f_vector", mVec3fVec, { "vec3f vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Vec3fVector>() == mVec3fVec);
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec3dVector> key = sc.declareAttribute<Vec3dVector>("vec3d_vector", mVec3dVec, { "vec3d vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Vec3dVector>() == mVec3dVec);
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec4fVector> key = sc.declareAttribute<Vec4fVector>("ve4f_vector", mVec4fVec, { "ve4f vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Vec4fVector>() == mVec4fVec);
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec4dVector> key = sc.declareAttribute<Vec4dVector>("vec4d_vector", mVec4dVec, { "vec4d vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Vec4dVector>() == mVec4dVec);
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Mat4fVector> key = sc.declareAttribute<Mat4fVector>("mat4f_vector", mMat4fVec, { "mat4f vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Mat4fVector>() == mMat4fVec);
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Mat4dVector> key = sc.declareAttribute<Mat4dVector>("mat4d_vector", mMat4dVec, { "mat4d vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Mat4dVector>() == mMat4dVec);
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<SceneObjectVector> key = sc.declareAttribute<SceneObjectVector>("scene_object_vector", mSceneObjectVec, { "scene object vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<SceneObjectVector>() == mSceneObjectVec);
        );
    );
    
    // Declaring attributes with existing names should throw.
    CPPUNIT_ASSERT_THROW(sc.declareAttribute<Bool>("bool", true), except::KeyError);

    // Declaring attributes after setComplete() should throw.
    sc.setComplete();
    CPPUNIT_ASSERT_THROW(
        sc.declareAttribute<Bool>("bool_2", true, { "bool 2" });
    , except::RuntimeError);
}

void
TestSceneClass::testDeclareBindable()
{
    SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

    // Make sure we can declare a bindable (not blurrable) attribute of each
    // attribute type, and that it doesn't throw.
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Bool> key = sc.declareAttribute<Bool>("bool", FLAGS_BINDABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Int> key = sc.declareAttribute<Int>("int", FLAGS_BINDABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Long> key = sc.declareAttribute<Long>("long", FLAGS_BINDABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Float> key = sc.declareAttribute<Float>("float", FLAGS_BINDABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Double> key = sc.declareAttribute<Double>("double", FLAGS_BINDABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<String> key = sc.declareAttribute<String>("string", FLAGS_BINDABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Rgb> key = sc.declareAttribute<Rgb>("rgb", FLAGS_BINDABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Rgba> key = sc.declareAttribute<Rgba>("rgba", FLAGS_BINDABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec2f> key = sc.declareAttribute<Vec2f>("vec2f", FLAGS_BINDABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec2d> key = sc.declareAttribute<Vec2d>("vec2d", FLAGS_BINDABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec3f> key = sc.declareAttribute<Vec3f>("vec3f", FLAGS_BINDABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec3d> key = sc.declareAttribute<Vec3d>("vec3d", FLAGS_BINDABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec4f> key = sc.declareAttribute<Vec4f>("vec4f", FLAGS_BINDABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec4d> key = sc.declareAttribute<Vec4d>("vec4d", FLAGS_BINDABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Mat4f> key = sc.declareAttribute<Mat4f>("mat4f", FLAGS_BINDABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Mat4d> key = sc.declareAttribute<Mat4d>("mat4d", FLAGS_BINDABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<SceneObject*> key = sc.declareAttribute<SceneObject*>("scene_object", FLAGS_BINDABLE, INTERFACE_GENERIC, { "scene object" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<BoolVector> key = sc.declareAttribute<BoolVector>("bool_vector", FLAGS_BINDABLE, INTERFACE_GENERIC, { "bool vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<IntVector> key = sc.declareAttribute<IntVector>("int_vector", FLAGS_BINDABLE, INTERFACE_GENERIC, { "int vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<LongVector> key = sc.declareAttribute<LongVector>("long_vector", FLAGS_BINDABLE, INTERFACE_GENERIC, { "long vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<FloatVector> key = sc.declareAttribute<FloatVector>("float_vector", FLAGS_BINDABLE, INTERFACE_GENERIC, { "float vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<DoubleVector> key = sc.declareAttribute<DoubleVector>("double_vector", FLAGS_BINDABLE, INTERFACE_GENERIC, { "double vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<StringVector> key = sc.declareAttribute<StringVector>("string_vector", FLAGS_BINDABLE, INTERFACE_GENERIC, { "string vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<RgbVector> key = sc.declareAttribute<RgbVector>("rgb_vector", FLAGS_BINDABLE, INTERFACE_GENERIC, { "rgb vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<RgbaVector> key = sc.declareAttribute<RgbaVector>("rgba_vector", FLAGS_BINDABLE, INTERFACE_GENERIC, { "rgba vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec2fVector> key = sc.declareAttribute<Vec2fVector>("vec2f_vector", FLAGS_BINDABLE, INTERFACE_GENERIC, { "vec2f vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec2dVector> key = sc.declareAttribute<Vec2dVector>("vec2d_vector", FLAGS_BINDABLE, INTERFACE_GENERIC, { "vec2d vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec3fVector> key = sc.declareAttribute<Vec3fVector>("vec3f_vector", FLAGS_BINDABLE, INTERFACE_GENERIC, { "vec3f vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec3dVector> key = sc.declareAttribute<Vec3dVector>("vec3d_vector", FLAGS_BINDABLE, INTERFACE_GENERIC, { "vec3d vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec4fVector> key = sc.declareAttribute<Vec4fVector>("vec4f_vector", FLAGS_BINDABLE, INTERFACE_GENERIC, { "vec4f vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec4dVector> key = sc.declareAttribute<Vec4dVector>("vec4d_vector", FLAGS_BINDABLE, INTERFACE_GENERIC, { "vec4d vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Mat4fVector> key = sc.declareAttribute<Mat4fVector>("mat4f_vector", FLAGS_BINDABLE, INTERFACE_GENERIC, { "mat4f vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Mat4dVector> key = sc.declareAttribute<Mat4dVector>("mat4d_vector", FLAGS_BINDABLE, INTERFACE_GENERIC, { "mat4d vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<SceneObjectVector> key = sc.declareAttribute<SceneObjectVector>("scene_object_vector", FLAGS_BINDABLE, INTERFACE_GENERIC, { "scene object vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
        );
    );
    
    // Declaring attributes with existing names should throw.
    CPPUNIT_ASSERT_THROW(sc.declareAttribute<Bool>("bool", FLAGS_BINDABLE), except::KeyError);

    // Declaring attributes after setComplete() should throw.
    sc.setComplete();
    CPPUNIT_ASSERT_THROW(
        sc.declareAttribute<Bool>("bool_2", FLAGS_BINDABLE, INTERFACE_GENERIC, { "bool 2" });
    , except::RuntimeError);
}

void
TestSceneClass::testDeclareBindableWithDefault()
{
    SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

    // Make sure we can declare a bindable (not blurrable) attribute of each
    // attribute type with a default, and that it doesn't throw.
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Bool> key = sc.declareAttribute<Bool>("bool", true, FLAGS_BINDABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Bool>() == true);
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Int> key = sc.declareAttribute<Int>("int", Int(100), FLAGS_BINDABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Int>() == Int(100));
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Long> key = sc.declareAttribute<Long>("long", Long(101), FLAGS_BINDABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Long>() == Long(101));
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Float> key = sc.declareAttribute<Float>("float", 1.0f, FLAGS_BINDABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f, attr->getDefaultValue<Float>(), 0.0001f);
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Double> key = sc.declareAttribute<Double>("double", 2.0, FLAGS_BINDABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, attr->getDefaultValue<Double>(), 0.0001);
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<String> key = sc.declareAttribute<String>("string", String("wat"), FLAGS_BINDABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<String>() == String("wat"));
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Rgb> key = sc.declareAttribute<Rgb>("rgb", Rgb(0.1f, 0.2f, 0.3f), FLAGS_BINDABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Rgb>() == Rgb(0.1f, 0.2f, 0.3f));
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Rgba> key = sc.declareAttribute<Rgba>("rgba", Rgba(0.1f, 0.2f, 0.3f, 0.4f), FLAGS_BINDABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Rgba>() == Rgba(0.1f, 0.2f, 0.3f, 0.4f));
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec2f> key = sc.declareAttribute<Vec2f>("vec2f", Vec2f(1.0f, 2.0f), FLAGS_BINDABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Vec2f>() == Vec2f(1.0f, 2.0f));
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec2d> key = sc.declareAttribute<Vec2d>("vec2d", Vec2d(1.0, 2.0), FLAGS_BINDABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Vec2d>() == Vec2d(1.0, 2.0));
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec3f> key = sc.declareAttribute<Vec3f>("vec3f", Vec3f(1.0f, 2.0f, 3.0f), FLAGS_BINDABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Vec3f>() == Vec3f(1.0f, 2.0f, 3.0f));
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec3d> key = sc.declareAttribute<Vec3d>("vec3d", Vec3d(1.0, 2.0, 3.0), FLAGS_BINDABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Vec3d>() == Vec3d(1.0, 2.0, 3.0));
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec4f> key = sc.declareAttribute<Vec4f>("vec4f", Vec4f(1.0f, 2.0f, 3.0f, 4.0f), FLAGS_BINDABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Vec4f>() == Vec4f(1.0f, 2.0f, 3.0f, 4.0f));
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec4d> key = sc.declareAttribute<Vec4d>("vec4d", Vec4d(1.0, 2.0, 3.0, 4.0), FLAGS_BINDABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Vec4d>() == Vec4d(1.0, 2.0, 3.0, 4.0));
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Mat4f> key = sc.declareAttribute<Mat4f>("mat4f", Mat4f(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f), FLAGS_BINDABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Mat4f>() == Mat4f(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f));
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Mat4d> key = sc.declareAttribute<Mat4d>("mat4d", Mat4d(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0), FLAGS_BINDABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Mat4d>() == Mat4d(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0));
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<SceneObject*> key = sc.declareAttribute<SceneObject*>("scene_object", (SceneObject*)0xdeadbeef, FLAGS_BINDABLE, INTERFACE_GENERIC, { "scene object" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<SceneObject*>() == (SceneObject*)0xdeadbeef);
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<BoolVector> key = sc.declareAttribute<BoolVector>("bool_vector", mBoolVec, FLAGS_BINDABLE, INTERFACE_GENERIC, { "bool vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<BoolVector>() == mBoolVec);
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<IntVector> key = sc.declareAttribute<IntVector>("int_vector", mIntVec, FLAGS_BINDABLE, INTERFACE_GENERIC, { "int vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<IntVector>() == mIntVec);
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<LongVector> key = sc.declareAttribute<LongVector>("long_vector", mLongVec, FLAGS_BINDABLE, INTERFACE_GENERIC, { "long vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<LongVector>() == mLongVec);
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<FloatVector> key = sc.declareAttribute<FloatVector>("float_vector", mFloatVec, FLAGS_BINDABLE, INTERFACE_GENERIC, { "float vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<FloatVector>() == mFloatVec);
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<DoubleVector> key = sc.declareAttribute<DoubleVector>("double_vector", mDoubleVec, FLAGS_BINDABLE, INTERFACE_GENERIC, { "double vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<DoubleVector>() == mDoubleVec);
        );
    );
#ifdef WORKING_STRINGVECTOR_ATTRIBUTE_DEFAULT
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<StringVector> key = sc.declareAttribute<StringVector>("string_vector", mStringVec, FLAGS_BINDABLE, INTERFACE_GENERIC, { "string vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<StringVector>() == mStringVec);
        );
    );
#endif
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<RgbVector> key = sc.declareAttribute<RgbVector>("rgb_vector", mRgbVec, FLAGS_BINDABLE, INTERFACE_GENERIC, { "rgb vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<RgbVector>() == mRgbVec);
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<RgbaVector> key = sc.declareAttribute<RgbaVector>("rgba_vector", mRgbaVec, FLAGS_BINDABLE, INTERFACE_GENERIC, { "rgba vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<RgbaVector>() == mRgbaVec);
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec2fVector> key = sc.declareAttribute<Vec2fVector>("vec2f_vector", mVec2fVec, FLAGS_BINDABLE, INTERFACE_GENERIC, { "vec2f vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Vec2fVector>() == mVec2fVec);
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec2dVector> key = sc.declareAttribute<Vec2dVector>("vec2d_vector", mVec2dVec, FLAGS_BINDABLE, INTERFACE_GENERIC, { "vec2d vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Vec2dVector>() == mVec2dVec);
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec3fVector> key = sc.declareAttribute<Vec3fVector>("vec3f_vector", mVec3fVec, FLAGS_BINDABLE, INTERFACE_GENERIC, { "vec3f vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Vec3fVector>() == mVec3fVec);
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec3dVector> key = sc.declareAttribute<Vec3dVector>("vec3d_vector", mVec3dVec, FLAGS_BINDABLE, INTERFACE_GENERIC, { "vec3d vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Vec3dVector>() == mVec3dVec);
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec4fVector> key = sc.declareAttribute<Vec4fVector>("vec4f_vector", mVec4fVec, FLAGS_BINDABLE, INTERFACE_GENERIC, { "vec4f vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Vec4fVector>() == mVec4fVec);
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec4dVector> key = sc.declareAttribute<Vec4dVector>("vec4d_vector", mVec4dVec, FLAGS_BINDABLE, INTERFACE_GENERIC, { "vec4d vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Vec4dVector>() == mVec4dVec);
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Mat4fVector> key = sc.declareAttribute<Mat4fVector>("mat4f_vector", mMat4fVec, FLAGS_BINDABLE, INTERFACE_GENERIC, { "mat4f vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Mat4fVector>() == mMat4fVec);
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Mat4dVector> key = sc.declareAttribute<Mat4dVector>("mat4d_vector", mMat4dVec, FLAGS_BINDABLE, INTERFACE_GENERIC, { "mat4d vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Mat4dVector>() == mMat4dVec);
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<SceneObjectVector> key = sc.declareAttribute<SceneObjectVector>("scene_object_vector", mSceneObjectVec, FLAGS_BINDABLE, INTERFACE_GENERIC, { "scene object vector" });
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(attr->isBindable());
            CPPUNIT_ASSERT(!attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<SceneObjectVector>() == mSceneObjectVec);
        );
    );
    
    // Declaring attributes with existing names should throw.
    CPPUNIT_ASSERT_THROW(sc.declareAttribute<Bool>("bool", true, FLAGS_BINDABLE), except::KeyError);

    // Declaring attributes after setComplete() should throw.
    sc.setComplete();
    CPPUNIT_ASSERT_THROW(
        sc.declareAttribute<Bool>("bool_2", true, FLAGS_BINDABLE, INTERFACE_GENERIC, { "bool 2" });
    , except::RuntimeError);
}

void
TestSceneClass::testDeclareBlurrable()
{
    SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

    // Make sure we can declare a blurrable (not bindable) attribute of each
    // blurrable attribute type, and that it doesn't throw.
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Int> key = sc.declareAttribute<Int>("int", FLAGS_BLURRABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Long> key = sc.declareAttribute<Long>("long", FLAGS_BLURRABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Float> key = sc.declareAttribute<Float>("float", FLAGS_BLURRABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Double> key = sc.declareAttribute<Double>("double", FLAGS_BLURRABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Rgb> key = sc.declareAttribute<Rgb>("rgb", FLAGS_BLURRABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Rgba> key = sc.declareAttribute<Rgba>("rgba", FLAGS_BLURRABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec2f> key = sc.declareAttribute<Vec2f>("vec2f", FLAGS_BLURRABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec2d> key = sc.declareAttribute<Vec2d>("vec2d", FLAGS_BLURRABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec3f> key = sc.declareAttribute<Vec3f>("vec3f", FLAGS_BLURRABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec3d> key = sc.declareAttribute<Vec3d>("vec3d", FLAGS_BLURRABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec4f> key = sc.declareAttribute<Vec4f>("vec4f", FLAGS_BLURRABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec4d> key = sc.declareAttribute<Vec4d>("vec4d", FLAGS_BLURRABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Mat4f> key = sc.declareAttribute<Mat4f>("mat4f", FLAGS_BLURRABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(attr->isBlurrable());
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Mat4d> key = sc.declareAttribute<Mat4d>("mat4d", FLAGS_BLURRABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(attr->isBlurrable());
        );
    );

    // Make sure if we declare a blurrable (not bindable) attribute of each
    // non-blurrable attribute type it throws.
    CPPUNIT_ASSERT_THROW(
        sc.declareAttribute<Bool>("bool", FLAGS_BLURRABLE);
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        sc.declareAttribute<String>("string", FLAGS_BLURRABLE);
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        sc.declareAttribute<SceneObject*>("scene_object", FLAGS_BLURRABLE, INTERFACE_GENERIC, { "scene object" });
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        sc.declareAttribute<BoolVector>("bool_vector", FLAGS_BLURRABLE, INTERFACE_GENERIC, { "bool vector" });
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        sc.declareAttribute<IntVector>("int_vector", FLAGS_BLURRABLE, INTERFACE_GENERIC, { "int vector" });
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        sc.declareAttribute<LongVector>("long_vector", FLAGS_BLURRABLE, INTERFACE_GENERIC, { "long vector" });
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        sc.declareAttribute<FloatVector>("float_vector", FLAGS_BLURRABLE, INTERFACE_GENERIC, { "float vector" });
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        sc.declareAttribute<DoubleVector>("double_vector", FLAGS_BLURRABLE, INTERFACE_GENERIC, { "double vector" });
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        sc.declareAttribute<StringVector>("string_vector", FLAGS_BLURRABLE, INTERFACE_GENERIC, { "string vector" });
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        sc.declareAttribute<RgbVector>("rgb_vector", FLAGS_BLURRABLE, INTERFACE_GENERIC, { "rgb vector" });
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        sc.declareAttribute<RgbaVector>("rgba_vector", FLAGS_BLURRABLE, INTERFACE_GENERIC, { "rgba vector" });
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        sc.declareAttribute<Vec2fVector>("vec2f_vector", FLAGS_BLURRABLE, INTERFACE_GENERIC, { "vec2f vector" });
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        sc.declareAttribute<Vec2dVector>("vec2d_vector", FLAGS_BLURRABLE, INTERFACE_GENERIC, { "vec2d vector" });
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        sc.declareAttribute<Vec3fVector>("vec3f_vector", FLAGS_BLURRABLE, INTERFACE_GENERIC, { "vec3f vector" });
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        sc.declareAttribute<Vec3dVector>("vec3d_vector", FLAGS_BLURRABLE, INTERFACE_GENERIC, { "vec3d vector" });
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        sc.declareAttribute<Vec4fVector>("vec4f_vector", FLAGS_BLURRABLE, INTERFACE_GENERIC, { "vec4f vector" });
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        sc.declareAttribute<Vec4dVector>("vec4d_vector", FLAGS_BLURRABLE, INTERFACE_GENERIC, { "vec4d vector" });
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        sc.declareAttribute<Mat4fVector>("mat4f_vector", FLAGS_BLURRABLE, INTERFACE_GENERIC, { "mat4f vector" });
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        sc.declareAttribute<Mat4dVector>("mat4d_vector", FLAGS_BLURRABLE, INTERFACE_GENERIC, { "mat4d vector" });
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        sc.declareAttribute<SceneObjectVector>("scene_object_vector", FLAGS_BLURRABLE, INTERFACE_GENERIC, { "scene object vector" });
    , except::TypeError);

    // Declaring attributes with existing names should throw.
    CPPUNIT_ASSERT_THROW(sc.declareAttribute<Float>("float", FLAGS_BLURRABLE), except::KeyError);

    // Declaring attributes after setComplete() should throw.
    sc.setComplete();
    CPPUNIT_ASSERT_THROW(
        sc.declareAttribute<Float>("float_2", FLAGS_BLURRABLE, INTERFACE_GENERIC, { "float 2" });
    , except::RuntimeError);
}

void
TestSceneClass::testDeclareBlurrableWithDefault()
{
    SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

    // Make sure we can declare a blurrable (not bindable) attribute of each
    // blurrable attribute type, and that it doesn't throw.
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Int> key = sc.declareAttribute<Int>("int", Int(100), FLAGS_BLURRABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Int>() == Int(100));
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Long> key = sc.declareAttribute<Long>("long", Long(101), FLAGS_BLURRABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Long>() == Long(101));
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Float> key = sc.declareAttribute<Float>("float", 1.0f, FLAGS_BLURRABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(attr->isBlurrable());
            CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f, attr->getDefaultValue<Float>(), 0.0001f);
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Double> key = sc.declareAttribute<Double>("double", 2.0, FLAGS_BLURRABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(attr->isBlurrable());
            CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, attr->getDefaultValue<Double>(), 0.0001);
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Rgb> key = sc.declareAttribute<Rgb>("rgb", Rgb(0.1f, 0.2f, 0.3f), FLAGS_BLURRABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Rgb>() == Rgb(0.1f, 0.2f, 0.3f));
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Rgba> key = sc.declareAttribute<Rgba>("rgba", Rgba(0.1f, 0.2f, 0.3f, 0.4f), FLAGS_BLURRABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Rgba>() == Rgba(0.1f, 0.2f, 0.3f, 0.4f));
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec2f> key = sc.declareAttribute<Vec2f>("vec2f", Vec2f(1.0f, 2.0f), FLAGS_BLURRABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Vec2f>() == Vec2f(1.0f, 2.0f));
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec2d> key = sc.declareAttribute<Vec2d>("vec2d", Vec2d(1.0, 2.0), FLAGS_BLURRABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Vec2d>() == Vec2d(1.0, 2.0));
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec3f> key = sc.declareAttribute<Vec3f>("vec3f", Vec3f(1.0f, 2.0f, 3.0f), FLAGS_BLURRABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Vec3f>() == Vec3f(1.0f, 2.0f, 3.0f));
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec3d> key = sc.declareAttribute<Vec3d>("vec3d", Vec3d(1.0, 2.0, 3.0), FLAGS_BLURRABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Vec3d>() == Vec3d(1.0, 2.0, 3.0));
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec4f> key = sc.declareAttribute<Vec4f>("vec4f", Vec4f(1.0f, 2.0f, 3.0f, 4.0f), FLAGS_BLURRABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Vec4f>() == Vec4f(1.0f, 2.0f, 3.0f, 4.0f));
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Vec4d> key = sc.declareAttribute<Vec4d>("vec4d", Vec4d(1.0, 2.0, 3.0, 4.0), FLAGS_BLURRABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Vec4d>() == Vec4d(1.0, 2.0, 3.0, 4.0));
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Mat4f> key = sc.declareAttribute<Mat4f>("mat4f", Mat4f(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f), FLAGS_BLURRABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Mat4f>() == Mat4f(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f));
        );
    );
    CPPUNIT_ASSERT_NO_THROW(
        AttributeKey<Mat4d> key = sc.declareAttribute<Mat4d>("mat4d", Mat4d(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0), FLAGS_BLURRABLE);
        CPPUNIT_ASSERT_NO_THROW(
            Attribute* attr = sc.getAttribute(key);
            CPPUNIT_ASSERT(!attr->isBindable());
            CPPUNIT_ASSERT(attr->isBlurrable());
            CPPUNIT_ASSERT(attr->getDefaultValue<Mat4d>() == Mat4d(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0));
        );
    );

    // Make sure if we declare a blurrable (not bindable) attribute of each
    // non-blurrable attribute type it throws.
    CPPUNIT_ASSERT_THROW(
        sc.declareAttribute<Bool>("bool", true, FLAGS_BLURRABLE);
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        sc.declareAttribute<String>("string", String("wat"), FLAGS_BLURRABLE);
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        sc.declareAttribute<SceneObject*>("scene_object", (SceneObject*)0xdeadbeef, FLAGS_BLURRABLE, INTERFACE_GENERIC, { "scene object" });
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        sc.declareAttribute<BoolVector>("bool_vector", mBoolVec, FLAGS_BLURRABLE, INTERFACE_GENERIC, { "bool vector" });
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        sc.declareAttribute<IntVector>("int_vector", mIntVec, FLAGS_BLURRABLE, INTERFACE_GENERIC, { "int vector" });
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        sc.declareAttribute<LongVector>("long_vector", mLongVec, FLAGS_BLURRABLE, INTERFACE_GENERIC, { "long vector" });
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        sc.declareAttribute<FloatVector>("float_vector", mFloatVec, FLAGS_BLURRABLE, INTERFACE_GENERIC, { "float vector" });
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        sc.declareAttribute<DoubleVector>("double_vector", mDoubleVec, FLAGS_BLURRABLE, INTERFACE_GENERIC, { "double vector" });
    , except::TypeError);
#ifdef WORKING_STRINGVECTOR_ATTRIBUTE_DEFAULT
    CPPUNIT_ASSERT_THROW(
        sc.declareAttribute<StringVector>("string_vector", mStringVec, FLAGS_BLURRABLE, INTERFACE_GENERIC, { "string vector" });
    , except::TypeError);
#endif
    CPPUNIT_ASSERT_THROW(
        sc.declareAttribute<RgbVector>("rgb_vector", mRgbVec, FLAGS_BLURRABLE, INTERFACE_GENERIC, { "rgb vector" });
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        sc.declareAttribute<RgbaVector>("rgba_vector", mRgbaVec, FLAGS_BLURRABLE, INTERFACE_GENERIC, { "rgba vector" });
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        sc.declareAttribute<Vec2fVector>("vec2f_vector", mVec2fVec, FLAGS_BLURRABLE, INTERFACE_GENERIC, { "vec2f vector" });
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        sc.declareAttribute<Vec2dVector>("vec2d_vector", mVec2dVec, FLAGS_BLURRABLE, INTERFACE_GENERIC, { "vec2d vector" });
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        sc.declareAttribute<Vec3fVector>("vec3f_vector", mVec3fVec, FLAGS_BLURRABLE, INTERFACE_GENERIC, { "vec3f vector" });
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        sc.declareAttribute<Vec3dVector>("vec3d_vector", mVec3dVec, FLAGS_BLURRABLE, INTERFACE_GENERIC, { "vec3d vector" });
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        sc.declareAttribute<Vec4fVector>("vec4f_vector", mVec4fVec, FLAGS_BLURRABLE, INTERFACE_GENERIC, { "vec4f vector" });
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        sc.declareAttribute<Vec4dVector>("vec4d_vector", mVec4dVec, FLAGS_BLURRABLE, INTERFACE_GENERIC, { "vec4d vector" });
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        sc.declareAttribute<Mat4fVector>("mat4f_vector", mMat4fVec, FLAGS_BLURRABLE, INTERFACE_GENERIC, { "mat4f vector" });
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        sc.declareAttribute<Mat4dVector>("mat4d_vector", mMat4dVec, FLAGS_BLURRABLE, INTERFACE_GENERIC, { "mat4d vector" });
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        sc.declareAttribute<SceneObjectVector>("scene_object_vector", mSceneObjectVec, FLAGS_BLURRABLE, INTERFACE_GENERIC, { "scene object vector" });
    , except::TypeError);

    // Declaring attributes with existing names should throw.
    CPPUNIT_ASSERT_THROW(sc.declareAttribute<Float>("float", 1.0f, FLAGS_BLURRABLE), except::KeyError);

    // Declaring attributes after setComplete() should throw.
    sc.setComplete();
    CPPUNIT_ASSERT_THROW(
        sc.declareAttribute<Float>("float_2", 1.0f, FLAGS_BLURRABLE, INTERFACE_GENERIC, { "float 2" });
    , except::RuntimeError);
}

void
TestSceneClass::testGetAttributeByKey()
{
    SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

    AttributeKey<Bool> oneKey = sc.declareAttribute<Bool>("one");
    AttributeKey<Int> twoKey = sc.declareAttribute<Int>("two", FLAGS_BINDABLE);
    AttributeKey<Float> threeKey = sc.declareAttribute<Float>("three", FLAGS_BLURRABLE);

    sc.setComplete();

    Attribute* oneAttr = nullptr; 
    Attribute* twoAttr = nullptr;
    Attribute* threeAttr = nullptr;
    CPPUNIT_ASSERT_NO_THROW(oneAttr = sc.getAttribute(oneKey));
    CPPUNIT_ASSERT_NO_THROW(twoAttr = sc.getAttribute(twoKey));
    CPPUNIT_ASSERT_NO_THROW(threeAttr = sc.getAttribute(threeKey));

    CPPUNIT_ASSERT_EQUAL(String("one"), oneAttr->getName());
    CPPUNIT_ASSERT_EQUAL(TYPE_BOOL, oneAttr->getType());
    CPPUNIT_ASSERT_EQUAL(FLAGS_NONE, oneAttr->getFlags());

    CPPUNIT_ASSERT_EQUAL(String("two"), twoAttr->getName());
    CPPUNIT_ASSERT_EQUAL(TYPE_INT, twoAttr->getType());
    CPPUNIT_ASSERT_EQUAL(FLAGS_BINDABLE, twoAttr->getFlags());

    CPPUNIT_ASSERT_EQUAL(String("three"), threeAttr->getName());
    CPPUNIT_ASSERT_EQUAL(TYPE_FLOAT, threeAttr->getType());
    CPPUNIT_ASSERT_EQUAL(FLAGS_BLURRABLE, threeAttr->getFlags());

    // Test the const version as well.
    const SceneClass* constClass = &sc;
    CPPUNIT_ASSERT_NO_THROW(
        const Attribute* constAttr = constClass->getAttribute(oneKey);
        CPPUNIT_ASSERT_EQUAL(String("one"), constAttr->getName());
        CPPUNIT_ASSERT_EQUAL(TYPE_BOOL, constAttr->getType());
        CPPUNIT_ASSERT_EQUAL(FLAGS_NONE, constAttr->getFlags());
    );
}

void
TestSceneClass::testGetAttributeByName()
{
    SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

    sc.declareAttribute<Bool>("one");
    sc.declareAttribute<Int>("two", FLAGS_BINDABLE);
    sc.declareAttribute<Float>("three", FLAGS_BLURRABLE);
    
    sc.setComplete();

    Attribute* oneAttr = nullptr; 
    Attribute* twoAttr = nullptr;
    Attribute* threeAttr = nullptr;
    CPPUNIT_ASSERT_NO_THROW(oneAttr = sc.getAttribute("one"));
    CPPUNIT_ASSERT_NO_THROW(twoAttr = sc.getAttribute("two"));
    CPPUNIT_ASSERT_NO_THROW(threeAttr = sc.getAttribute("three"));

    CPPUNIT_ASSERT_EQUAL(String("one"), oneAttr->getName());
    CPPUNIT_ASSERT_EQUAL(TYPE_BOOL, oneAttr->getType());
    CPPUNIT_ASSERT_EQUAL(FLAGS_NONE, oneAttr->getFlags());

    CPPUNIT_ASSERT_EQUAL(String("two"), twoAttr->getName());
    CPPUNIT_ASSERT_EQUAL(TYPE_INT, twoAttr->getType());
    CPPUNIT_ASSERT_EQUAL(FLAGS_BINDABLE, twoAttr->getFlags());

    CPPUNIT_ASSERT_EQUAL(String("three"), threeAttr->getName());
    CPPUNIT_ASSERT_EQUAL(TYPE_FLOAT, threeAttr->getType());
    CPPUNIT_ASSERT_EQUAL(FLAGS_BLURRABLE, threeAttr->getFlags());

    // Test the const version as well.
    const SceneClass* constClass = &sc;
    CPPUNIT_ASSERT_NO_THROW(
        const Attribute* constAttr = constClass->getAttribute("one");
        CPPUNIT_ASSERT_EQUAL(String("one"), constAttr->getName());
        CPPUNIT_ASSERT_EQUAL(TYPE_BOOL, constAttr->getType());
        CPPUNIT_ASSERT_EQUAL(FLAGS_NONE, constAttr->getFlags());
    );
}

void
TestSceneClass::testGetAttributeKeyByName()
{
    SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

    AttributeKey<Bool> oneKey = sc.declareAttribute<Bool>("one");
    sc.setComplete();

    AttributeKey<Bool> anotherOneKey = sc.getAttributeKey<Bool>("one");

    CPPUNIT_ASSERT(oneKey == anotherOneKey);
}

void
TestSceneClass::testIterateAttributes()
{
    SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

    sc.declareAttribute<Bool>("one");
    sc.declareAttribute<Int>("two", FLAGS_BINDABLE);
    sc.declareAttribute<Float>("three", FLAGS_BLURRABLE);
    
    sc.setComplete();

    Attribute* oneAttr = sc.getAttribute("one");
    Attribute* twoAttr = sc.getAttribute("two");
    Attribute* threeAttr = sc.getAttribute("three");

    SceneClass::AttributeConstIterator iter = sc.beginAttributes();
    CPPUNIT_ASSERT(*iter == oneAttr);
    ++iter;
    CPPUNIT_ASSERT(*iter == twoAttr);
    ++iter;
    CPPUNIT_ASSERT(*iter == threeAttr);
    ++iter;
    CPPUNIT_ASSERT(iter == sc.endAttributes());
}

void
TestSceneClass::testMemoryLayout()
{
    // Assumes a cache line size of 64 bytes (which should be the case for
    // all modern processors).

    // Verify some basic assumptions about data type size.
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), sizeof(Bool));
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(4), sizeof(Float));
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(8), sizeof(Double));
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(12), sizeof(Vec3f));
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(24), sizeof(Vec3d));
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(16), sizeof(Vec4f));
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(32), sizeof(Vec4d));
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(64), sizeof(Mat4f));
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(128), sizeof(Mat4d));
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(8), sizeof(SceneObject*));

    // Check alignment for types larger than a cache line.
    {
        SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

        AttributeKey<Mat4d> matKey = sc.declareAttribute<Mat4d>("mat");
        CPPUNIT_ASSERT(matKey.mOffset == 0);
    }
    {
        SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

        AttributeKey<Bool> boolKey = sc.declareAttribute<Bool>("bool");
        CPPUNIT_ASSERT(boolKey.mOffset == 0);

        AttributeKey<Mat4d> matKey = sc.declareAttribute<Mat4d>("mat");
        CPPUNIT_ASSERT(matKey.mOffset == 64);
    }
    {
        SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

        AttributeKey<Bool> boolKey = sc.declareAttribute<Bool>("bool");
        CPPUNIT_ASSERT(boolKey.mOffset == 0);

        AttributeKey<Mat4d> matKey = sc.declareAttribute<Mat4d>("mat", FLAGS_BLURRABLE);
        CPPUNIT_ASSERT(matKey.mOffset == 64);
    }

    // Check alignment for types exactly equal to a cache line.
    {
        SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

        AttributeKey<Mat4f> matKey = sc.declareAttribute<Mat4f>("mat");
        CPPUNIT_ASSERT(matKey.mOffset == 0);
    }
    {
        SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

        AttributeKey<Bool> boolKey = sc.declareAttribute<Bool>("bool");
        CPPUNIT_ASSERT(boolKey.mOffset == 0);

        AttributeKey<Mat4f> matKey = sc.declareAttribute<Mat4f>("mat");
        CPPUNIT_ASSERT(matKey.mOffset == 64);
    }
    {
        SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

        AttributeKey<Bool> boolKey = sc.declareAttribute<Bool>("bool");
        CPPUNIT_ASSERT(boolKey.mOffset == 0);

        AttributeKey<Mat4f> matKey = sc.declareAttribute<Mat4f>("mat", FLAGS_BLURRABLE);
        CPPUNIT_ASSERT(matKey.mOffset == 64);
    }

    // Check alignment for types smaller than a cache line.
    {
        SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

        AttributeKey<Vec3f> vecKey = sc.declareAttribute<Vec3f>("vec");
        CPPUNIT_ASSERT(vecKey.mOffset == 0);
    }
    {
        SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

        AttributeKey<Bool> boolKey = sc.declareAttribute<Bool>("bool");
        CPPUNIT_ASSERT(boolKey.mOffset == 0);

        AttributeKey<Vec3f> vecKey = sc.declareAttribute<Vec3f>("vec");
        CPPUNIT_ASSERT(vecKey.mOffset == 4);
    }
    {
        SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

        AttributeKey<Bool> boolKey = sc.declareAttribute<Bool>("bool");
        CPPUNIT_ASSERT(boolKey.mOffset == 0);

        AttributeKey<Vec3f> vec3fKey = sc.declareAttribute<Vec3f>("vec3f");
        CPPUNIT_ASSERT(vec3fKey.mOffset == 4);

        AttributeKey<Double> doubleKey = sc.declareAttribute<Double>("double", FLAGS_BLURRABLE);
        CPPUNIT_ASSERT(doubleKey.mOffset == 16);

        AttributeKey<Float> floatKey = sc.declareAttribute<Float>("float", FLAGS_BLURRABLE);
        CPPUNIT_ASSERT(floatKey.mOffset == 32);

        AttributeKey<Double> double2Key = sc.declareAttribute<Double>("double_2", FLAGS_BLURRABLE, INTERFACE_GENERIC, { "double 2" });
        CPPUNIT_ASSERT(double2Key.mOffset == 40);

        AttributeKey<Float> float2Key = sc.declareAttribute<Float>("float_2", FLAGS_BLURRABLE, INTERFACE_GENERIC, { "float 2" });
        CPPUNIT_ASSERT(float2Key.mOffset == 56);

        AttributeKey<SceneObject*> sceneObjectKey = sc.declareAttribute<SceneObject*>("scene_object", { "scene object" });
        CPPUNIT_ASSERT(sceneObjectKey.mOffset == 64);
    }

    // Check alignment for straddling cache lines.
    {
        SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

        AttributeKey<Vec3d> vec3d1Key = sc.declareAttribute<Vec3d>("vec3d_1", FLAGS_BLURRABLE, INTERFACE_GENERIC, { "vec3d 1" });
        CPPUNIT_ASSERT(vec3d1Key.mOffset == 0);

        AttributeKey<Vec3d> vec3d2Key = sc.declareAttribute<Vec3d>("vec3d_2", { "vec3d 2" });
        CPPUNIT_ASSERT(vec3d2Key.mOffset == 64);
    }
}

void
TestSceneClass::testCreateDestroyObject()
{
    SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

    // Attempting to create or destroy an object before the SceneClass is
    // complete should throw.
    CPPUNIT_ASSERT_THROW(sc.createObject("awesome"), except::RuntimeError);

    SceneObject* possum = (SceneObject*)0xdeadbeef;
    CPPUNIT_ASSERT_THROW(sc.destroyObject(possum), except::RuntimeError);
}

void
TestSceneClass::testAttributeStorage()
{
    {
        SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

        AttributeKey<Bool> key = sc.declareAttribute<Bool>("attr", true);
        sc.setComplete();
        void* storage = sc.createStorage();

        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == true);
        sc.setValue(storage, key, TIMESTEP_BEGIN, false);
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == false);

        sc.destroyStorage(storage);
    }
    {
        SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

        AttributeKey<Int> key = sc.declareAttribute<Int>("attr", Int(100), FLAGS_BLURRABLE);
        sc.setComplete();
        void* storage = sc.createStorage();

        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == Int(100));
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_END) == Int(100));
        sc.setValue(storage, key, TIMESTEP_BEGIN, Int(101));
        sc.setValue(storage, key, TIMESTEP_END, Int(102));
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == Int(101));
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_END) == Int(102));

        sc.destroyStorage(storage);
    }
    {
        SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

        AttributeKey<Long> key = sc.declareAttribute<Long>("attr", Long(100), FLAGS_BLURRABLE);
        sc.setComplete();
        void* storage = sc.createStorage();

        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == Long(100));
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_END) == Long(100));
        sc.setValue(storage, key, TIMESTEP_BEGIN, Long(101));
        sc.setValue(storage, key, TIMESTEP_END, Long(102));
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == Long(101));
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_END) == Long(102));

        sc.destroyStorage(storage);
    }
    {
        SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

        AttributeKey<Float> key = sc.declareAttribute<Float>("attr", 1.0f, FLAGS_BLURRABLE);
        sc.setComplete();
        void* storage = sc.createStorage();

        CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f, sc.getValue(storage, key, TIMESTEP_BEGIN), 0.0001f);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0f, sc.getValue(storage, key, TIMESTEP_END), 0.0001f);
        sc.setValue(storage, key, TIMESTEP_BEGIN, 2.0f);
        sc.setValue(storage, key, TIMESTEP_END, 3.0f);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0f, sc.getValue(storage, key, TIMESTEP_BEGIN), 0.0001f);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0f, sc.getValue(storage, key, TIMESTEP_END), 0.0001f);

        sc.destroyStorage(storage);
    }
    {
        SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

        AttributeKey<Double> key = sc.declareAttribute<Double>("attr", 1.0, FLAGS_BLURRABLE);
        sc.setComplete();
        void* storage = sc.createStorage();

        CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, sc.getValue(storage, key, TIMESTEP_BEGIN), 0.0001f);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, sc.getValue(storage, key, TIMESTEP_END), 0.0001f);
        sc.setValue(storage, key, TIMESTEP_BEGIN, 2.0);
        sc.setValue(storage, key, TIMESTEP_END, 3.0);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, sc.getValue(storage, key, TIMESTEP_BEGIN), 0.0001f);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0, sc.getValue(storage, key, TIMESTEP_END), 0.0001f);

        sc.destroyStorage(storage);
    }
    {
        SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

        AttributeKey<String> key = sc.declareAttribute<String>("attr", String("wat"));
        sc.setComplete();
        void* storage = sc.createStorage();

        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == String("wat"));
        sc.setValue(storage, key, TIMESTEP_BEGIN, String("pizza"));
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == String("pizza"));

        sc.destroyStorage(storage);
    }
    {
        SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

        AttributeKey<Rgb> key = sc.declareAttribute<Rgb>("attr", Rgb(0.1f, 0.2f, 0.3f), FLAGS_BLURRABLE);
        sc.setComplete();
        void* storage = sc.createStorage();

        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == Rgb(0.1f, 0.2f, 0.3f));
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_END) == Rgb(0.1f, 0.2f, 0.3f));
        sc.setValue(storage, key, TIMESTEP_BEGIN, Rgb(0.4f, 0.5f, 0.6f));
        sc.setValue(storage, key, TIMESTEP_END, Rgb(0.7f, 0.8f, 0.9f));
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == Rgb(0.4f, 0.5f, 0.6f));
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_END) == Rgb(0.7f, 0.8f, 0.9f));

        sc.destroyStorage(storage);
    }
    {
        SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

        AttributeKey<Rgba> key = sc.declareAttribute<Rgba>("attr", Rgba(0.1f, 0.2f, 0.3f, 0.4f), FLAGS_BLURRABLE);
        sc.setComplete();
        void* storage = sc.createStorage();

        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == Rgba(0.1f, 0.2f, 0.3f, 0.4f));
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_END) == Rgba(0.1f, 0.2f, 0.3f, 0.4f));
        sc.setValue(storage, key, TIMESTEP_BEGIN, Rgba(0.4f, 0.5f, 0.6f, 0.7f));
        sc.setValue(storage, key, TIMESTEP_END, Rgba(0.7f, 0.8f, 0.9f, 0.1f));
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == Rgba(0.4f, 0.5f, 0.6f, 0.7f));
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_END) == Rgba(0.7f, 0.8f, 0.9f, 0.1f));

        sc.destroyStorage(storage);
    }
    {
        SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

        AttributeKey<Vec2f> key = sc.declareAttribute<Vec2f>("attr", Vec2f(1.0f, 2.0f), FLAGS_BLURRABLE);
        sc.setComplete();
        void* storage = sc.createStorage();

        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == Vec2f(1.0f, 2.0f));
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_END) == Vec2f(1.0f, 2.0f));
        sc.setValue(storage, key, TIMESTEP_BEGIN, Vec2f(3.0f, 4.0f));
        sc.setValue(storage, key, TIMESTEP_END, Vec2f(5.0f, 6.0f));
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == Vec2f(3.0f, 4.0f));
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_END) == Vec2f(5.0f, 6.0f));

        sc.destroyStorage(storage);
    }
    {
        SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

        AttributeKey<Vec2d> key = sc.declareAttribute<Vec2d>("attr", Vec2d(1.0, 2.0), FLAGS_BLURRABLE);
        sc.setComplete();
        void* storage = sc.createStorage();

        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == Vec2d(1.0, 2.0));
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_END) == Vec2d(1.0, 2.0));
        sc.setValue(storage, key, TIMESTEP_BEGIN, Vec2d(3.0, 4.0));
        sc.setValue(storage, key, TIMESTEP_END, Vec2d(5.0, 6.0));
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == Vec2d(3.0, 4.0));
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_END) == Vec2d(5.0, 6.0));

        sc.destroyStorage(storage);
    }
    {
        SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

        AttributeKey<Vec3f> key = sc.declareAttribute<Vec3f>("attr", Vec3f(1.0f, 2.0f, 3.0f), FLAGS_BLURRABLE);
        sc.setComplete();
        void* storage = sc.createStorage();

        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == Vec3f(1.0f, 2.0f, 3.0f));
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_END) == Vec3f(1.0f, 2.0f, 3.0f));
        sc.setValue(storage, key, TIMESTEP_BEGIN, Vec3f(3.0f, 4.0f, 5.0f));
        sc.setValue(storage, key, TIMESTEP_END, Vec3f(5.0f, 6.0f, 7.0f));
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == Vec3f(3.0f, 4.0f, 5.0f));
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_END) == Vec3f(5.0f, 6.0f, 7.0f));

        sc.destroyStorage(storage);
    }
    {
        SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

        AttributeKey<Vec3d> key = sc.declareAttribute<Vec3d>("attr", Vec3d(1.0, 2.0, 3.0), FLAGS_BLURRABLE);
        sc.setComplete();
        void* storage = sc.createStorage();

        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == Vec3d(1.0, 2.0, 3.0));
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_END) == Vec3d(1.0, 2.0, 3.0));
        sc.setValue(storage, key, TIMESTEP_BEGIN, Vec3d(3.0, 4.0, 5.0));
        sc.setValue(storage, key, TIMESTEP_END, Vec3d(5.0, 6.0, 7.0));
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == Vec3d(3.0, 4.0, 5.0));
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_END) == Vec3d(5.0, 6.0, 7.0));

        sc.destroyStorage(storage);
    }
    {
        SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

        AttributeKey<Vec4f> key = sc.declareAttribute<Vec4f>("attr", Vec4f(1.0f, 2.0f, 3.0f, 4.0f), FLAGS_BLURRABLE);
        sc.setComplete();
        void* storage = sc.createStorage();

        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == Vec4f(1.0f, 2.0f, 3.0f, 4.0f));
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_END) == Vec4f(1.0f, 2.0f, 3.0f, 4.0f));
        sc.setValue(storage, key, TIMESTEP_BEGIN, Vec4f(4.0f, 5.0f, 6.0f, 7.0f));
        sc.setValue(storage, key, TIMESTEP_END, Vec4f(7.0f, 8.0f, 9.0f, 10.0f));
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == Vec4f(4.0f, 5.0f, 6.0f, 7.0f));
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_END) == Vec4f(7.0f, 8.0f, 9.0f, 10.0f));

        sc.destroyStorage(storage);
    }
    {
        SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

        AttributeKey<Vec4d> key = sc.declareAttribute<Vec4d>("attr", Vec4d(1.0, 2.0, 3.0, 4.0), FLAGS_BLURRABLE);
        sc.setComplete();
        void* storage = sc.createStorage();

        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == Vec4d(1.0, 2.0, 3.0, 4.0));
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_END) == Vec4d(1.0, 2.0, 3.0, 4.0));
        sc.setValue(storage, key, TIMESTEP_BEGIN, Vec4d(4.0, 5.0, 6.0, 7.0));
        sc.setValue(storage, key, TIMESTEP_END, Vec4d(7.0, 8.0, 9.0, 10.0));
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == Vec4d(4.0, 5.0, 6.0, 7.0));
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_END) == Vec4d(7.0, 8.0, 9.0, 10.0));

        sc.destroyStorage(storage);
    }
    {
        SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

        AttributeKey<Mat4f> key = sc.declareAttribute<Mat4f>("attr", Mat4f(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f), FLAGS_BLURRABLE);
        sc.setComplete();
        void* storage = sc.createStorage();

        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == Mat4f(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f));
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_END) == Mat4f(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f));
        sc.setValue(storage, key, TIMESTEP_BEGIN, Mat4f(17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f, 24.0f, 25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f, 32.0f));
        sc.setValue(storage, key, TIMESTEP_END, Mat4f(16.0f, 15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f));
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == Mat4f(17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f, 24.0f, 25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f, 32.0f));
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_END) == Mat4f(16.0f, 15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f));

        sc.destroyStorage(storage);
    }
    {
        SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

        AttributeKey<Mat4d> key = sc.declareAttribute<Mat4d>("attr", Mat4d(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0), FLAGS_BLURRABLE);
        sc.setComplete();
        void* storage = sc.createStorage();

        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == Mat4d(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0));
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_END) == Mat4d(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0));
        sc.setValue(storage, key, TIMESTEP_BEGIN, Mat4d(17.0, 18.0, 19.0, 20.0, 21.0, 22.0, 23.0, 24.0, 25.0, 26.0, 27.0, 28.0, 29.0, 30.0, 31.0, 32.0));
        sc.setValue(storage, key, TIMESTEP_END, Mat4d(16.0, 15.0, 14.0, 13.0, 12.0, 11.0, 10.0, 9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0));
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == Mat4d(17.0, 18.0, 19.0, 20.0, 21.0, 22.0, 23.0, 24.0, 25.0, 26.0, 27.0, 28.0, 29.0, 30.0, 31.0, 32.0));
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_END) == Mat4d(16.0, 15.0, 14.0, 13.0, 12.0, 11.0, 10.0, 9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0));

        sc.destroyStorage(storage);
    }
    {
        SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

        AttributeKey<SceneObject*> key = sc.declareAttribute<SceneObject*>("attr", (SceneObject*)0xdeadbeef);
        sc.setComplete();
        void* storage = sc.createStorage();

        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == (SceneObject*)0xdeadbeef);
        sc.setValue(storage, key, TIMESTEP_BEGIN, (SceneObject*)0xc001d00d);
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == (SceneObject*)0xc001d00d);

        sc.destroyStorage(storage);
    }
    {
        SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

        AttributeKey<BoolVector> key = sc.declareAttribute<BoolVector>("attr", mBoolVec);
        sc.setComplete();
        void* storage = sc.createStorage();

        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == mBoolVec);
        sc.setValue(storage, key, TIMESTEP_BEGIN, mBoolVec2);
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == mBoolVec2);

        sc.destroyStorage(storage);
    }
    {
        SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

        AttributeKey<IntVector> key = sc.declareAttribute<IntVector>("attr", mIntVec);
        sc.setComplete();
        void* storage = sc.createStorage();

        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == mIntVec);
        sc.setValue(storage, key, TIMESTEP_BEGIN, mIntVec2);
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == mIntVec2);

        sc.destroyStorage(storage);
    }
    {
        SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

        AttributeKey<LongVector> key = sc.declareAttribute<LongVector>("attr", mLongVec);
        sc.setComplete();
        void* storage = sc.createStorage();

        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == mLongVec);
        sc.setValue(storage, key, TIMESTEP_BEGIN, mLongVec2);
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == mLongVec2);

        sc.destroyStorage(storage);
    }
    {
        SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

        AttributeKey<FloatVector> key = sc.declareAttribute<FloatVector>("attr", mFloatVec);
        sc.setComplete();
        void* storage = sc.createStorage();

        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == mFloatVec);
        sc.setValue(storage, key, TIMESTEP_BEGIN, mFloatVec2);
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == mFloatVec2);

        sc.destroyStorage(storage);
    }
    {
        SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

        AttributeKey<DoubleVector> key = sc.declareAttribute<DoubleVector>("attr", mDoubleVec);
        sc.setComplete();
        void* storage = sc.createStorage();

        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == mDoubleVec);
        sc.setValue(storage, key, TIMESTEP_BEGIN, mDoubleVec2);
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == mDoubleVec2);

        sc.destroyStorage(storage);
    }
#ifdef WORKING_STRINGVECTOR_ATTRIBUTE_DEFAULT
    {
        SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

        AttributeKey<StringVector> key = sc.declareAttribute<StringVector>("attr", mStringVec, {});
        sc.setComplete();
        void* storage = sc.createStorage();

        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == mStringVec);
        sc.setValue(storage, key, TIMESTEP_BEGIN, mStringVec2);
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == mStringVec2);

        sc.destroyStorage(storage);
    }
#endif
    {
        SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

        AttributeKey<RgbVector> key = sc.declareAttribute<RgbVector>("attr", mRgbVec);
        sc.setComplete();
        void* storage = sc.createStorage();

        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == mRgbVec);
        sc.setValue(storage, key, TIMESTEP_BEGIN, mRgbVec2);
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == mRgbVec2);

        sc.destroyStorage(storage);
    }
    {
        SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

        AttributeKey<RgbaVector> key = sc.declareAttribute<RgbaVector>("attr", mRgbaVec);
        sc.setComplete();
        void* storage = sc.createStorage();

        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == mRgbaVec);
        sc.setValue(storage, key, TIMESTEP_BEGIN, mRgbaVec2);
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == mRgbaVec2);

        sc.destroyStorage(storage);
    }
    {
        SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

        AttributeKey<Vec2fVector> key = sc.declareAttribute<Vec2fVector>("attr", mVec2fVec);
        sc.setComplete();
        void* storage = sc.createStorage();

        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == mVec2fVec);
        sc.setValue(storage, key, TIMESTEP_BEGIN, mVec2fVec2);
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == mVec2fVec2);

        sc.destroyStorage(storage);
    }
    {
        SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

        AttributeKey<Vec2dVector> key = sc.declareAttribute<Vec2dVector>("attr", mVec2dVec);
        sc.setComplete();
        void* storage = sc.createStorage();

        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == mVec2dVec);
        sc.setValue(storage, key, TIMESTEP_BEGIN, mVec2dVec2);
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == mVec2dVec2);

        sc.destroyStorage(storage);
    }
    {
        SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

        AttributeKey<Vec3fVector> key = sc.declareAttribute<Vec3fVector>("attr", mVec3fVec);
        sc.setComplete();
        void* storage = sc.createStorage();

        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == mVec3fVec);
        sc.setValue(storage, key, TIMESTEP_BEGIN, mVec3fVec2);
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == mVec3fVec2);

        sc.destroyStorage(storage);
    }
    {
        SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

        AttributeKey<Vec3dVector> key = sc.declareAttribute<Vec3dVector>("attr", mVec3dVec);
        sc.setComplete();
        void* storage = sc.createStorage();

        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == mVec3dVec);
        sc.setValue(storage, key, TIMESTEP_BEGIN, mVec3dVec2);
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == mVec3dVec2);

        sc.destroyStorage(storage);
    }
    {
        SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

        AttributeKey<Vec4fVector> key = sc.declareAttribute<Vec4fVector>("attr", mVec4fVec);
        sc.setComplete();
        void* storage = sc.createStorage();

        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == mVec4fVec);
        sc.setValue(storage, key, TIMESTEP_BEGIN, mVec4fVec2);
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == mVec4fVec2);

        sc.destroyStorage(storage);
    }
    {
        SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

        AttributeKey<Vec4dVector> key = sc.declareAttribute<Vec4dVector>("attr", mVec4dVec);
        sc.setComplete();
        void* storage = sc.createStorage();

        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == mVec4dVec);
        sc.setValue(storage, key, TIMESTEP_BEGIN, mVec4dVec2);
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == mVec4dVec2);

        sc.destroyStorage(storage);
    }
    {
        SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

        AttributeKey<Mat4fVector> key = sc.declareAttribute<Mat4fVector>("attr", mMat4fVec);
        sc.setComplete();
        void* storage = sc.createStorage();

        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == mMat4fVec);
        sc.setValue(storage, key, TIMESTEP_BEGIN, mMat4fVec2);
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == mMat4fVec2);

        sc.destroyStorage(storage);
    }
    {
        SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

        AttributeKey<Mat4dVector> key = sc.declareAttribute<Mat4dVector>("attr", mMat4dVec);
        sc.setComplete();
        void* storage = sc.createStorage();

        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == mMat4dVec);
        sc.setValue(storage, key, TIMESTEP_BEGIN, mMat4dVec2);
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == mMat4dVec2);

        sc.destroyStorage(storage);
    }
    {
        SceneClass sc(&mContext, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", "."));

        AttributeKey<SceneObjectVector> key = sc.declareAttribute<SceneObjectVector>("attr", mSceneObjectVec);
        sc.setComplete();
        void* storage = sc.createStorage();

        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == mSceneObjectVec);
        sc.setValue(storage, key, TIMESTEP_BEGIN, mSceneObjectVec2);
        CPPUNIT_ASSERT(sc.getValue(storage, key, TIMESTEP_BEGIN) == mSceneObjectVec2);

        sc.destroyStorage(storage);
    }
}

} // namespace unittest
} // namespace rdl2
} // namespace scene_rdl2

