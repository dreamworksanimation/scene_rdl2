// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file TestCol3.h
#pragma once

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

namespace scene_rdl2 {
namespace common {
namespace math {
namespace ispc {
namespace unittest {

class TestCol3 : public CppUnit::TestFixture
{
public:
    void create();
    void clamp();
    void lerp();
    void isBlack();
    void minus();
    void mult();
    void plus();
    void sPostMult();
    void sPreMult();
    void isEqual();
    void isEqualFixedEps();
    void max();
    void rcp();
    void sqrt();
    void luminance();

    CPPUNIT_TEST_SUITE(TestCol3);
    CPPUNIT_TEST(create);
    CPPUNIT_TEST(clamp);
    CPPUNIT_TEST(lerp);
    CPPUNIT_TEST(isBlack);
    CPPUNIT_TEST(minus);
    CPPUNIT_TEST(mult);
    CPPUNIT_TEST(plus);
    CPPUNIT_TEST(sPostMult);
    CPPUNIT_TEST(sPreMult);
    CPPUNIT_TEST(isEqual);
    CPPUNIT_TEST(isEqualFixedEps);
    CPPUNIT_TEST(max);
    CPPUNIT_TEST(rcp);
    CPPUNIT_TEST(sqrt);
    CPPUNIT_TEST(luminance);
    CPPUNIT_TEST_SUITE_END();
};

} // namespace scene_rdl2
} // namespace common
} // namespace math
} // namespace ispc
} // namespace unittest

