// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file TestAsA.h
#pragma once

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

namespace scene_rdl2 {
namespace common {
namespace math {
namespace ispc {
namespace unittest {

class TestAsA : public CppUnit::TestFixture
{
public:
    void asAVec2();
    void asAVec3();
    void asAVec4();
    void asACol3();
    void asAColor();
    void asACol4();
    void asArray();

    CPPUNIT_TEST_SUITE(TestAsA);
    CPPUNIT_TEST(asAVec2);
    CPPUNIT_TEST(asAVec3);
    CPPUNIT_TEST(asAVec4);
    CPPUNIT_TEST(asACol3);
    CPPUNIT_TEST(asAColor);
    CPPUNIT_TEST(asACol4);
    CPPUNIT_TEST(asArray);
    CPPUNIT_TEST_SUITE_END();
};

} // namespace scene_rdl2
} // namespace common
} // namespace math
} // namespace ispc
} // namespace unittest

