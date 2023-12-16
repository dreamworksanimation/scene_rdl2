// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>


namespace scene_rdl2 {
namespace common {
namespace math {
namespace ispc {
namespace unittest {


class TestTranscendental : public CppUnit::TestFixture
{
public:
    void testRcp();
    void testDwAcos();

    CPPUNIT_TEST_SUITE(TestTranscendental);
    CPPUNIT_TEST(testRcp);
    CPPUNIT_TEST(testDwAcos);
    CPPUNIT_TEST_SUITE_END();
};

} // namespace scene_rdl2
} // namespace common
} // namespace math
} // namespace ispc
} // namespace unittest

