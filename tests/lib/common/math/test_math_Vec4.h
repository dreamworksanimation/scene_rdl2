// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cppunit/extensions/HelperMacros.h>

#include <scene_rdl2/common/math/Vec4.h>

class TestCommonMathVec4: public CppUnit::TestCase
{
public:
    CPPUNIT_TEST_SUITE(TestCommonMathVec4);
    CPPUNIT_TEST(benchmark);
    CPPUNIT_TEST(testConstruct);
    CPPUNIT_TEST(testCopy);
    CPPUNIT_TEST(testAccessor);
    CPPUNIT_TEST(testAdd);
    CPPUNIT_TEST(testSubtract);
    CPPUNIT_TEST(testMultiply);
    CPPUNIT_TEST(testDivide);
    CPPUNIT_TEST(testInverse);
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
};


