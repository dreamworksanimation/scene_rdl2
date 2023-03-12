// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file TestXformv.h
#pragma once

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

namespace scene_rdl2 {
namespace common {
namespace math {
namespace ispc {
namespace unittest {

class TestXformv : public CppUnit::TestFixture
{
public:
    void testCreate();
    void testInverse();
    void testTransformPoint();
    void testTransformVector();
    void testTransformNormal();
    void testXformMultXform();
    void testSelect();

    CPPUNIT_TEST_SUITE(TestXformv);
    CPPUNIT_TEST(testCreate);
    CPPUNIT_TEST(testInverse);
    CPPUNIT_TEST(testTransformPoint);
    CPPUNIT_TEST(testTransformVector);
    CPPUNIT_TEST(testXformMultXform);
    CPPUNIT_TEST(testSelect);
    CPPUNIT_TEST_SUITE_END();
};

} // namespace scene_rdl2
} // namespace common
} // namespace math
} // namespace ispc
} // namespace unittest

