// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <scene_rdl2/scene/rdl2/rdl2.h>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

namespace scene_rdl2 {
namespace rdl2 {
namespace unittest {

class TestTypes : public CppUnit::TestFixture
{
public:
    void setUp();
    void tearDown();

    void testConvertBoolFromString();
    void testConvertIntFromString();
    void testConvertLongFromString();
    void testConvertFloatFromString();
    void testConvertDoubleFromString();
    void testConvertStringFromString();
    void testConvertRgbFromString();
    void testConvertRgbaFromString();
    void testConvertVec2fFromString();
    void testConvertVec2dFromString();
    void testConvertVec3fFromString();
    void testConvertVec3dFromString();
    void testConvertVec4fFromString();
    void testConvertVec4dFromString();
    void testConvertMat4fFromString();
    void testConvertMat4dFromString();
    void testConvertSceneObjectFromString();
    void testConvertBoolVectorFromString();
    void testConvertIntVectorFromString();
    void testConvertLongVectorFromString();
    void testConvertFloatVectorFromString();
    void testConvertDoubleVectorFromString();
    void testConvertStringVectorFromString();
    void testConvertRgbVectorFromString();
    void testConvertRgbaVectorFromString();
    void testConvertVec2fVectorFromString();
    void testConvertVec2dVectorFromString();
    void testConvertVec3fVectorFromString();
    void testConvertVec3dVectorFromString();
    void testConvertVec4fVectorFromString();
    void testConvertVec4dVectorFromString();
    void testConvertMat4fVectorFromString();
    void testConvertMat4dVectorFromString();
    void testConvertSceneObjectVectorFromString();

    CPPUNIT_TEST_SUITE(TestTypes);
    CPPUNIT_TEST(testConvertBoolFromString);
    CPPUNIT_TEST(testConvertIntFromString);
    CPPUNIT_TEST(testConvertLongFromString);
    CPPUNIT_TEST(testConvertFloatFromString);
    CPPUNIT_TEST(testConvertDoubleFromString);
    CPPUNIT_TEST(testConvertStringFromString);
    CPPUNIT_TEST(testConvertRgbFromString);
    CPPUNIT_TEST(testConvertRgbaFromString);
    CPPUNIT_TEST(testConvertVec2fFromString);
    CPPUNIT_TEST(testConvertVec2dFromString);
    CPPUNIT_TEST(testConvertVec3fFromString);
    CPPUNIT_TEST(testConvertVec3dFromString);
    CPPUNIT_TEST(testConvertVec4fFromString);
    CPPUNIT_TEST(testConvertVec4dFromString);
    CPPUNIT_TEST(testConvertMat4fFromString);
    CPPUNIT_TEST(testConvertMat4dFromString);
    CPPUNIT_TEST(testConvertSceneObjectFromString);
    CPPUNIT_TEST(testConvertBoolVectorFromString);
    CPPUNIT_TEST(testConvertIntVectorFromString);
    CPPUNIT_TEST(testConvertLongVectorFromString);
    CPPUNIT_TEST(testConvertFloatVectorFromString);
    CPPUNIT_TEST(testConvertDoubleVectorFromString);
    CPPUNIT_TEST(testConvertStringVectorFromString);
    CPPUNIT_TEST(testConvertRgbVectorFromString);
    CPPUNIT_TEST(testConvertRgbaVectorFromString);
    CPPUNIT_TEST(testConvertVec2fVectorFromString);
    CPPUNIT_TEST(testConvertVec2dVectorFromString);
    CPPUNIT_TEST(testConvertVec3fVectorFromString);
    CPPUNIT_TEST(testConvertVec3dVectorFromString);
    CPPUNIT_TEST(testConvertVec4fVectorFromString);
    CPPUNIT_TEST(testConvertVec4dVectorFromString);
    CPPUNIT_TEST(testConvertMat4fVectorFromString);
    CPPUNIT_TEST(testConvertMat4dVectorFromString);
    CPPUNIT_TEST(testConvertSceneObjectVectorFromString);
    CPPUNIT_TEST_SUITE_END();
};

} // namespace unittest
} // namespace rdl2
} // namespace scene_rdl2


