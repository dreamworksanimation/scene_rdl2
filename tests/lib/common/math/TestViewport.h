// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <scene_rdl2/common/math/Viewport.h>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class TestViewport : public CppUnit::TestCase
{
public:
    void setUp();
    void tearDown();

    void testDefaultCtor();
    void testPiecewiseCtor();
    void testVectorCtor();
    void testRegionCtor();

    void testEqual();
    void testNotEqual();

    void testMin();
    void testMax();
    void testWidth();
    void testHeight();

    void testContains();

    CPPUNIT_TEST_SUITE(TestViewport);
    CPPUNIT_TEST(testDefaultCtor);
    CPPUNIT_TEST(testPiecewiseCtor);
    CPPUNIT_TEST(testVectorCtor);
    CPPUNIT_TEST(testRegionCtor);
    CPPUNIT_TEST(testEqual);
    CPPUNIT_TEST(testNotEqual);
    CPPUNIT_TEST(testMin);
    CPPUNIT_TEST(testMax);
    CPPUNIT_TEST(testWidth);
    CPPUNIT_TEST(testHeight);
    CPPUNIT_TEST(testContains);
    CPPUNIT_TEST_SUITE_END();
};

