// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <scene_rdl2/scene/rdl2/rdl2.h>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <memory>

namespace scene_rdl2 {
namespace rdl2 {
namespace unittest {

class TestLayer : public CppUnit::TestFixture
{
public:
    void setUp();
    void tearDown();

    /// Test that we can make assignments and look them up.
    void testAssignAndLookup();

    /// Test that we can make and lookup default part assignments.
    void testDefaultAssignments();

    /// Test that the Layer clear() method works properly.
    void testClearLayer();

    /// Test that the Layer iterators work as expected.
    void testIterators();

    /// Test that looking up GeometrySets for a layer works as expected.
    void testContextLookup();

    void testSerialize();

    CPPUNIT_TEST_SUITE(TestLayer);
    CPPUNIT_TEST(testAssignAndLookup);
    CPPUNIT_TEST(testDefaultAssignments);
    CPPUNIT_TEST(testClearLayer);
    CPPUNIT_TEST(testIterators);
    CPPUNIT_TEST(testContextLookup);
    CPPUNIT_TEST(testSerialize);
    CPPUNIT_TEST_SUITE_END();

private:
    std::unique_ptr<SceneContext> mContext;
};

} // namespace unittest
} // namespace rdl2
} // namespace scene_rdl2

