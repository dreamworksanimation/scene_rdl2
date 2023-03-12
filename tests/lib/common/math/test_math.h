// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cppunit/extensions/HelperMacros.h>


class TestCommonMath : public CppUnit::TestCase
{
public:
    CPPUNIT_TEST_SUITE(TestCommonMath);
    CPPUNIT_TEST(testBasic);
    CPPUNIT_TEST(testOps);
    CPPUNIT_TEST_SUITE_END();

    void testBasic();
    void testOps();
};


