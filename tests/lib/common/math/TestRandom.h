// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <scene_rdl2/common/math/Viewport.h>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class TestRandom : public CppUnit::TestCase
{
public:
    void setUp();
    void tearDown();

    void testUInt();
    void testBoundedUInt();
    void testFloat();
    void testDouble();

    CPPUNIT_TEST_SUITE(TestRandom);
    CPPUNIT_TEST(testUInt);
    CPPUNIT_TEST(testBoundedUInt);
    CPPUNIT_TEST(testFloat);
    CPPUNIT_TEST(testDouble);
    CPPUNIT_TEST_SUITE_END();
};

