// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

namespace scene_rdl2 {
namespace rdl2 {
namespace unittest {

class TestDsoFinder : public CppUnit::TestFixture
{
public:
    void setUp() {}
    void tearDown() {}

    void testGuessDsoPath();

    CPPUNIT_TEST_SUITE(TestDsoFinder);
    CPPUNIT_TEST(testGuessDsoPath);
    CPPUNIT_TEST_SUITE_END();
};

} // namespace unittest
} // namespace rdl2
} // namespace scene_rdl2

