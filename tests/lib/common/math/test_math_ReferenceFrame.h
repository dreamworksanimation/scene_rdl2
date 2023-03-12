// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file test_math_ReferenceFrame.h

#pragma once

#include <scene_rdl2/common/math/ReferenceFrame.h>

#include <cppunit/extensions/HelperMacros.h>

class TestCommonMathReferenceFrame: public CppUnit::TestCase
{
public:
    CPPUNIT_TEST_SUITE(TestCommonMathReferenceFrame);
    CPPUNIT_TEST(testCtor);
    CPPUNIT_TEST(testGet);
    CPPUNIT_TEST(testXform);
    CPPUNIT_TEST_SUITE_END();

    void testCtor();
    void testGet();
    void testXform();
};

