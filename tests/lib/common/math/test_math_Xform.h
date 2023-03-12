// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cppunit/extensions/HelperMacros.h>

#include <scene_rdl2/common/math/Math.h>

class TestCommonMathXform : public CppUnit::TestCase
{
public:
    CPPUNIT_TEST_SUITE(TestCommonMathXform);
    CPPUNIT_TEST(benchmark);
    CPPUNIT_TEST(testConstruct);
    CPPUNIT_TEST(testCopy);
    CPPUNIT_TEST(testAccessor);
    CPPUNIT_TEST(testAdd);
    CPPUNIT_TEST(testSubtract);
    CPPUNIT_TEST(testMultiply);
    CPPUNIT_TEST(testDivide);
    CPPUNIT_TEST(testInverse);
    CPPUNIT_TEST(testTransform);
    CPPUNIT_TEST(testScale);
    CPPUNIT_TEST(testRotate);
    CPPUNIT_TEST(testLerp);
    CPPUNIT_TEST(testDecompose);
    CPPUNIT_TEST(testXformComponent);
    CPPUNIT_TEST(testBBox);
    CPPUNIT_TEST(testBBoxRotation);
    CPPUNIT_TEST_SUITE_END();

    void benchmark();
    void testConstruct();
    void testCopy();
    void testAccessor();
    void testAdd();
    void testSubtract();
    void testMultiply();
    void testDivide();
    void testInverse();
    void testTransform();
    void testScale();
    void testRotate();
    void testLerp();
    void testDecompose();
    void testXformComponent();
    void testBBox();
    void testBBoxRotation();
};


