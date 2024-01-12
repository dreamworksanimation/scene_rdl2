// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file TestQuaternion.h
#pragma once

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

namespace scene_rdl2 {
namespace common {
namespace math {
namespace ispc {
namespace unittest {

class TestQuaternion : public CppUnit::TestFixture
{
public:
    void testCreate();
    void testGetV();
    void testConjugate();
    void testScalarPrePlus();
    void testScalarPostPlus();
    void testPlus();
    void testScalarPreMinus();
    void testScalarPostMinus();
    void testMinus();
    void testScalarPreMult();
    void testScalarPostMult();
    void testMult();
    void testVecPostMult();
    void testTransform();
    void testScalarPreDiv();
    void testScalarPostDiv();
    void testDiv();
    void testIsEqual();
    void testIsEqualFixedEps();
    void testDot();
    void testNormalize();
    void testIsNormalized();
    void testRcp();
    void testSlerp();

    CPPUNIT_TEST_SUITE(TestQuaternion);
    CPPUNIT_TEST(testCreate);
    CPPUNIT_TEST(testGetV);
    CPPUNIT_TEST(testConjugate);
    CPPUNIT_TEST(testScalarPrePlus);
    CPPUNIT_TEST(testScalarPostPlus);
    CPPUNIT_TEST(testPlus);
    CPPUNIT_TEST(testScalarPreMinus);
    CPPUNIT_TEST(testScalarPostMinus);
    CPPUNIT_TEST(testMinus);
    CPPUNIT_TEST(testScalarPreMult);
    CPPUNIT_TEST(testScalarPostMult);
    CPPUNIT_TEST(testMult);
    CPPUNIT_TEST(testVecPostMult);
    CPPUNIT_TEST(testTransform);
    CPPUNIT_TEST(testScalarPreDiv);
    CPPUNIT_TEST(testScalarPostDiv);
    CPPUNIT_TEST(testDiv);
    CPPUNIT_TEST(testIsEqual);
    CPPUNIT_TEST(testIsEqualFixedEps);
    CPPUNIT_TEST(testDot);
    CPPUNIT_TEST(testNormalize);
    CPPUNIT_TEST(testIsNormalized);
    CPPUNIT_TEST(testRcp);
    CPPUNIT_TEST(testSlerp);
    CPPUNIT_TEST_SUITE_END();
};

} // namespace scene_rdl2
} // namespace common
} // namespace math
} // namespace ispc
} // namespace unittest

