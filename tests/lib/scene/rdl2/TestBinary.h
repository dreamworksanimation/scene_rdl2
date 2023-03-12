// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <scene_rdl2/scene/rdl2/rdl2.h>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

namespace scene_rdl2 {
namespace rdl2 {
namespace unittest {

class TestBinary : public CppUnit::TestFixture
{
public:
    void setUp();
    void tearDown();

    /// Test basic roundtrip functionality of the BinaryWriter and BinaryReader.
    void testRoundtrip();

    /// Test transient encoding for minor data compression and decoding performance.
    void testTransientEncoding();

    /// Test delta encoding for major data compression.
    void testDeltaEncoding();

    /// Test that we can serialize and deserialize null SceneObject references
    /// and bindings.
    void testNullReferences();

    CPPUNIT_TEST_SUITE(TestBinary);
    CPPUNIT_TEST(testRoundtrip);
    CPPUNIT_TEST(testTransientEncoding);
    CPPUNIT_TEST(testDeltaEncoding);
    CPPUNIT_TEST(testNullReferences);
    CPPUNIT_TEST_SUITE_END();

private:
    BoolVector mBoolVec2;
    IntVector mIntVec2;
    LongVector mLongVec2;
    FloatVector mFloatVec2;
    DoubleVector mDoubleVec2;
    StringVector mStringVec2;
    RgbVector mRgbVec2;
    RgbaVector mRgbaVec2;
    Vec2fVector mVec2fVec2;
    Vec2dVector mVec2dVec2;
    Vec3fVector mVec3fVec2;
    Vec3dVector mVec3dVec2;
    Vec4fVector mVec4fVec2;
    Vec4dVector mVec4dVec2;
    Mat4fVector mMat4fVec2;
    Mat4dVector mMat4dVec2;
};

} // namespace unittest
} // namespace rdl2
} // namespace scene_rdl2

