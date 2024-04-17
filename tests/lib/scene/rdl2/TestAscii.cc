// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "TestAscii.h"

#include <scene_rdl2/scene/rdl2/AttributeKey.h>
#include <scene_rdl2/scene/rdl2/AsciiReader.h>
#include <scene_rdl2/scene/rdl2/AsciiWriter.h>
#include <scene_rdl2/scene/rdl2/SceneClass.h>
#include <scene_rdl2/scene/rdl2/SceneContext.h>
#include <scene_rdl2/scene/rdl2/SceneObject.h>

#include <scene_rdl2/common/except/exceptions.h>

#include <cppunit/extensions/HelperMacros.h>

#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>

#ifdef _TEST_ASCII_DO_TEST_MEMORY
#include <sys/types.h>
#include <sys/unistd.h>
#endif

namespace scene_rdl2 {
namespace rdl2 {
namespace unittest {

void
TestAscii::setUp()
{
    mBoolVec2.clear();
    mBoolVec2.push_back(false);
    mBoolVec2.push_back(true);

    mIntVec2.clear();
    mIntVec2.push_back(Int(42));
    mIntVec2.push_back(Int(43));

    mLongVec2.clear();
    mLongVec2.push_back(Long(44));
    mLongVec2.push_back(Long(45));

    mFloatVec2.clear();
    mFloatVec2.push_back(4.0f);
    mFloatVec2.push_back(5.0f);

    mDoubleVec2.clear();
    mDoubleVec2.push_back(4.0);
    mDoubleVec2.push_back(5.0);

    mStringVec2.clear();
    mStringVec2.push_back("c");
    mStringVec2.push_back("d");

    mRgbVec2.clear();
    mRgbVec2.push_back(Rgb(0.5f, 0.6f, 0.7f));
    mRgbVec2.push_back(Rgb(0.8f, 0.9f, 0.1f));

    mRgbaVec2.clear();
    mRgbaVec2.push_back(Rgba(0.5f, 0.6f, 0.7f, 0.8f));
    mRgbaVec2.push_back(Rgba(0.9f, 0.1f, 0.2f, 0.3f));

    mVec2fVec2.clear();
    mVec2fVec2.push_back(Vec2f(4.0f, 5.0f));
    mVec2fVec2.push_back(Vec2f(6.0f, 7.0f));

    mVec2dVec2.clear();
    mVec2dVec2.push_back(Vec2d(4.0, 5.0));
    mVec2dVec2.push_back(Vec2d(6.0, 7.0));

    mVec3fVec2.clear();
    mVec3fVec2.push_back(Vec3f(4.0f, 5.0f, 6.0f));
    mVec3fVec2.push_back(Vec3f(6.0f, 7.0f, 8.0f));

    mVec3dVec2.clear();
    mVec3dVec2.push_back(Vec3d(1.0, 2.0, 3.0));
    mVec3dVec2.push_back(Vec3d(4.0, 5.0, 6.0));

    mVec4fVec2.clear();
    mVec4fVec2.push_back(Vec4f(4.0f, 5.0f, 6.0f, 7.0f));
    mVec4fVec2.push_back(Vec4f(7.0f, 8.0f, 9.0f, 10.0f));

    mVec4dVec2.clear();
    mVec4dVec2.push_back(Vec4d(1.0, 2.0, 3.0, 4.0f));
    mVec4dVec2.push_back(Vec4d(5.0, 6.0, 7.0, 8.0f));

    mMat4fVec2.clear();
    mMat4fVec2.push_back(Mat4f(16.0f, 15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f));
    mMat4fVec2.push_back(Mat4f(32.0f, 31.0f, 30.0f, 29.0f, 28.0f, 27.0f, 26.0f, 25.0f, 24.0f, 23.0f, 22.0f, 21.0f, 20.0f, 19.0f, 18.0f, 17.0f));

    mMat4dVec2.clear();
    mMat4dVec2.push_back(Mat4d(16.0, 15.0, 14.0, 13.0, 12.0, 11.0, 10.0, 9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0));
    mMat4dVec2.push_back(Mat4d(32.0, 31.0, 30.0, 29.0, 28.0, 27.0, 26.0, 25.0, 24.0, 23.0, 22.0, 21.0, 20.0, 19.0, 18.0, 17.0));
}

void
TestAscii::tearDown()
{
}

void
TestAscii::testRoundtrip()
{
    // Create the context, load a class, and create some objects.
    SceneContext context;
    const SceneClass* sc = context.createSceneClass("ExtensiveObject");
    SceneObject* pizza = context.createSceneObject("ExtensiveObject", "/seq/shot/pizza");
    SceneObject* cookie = context.createSceneObject("ExtensiveObject", "/seq/shot/cookie");
    SceneObject* teapot = context.createSceneObject("FakeTeapot", "/seq/shot/teapot");
    SceneObject* light = context.createSceneObject("FakeLight", "/seq/shot/light");
    SceneObject* material = context.createSceneObject("FakeMaterial", "/seq/shot/material");

    SceneObjectVector sceneObjectVec2;
    sceneObjectVec2.push_back(light);
    sceneObjectVec2.push_back(material);

    // Grab AttributeKeys for all the attributes.
    AttributeKey<Bool> boolKey = sc->getAttributeKey<Bool>("bool");
    AttributeKey<Int> intKey = sc->getAttributeKey<Int>("int");
    AttributeKey<int64_t> longKey = sc->getAttributeKey<int64_t>("long");
    AttributeKey<Float> floatKey = sc->getAttributeKey<Float>("float");
    AttributeKey<Double> doubleKey = sc->getAttributeKey<Double>("double");
    AttributeKey<String> stringKey = sc->getAttributeKey<String>("string");
    AttributeKey<Rgb> rgbKey = sc->getAttributeKey<Rgb>("rgb");
    AttributeKey<Rgba> rgbaKey = sc->getAttributeKey<Rgba>("rgba");
    AttributeKey<Vec2f> vec2fKey = sc->getAttributeKey<Vec2f>("vec2f");
    AttributeKey<Vec2d> vec2dKey = sc->getAttributeKey<Vec2d>("vec2d");
    AttributeKey<Vec3f> vec3fKey = sc->getAttributeKey<Vec3f>("vec3f");
    AttributeKey<Vec3d> vec3dKey = sc->getAttributeKey<Vec3d>("vec3d");
    AttributeKey<Vec4f> vec4fKey = sc->getAttributeKey<Vec4f>("vec4f");
    AttributeKey<Vec4d> vec4dKey = sc->getAttributeKey<Vec4d>("vec4d");
    AttributeKey<Mat4f> mat4fKey = sc->getAttributeKey<Mat4f>("mat4f");
    AttributeKey<Mat4d> mat4dKey = sc->getAttributeKey<Mat4d>("mat4d");
    AttributeKey<SceneObject*> sceneObjectKey = sc->getAttributeKey<SceneObject*>("scene object");
    AttributeKey<BoolVector> boolVecKey = sc->getAttributeKey<BoolVector>("bool vector");
    AttributeKey<IntVector> intVecKey = sc->getAttributeKey<IntVector>("int vector");
    AttributeKey<LongVector> longVecKey = sc->getAttributeKey<LongVector>("long vector");
    AttributeKey<FloatVector> floatVecKey = sc->getAttributeKey<FloatVector>("float vector");
    AttributeKey<DoubleVector> doubleVecKey = sc->getAttributeKey<DoubleVector>("double vector");
    AttributeKey<StringVector> stringVecKey = sc->getAttributeKey<StringVector>("string vector");
    AttributeKey<RgbVector> rgbVecKey = sc->getAttributeKey<RgbVector>("rgb vector");
    AttributeKey<RgbaVector> rgbaVecKey = sc->getAttributeKey<RgbaVector>("rgba vector");
    AttributeKey<Vec2fVector> vec2fVecKey = sc->getAttributeKey<Vec2fVector>("vec2f vector");
    AttributeKey<Vec2dVector> vec2dVecKey = sc->getAttributeKey<Vec2dVector>("vec2d vector");
    AttributeKey<Vec3fVector> vec3fVecKey = sc->getAttributeKey<Vec3fVector>("vec3f vector");
    AttributeKey<Vec3dVector> vec3dVecKey = sc->getAttributeKey<Vec3dVector>("vec3d vector");
    AttributeKey<Vec4fVector> vec4fVecKey = sc->getAttributeKey<Vec4fVector>("vec4f vector");
    AttributeKey<Vec4dVector> vec4dVecKey = sc->getAttributeKey<Vec4dVector>("vec4d vector");
    AttributeKey<Mat4fVector> mat4fVecKey = sc->getAttributeKey<Mat4fVector>("mat4f vector");
    AttributeKey<Mat4dVector> mat4dVecKey = sc->getAttributeKey<Mat4dVector>("mat4d vector");
    AttributeKey<SceneObjectVector> sceneObjectVecKey = sc->getAttributeKey<SceneObjectVector>("scene object vector");

    // Set all of pizza's attributes, leave cookie's at their defaults.
    pizza->beginUpdate();
    pizza->set(boolKey, false);
    pizza->set(intKey, Int(100), TIMESTEP_BEGIN);
    pizza->set(intKey, Int(101), TIMESTEP_END);
    pizza->set(longKey, Long(102), TIMESTEP_BEGIN);
    pizza->set(longKey, Long(103), TIMESTEP_END);
    pizza->set(floatKey, 3.0f, TIMESTEP_BEGIN);
    pizza->set(floatKey, 4.0f, TIMESTEP_END);
    pizza->set(doubleKey, 5.0, TIMESTEP_BEGIN);
    pizza->set(doubleKey, 6.0, TIMESTEP_END);
    pizza->set(stringKey, String("not a pizza"));
    pizza->setBinding(stringKey, cookie);
    pizza->set(rgbKey, Rgb(0.2f, 0.3f, 0.4f), TIMESTEP_BEGIN);
    pizza->set(rgbKey, Rgb(0.3f, 0.4f, 0.5f), TIMESTEP_END);
    pizza->set(rgbaKey, Rgba(0.2f, 0.3f, 0.4f, 0.5f), TIMESTEP_BEGIN);
    pizza->set(rgbaKey, Rgba(0.3f, 0.4f, 0.5f, 0.6f), TIMESTEP_END);
    pizza->set(vec2fKey, Vec2f(1.0f, 2.0f), TIMESTEP_BEGIN);
    pizza->set(vec2fKey, Vec2f(2.0f, 3.0f), TIMESTEP_END);
    pizza->set(vec2dKey, Vec2d(2.0, 3.0), TIMESTEP_BEGIN);
    pizza->set(vec2dKey, Vec2d(3.0, 4.0), TIMESTEP_END);
    pizza->set(vec3fKey, Vec3f(1.0f, 2.0f, 3.0f), TIMESTEP_BEGIN);
    pizza->set(vec3fKey, Vec3f(2.0f, 3.0f, 4.0f), TIMESTEP_END);
    pizza->set(vec3dKey, Vec3d(2.0, 3.0, 4.0), TIMESTEP_BEGIN);
    pizza->set(vec3dKey, Vec3d(3.0, 4.0, 5.0), TIMESTEP_END);
    pizza->set(vec4fKey, Vec4f(1.0f, 2.0f, 3.0f, 4.0f), TIMESTEP_BEGIN);
    pizza->set(vec4fKey, Vec4f(2.0f, 3.0f, 4.0f, 5.0f), TIMESTEP_END);
    pizza->set(vec4dKey, Vec4d(2.0, 3.0, 4.0, 5.0), TIMESTEP_BEGIN);
    pizza->set(vec4dKey, Vec4d(3.0, 4.0, 5.0, 6.0), TIMESTEP_END);

    pizza->set(mat4fKey, Mat4f(16.0f, 15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f), TIMESTEP_BEGIN);
    pizza->set(mat4fKey, Mat4f(32.0f, 31.0f, 30.0f, 29.0f, 28.0f, 27.0f, 26.0f, 25.0f, 24.0f, 23.0f, 22.0f, 21.0f, 20.0f, 19.0f, 18.0f, 17.0f), TIMESTEP_END);
    pizza->set(mat4dKey, Mat4d(16.0, 15.0, 14.0, 13.0, 12.0, 11.0, 10.0, 9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0), TIMESTEP_BEGIN);
    pizza->set(mat4dKey, Mat4d(32.0, 31.0, 30.0, 29.0, 28.0, 27.0, 26.0, 25.0, 24.0, 23.0, 22.0, 21.0, 20.0, 19.0, 18.0, 17.0), TIMESTEP_END);
    pizza->set(sceneObjectKey, teapot);
    pizza->set(boolVecKey, mBoolVec2);
    pizza->set(intVecKey, mIntVec2);
    pizza->set(longVecKey, mLongVec2);
    pizza->set(floatVecKey, mFloatVec2);
    pizza->set(doubleVecKey, mDoubleVec2);
    pizza->set(stringVecKey, mStringVec2);
    pizza->set(rgbVecKey, mRgbVec2);
    pizza->set(rgbaVecKey, mRgbaVec2);
    pizza->set(vec2fVecKey, mVec2fVec2);
    pizza->set(vec2dVecKey, mVec2dVec2);
    pizza->set(vec3fVecKey, mVec3fVec2);
    pizza->set(vec3dVecKey, mVec3dVec2);
    pizza->set(vec4fVecKey, mVec4fVec2);
    pizza->set(vec4dVecKey, mVec4dVec2);
    pizza->set(mat4fVecKey, mMat4fVec2);
    pizza->set(mat4dVecKey, mMat4dVec2);
    pizza->set(sceneObjectVecKey, sceneObjectVec2);
    pizza->endUpdate();

    // Create Metadata scene object, which is formatted differently from
    // a generic scene object
    SceneClass* sc2 = context.createSceneClass("Metadata");
    SceneObject* metadata = context.createSceneObject("Metadata", "/seq/shot/metadata");
    AttributeKey<StringVector> names = sc2->getAttributeKey<StringVector>("name");
    AttributeKey<StringVector> types = sc2->getAttributeKey<StringVector>("type");
    AttributeKey<StringVector> values = sc2->getAttributeKey<StringVector>("value");
    metadata->beginUpdate();
    metadata->set(names, {"blah", "foo"});
    metadata->set(types, {"int", "string"});
    metadata->set(values, {"2", "abcd"});
    metadata->endUpdate();

    // Write it out.
    AsciiWriter writer(context);
    writer.toFile("roundtrip.rdla");

    // Create a fresh SceneContext and read in the Ascii file.
    SceneContext readContext;
    AsciiReader reader(readContext);
    reader.fromFile("roundtrip.rdla");

    SceneObject* readPizza = readContext.getSceneObject("/seq/shot/pizza");
    SceneObject* readCookie = readContext.getSceneObject("/seq/shot/cookie");
    SceneObject* readMetadata = readContext.getSceneObject("/seq/shot/metadata");

    // Verify that pizza is the same.
    CPPUNIT_ASSERT(pizza->get(boolKey) == readPizza->get(boolKey));
    CPPUNIT_ASSERT(pizza->get(intKey, TIMESTEP_BEGIN) == readPizza->get(intKey, TIMESTEP_BEGIN));
    CPPUNIT_ASSERT(pizza->get(intKey, TIMESTEP_END) == readPizza->get(intKey, TIMESTEP_END));
    CPPUNIT_ASSERT(pizza->get(longKey, TIMESTEP_BEGIN) == readPizza->get(longKey, TIMESTEP_BEGIN));
    CPPUNIT_ASSERT(pizza->get(longKey, TIMESTEP_END) == readPizza->get(longKey, TIMESTEP_END));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(pizza->get(floatKey, TIMESTEP_BEGIN), readPizza->get(floatKey, TIMESTEP_BEGIN), 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(pizza->get(floatKey, TIMESTEP_END), readPizza->get(floatKey, TIMESTEP_END), 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(pizza->get(doubleKey, TIMESTEP_BEGIN), readPizza->get(doubleKey, TIMESTEP_BEGIN), 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(pizza->get(doubleKey, TIMESTEP_END), readPizza->get(doubleKey, TIMESTEP_END), 0.0001f);
    CPPUNIT_ASSERT(pizza->get(stringKey) == readPizza->get(stringKey));
    CPPUNIT_ASSERT(pizza->getBinding(stringKey)->getName() == readPizza->getBinding(stringKey)->getName());
    CPPUNIT_ASSERT(pizza->get(rgbKey, TIMESTEP_BEGIN) == readPizza->get(rgbKey, TIMESTEP_BEGIN));
    CPPUNIT_ASSERT(pizza->get(rgbKey, TIMESTEP_END) == readPizza->get(rgbKey, TIMESTEP_END));
    CPPUNIT_ASSERT(pizza->get(rgbaKey, TIMESTEP_BEGIN) == readPizza->get(rgbaKey, TIMESTEP_BEGIN));
    CPPUNIT_ASSERT(pizza->get(rgbaKey, TIMESTEP_END) == readPizza->get(rgbaKey, TIMESTEP_END));
    CPPUNIT_ASSERT(pizza->get(vec2fKey, TIMESTEP_BEGIN) == readPizza->get(vec2fKey, TIMESTEP_BEGIN));
    CPPUNIT_ASSERT(pizza->get(vec2fKey, TIMESTEP_END) == readPizza->get(vec2fKey, TIMESTEP_END));
    CPPUNIT_ASSERT(pizza->get(vec2dKey, TIMESTEP_BEGIN) == readPizza->get(vec2dKey, TIMESTEP_BEGIN));
    CPPUNIT_ASSERT(pizza->get(vec2dKey, TIMESTEP_END) == readPizza->get(vec2dKey, TIMESTEP_END));
    CPPUNIT_ASSERT(pizza->get(vec3fKey, TIMESTEP_BEGIN) == readPizza->get(vec3fKey, TIMESTEP_BEGIN));
    CPPUNIT_ASSERT(pizza->get(vec3fKey, TIMESTEP_END) == readPizza->get(vec3fKey, TIMESTEP_END));
    CPPUNIT_ASSERT(pizza->get(vec3dKey, TIMESTEP_BEGIN) == readPizza->get(vec3dKey, TIMESTEP_BEGIN));
    CPPUNIT_ASSERT(pizza->get(vec3dKey, TIMESTEP_END) == readPizza->get(vec3dKey, TIMESTEP_END));
    CPPUNIT_ASSERT(pizza->get(vec4fKey, TIMESTEP_BEGIN) == readPizza->get(vec4fKey, TIMESTEP_BEGIN));
    CPPUNIT_ASSERT(pizza->get(vec4fKey, TIMESTEP_END) == readPizza->get(vec4fKey, TIMESTEP_END));
    CPPUNIT_ASSERT(pizza->get(vec4dKey, TIMESTEP_BEGIN) == readPizza->get(vec4dKey, TIMESTEP_BEGIN));
    CPPUNIT_ASSERT(pizza->get(vec4dKey, TIMESTEP_END) == readPizza->get(vec4dKey, TIMESTEP_END));
    CPPUNIT_ASSERT(pizza->get(mat4fKey, TIMESTEP_BEGIN) == readPizza->get(mat4fKey, TIMESTEP_BEGIN));
    CPPUNIT_ASSERT(pizza->get(mat4fKey, TIMESTEP_END) == readPizza->get(mat4fKey, TIMESTEP_END));
    CPPUNIT_ASSERT(pizza->get(mat4dKey, TIMESTEP_BEGIN) == readPizza->get(mat4dKey, TIMESTEP_BEGIN));
    CPPUNIT_ASSERT(pizza->get(mat4dKey, TIMESTEP_END) == readPizza->get(mat4dKey, TIMESTEP_END));
    const SceneObject* readTeapot = readPizza->get(sceneObjectKey);
    CPPUNIT_ASSERT(readTeapot->getName() == "/seq/shot/teapot");
    CPPUNIT_ASSERT(readTeapot->getSceneClass().getName() == "FakeTeapot");
    CPPUNIT_ASSERT(pizza->get(boolVecKey) == readPizza->get(boolVecKey));
    CPPUNIT_ASSERT(pizza->get(intVecKey) == readPizza->get(intVecKey));
    CPPUNIT_ASSERT(pizza->get(longVecKey) == readPizza->get(longVecKey));
    CPPUNIT_ASSERT(pizza->get(floatVecKey) == readPizza->get(floatVecKey));
    CPPUNIT_ASSERT(pizza->get(doubleVecKey) == readPizza->get(doubleVecKey));
    CPPUNIT_ASSERT(pizza->get(stringVecKey) == readPizza->get(stringVecKey));
    CPPUNIT_ASSERT(pizza->get(rgbVecKey) == readPizza->get(rgbVecKey));
    CPPUNIT_ASSERT(pizza->get(rgbaVecKey) == readPizza->get(rgbaVecKey));
    CPPUNIT_ASSERT(pizza->get(vec2fVecKey) == readPizza->get(vec2fVecKey));
    CPPUNIT_ASSERT(pizza->get(vec2dVecKey) == readPizza->get(vec2dVecKey));
    CPPUNIT_ASSERT(pizza->get(vec3fVecKey) == readPizza->get(vec3fVecKey));
    CPPUNIT_ASSERT(pizza->get(vec3dVecKey) == readPizza->get(vec3dVecKey));
    CPPUNIT_ASSERT(pizza->get(vec4fVecKey) == readPizza->get(vec4fVecKey));
    CPPUNIT_ASSERT(pizza->get(vec4dVecKey) == readPizza->get(vec4dVecKey));
    CPPUNIT_ASSERT(pizza->get(mat4fVecKey) == readPizza->get(mat4fVecKey));
    CPPUNIT_ASSERT(pizza->get(mat4dVecKey) == readPizza->get(mat4dVecKey));
    SceneObjectVector things = readPizza->get(sceneObjectVecKey);
    CPPUNIT_ASSERT(things[0]->getName() == "/seq/shot/light");
    CPPUNIT_ASSERT(things[0]->getSceneClass().getName() == "FakeLight");
    CPPUNIT_ASSERT(things[1]->getName() == "/seq/shot/material");
    CPPUNIT_ASSERT(things[1]->getSceneClass().getName() == "FakeMaterial");

    // Verify that cookie is the same.
    CPPUNIT_ASSERT(cookie->get(boolKey) == readCookie->get(boolKey));
    CPPUNIT_ASSERT(cookie->get(intKey, TIMESTEP_BEGIN) == readCookie->get(intKey, TIMESTEP_BEGIN));
    CPPUNIT_ASSERT(cookie->get(intKey, TIMESTEP_END) == readCookie->get(intKey, TIMESTEP_END));
    CPPUNIT_ASSERT(cookie->get(longKey, TIMESTEP_BEGIN) == readCookie->get(longKey, TIMESTEP_BEGIN));
    CPPUNIT_ASSERT(cookie->get(longKey, TIMESTEP_END) == readCookie->get(longKey, TIMESTEP_END));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(cookie->get(floatKey, TIMESTEP_BEGIN), readCookie->get(floatKey, TIMESTEP_BEGIN), 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(cookie->get(floatKey, TIMESTEP_END), readCookie->get(floatKey, TIMESTEP_END), 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(cookie->get(doubleKey, TIMESTEP_BEGIN), readCookie->get(doubleKey, TIMESTEP_BEGIN), 0.0001f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(cookie->get(doubleKey, TIMESTEP_END), readCookie->get(doubleKey, TIMESTEP_END), 0.0001f);
    CPPUNIT_ASSERT(cookie->get(stringKey) == readCookie->get(stringKey));
    CPPUNIT_ASSERT(cookie->getBinding(stringKey) == readCookie->getBinding(stringKey));
    CPPUNIT_ASSERT(cookie->get(rgbKey, TIMESTEP_BEGIN) == readCookie->get(rgbKey, TIMESTEP_BEGIN));
    CPPUNIT_ASSERT(cookie->get(rgbKey, TIMESTEP_END) == readCookie->get(rgbKey, TIMESTEP_END));
    CPPUNIT_ASSERT(cookie->get(rgbaKey, TIMESTEP_BEGIN) == readCookie->get(rgbaKey, TIMESTEP_BEGIN));
    CPPUNIT_ASSERT(cookie->get(rgbaKey, TIMESTEP_END) == readCookie->get(rgbaKey, TIMESTEP_END));
    CPPUNIT_ASSERT(cookie->get(vec2fKey, TIMESTEP_BEGIN) == readCookie->get(vec2fKey, TIMESTEP_BEGIN));
    CPPUNIT_ASSERT(cookie->get(vec2fKey, TIMESTEP_END) == readCookie->get(vec2fKey, TIMESTEP_END));
    CPPUNIT_ASSERT(cookie->get(vec2dKey, TIMESTEP_BEGIN) == readCookie->get(vec2dKey, TIMESTEP_BEGIN));
    CPPUNIT_ASSERT(cookie->get(vec2dKey, TIMESTEP_END) == readCookie->get(vec2dKey, TIMESTEP_END));
    CPPUNIT_ASSERT(cookie->get(vec3fKey, TIMESTEP_BEGIN) == readCookie->get(vec3fKey, TIMESTEP_BEGIN));
    CPPUNIT_ASSERT(cookie->get(vec3fKey, TIMESTEP_END) == readCookie->get(vec3fKey, TIMESTEP_END));
    CPPUNIT_ASSERT(cookie->get(vec3dKey, TIMESTEP_BEGIN) == readCookie->get(vec3dKey, TIMESTEP_BEGIN));
    CPPUNIT_ASSERT(cookie->get(vec3dKey, TIMESTEP_END) == readCookie->get(vec3dKey, TIMESTEP_END));
    CPPUNIT_ASSERT(cookie->get(vec4fKey, TIMESTEP_BEGIN) == readCookie->get(vec4fKey, TIMESTEP_BEGIN));
    CPPUNIT_ASSERT(cookie->get(vec4fKey, TIMESTEP_END) == readCookie->get(vec4fKey, TIMESTEP_END));
    CPPUNIT_ASSERT(cookie->get(vec4dKey, TIMESTEP_BEGIN) == readCookie->get(vec4dKey, TIMESTEP_BEGIN));
    CPPUNIT_ASSERT(cookie->get(vec4dKey, TIMESTEP_END) == readCookie->get(vec4dKey, TIMESTEP_END));
    CPPUNIT_ASSERT(cookie->get(mat4fKey, TIMESTEP_BEGIN) == readCookie->get(mat4fKey, TIMESTEP_BEGIN));
    CPPUNIT_ASSERT(cookie->get(mat4fKey, TIMESTEP_END) == readCookie->get(mat4fKey, TIMESTEP_END));
    CPPUNIT_ASSERT(cookie->get(mat4dKey, TIMESTEP_BEGIN) == readCookie->get(mat4dKey, TIMESTEP_BEGIN));
    CPPUNIT_ASSERT(cookie->get(mat4dKey, TIMESTEP_END) == readCookie->get(mat4dKey, TIMESTEP_END));
    CPPUNIT_ASSERT(cookie->get(sceneObjectKey) == readCookie->get(sceneObjectKey)); // nullptr
    CPPUNIT_ASSERT(cookie->get(boolVecKey) == readCookie->get(boolVecKey));
    CPPUNIT_ASSERT(cookie->get(intVecKey) == readCookie->get(intVecKey));
    CPPUNIT_ASSERT(cookie->get(longVecKey) == readCookie->get(longVecKey));
    CPPUNIT_ASSERT(cookie->get(floatVecKey) == readCookie->get(floatVecKey));
    CPPUNIT_ASSERT(cookie->get(doubleVecKey) == readCookie->get(doubleVecKey));
    CPPUNIT_ASSERT(cookie->get(stringVecKey) == readCookie->get(stringVecKey));
    CPPUNIT_ASSERT(cookie->get(rgbVecKey) == readCookie->get(rgbVecKey));
    CPPUNIT_ASSERT(cookie->get(rgbaVecKey) == readCookie->get(rgbaVecKey));
    CPPUNIT_ASSERT(cookie->get(vec2fVecKey) == readCookie->get(vec2fVecKey));
    CPPUNIT_ASSERT(cookie->get(vec2dVecKey) == readCookie->get(vec2dVecKey));
    CPPUNIT_ASSERT(cookie->get(vec3fVecKey) == readCookie->get(vec3fVecKey));
    CPPUNIT_ASSERT(cookie->get(vec3dVecKey) == readCookie->get(vec3dVecKey));
    CPPUNIT_ASSERT(cookie->get(vec4fVecKey) == readCookie->get(vec4fVecKey));
    CPPUNIT_ASSERT(cookie->get(vec4dVecKey) == readCookie->get(vec4dVecKey));
    CPPUNIT_ASSERT(cookie->get(mat4fVecKey) == readCookie->get(mat4fVecKey));
    CPPUNIT_ASSERT(cookie->get(mat4dVecKey) == readCookie->get(mat4dVecKey));
    CPPUNIT_ASSERT(cookie->get(sceneObjectVecKey) == readCookie->get(sceneObjectVecKey)); // empty

    // Verify that metadata looks the same
    CPPUNIT_ASSERT(metadata->get(names)[0] == readMetadata->get(names)[0]);
    CPPUNIT_ASSERT(metadata->get(types)[0] == readMetadata->get(types)[0]);
    CPPUNIT_ASSERT(metadata->get(values)[0] == readMetadata->get(values)[0]);
}

void
TestAscii::testDeltaEncoding()
{
    // Create the context and load some classes.
    SceneContext context;
    const SceneClass* fakeTeapot = context.createSceneClass("FakeTeapot");
    AttributeKey<Float> fakenessKey = fakeTeapot->getAttributeKey<Float>("fakeness");

    // Create Metadata object
    SceneClass* sc2 = context.createSceneClass("Metadata");
    AttributeKey<StringVector> names = sc2->getAttributeKey<StringVector>("name");
    AttributeKey<StringVector> types = sc2->getAttributeKey<StringVector>("type");
    AttributeKey<StringVector> values = sc2->getAttributeKey<StringVector>("value");

    // New objects, even with no attributes set, should be encoded.
    SceneObject* teapot = context.createSceneObject("FakeTeapot", "/seq/shot/teapot");
    SceneObject* metadata = context.createSceneObject("Metadata", "/seq/shot/metadata");
    AsciiWriter writer1(context);
    writer1.setDeltaEncoding(true);
    writer1.toFile("delta1.rdla");
    SceneContext readContext1;
    AsciiReader reader1(readContext1);
    reader1.fromFile("delta1.rdla");
    CPPUNIT_ASSERT(readContext1.getSceneObject("/seq/shot/teapot"));
    CPPUNIT_ASSERT(readContext1.getSceneObject("/seq/shot/metadata"));

    context.commitAllChanges();

    // Delta encoding with no changes should result in no update.
    AsciiWriter writer2(context);
    writer2.setDeltaEncoding(true);
    writer2.toFile("delta2.rdla");
    SceneContext readContext2;
    AsciiReader reader2(readContext2);
    reader2.fromFile("delta2.rdla");
    CPPUNIT_ASSERT_THROW(
        readContext2.getSceneObject("/seq/shot/teapot");
    , except::KeyError);

    // Changing an attribute should result in an update.
    teapot->beginUpdate();
    teapot->set(fakenessKey, 99.99f);
    teapot->endUpdate();

    metadata->beginUpdate();
    metadata->set(names, {"blah", "foo"});
    metadata->set(types, {"int", "string"});
    metadata->set(values, {"2", "abcd"});
    metadata->endUpdate();

    AsciiWriter writer3(context);
    writer3.setDeltaEncoding(true);
    writer3.toFile("delta3.rdla");
    SceneContext readContext3;
    AsciiReader reader3(readContext3);
    reader3.fromFile("delta3.rdla");
    CPPUNIT_ASSERT_NO_THROW(
        CPPUNIT_ASSERT_DOUBLES_EQUAL(99.99f,
            readContext3.getSceneObject("/seq/shot/teapot")->get(fakenessKey), 0.0001f);
    );

    SceneObject* readMetadata = readContext3.getSceneObject("/seq/shot/metadata");
    CPPUNIT_ASSERT(metadata->get(names) == readMetadata->get(names));
    CPPUNIT_ASSERT(metadata->get(types) == readMetadata->get(types));
    CPPUNIT_ASSERT(metadata->get(values) == readMetadata->get(values));
}

void
TestAscii::testNullReferences()
{
    SceneContext context;
    const SceneClass* sceneClass = context.createSceneClass("ExtensiveObject");

    SceneObject* pizza = context.createSceneObject("ExtensiveObject", "/seq/shot/pizza");
    SceneObject* cookie = context.createSceneObject("ExtensiveObject", "/seq/shot/cookie");
    SceneObject* mango = context.createSceneObject("ExtensiveObject", "/seq/shot/mango");
    SceneObject* apple = context.createSceneObject("ExtensiveObject", "/seq/shot/apple");
    SceneObject* sharknado = context.createSceneObject("ExtensiveObject", "/seq/shot/sharknado");
    SceneObject* explosion = context.createSceneObject("ExtensiveObject", "/seq/shot/explosion");

    AttributeKey<SceneObject*> sceneObjectKey = sceneClass->getAttributeKey<SceneObject*>("scene object");
    AttributeKey<SceneObjectVector> sceneObjectVecKey = sceneClass->getAttributeKey<SceneObjectVector>("scene object vector");
    AttributeKey<String> stringKey = sceneClass->getAttributeKey<String>("string");

    // Set the attributes and a binding to null explicitly.
    pizza->beginUpdate();
    pizza->set(sceneObjectKey, nullptr);
    SceneObjectVector things;
    things.push_back(mango);
    things.push_back(nullptr);
    things.push_back(sharknado);
    pizza->set(sceneObjectVecKey, things);
    pizza->set(stringKey, String("not a pizza"));
    pizza->setBinding(stringKey, nullptr);
    pizza->endUpdate();

    // Sanity check
    CPPUNIT_ASSERT(pizza->get(sceneObjectKey) == nullptr);
    CPPUNIT_ASSERT(pizza->get(sceneObjectVecKey)[1] == nullptr);
    CPPUNIT_ASSERT(pizza->getBinding(stringKey) == nullptr);

    // Write out the Ascii file.
    AsciiWriter writer(context);
    writer.toFile("nullrefs.rdla");

    // Set the attributes back to something non-null.
    pizza->beginUpdate();
    pizza->set(sceneObjectKey, cookie);
    SceneObjectVector things2;
    things2.push_back(mango);
    things2.push_back(apple);
    things2.push_back(sharknado);
    pizza->set(sceneObjectVecKey, things2);
    pizza->setBinding(stringKey, explosion);
    pizza->endUpdate();

    // Sanity check
    CPPUNIT_ASSERT(pizza->get(sceneObjectKey) != nullptr);
    CPPUNIT_ASSERT(pizza->get(sceneObjectVecKey)[1] != nullptr);
    CPPUNIT_ASSERT(pizza->getBinding(stringKey) != nullptr);

    // Read the null data back in.
    AsciiReader reader(context);
    reader.fromFile("nullrefs.rdla");

    // Verify nulls and defaults
    CPPUNIT_ASSERT(pizza->get(sceneObjectKey) == nullptr);
    CPPUNIT_ASSERT(pizza->get(sceneObjectVecKey)[1] == nullptr);
    CPPUNIT_ASSERT(pizza->getBinding(stringKey) == nullptr);
    CPPUNIT_ASSERT(pizza->get(stringKey) == "not a pizza");
}

#ifdef _TEST_ASCII_DO_TEST_MEMORY

namespace {
    void meminfo() {
        pid_t pid = getpid();
        std::ostringstream oss;
        oss << "/proc/" << pid << "/status";
        std::ifstream file;
        file.open(oss.str(), std::ifstream::in);
        char buffer[256];
        while (file.good()) {
            file.getline(buffer, 256);
            if (strstr(buffer, "VmRSS:")) {
                std::cout << buffer << std::endl;
                break;
            }
        }
        file.close();
    }
}

void
TestAscii::testMemory()
{
    SceneContext context;
    const SceneClass* sceneClass = context.createSceneClass("ExtensiveObject");

    SceneObject* pizza = context.createSceneObject("ExtensiveObject", "/seq/shot/pizza");
    SceneObject* cookie = context.createSceneObject("ExtensiveObject", "/seq/shot/cookie");
    SceneObject* mango = context.createSceneObject("ExtensiveObject", "/seq/shot/mango");
    SceneObject* apple = context.createSceneObject("ExtensiveObject", "/seq/shot/apple");
    SceneObject* sharknado = context.createSceneObject("ExtensiveObject", "/seq/shot/sharknado");
    SceneObject* explosion = context.createSceneObject("ExtensiveObject", "/seq/shot/explosion");

    AttributeKey<SceneObject*> sceneObjectKey = sceneClass->getAttributeKey<SceneObject*>("scene object");
    AttributeKey<SceneObjectVector> sceneObjectVecKey = sceneClass->getAttributeKey<SceneObjectVector>("scene object vector");
    AttributeKey<String> stringKey = sceneClass->getAttributeKey<String>("string");

    // Set the attributes back to something non-null.
    pizza->beginUpdate();
    pizza->set(sceneObjectKey, cookie);
    SceneObjectVector things2;
    things2.push_back(mango);
    things2.push_back(apple);
    things2.push_back(sharknado);
    pizza->set(sceneObjectVecKey, things2);
    pizza->setBinding(stringKey, explosion);
    pizza->endUpdate();

    // Write out the Ascii file.
    AsciiWriter writer(context);
    writer.toFile("memory.rdla");

    for (int iter = 0; iter < 100000; ++iter) {
        // Read the null data back in.
        AsciiReader reader(context);
        reader.fromFile("memory.rdla");
        meminfo();
    }
}
#endif

void TestAscii::testAttributeAlias()
{
    // set attributes in rdla file using aliases
    static const char *rdlaCode =
        "ExtensiveObject(\"/seq/shot/pizza\") {\n"
        "    [\"bool vector\"] = { false, true, false},\n"
        "    [\"int vector\"] = { 1, 2, 3},\n"
        "}\n";

    SceneContext context;
    const SceneClass *sceneClass = context.createSceneClass("ExtensiveObject");
    AsciiReader reader(context);
    reader.fromString(rdlaCode);

    // lookup attributes by real name
    AttributeKey<BoolVector> boolVectorKey = sceneClass->getAttributeKey<BoolVector>("bool_vector");
    AttributeKey<IntVector> intVectorKey = sceneClass->getAttributeKey<IntVector>("int_vector");

    // ensure that the appropriate values were set
    const SceneObject *obj = context.getSceneObject("/seq/shot/pizza");
    const BoolVector &bv = obj->get(boolVectorKey);
    CPPUNIT_ASSERT(bv[0] == false);
    CPPUNIT_ASSERT(bv[1] == true);
    CPPUNIT_ASSERT(bv[2] == false);
    const IntVector &iv = obj->get(intVectorKey);
    CPPUNIT_ASSERT(iv[0] == 1);
    CPPUNIT_ASSERT(iv[1] == 2);
    CPPUNIT_ASSERT(iv[2] == 3);
}

void
TestAscii::testDenormals()
{
    // Test positive and negative denormals
    for (int sign = 0; sign <= 1; sign++)
    {
        // Start with the smallest single-precision denormal, using a double to represent it
        // so that arithmetic on it isn't affected by the relevant CPU flags (FTZ, DAZ)
        double denormal = sign ? -0x1.0p-149 : 0x1.0p-149;

        // ...and its corresponding bit pattern
        int bits = sign ? 0x80000001 : 1;

        // Outer and inner loops together will cover all denormals
        for (int iOuter = 0; iOuter < 32768; iOuter++)
        {
            SceneContext context;
            const SceneClass *sceneClass = context.createSceneClass("ExtensiveObject");
            AsciiReader reader(context);

            // Start the rdla string
            std::ostringstream rdlaStreamObj;
            rdlaStreamObj << "ExtensiveObject(\"/seq/shot/pizza\") {\n"
                             "    [\"float_vector\"] = { ";

            // We need at least 8 decimal digits of precision to capture the mantissa 23 bits of a denormal
            rdlaStreamObj << std::setprecision(8);

            // Put 256 consecutive denormals into the float_vector in the rdla string
            for (int iInner = 0; iInner < 256; iInner++)
            {
                rdlaStreamObj << denormal << ",";

                // Step denormal by 1 ULP
                denormal += sign ? -0x1.0p-149 : 0x1.0p-149;
            }

            // Finish the rdla string
            rdlaStreamObj << " },\n"
                             "}\n";

            // Read the string
            reader.fromString(rdlaStreamObj.str().c_str());

            // Lookup attribute by real name
            AttributeKey<FloatVector> floatVectorKey = sceneClass->getAttributeKey<FloatVector>("float_vector");

            // Check that the denormals are read back correctly
            const SceneObject *obj = context.getSceneObject("/seq/shot/pizza");
            const FloatVector &fv = obj->get(floatVectorKey);
            union { int32_t i; float f; } readBits;

            for (int iInner = 0; iInner < 256; iInner++)
            {
                readBits.f = fv[iInner];
                CPPUNIT_ASSERT(readBits.i == bits);

                // Step corresponding bit pattern
                bits++;
            }
        }
    }
}

} // namespace unittest
} // namespace rdl2
} // namespace scene_rdl2

