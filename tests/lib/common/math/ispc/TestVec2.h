// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file TestVec2.h
#pragma once

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

namespace scene_rdl2 {
namespace common {
namespace math {
namespace ispc {
namespace unittest {

class TestVec2 : public CppUnit::TestFixture
{
public:
    void testCreate();
    void testPlus();
    void testMinus();
    void testScalarPreMult();
    void testScalarPostMult();
    void tetsMult();
    void testScalarPreDiv();
    void testScalarPostDiv();
    void testDiv();
    void testIsEqual();
    void testIsEqualFixedEps();
    void testIsZero();
    void testDot();
    void testLength();
    void testLengthSqr();
    void testNormalize();
    void testAbs();
    void testNeg();
    void testRcp();
    void testIsNormalized();
    void testLerp();

    CPPUNIT_TEST_SUITE(TestVec2);
    CPPUNIT_TEST(testCreate);
    CPPUNIT_TEST(testPlus);
    CPPUNIT_TEST(testMinus);
    CPPUNIT_TEST(testScalarPreMult);
    CPPUNIT_TEST(testScalarPostMult);
    CPPUNIT_TEST(tetsMult);
    CPPUNIT_TEST(testScalarPreDiv);
    CPPUNIT_TEST(testScalarPostDiv);
    CPPUNIT_TEST(testDiv);
    CPPUNIT_TEST(testIsEqual);
    CPPUNIT_TEST(testIsEqualFixedEps);
    CPPUNIT_TEST(testIsZero);
    CPPUNIT_TEST(testDot);
    CPPUNIT_TEST(testLength);
    CPPUNIT_TEST(testLengthSqr);
    CPPUNIT_TEST(testNormalize);
    CPPUNIT_TEST(testAbs);
    CPPUNIT_TEST(testNeg);
    CPPUNIT_TEST(testRcp);
    CPPUNIT_TEST(testIsNormalized);
    CPPUNIT_TEST(testLerp);
    CPPUNIT_TEST_SUITE_END();
};

} // namespace scene_rdl2
} // namespace common
} // namespace math
} // namespace ispc
} // namespace unittest

