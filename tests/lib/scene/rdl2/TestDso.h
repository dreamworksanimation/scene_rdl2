// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <scene_rdl2/scene/rdl2/rdl2.h>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

namespace scene_rdl2 {
namespace rdl2 {
namespace unittest {

class TestDso : public CppUnit::TestFixture
{
public:
    void setUp();
    void tearDown();

    /// Test the file path getter.
    void testGetFilePath();

    /// Test the valid DSO checker.
    void testIsValidDso();

    /// Test that we can find DSOs correctly from a search path.
    void testFindDso();

    /// Test lazy loading of the DSO symbols.
    void testLazyLoading();

    /// Test that missing symbols throw an exception when loaded.
    void testMissingSymbols();

    CPPUNIT_TEST_SUITE(TestDso);
    CPPUNIT_TEST(testGetFilePath);
    CPPUNIT_TEST(testIsValidDso);
    CPPUNIT_TEST(testFindDso);
    CPPUNIT_TEST(testLazyLoading);
    CPPUNIT_TEST(testMissingSymbols);
    CPPUNIT_TEST_SUITE_END();
};

} // namespace unittest
} // namespace rdl2
} // namespace scene_rdl2

