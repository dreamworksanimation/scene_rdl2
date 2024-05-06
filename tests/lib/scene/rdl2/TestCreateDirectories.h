// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

namespace scene_rdl2 {
namespace rdl2 {
namespace unittest {

class TestCreateDirectories : public CppUnit::TestFixture
{
public:
    void setUp();
    void tearDown();

    /// Test creating a path
    void testCreateDirectories();

    CPPUNIT_TEST_SUITE(TestCreateDirectories);
    CPPUNIT_TEST(testCreateDirectories);
    CPPUNIT_TEST_SUITE_END();
};

} // namespace unittest
} // namespace rdl2
} // namespace scene_rdl2

