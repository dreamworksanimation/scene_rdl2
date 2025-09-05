// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file Test.h
#pragma once

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

namespace scene_rdl2 {
namespace common {
namespace math {
namespace ispc {
namespace unittest {

class Test : public CppUnit::TestFixture
{
public:
    void isEqual();
    void isEqualFixedEps();
    void isZero();
    void isOne();
    void isNormalizedLengthSqr();
    void isFinite();
    void isInf();
    void isNormal();
    void lerp();
    void deg2rad();
    void rad2deg();
    void bias();
    void gain();
    void trunc();
    void saturate();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(isEqual);
    CPPUNIT_TEST(isEqualFixedEps);
    CPPUNIT_TEST(isZero);
    CPPUNIT_TEST(isOne);
    CPPUNIT_TEST(isNormalizedLengthSqr);
    CPPUNIT_TEST(isFinite);
    CPPUNIT_TEST(isInf);
    CPPUNIT_TEST(isNormal);
    CPPUNIT_TEST(lerp);
    CPPUNIT_TEST(deg2rad);
    CPPUNIT_TEST(rad2deg);
    CPPUNIT_TEST(bias);
    CPPUNIT_TEST(gain);
    CPPUNIT_TEST(trunc);
    CPPUNIT_TEST(saturate);
    CPPUNIT_TEST_SUITE_END();
};

} // namespace scene_rdl2
} // namespace common
} // namespace math
} // namespace ispc
} // namespace unittest

