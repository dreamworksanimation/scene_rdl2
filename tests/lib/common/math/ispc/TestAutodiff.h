// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file TestAutodiff.h
#pragma once

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

namespace scene_rdl2 {
namespace common {
namespace math {
namespace ispc {
namespace unittest {

class TestAutodiff : public CppUnit::TestFixture
{
public:
    void ctor();
    void comparison();
    void evaluation();
    void opOverloads();
    void algebra3();
    void acos();
    void asin();
    void atan();
    void atan2();
    void bias();
    void ceil();
    void cos();
    void gain();
    void exp();
    void floor();
    void fmod();
    void log();
    void matrix();
    void pow();
    void rcp();
    void rsqrt();
    void min();
    void max();
    void saturate();
    void sin();
    void sincos();
    void sqrt();
    void tan();
    void trunc();
    void col3();
    void vec2();
    void vec3();
    void vec4();

    CPPUNIT_TEST_SUITE(TestAutodiff);
    CPPUNIT_TEST(ctor);
    CPPUNIT_TEST(comparison);
    CPPUNIT_TEST(opOverloads);
    CPPUNIT_TEST(evaluation);
    CPPUNIT_TEST(algebra3);
    CPPUNIT_TEST(acos);
    CPPUNIT_TEST(asin);
    CPPUNIT_TEST(atan);
    CPPUNIT_TEST(atan2);
    CPPUNIT_TEST(bias);
    CPPUNIT_TEST(ceil);
    CPPUNIT_TEST(cos);
    CPPUNIT_TEST(gain);
    CPPUNIT_TEST(exp);
    CPPUNIT_TEST(floor);
    CPPUNIT_TEST(fmod);
    CPPUNIT_TEST(log);
    CPPUNIT_TEST(matrix);
    CPPUNIT_TEST(pow);
    CPPUNIT_TEST(rcp);
    CPPUNIT_TEST(rsqrt);
    CPPUNIT_TEST(min);
    CPPUNIT_TEST(max);
    CPPUNIT_TEST(saturate);
    CPPUNIT_TEST(sin);
    CPPUNIT_TEST(sincos);
    CPPUNIT_TEST(sqrt);
    CPPUNIT_TEST(tan);
    CPPUNIT_TEST(trunc);
    CPPUNIT_TEST(col3);
    CPPUNIT_TEST(vec2);
    CPPUNIT_TEST(vec3);
    CPPUNIT_TEST(vec4);
    CPPUNIT_TEST_SUITE_END();
};

} // namespace scene_rdl2
} // namespace common
} // namespace math
} // namespace ispc
} // namespace unittest

