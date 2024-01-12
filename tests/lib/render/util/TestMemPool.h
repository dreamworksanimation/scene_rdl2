// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#pragma once
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>

namespace scene_rdl2 {
namespace alloc {

class TestMemPool : public CppUnit::TestFixture
{
public:
    CPPUNIT_TEST_SUITE(TestMemPool);
    CPPUNIT_TEST(testMemBlocks);
    CPPUNIT_TEST(testThreadSafety);
    CPPUNIT_TEST_SUITE_END();

    void testMemBlocks();
    void testThreadSafety();
};

} // namespace pbr
} // namespace scene_rdl2


