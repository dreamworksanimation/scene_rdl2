// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file TestUserData.cc

#include "TestUserData.h"

#include <scene_rdl2/scene/rdl2/AsciiReader.h>
#include <scene_rdl2/scene/rdl2/AsciiWriter.h>
#include <scene_rdl2/scene/rdl2/BinaryReader.h>
#include <scene_rdl2/scene/rdl2/BinaryWriter.h>
#include <scene_rdl2/scene/rdl2/SceneContext.h>

namespace scene_rdl2 {
namespace rdl2 {
namespace unittest {

void
TestUserData::setUp()
{
    mContext.reset(new SceneContext);
    mUserDataName = "/testUserData";
    CPPUNIT_ASSERT(mContext);
    UserData *ud = mContext->createSceneObject("UserData",
        mUserDataName)->asA<UserData>();

    // check defaults
    CPPUNIT_ASSERT(!ud->hasBoolData());
    CPPUNIT_ASSERT(!ud->hasIntData());
    CPPUNIT_ASSERT(!ud->hasFloatData());
    CPPUNIT_ASSERT(!ud->hasStringData());
    CPPUNIT_ASSERT(!ud->hasColorData());
    CPPUNIT_ASSERT(!ud->hasVec2fData());
    CPPUNIT_ASSERT(!ud->hasVec3fData());
    CPPUNIT_ASSERT(!ud->hasMat4fData());

    // sets
    mBoolKey = "test_bool_var";
    mBoolValues = {true, false, true};
    mIntKey = "test_int_var";
    mIntValues = {1, 2, 3};
    mFloatKey = "test_float_var";
    mFloatValues = {4.0f, 5.0f, 6.0f, 7.0f};
    mStringKey = "test_string_var";
    mStringValues = {"foo", "bar"};
    mColorKey = "test_color_var";
    mColorValues = {math::Color(1, 0, 0), math::Color(0, 1, 0), math::Color(0, 0, 1)};
    mVec2fKey = "test_vec2f_var";
    mVec2fValues = {Vec2f(1, 3), Vec2f(5, 7)};
    mVec3fKey = "test_vec3f_var";
    mVec3fValues = {Vec3f{2, 4, 6}, Vec3f(8, 10, 12), Vec3f(14, 16, 18)};
    mMat4fKey = "test_mat4f_var";
    mMat4fValues = {
        Mat4f(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1),
        Mat4f(1, 3, 5, 7, 2, 4, 6, 8, 3, 5, 7, 9, 4, 6, 8, 0)};

    ud->beginUpdate();
    ud->setBoolData(mBoolKey, mBoolValues);
    ud->setIntData(mIntKey, mIntValues);
    ud->setFloatData(mFloatKey, mFloatValues);
    ud->setStringData(mStringKey, mStringValues);
    ud->setColorData(mColorKey, mColorValues);
    ud->setVec2fData(mVec2fKey, mVec2fValues);
    ud->setVec3fData(mVec3fKey, mVec3fValues);
    ud->setMat4fData(mMat4fKey, mMat4fValues);
    ud->endUpdate();
}

void
TestUserData::tearDown()
{
}

void
TestUserData::testSetup()
{
    UserData* ud = mContext->getSceneObject(mUserDataName)->asA<UserData>();
    CPPUNIT_ASSERT(ud->getBoolKey() == mBoolKey);
    const auto& bools = ud->getBoolValues();
    for (size_t i = 0; i < bools.size(); ++i) {
        CPPUNIT_ASSERT(bools[i] == mBoolValues[i]);
    }
    CPPUNIT_ASSERT(ud->getIntKey() == mIntKey);
    const auto& ints = ud->getIntValues();
    for (size_t i = 0; i < ints.size(); ++i) {
        CPPUNIT_ASSERT(ints[i] == mIntValues[i]);
    }
    CPPUNIT_ASSERT(ud->getFloatKey() == mFloatKey);
    const auto& floats = ud->getFloatValues();
    for (size_t i = 0; i < floats.size(); ++i) {
        CPPUNIT_ASSERT(floats[i] == mFloatValues[i]);
    }
    CPPUNIT_ASSERT(ud->getStringKey() == mStringKey);
    const auto& strings = ud->getStringValues();
    for (size_t i = 0; i < strings.size(); ++i) {
        CPPUNIT_ASSERT(strings[i] == mStringValues[i]);
    }
    CPPUNIT_ASSERT(ud->getColorKey() == mColorKey);
    const auto& colors = ud->getColorValues();
    for (size_t i = 0; i < colors.size(); ++i) {
        CPPUNIT_ASSERT(colors[i] == mColorValues[i]);
    }
    CPPUNIT_ASSERT(ud->getVec2fKey() == mVec2fKey);
    const auto& vec2fs = ud->getVec2fValues();
    for (size_t i = 0; i < vec2fs.size(); ++i) {
        CPPUNIT_ASSERT(vec2fs[i] == mVec2fValues[i]);
    }
    CPPUNIT_ASSERT(ud->getVec3fKey() == mVec3fKey);
    const auto& vec3fs = ud->getVec3fValues();
    for (size_t i = 0; i < vec3fs.size(); ++i) {
        CPPUNIT_ASSERT(vec3fs[i] == mVec3fValues[i]);
    }
    CPPUNIT_ASSERT(ud->getMat4fKey() == mMat4fKey);
    const auto& mat4fs = ud->getMat4fValues();
    for (size_t i = 0; i < mat4fs.size(); ++i) {
        CPPUNIT_ASSERT(mat4fs[i] == mMat4fValues[i]);
    }
}

void
TestUserData::testAscii()
{
    AsciiWriter writer(*mContext);
    writer.toFile("UserData.rdla");
    SceneContext reContext;
    AsciiReader reader(reContext);
    reader.fromFile("UserData.rdla");
    compare(*mContext, reContext);
}

void
TestUserData::testBinary()
{
    BinaryWriter writer(*mContext);
    writer.toFile("UserData.rdlb");
    SceneContext reContext;
    BinaryReader reader(reContext);
    reader.fromFile("UserData.rdlb");
    compare(*mContext, reContext);
}

void
TestUserData::compare(SceneContext const &a, SceneContext const &b) const
{
    const UserData* ud1 = a.getSceneObject(mUserDataName)->asA<UserData>();
    const UserData* ud2 = b.getSceneObject(mUserDataName)->asA<UserData>();

    CPPUNIT_ASSERT(ud1->getBoolKey() == ud2->getBoolKey());
    const auto& bools1= ud1->getBoolValues();
    const auto& bools2= ud2->getBoolValues();
    CPPUNIT_ASSERT(bools1.size() == bools2.size());
    for (size_t i = 0; i < bools1.size(); ++i) {
        CPPUNIT_ASSERT(bools1[i] == bools2[i]);
    }

    CPPUNIT_ASSERT(ud1->getIntKey() == ud2->getIntKey());
    const auto& ints1= ud1->getIntValues();
    const auto& ints2= ud2->getIntValues();
    CPPUNIT_ASSERT(ints1.size() == ints2.size());
    for (size_t i = 0; i < ints1.size(); ++i) {
        CPPUNIT_ASSERT(ints1[i] == ints2[i]);
    }

    CPPUNIT_ASSERT(ud1->getFloatKey() == ud2->getFloatKey());
    const auto& floats1= ud1->getFloatValues();
    const auto& floats2= ud2->getFloatValues();
    CPPUNIT_ASSERT(floats1.size() == floats2.size());
    for (size_t i = 0; i < floats1.size(); ++i) {
        CPPUNIT_ASSERT(floats1[i] == floats2[i]);
    }

    CPPUNIT_ASSERT(ud1->getStringKey() == ud2->getStringKey());
    const auto& strings1= ud1->getStringValues();
    const auto& strings2= ud2->getStringValues();
    CPPUNIT_ASSERT(strings1.size() == strings2.size());
    for (size_t i = 0; i < strings1.size(); ++i) {
        CPPUNIT_ASSERT(strings1[i] == strings2[i]);
    }

    CPPUNIT_ASSERT(ud1->getColorKey() == ud2->getColorKey());
    const auto& colors1= ud1->getColorValues();
    const auto& colors2= ud2->getColorValues();
    CPPUNIT_ASSERT(colors1.size() == colors2.size());
    for (size_t i = 0; i < colors1.size(); ++i) {
        CPPUNIT_ASSERT(colors1[i] == colors2[i]);
    }

    CPPUNIT_ASSERT(ud1->getVec2fKey() == ud2->getVec2fKey());
    const auto& vec2fs1= ud1->getVec2fValues();
    const auto& vec2fs2= ud2->getVec2fValues();
    CPPUNIT_ASSERT(vec2fs1.size() == vec2fs2.size());
    for (size_t i = 0; i < vec2fs1.size(); ++i) {
        CPPUNIT_ASSERT(vec2fs1[i] == vec2fs2[i]);
    }

    CPPUNIT_ASSERT(ud1->getVec3fKey() == ud2->getVec3fKey());
    const auto& vec3fs1= ud1->getVec3fValues();
    const auto& vec3fs2= ud2->getVec3fValues();
    CPPUNIT_ASSERT(vec3fs1.size() == vec3fs2.size());
    for (size_t i = 0; i < vec3fs1.size(); ++i) {
        CPPUNIT_ASSERT(vec3fs1[i] == vec3fs2[i]);
    }

    CPPUNIT_ASSERT(ud1->getMat4fKey() == ud2->getMat4fKey());
    const auto& mat4fs1= ud1->getMat4fValues();
    const auto& mat4fs2= ud2->getMat4fValues();
    CPPUNIT_ASSERT(mat4fs1.size() == mat4fs2.size());
    for (size_t i = 0; i < mat4fs1.size(); ++i) {
        CPPUNIT_ASSERT(mat4fs1[i] == mat4fs2[i]);
    }
}

} // namespace unittest
} // namespace rdl2
} // namespace scene_rdl2

