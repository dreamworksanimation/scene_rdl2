// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>

namespace scene_rdl2 {
namespace grid_util {
namespace unittest {

class TestBinPacketDictionary : public CppUnit::TestFixture
{
public:
    void setUp() {}
    void tearDown() {}

    void testSimpleData();

    CPPUNIT_TEST_SUITE(TestBinPacketDictionary);
    CPPUNIT_TEST(testSimpleData);
    CPPUNIT_TEST_SUITE_END();
};

} // namespace unittest
} // namespace grid_util
} // namespace scene_rdl2
