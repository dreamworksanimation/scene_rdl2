// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "TestSceneObject.h"

#include <scene_rdl2/scene/rdl2/AttributeKey.h>
#include <scene_rdl2/scene/rdl2/Dso.h>
#include <scene_rdl2/scene/rdl2/SceneClass.h>
#include <scene_rdl2/scene/rdl2/SceneObject.h>
#include <scene_rdl2/scene/rdl2/Types.h>

#include <string>

namespace scene_rdl2 {
namespace rdl2 {
namespace unittest {

void
TestSceneObject::setUp()
{
    mDsoClass.reset(new SceneClass(nullptr, "ExampleObject", ObjectFactory::createDsoFactory("ExampleObject", ".")));

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
    mVec4fVec2.push_back(Vec4f(4.0f, 5.0f, 6.0f, 7.0f));
    mVec4fVec2.push_back(Vec4f(7.0f, 8.0f, 9.0f, 10.0f));

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
    mSceneObjectVec.push_back(nullptr);
    mSceneObjectVec.push_back(nullptr);

    mSceneObjectVec2.clear();
    mSceneObjectVec2.push_back(nullptr);
    mSceneObjectVec2.push_back(nullptr);

    mBoolKey = mDsoClass->declareAttribute<Bool>("bool", true);
    mIntKey = mDsoClass->declareAttribute<Int>("int", Int(100), FLAGS_BLURRABLE);
    mLongKey = mDsoClass->declareAttribute<Long>("Long", Long(101), FLAGS_BLURRABLE);
    mFloatKey = mDsoClass->declareAttribute<Float>("float", 1.0f, FLAGS_BLURRABLE);
    mDoubleKey = mDsoClass->declareAttribute<Double>("double", 2.0, FLAGS_BLURRABLE);
    mStringKey = mDsoClass->declareAttribute<String>("string", String("wat"));
    mRgbKey = mDsoClass->declareAttribute<Rgb>("rgb", Rgb(0.1f, 0.2f, 0.3f), FLAGS_BLURRABLE);
    mRgbaKey = mDsoClass->declareAttribute<Rgba>("rgba", Rgba(0.1f, 0.2f, 0.3f, 0.4f), FLAGS_BLURRABLE);
    mVec2fKey = mDsoClass->declareAttribute<Vec2f>("vec2f", Vec2f(1.0f, 2.0f), FLAGS_BLURRABLE);
    mVec2dKey = mDsoClass->declareAttribute<Vec2d>("vec2d", Vec2d(1.0, 2.0), FLAGS_BLURRABLE);
    mVec3fKey = mDsoClass->declareAttribute<Vec3f>("vec3f", Vec3f(1.0f, 2.0f, 3.0f), FLAGS_BLURRABLE);
    mVec3dKey = mDsoClass->declareAttribute<Vec3d>("vec3d", Vec3d(1.0, 2.0, 3.0), FLAGS_BLURRABLE);
    mVec4fKey = mDsoClass->declareAttribute<Vec4f>("vec4f", Vec4f(1.0f, 2.0f, 3.0f, 4.0f), FLAGS_BLURRABLE);
    mVec4dKey = mDsoClass->declareAttribute<Vec4d>("vec4d", Vec4d(1.0, 2.0, 3.0, 4.0), FLAGS_BLURRABLE);
    mMat4fKey = mDsoClass->declareAttribute<Mat4f>("mat4f", Mat4f(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f), FLAGS_BLURRABLE);
    mMat4dKey = mDsoClass->declareAttribute<Mat4d>("mat4d", Mat4d(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0), FLAGS_BLURRABLE);
    mSceneObjectKey = mDsoClass->declareAttribute<SceneObject*>("scene_object", nullptr, { "scene object" });
    mBoolVectorKey = mDsoClass->declareAttribute<BoolVector>("bool_vector", mBoolVec, { "bool vector" });
    mIntVectorKey = mDsoClass->declareAttribute<IntVector>("int_vector", mIntVec, { "int vector" });
    mLongVectorKey = mDsoClass->declareAttribute<LongVector>("long_vector", mLongVec, { "Long vector" });
    mFloatVectorKey = mDsoClass->declareAttribute<FloatVector>("float_vector", mFloatVec, { "float vector" });
    mDoubleVectorKey = mDsoClass->declareAttribute<DoubleVector>("double_vector", mDoubleVec, { "double vector" });
    mStringVectorKey = mDsoClass->declareAttribute<StringVector>("string_vector", mStringVec, { "string vector" });
    mRgbVectorKey = mDsoClass->declareAttribute<RgbVector>("rgb_vector", mRgbVec, { "rgb vector" });
    mRgbaVectorKey = mDsoClass->declareAttribute<RgbaVector>("rgba_vector", mRgbaVec, { "rgba vector" });
    mVec2fVectorKey = mDsoClass->declareAttribute<Vec2fVector>("vec2f_vector", mVec2fVec, { "vec2f vector" });
    mVec2dVectorKey = mDsoClass->declareAttribute<Vec2dVector>("vec2d_vector", mVec2dVec, { "vec2d vector" });
    mVec3fVectorKey = mDsoClass->declareAttribute<Vec3fVector>("vec3f_vector", mVec3fVec, { "vec3f vector" });
    mVec3dVectorKey = mDsoClass->declareAttribute<Vec3dVector>("vec3d_vector", mVec3dVec, { "vec3d vector" });
    mVec4fVectorKey = mDsoClass->declareAttribute<Vec4fVector>("vec4f_vector", mVec4fVec, { "vec4f vector" });
    mVec4dVectorKey = mDsoClass->declareAttribute<Vec4dVector>("vec4d_vector", mVec4dVec, { "vec4d vector" });
    mMat4fVectorKey = mDsoClass->declareAttribute<Mat4fVector>("mat4f_vector", mMat4fVec, { "mat4f vector" });
    mMat4dVectorKey = mDsoClass->declareAttribute<Mat4dVector>("mat4d_vector", mMat4dVec, { "mat4d vector" });
    mSceneObjectVectorKey = mDsoClass->declareAttribute<SceneObjectVector>("scene_object_vector", mSceneObjectVec, { "scene object vector" });

    mBindableKey = mDsoClass->declareAttribute<Float>("bindable", FLAGS_BINDABLE);

    mDsoClass->setComplete();
}

void
TestSceneObject::tearDown()
{
}

void TestSceneObject::testGetClass()
{
    SceneObject* obj = mDsoClass->createObject("/seq/shot/pepperoni");
    CPPUNIT_ASSERT(obj->getSceneClass().getName() == "ExampleObject");
    mDsoClass->destroyObject(obj);
}

void
TestSceneObject::testGetName()
{
    SceneObject* obj = mDsoClass->createObject("/seq/shot/pepperoni");
    CPPUNIT_ASSERT(obj->getName() == "/seq/shot/pepperoni");
    mDsoClass->destroyObject(obj);
}

void
TestSceneObject::testTimestepGetsAndSets()
{
    SceneObject* obj = mDsoClass->createObject("/seq/shot/pepperoni");

    obj->beginUpdate();

    // Test a non-blurrable core type.
    CPPUNIT_ASSERT(obj->get<Bool>(mBoolKey, TIMESTEP_BEGIN) == true);
    CPPUNIT_ASSERT(obj->get<Bool>(mBoolKey, TIMESTEP_END) == true);

    obj->set<Bool>(mBoolKey, false, TIMESTEP_BEGIN);
    CPPUNIT_ASSERT(obj->get<Bool>(mBoolKey, TIMESTEP_BEGIN) == false);
    CPPUNIT_ASSERT(obj->get<Bool>(mBoolKey, TIMESTEP_END) == false);

    obj->set<Bool>(mBoolKey, true, TIMESTEP_END);
    CPPUNIT_ASSERT(obj->get<Bool>(mBoolKey, TIMESTEP_BEGIN) == true);
    CPPUNIT_ASSERT(obj->get<Bool>(mBoolKey, TIMESTEP_END) == true);

    // Test a blurrable core type.
    CPPUNIT_ASSERT(obj->get<Int>(mIntKey, TIMESTEP_BEGIN) == Int(100));
    CPPUNIT_ASSERT(obj->get<Int>(mIntKey, TIMESTEP_END) == Int(100));

    obj->set<Int>(mIntKey, Int(42), TIMESTEP_BEGIN);
    CPPUNIT_ASSERT(obj->get<Int>(mIntKey, TIMESTEP_BEGIN) == Int(42));
    CPPUNIT_ASSERT(obj->get<Int>(mIntKey, TIMESTEP_END) == Int(100));

    obj->set<Int>(mIntKey, Int(43), TIMESTEP_END);
    CPPUNIT_ASSERT(obj->get<Int>(mIntKey, TIMESTEP_BEGIN) == Int(42));
    CPPUNIT_ASSERT(obj->get<Int>(mIntKey, TIMESTEP_END) == Int(43));

    // Test a blurrable complex type.
    CPPUNIT_ASSERT(obj->get<Rgb>(mRgbKey, TIMESTEP_BEGIN) == Rgb(0.1f, 0.2f, 0.3f));
    CPPUNIT_ASSERT(obj->get<Rgb>(mRgbKey, TIMESTEP_END) == Rgb(0.1f, 0.2f, 0.3f));

    obj->set<Rgb>(mRgbKey, Rgb(0.4f, 0.5f, 0.6f), TIMESTEP_BEGIN);
    CPPUNIT_ASSERT(obj->get<Rgb>(mRgbKey, TIMESTEP_BEGIN) == Rgb(0.4f, 0.5f, 0.6f));
    CPPUNIT_ASSERT(obj->get<Rgb>(mRgbKey, TIMESTEP_END) == Rgb(0.1f, 0.2f, 0.3f));

    obj->set<Rgb>(mRgbKey, Rgb(0.7f, 0.8f, 0.9f), TIMESTEP_END);
    CPPUNIT_ASSERT(obj->get<Rgb>(mRgbKey, TIMESTEP_BEGIN) == Rgb(0.4f, 0.5f, 0.6f));
    CPPUNIT_ASSERT(obj->get<Rgb>(mRgbKey, TIMESTEP_END) == Rgb(0.7f, 0.8f, 0.9f));

    // Test a vector type.
    CPPUNIT_ASSERT(obj->get<FloatVector>(mFloatVectorKey, TIMESTEP_BEGIN) == mFloatVec);
    CPPUNIT_ASSERT(obj->get<FloatVector>(mFloatVectorKey, TIMESTEP_END) == mFloatVec);

    obj->set<FloatVector>(mFloatVectorKey, mFloatVec2, TIMESTEP_BEGIN);
    CPPUNIT_ASSERT(obj->get<FloatVector>(mFloatVectorKey, TIMESTEP_BEGIN) == mFloatVec2);
    CPPUNIT_ASSERT(obj->get<FloatVector>(mFloatVectorKey, TIMESTEP_END) == mFloatVec2);

    obj->set<FloatVector>(mFloatVectorKey, mFloatVec, TIMESTEP_END);
    CPPUNIT_ASSERT(obj->get<FloatVector>(mFloatVectorKey, TIMESTEP_BEGIN) == mFloatVec);
    CPPUNIT_ASSERT(obj->get<FloatVector>(mFloatVectorKey, TIMESTEP_END) == mFloatVec);

    obj->endUpdate();

    mDsoClass->destroyObject(obj);
}

void
TestSceneObject::testSimpleGetsAndSets()
{
    SceneObject* obj = mDsoClass->createObject("/seq/shot/pepperoni");

    obj->beginUpdate();

    // Test a non-blurrable core type.
    CPPUNIT_ASSERT(obj->get<Bool>(mBoolKey) == true);

    obj->set<Bool>(mBoolKey, false);
    CPPUNIT_ASSERT(obj->get<Bool>(mBoolKey) == false);

    // Test a blurrable core type.
    CPPUNIT_ASSERT(obj->get<Int>(mIntKey) == Int(100));

    obj->set<Int>(mIntKey, Int(42));
    CPPUNIT_ASSERT(obj->get<Int>(mIntKey) == Int(42));
    CPPUNIT_ASSERT(obj->get<Int>(mIntKey, TIMESTEP_BEGIN) == Int(42));
    CPPUNIT_ASSERT(obj->get<Int>(mIntKey, TIMESTEP_END) == Int(42));

    // Test a blurrable complex type.
    CPPUNIT_ASSERT(obj->get<Rgb>(mRgbKey) == Rgb(0.1f, 0.2f, 0.3f));

    obj->set<Rgb>(mRgbKey, Rgb(0.4f, 0.5f, 0.6f));
    CPPUNIT_ASSERT(obj->get<Rgb>(mRgbKey) == Rgb(0.4f, 0.5f, 0.6f));
    CPPUNIT_ASSERT(obj->get<Rgb>(mRgbKey, TIMESTEP_BEGIN) == Rgb(0.4f, 0.5f, 0.6f));
    CPPUNIT_ASSERT(obj->get<Rgb>(mRgbKey, TIMESTEP_END) == Rgb(0.4f, 0.5f, 0.6f));

    // Test a vector type.
    CPPUNIT_ASSERT(obj->get<FloatVector>(mFloatVectorKey) == mFloatVec);

    obj->set<FloatVector>(mFloatVectorKey, mFloatVec2);
    CPPUNIT_ASSERT(obj->get<FloatVector>(mFloatVectorKey) == mFloatVec2);

    obj->endUpdate();

    mDsoClass->destroyObject(obj);
}

//void
//TestSceneObject::testInterpolatedGet()
//{
//    SceneObject* obj = mDsoClass->createObject("/seq/shot/pepperoni");
//
//
//    obj->beginUpdate();
//
//    // Test a blurrable core type.
//    obj->set<Int>(mIntKey, Int(20), TIMESTEP_BEGIN);
//    obj->set<Int>(mIntKey, Int(40), TIMESTEP_END);
//    Int intResult = obj->get<Int>(mIntKey, 0.5f);
//    CPPUNIT_ASSERT(intResult == Int(30));
//
//    // Test a blurrable complex type.
//    obj->set<Rgb>(mRgbKey, Rgb(0.2f, 0.4f, 0.6f), TIMESTEP_BEGIN);
//    obj->set<Rgb>(mRgbKey, Rgb(0.4f, 0.6f, 0.8f), TIMESTEP_END);
//    Rgb rgbResult = obj->get<Rgb>(mRgbKey, 0.5f);
//    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.3f, rgbResult.r, 0.0001f);
//    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5f, rgbResult.g, 0.0001f);
//    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.7f, rgbResult.b, 0.0001f);
//
//    obj->endUpdate();
//
//    mDsoClass->destroyObject(obj);
//}

void
TestSceneObject::testConvenienceGetsAndSets()
{
    SceneObject* obj = mDsoClass->createObject("/seq/shot/pizza");

    obj->beginUpdate();

    // Test a normal get/set.
    CPPUNIT_ASSERT(obj->get<Bool>("bool") == true);

    obj->set<Bool>("bool", false);
    CPPUNIT_ASSERT(obj->get<Bool>("bool") == false);

    CPPUNIT_ASSERT_THROW(
        obj->get<Bool>("not an attribute");
    , except::KeyError);
    CPPUNIT_ASSERT_THROW(
        obj->set<Bool>("not an attribute", true);
    , except::KeyError);
    CPPUNIT_ASSERT_THROW(
        obj->get<String>("bool");
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        obj->set<String>("bool", String("string value"));
    , except::TypeError);
    
    // Test a timestep get/set.
    CPPUNIT_ASSERT(obj->get<Int>("int", TIMESTEP_BEGIN) == Int(100));
    CPPUNIT_ASSERT(obj->get<Int>("int", TIMESTEP_END) == Int(100));

    obj->set<Int>("int", Int(42), TIMESTEP_BEGIN);
    CPPUNIT_ASSERT(obj->get<Int>("int", TIMESTEP_BEGIN) == Int(42));
    CPPUNIT_ASSERT(obj->get<Int>("int", TIMESTEP_END) == Int(100));

    obj->set<Int>("int", Int(43), TIMESTEP_END);
    CPPUNIT_ASSERT(obj->get<Int>("int", TIMESTEP_BEGIN) == Int(42));
    CPPUNIT_ASSERT(obj->get<Int>("int", TIMESTEP_END) == Int(43));

    CPPUNIT_ASSERT_THROW(
        obj->get<Bool>("not an attribute", TIMESTEP_BEGIN);
    , except::KeyError);
    CPPUNIT_ASSERT_THROW(
        obj->set<Bool>("not an attribute", true, TIMESTEP_BEGIN);
    , except::KeyError);
    CPPUNIT_ASSERT_THROW(
        obj->get<String>("bool", TIMESTEP_BEGIN);
    , except::TypeError);
    CPPUNIT_ASSERT_THROW(
        obj->set<String>("bool", String("string value"), TIMESTEP_BEGIN);
    , except::TypeError);

    obj->endUpdate();

    mDsoClass->destroyObject(obj);
}

void
TestSceneObject::testResetToDefault()
{
    SceneObject* obj = mDsoClass->createObject("/seq/shot/pizza");

    // Set some values and verify them.
    obj->beginUpdate();
    obj->set(mIntKey, Int(9001));
    obj->set(mStringKey, String("hello"));
    obj->set(mLongKey, Long(9002));
    obj->endUpdate();

    CPPUNIT_ASSERT(obj->get(mIntKey) == Int(9001));
    CPPUNIT_ASSERT(obj->get(mStringKey) == String("hello"));
    CPPUNIT_ASSERT(obj->get(mLongKey) == Long(9002));

    // Reset 2 of the 3 to the default.
    obj->beginUpdate();
    obj->resetToDefault(mIntKey);
    obj->resetToDefault("string");
    obj->endUpdate();

    CPPUNIT_ASSERT(obj->get(mIntKey) == Int(100));
    CPPUNIT_ASSERT(obj->get(mStringKey) == String("wat"));
    CPPUNIT_ASSERT(obj->get(mLongKey) == Long(9002));

    // Reset to default must be during an update.
    CPPUNIT_ASSERT_THROW(obj->resetToDefault(mIntKey), except::RuntimeError);
    mDsoClass->destroyObject(obj);
}

void
TestSceneObject::testAttributeSetMask()
{
    SceneObject* obj = mDsoClass->createObject("/seq/shot/pizza");

    // check mask is initially clear
    CPPUNIT_ASSERT(obj->mAttributeSetMask.test(mIntKey.mIndex) == false);
    CPPUNIT_ASSERT(obj->mAttributeSetMask.test(mStringKey.mIndex) == false);

    // test resetToDefault doesn't set mask
    obj->beginUpdate();
    obj->resetToDefault(mIntKey);
    obj->resetToDefault(mStringKey);
    obj->endUpdate();
    CPPUNIT_ASSERT(obj->mAttributeSetMask.test(mIntKey.mIndex) == false);
    CPPUNIT_ASSERT(obj->mAttributeSetMask.test(mStringKey.mIndex) == false);

    // setting a value does set mask
    obj->beginUpdate();
    obj->set(mIntKey, Int(9001));
    obj->set(mStringKey, String("hello"));
    obj->endUpdate();
    CPPUNIT_ASSERT(obj->mAttributeSetMask.test(mIntKey.mIndex) == true);
    CPPUNIT_ASSERT(obj->mAttributeSetMask.test(mStringKey.mIndex) == true);

    // commitChanges clears mask
    obj->commitChanges();
    CPPUNIT_ASSERT(obj->mAttributeSetMask.test(mIntKey.mIndex) == false);
    CPPUNIT_ASSERT(obj->mAttributeSetMask.test(mStringKey.mIndex) == false);

    // setting to same value doesn't set mask
    obj->beginUpdate();
    obj->set(mIntKey, Int(9001));
    obj->set(mStringKey, String("hello"));
     obj->endUpdate();
    CPPUNIT_ASSERT(obj->mAttributeSetMask.test(mIntKey.mIndex) == false);
    CPPUNIT_ASSERT(obj->mAttributeSetMask.test(mStringKey.mIndex) == false);

    // test resetToDefault sets mask
    obj->beginUpdate();
    obj->resetToDefault(mIntKey);
    obj->resetToDefault(mStringKey); 
    obj->endUpdate();
    CPPUNIT_ASSERT(obj->mAttributeSetMask.test(mIntKey.mIndex) == true);
    CPPUNIT_ASSERT(obj->mAttributeSetMask.test(mStringKey.mIndex) == true);

    mDsoClass->destroyObject(obj);
}

void
TestSceneObject::testResetAllToDefault()
{
    SceneObject* obj = mDsoClass->createObject("/seq/shot/pizza");

    // Set some values and verify them.
    obj->beginUpdate();
    obj->set(mIntKey, Int(9001));
    obj->set(mStringKey, String("hello"));
    obj->set(mLongKey, Long(9002));
    obj->endUpdate();

    CPPUNIT_ASSERT(obj->get(mIntKey) == Int(9001));
    CPPUNIT_ASSERT(obj->get(mStringKey) == String("hello"));
    CPPUNIT_ASSERT(obj->get(mLongKey) == Long(9002));

    // Reset everything to their defaults.
    obj->beginUpdate();
    obj->resetAllToDefault();
    obj->endUpdate();

    CPPUNIT_ASSERT(obj->get(mIntKey) == Int(100));
    CPPUNIT_ASSERT(obj->get(mStringKey) == String("wat"));
    CPPUNIT_ASSERT(obj->get(mLongKey) == Long(101));

    // Reset to default must be during an update.
    CPPUNIT_ASSERT_THROW(obj->resetAllToDefault(), except::RuntimeError);

    mDsoClass->destroyObject(obj);
}

void
TestSceneObject::testBindings()
{
    SceneObject* bindee = mDsoClass->createObject("/seq/shot/bindee");
    SceneObject* binder = mDsoClass->createObject("/seq/shot/binder");
    const SceneObject* constBinder = binder;

    binder->beginUpdate();

    // By default, the binding should be null.
    CPPUNIT_ASSERT_NO_THROW(
        CPPUNIT_ASSERT(binder->getBinding(mBindableKey) == nullptr);
        CPPUNIT_ASSERT(constBinder->getBinding(mBindableKey) == nullptr);
    );

    // Try setting the binding and verify that it is set.
    CPPUNIT_ASSERT_NO_THROW(
        binder->setBinding(mBindableKey, bindee);
        CPPUNIT_ASSERT(binder->getBinding(mBindableKey) == bindee);
        CPPUNIT_ASSERT(constBinder->getBinding(mBindableKey) == bindee);
    );

    // Getting or setting a non-bindable key should throw.
    CPPUNIT_ASSERT_THROW(
        binder->setBinding(mFloatKey, bindee);
    , except::RuntimeError);
    CPPUNIT_ASSERT_THROW(
        binder->getBinding(mFloatKey);
    , except::RuntimeError);
    CPPUNIT_ASSERT_THROW(
        constBinder->getBinding(mFloatKey);
    , except::RuntimeError);

    binder->endUpdate();

    mDsoClass->destroyObject(bindee);
    mDsoClass->destroyObject(binder);
}

class ExtensionTest : public SceneObject::Extension
{
public:
    ExtensionTest(const SceneObject & owner, 
        int i, int & j, const int & k, int && l): // different argument types
        mOwner(owner), mI(i), mJ(j), mK(k), mL(l)
    {
        mOwner.warn("ExtensionTest(...)");
    }
    
