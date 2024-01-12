// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file TestXform.h
#pragma once

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

namespace scene_rdl2 {
namespace common {
namespace math {
namespace ispc {
namespace unittest {

class TestXform : public CppUnit::TestFixture
{
public:
    void testCreate();
    void testInverse();
    void testRow();
    void testIdentity();
    void testTranslation();
    void testRotation();
    void testScale();
    void testLookAtPoint();
    void testAdd();
    void testMinus();
    void testSMultXform();
    void testXformMultXform();
    void testTransformPoint();
    void testTransformVector();
    void testTransformNormal();
    void testIsEqual();
    void testIsEqualFixedEps();
    void testSlerp();

    CPPUNIT_TEST_SUITE(TestXform);
    CPPUNIT_TEST(testCreate);
    CPPUNIT_TEST(testInverse);
    CPPUNIT_TEST(testRow);
    CPPUNIT_TEST(testIdentity);
    CPPUNIT_TEST(testTranslation);
    CPPUNIT_TEST(testRotation);
    CPPUNIT_TEST(testScale);
    CPPUNIT_TEST(testLookAtPoint);
    CPPUNIT_TEST(testAdd);
    CPPUNIT_TEST(testMinus);
    CPPUNIT_TEST(testSMultXform);
    CPPUNIT_TEST(testXformMultXform);
    CPPUNIT_TEST(testTransformPoint);
    CPPUNIT_TEST(testTransformVector);
    CPPUNIT_TEST(testTransformNormal);
    CPPUNIT_TEST(testIsEqual);
    CPPUNIT_TEST(testIsEqualFixedEps);
    CPPUNIT_TEST(testSlerp);
    CPPUNIT_TEST_SUITE_END();
};

} // namespace scene_rdl2
} // namespace common
} // namespace math
} // namespace ispc
} // namespace unittest

