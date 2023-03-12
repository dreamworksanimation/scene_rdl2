// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <scene_rdl2/scene/rdl2/rdl2.h>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <memory>

namespace scene_rdl2 {
namespace rdl2 {
namespace unittest {

class TestSceneObject : public CppUnit::TestFixture
{
public:
    void setUp();
    void tearDown();

    /// Test that we can get the SceneClass of a SceneObject.
    void testGetClass();

    /// Test that we can get the name of a SceneObject.
    void testGetName();

    /// Test that we can do attribute value gets and sets on specific timesteps.
    void testTimestepGetsAndSets();

    /// Test that we can do attribute value gets and sets without timesteps.
    void testSimpleGetsAndSets();

    // TODO: Revise this test. Needs to take camera shutter and motion steps
    // into account now.
    /// Test that we can do attribute value gets that are interpolated.
    //void testInterpolatedGet();

    /// Test that we can do attribute value gets and sets with the convenience
    /// getters and setters.
    void testConvenienceGetsAndSets();

    /// Test that we can reset individual attributes to their default value
    /// with AttributeKeys and string names.
    void testResetToDefault();

    /// Test that we can reset all attributes in an object to their default.
    void testResetAllToDefault();

    /// Test that the AttributeSetMask is updated correctly by set, resetToDefault
    /// and commitChanges
    void testAttributeSetMask();

    /// Test that we can get and set attribute bindings.
    void testBindings();
    
    /// Test that we can getOrCreate() Extensions with various arguments types.
    /// Mostly a compilation test.
    void testExtension();

    CPPUNIT_TEST_SUITE(TestSceneObject);
    CPPUNIT_TEST(testGetClass);
    CPPUNIT_TEST(testGetName);
    CPPUNIT_TEST(testTimestepGetsAndSets);
    CPPUNIT_TEST(testSimpleGetsAndSets);
    //CPPUNIT_TEST(testInterpolatedGet);
    CPPUNIT_TEST(testConvenienceGetsAndSets);
    CPPUNIT_TEST(testResetToDefault);
    CPPUNIT_TEST(testResetAllToDefault);
    CPPUNIT_TEST(testAttributeSetMask);
    CPPUNIT_TEST(testBindings);
    CPPUNIT_TEST(testExtension);
    CPPUNIT_TEST_SUITE_END();

private:
    std::unique_ptr<SceneClass> mDsoClass;

    AttributeKey<scene_rdl2::rdl2::Bool> mBoolKey;
    AttributeKey<scene_rdl2::rdl2::Int> mIntKey;
    AttributeKey<scene_rdl2::rdl2::Long> mLongKey;
    AttributeKey<scene_rdl2::rdl2::Float> mFloatKey;
    AttributeKey<scene_rdl2::rdl2::Double> mDoubleKey;
    AttributeKey<scene_rdl2::rdl2::String> mStringKey;
    AttributeKey<scene_rdl2::rdl2::Rgb> mRgbKey;
    AttributeKey<scene_rdl2::rdl2::Rgba> mRgbaKey;
    AttributeKey<scene_rdl2::rdl2::Vec2f> mVec2fKey;
    AttributeKey<scene_rdl2::rdl2::Vec2d> mVec2dKey;
    AttributeKey<scene_rdl2::rdl2::Vec3f> mVec3fKey;
    AttributeKey<scene_rdl2::rdl2::Vec3d> mVec3dKey;
    AttributeKey<scene_rdl2::rdl2::Vec4f> mVec4fKey;
    AttributeKey<scene_rdl2::rdl2::Vec4d> mVec4dKey;
    AttributeKey<scene_rdl2::rdl2::Mat4f> mMat4fKey;
    AttributeKey<scene_rdl2::rdl2::Mat4d> mMat4dKey;
    AttributeKey<scene_rdl2::rdl2::SceneObject*> mSceneObjectKey;
    AttributeKey<scene_rdl2::rdl2::BoolVector> mBoolVectorKey;
    AttributeKey<scene_rdl2::rdl2::IntVector> mIntVectorKey;
    AttributeKey<scene_rdl2::rdl2::LongVector> mLongVectorKey;
    AttributeKey<scene_rdl2::rdl2::FloatVector> mFloatVectorKey;
    AttributeKey<scene_rdl2::rdl2::DoubleVector> mDoubleVectorKey;
    AttributeKey<scene_rdl2::rdl2::StringVector> mStringVectorKey;
    AttributeKey<scene_rdl2::rdl2::RgbVector> mRgbVectorKey;
    AttributeKey<scene_rdl2::rdl2::RgbaVector> mRgbaVectorKey;
    AttributeKey<scene_rdl2::rdl2::Vec2fVector> mVec2fVectorKey;
    AttributeKey<scene_rdl2::rdl2::Vec2dVector> mVec2dVectorKey;
    AttributeKey<scene_rdl2::rdl2::Vec3fVector> mVec3fVectorKey;
    AttributeKey<scene_rdl2::rdl2::Vec3dVector> mVec3dVectorKey;
    AttributeKey<scene_rdl2::rdl2::Vec4fVector> mVec4fVectorKey;
    AttributeKey<scene_rdl2::rdl2::Vec4dVector> mVec4dVectorKey;
    AttributeKey<scene_rdl2::rdl2::Mat4fVector> mMat4fVectorKey;
    AttributeKey<scene_rdl2::rdl2::Mat4dVector> mMat4dVectorKey;
    AttributeKey<scene_rdl2::rdl2::SceneObjectVector> mSceneObjectVectorKey;

    AttributeKey<scene_rdl2::rdl2::Float> mBindableKey;

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

