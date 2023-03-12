// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file TestRandom.h
#pragma once

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

namespace scene_rdl2 {
namespace common {
namespace math {
namespace ispc {
namespace unittest {

class TestRandom : public CppUnit::TestFixture
{
public:
    void testSequence();

    CPPUNIT_TEST_SUITE(TestRandom);
    CPPUNIT_TEST(testSequence);
    CPPUNIT_TEST_SUITE_END();
};

} // namespace unittest
} // namespace ispc
} // namespace math
} // namespace common
} // namespace scene_rdl2

