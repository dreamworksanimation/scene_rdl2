// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file TestMat4.h
#pragma once

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

namespace scene_rdl2 {
namespace common {
namespace math {
namespace ispc {
namespace unittest {

class TestMat4 : public CppUnit::TestFixture
{
public:
    void testCreate();
    void testAdd();
    void testMinus();
    void testScalarMultMat();
    void testMatMultScalar();
    void testVecMultMat();
    void testMatMultVec();
    void testMatMultMat();
    void testIsEqual();
    void testIsEqualFixedEps();
    void testDet();
    void testTranspose();
    void testAdjoint();
    void testInverse();
    void testSetToIdentity();
    void testSetToScale();
    void testSetToRotation();
    void testTransform();
    void testTransformNormal();
    void testTransformH();
    void testTransformPoint();

    CPPUNIT_TEST_SUITE(TestMat4);
    CPPUNIT_TEST(testCreate);
    CPPUNIT_TEST(testAdd);
    CPPUNIT_TEST(testMinus);
    CPPUNIT_TEST(testScalarMultMat);
    CPPUNIT_TEST(testMatMultScalar);
    CPPUNIT_TEST(testVecMultMat);
    CPPUNIT_TEST(testMatMultVec);
    CPPUNIT_TEST(testMatMultMat);
    CPPUNIT_TEST(testIsEqual);
    CPPUNIT_TEST(testIsEqualFixedEps);
    CPPUNIT_TEST(testDet);
    CPPUNIT_TEST(testTranspose);
    CPPUNIT_TEST(testAdjoint);
    CPPUNIT_TEST(testInverse);
    CPPUNIT_TEST(testSetToIdentity);
    CPPUNIT_TEST(testSetToScale);
    CPPUNIT_TEST(testSetToRotation);
    CPPUNIT_TEST(testTransform);
    CPPUNIT_TEST(testTransformNormal);
    CPPUNIT_TEST(testTransformH);
    CPPUNIT_TEST(testTransformPoint);
    CPPUNIT_TEST_SUITE_END();
};

} // namespace scene_rdl2
} // namespace common
} // namespace math
} // namespace ispc
} // namespace unittest

