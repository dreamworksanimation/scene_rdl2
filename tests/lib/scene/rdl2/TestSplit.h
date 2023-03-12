// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

// Test split mode writing a context to both rdla and rdlb

#include <scene_rdl2/scene/rdl2/rdl2.h>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

namespace scene_rdl2 {
namespace rdl2 {
namespace unittest {

class TestSplit : public CppUnit::TestFixture
{
public:
    void setUp();
    void tearDown();

    /// Test basic roundtrip functionality
    void testRoundtrip();

    CPPUNIT_TEST_SUITE(TestSplit);
    CPPUNIT_TEST(testRoundtrip);
    CPPUNIT_TEST_SUITE_END();

private:

    Vec3fVector mShortVec;
    Vec3fVector mLongVec;
};

} // namespace unittest
} // namespace rdl2
} // namespace scene_rdl2

