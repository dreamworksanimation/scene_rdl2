// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <scene_rdl2/scene/rdl2/rdl2.h>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

namespace scene_rdl2 {
namespace rdl2 {
namespace unittest {

class TestSceneClass : public CppUnit::TestFixture
{
public:
    void setUp();
    void tearDown();

    /// Test the name getter.
    void testGetName();

    /// Test declaring a simple attribute for all attribute types.
    void testDeclareSimple();

    /// Test declaring a simple attribute with a default for all attribute types.
    void testDeclareSimpleWithDefault();

    /// Test declaring a bindable attribute for all attribute types.
    void testDeclareBindable();

    /// Test declaring a bindable attribute with a default for all attribute types.
    void testDeclareBindableWithDefault();

    /// Test declaring a blurrable attribute for all attribute types.
    void testDeclareBlurrable();

    /// Test declaring a blurrable attribute with a default for all attribute types.
    void testDeclareBlurrableWithDefault();

    /// Test getting an Attribute by its AttributeKey.
    void testGetAttributeByKey();

    /// Test getting an Attribute by its string name.
    void testGetAttributeByName();

    /// Test getting an AttributeKey by its string name.
    void testGetAttributeKeyByName();

    /// Test that attribute iteration works.
    void testIterateAttributes();

    /// Test that the memory layout of attribute values is correct.
    void testMemoryLayout();

    /// Test that sanity checks are in place for createObject() and destroyObject().
    void testCreateDestroyObject();

    /// Test that attribute storage is created correctly and that we can get
    /// and set values in it.
    void testAttributeStorage();

    CPPUNIT_TEST_SUITE(TestSceneClass);
    CPPUNIT_TEST(testGetName);
    CPPUNIT_TEST(testDeclareSimple);
    CPPUNIT_TEST(testDeclareSimpleWithDefault);
    CPPUNIT_TEST(testDeclareBindable);
    CPPUNIT_TEST(testDeclareBindableWithDefault);
    CPPUNIT_TEST(testDeclareBlurrable);
    CPPUNIT_TEST(testDeclareBlurrableWithDefault);
    CPPUNIT_TEST(testGetAttributeByKey);
    CPPUNIT_TEST(testGetAttributeByName);
    CPPUNIT_TEST(testGetAttributeKeyByName);
    CPPUNIT_TEST(testIterateAttributes);
    CPPUNIT_TEST(testMemoryLayout);
    CPPUNIT_TEST(testCreateDestroyObject);
    CPPUNIT_TEST(testAttributeStorage);
    CPPUNIT_TEST_SUITE_END();

private:
    SceneContext mContext;

    BoolVector mBoolVec;
    BoolVector mBoolVec2;
    IntVector mIntVec;
    IntVector mIntVec2;
    LongVector mLongVec;
    LongVector mLongVec2;
    FloatVector mFloatVec;
    FloatVector mFloatVec2;
    DoubleVector mDoubleVec;
    DoubleVector mDoubleVec2;
    StringVector mStringVec;
    StringVector mStringVec2;
    RgbVector mRgbVec;
    RgbVector mRgbVec2;
    RgbaVector mRgbaVec;
    RgbaVector mRgbaVec2;
    Vec2fVector mVec2fVec;
    Vec2fVector mVec2fVec2;
    Vec2dVector mVec2dVec;
    Vec2dVector mVec2dVec2;
    Vec3fVector mVec3fVec;
    Vec3fVector mVec3fVec2;
    Vec3dVector mVec3dVec;
    Vec3dVector mVec3dVec2;
    Vec4fVector mVec4fVec;
    Vec4fVector mVec4fVec2;
    Vec4dVector mVec4dVec;
    Vec4dVector mVec4dVec2;
    Mat4fVector mMat4fVec;
    Mat4fVector mMat4fVec2;
    Mat4dVector mMat4dVec;
    Mat4dVector mMat4dVec2;
    SceneObjectVector mSceneObjectVec;
    SceneObjectVector mSceneObjectVec2;
};

} // namespace unittest
} // namespace rdl2
} // namespace scene_rdl2

