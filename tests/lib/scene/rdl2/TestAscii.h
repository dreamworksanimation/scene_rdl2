// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once


// Uncomment out to do a memory test of the reader/writer loop
// #define _TEST_ASCII_DO_TEST_MEMORY

#include <scene_rdl2/scene/rdl2/rdl2.h>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

namespace scene_rdl2 {
namespace rdl2 {
namespace unittest {

class TestAscii : public CppUnit::TestFixture
{
public:
    void setUp();
    void tearDown();

    /// Test basic roundtrip functionality of the AsciiWriter and AsciiReader.
    void testRoundtrip();

    /// Test delta encoding for major data compression.
    void testDeltaEncoding();

    /// Test that we can serialize and deserialize null SceneObject references
    /// and bindings.
    void testNullReferences();

    /// Test that attribute aliases work
    void testAttributeAlias();

#ifdef _TEST_ASCII_DO_TEST_MEMORY
    /// Test to ensure that no memory leaks for the AsciiReader/Writer
    void testMemory();
#endif

    /// Test that denormal floats are correctly supported
    void testDenormals();

    CPPUNIT_TEST_SUITE(TestAscii);
    CPPUNIT_TEST(testRoundtrip);
    CPPUNIT_TEST(testDeltaEncoding);
    CPPUNIT_TEST(testNullReferences);
    CPPUNIT_TEST(testAttributeAlias);
#ifdef _TEST_ASCII_DO_TEST_MEMORY
    CPPUNIT_TEST(testMemory);
#endif
    //CPPUNIT_TEST(testDenormals);  // TODO: reinstate this test after making it quicker
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

