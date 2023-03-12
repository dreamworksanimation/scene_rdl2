// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cppunit/extensions/HelperMacros.h>

#include <scene_rdl2/common/math/Color.h>

class TestCommonMathColor: public CppUnit::TestCase
{
public:
    CPPUNIT_TEST_SUITE(TestCommonMathColor);
    CPPUNIT_TEST(testCopy);
    CPPUNIT_TEST(testUnary);
    CPPUNIT_TEST(testBinary);
    CPPUNIT_TEST(testAssignment);
    CPPUNIT_TEST(testReductions);
    CPPUNIT_TEST(testComparisons);
    CPPUNIT_TEST(testSpecial);
    CPPUNIT_TEST_SUITE_END();
    
    void testCopy();
    void testUnary();
    void testBinary();
    void testAssignment();
    void testReductions();
    void testComparisons();
    void testSpecial();
};


