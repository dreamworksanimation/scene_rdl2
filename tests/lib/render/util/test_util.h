// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

// Include this before any other includes!
#include <scene_rdl2/common/platform/Platform.h>

#include <cppunit/extensions/HelperMacros.h>

class TestCommonUtil: public CppUnit::TestCase
{
public:
    CPPUNIT_TEST_SUITE(TestCommonUtil);
    CPPUNIT_TEST(testCtorAlloc);
    CPPUNIT_TEST(testAlloc);
    CPPUNIT_TEST(testArenaAllocator);
    CPPUNIT_TEST(testAlignedAllocator);
    CPPUNIT_TEST(testRoundDownToPowerOfTwo);
    CPPUNIT_TEST(testIndexableArray);
    CPPUNIT_TEST(testIntegerSequence);
    CPPUNIT_TEST(testSManip);
    CPPUNIT_TEST(testGUID);
    CPPUNIT_TEST(testGetEnv);
    CPPUNIT_TEST_SUITE_END();

    void testCtorAlloc();
    void testAlloc();
    void testArenaAllocator();
    void testAlignedAllocator();
    void testRoundDownToPowerOfTwo();
    void testIndexableArray();
    void testIntegerSequence();
    void testSManip();
    void testGUID();
    void testGetEnv();
};


