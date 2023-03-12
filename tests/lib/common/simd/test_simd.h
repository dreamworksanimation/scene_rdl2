// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cppunit/extensions/HelperMacros.h>

class TestCommonSIMD: public CppUnit::TestCase
{
public:
    CPPUNIT_TEST_SUITE(TestCommonSIMD);
    CPPUNIT_TEST(testBasic);
    CPPUNIT_TEST(testOps);
    CPPUNIT_TEST(testAVXAtan);
    CPPUNIT_TEST(testAVXAtan2);
    CPPUNIT_TEST(testSSEAtan);
    CPPUNIT_TEST(testSSEAtan2);
    CPPUNIT_TEST_SUITE_END();

    void testBasic();
    void testOps();
    void testAVXAtan();
    void testAVXAtan2();
    void testSSEAtan();
    void testSSEAtan2();
};


