// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cppunit/extensions/HelperMacros.h>

#include <scene_rdl2/common/math/Math.h>
#include <scene_rdl2/common/math/Vec3.h>

class TestCommonMathMat3 : public CppUnit::TestCase
{
public:
    CPPUNIT_TEST_SUITE(TestCommonMathMat3);
    CPPUNIT_TEST(benchmark);
    CPPUNIT_TEST(testConstruct);
    CPPUNIT_TEST(testCopy);
    CPPUNIT_TEST(testAccessor);
    CPPUNIT_TEST(testAdd);
    CPPUNIT_TEST(testSubtract);
    CPPUNIT_TEST(testMultiply);
    CPPUNIT_TEST(testDivide);
    CPPUNIT_TEST(testDet);
    CPPUNIT_TEST(testAdjoint);
    CPPUNIT_TEST(testInverse);
    CPPUNIT_TEST(testTransform);
    CPPUNIT_TEST(testScale);
    CPPUNIT_TEST(testRotate);
    CPPUNIT_TEST(testTranspose);
    CPPUNIT_TEST(testFrame);
    CPPUNIT_TEST(testQuaternion);
    CPPUNIT_TEST(testSlerp);
    CPPUNIT_TEST(testDecompose);
    CPPUNIT_TEST_SUITE_END();

    void benchmark();
    void testConstruct();
    void testCopy();
    void testAccessor();
    void testAdd();
    void testSubtract();
    void testMultiply();
    void testDivide();
    void testDet();
    void testAdjoint();
    void testInverse();
    void testTransform();
    void testScale();
    void testRotate();
    void testTranspose();
    void testFrame();
    void testQuaternion();
    void testSlerp();
    void testDecompose();
};