    virtual ~ExtensionTest()
    {
        mOwner.warn("~ExtensionTest()");
    }
    
    void check() const
    {
        // This is mostly a compilation test, so check() is simple.
        mOwner.warn("ExtensionTest::check()");
        CPPUNIT_ASSERT(mI == 1);
        CPPUNIT_ASSERT(mJ == 2);
        CPPUNIT_ASSERT(mK == 3);
        CPPUNIT_ASSERT(mL == 4);
    }

protected:
    const SceneObject & mOwner;
    int mI;
    int & mJ;
    const int & mK;
    const int mL;
};

void
TestSceneObject::testExtension()
{
    int one = 1;
    int two = 2;
    int three = 3;
    
    const int One = 1;
    const int Three = 3;
    
    SceneObject * obj = nullptr;
    
    // Try creating objects with various types of arguments.
    
    // Lvalues where possible.
    obj = mDsoClass->createObject("/seq/shot/pizza");
    obj->getOrCreate<ExtensionTest>(one, two, three, 4);
    obj->get<ExtensionTest>().check();
    mDsoClass->destroyObject(obj);

    // Const lvalues where possible.
    obj = mDsoClass->createObject("/seq/shot/pizza");
    obj->getOrCreate<ExtensionTest>(One, two, Three, 4);
    obj->get<ExtensionTest>().check();
    mDsoClass->destroyObject(obj);

    // Rvalues where possible.
    obj = mDsoClass->createObject("/seq/shot/pizza");
    obj->getOrCreate<ExtensionTest>(1, two, 3, 4);
    obj->get<ExtensionTest>().check();
    mDsoClass->destroyObject(obj);
}

} // namespace unittest
} // namespace rdl2
} // namespace scene_rdl2

