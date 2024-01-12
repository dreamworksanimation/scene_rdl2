// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cppunit/extensions/HelperMacros.h>


class TestCommonMathTranscendental : public CppUnit::TestCase
{
public:
    CPPUNIT_TEST_SUITE(TestCommonMathTranscendental);
    CPPUNIT_TEST(testRcp);
    CPPUNIT_TEST(testAcos);
    CPPUNIT_TEST_SUITE_END();

    void testRcp();
    void testAcos();
};


