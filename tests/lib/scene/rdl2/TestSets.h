// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <scene_rdl2/scene/rdl2/rdl2.h>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <memory>

namespace scene_rdl2 {
namespace rdl2 {
namespace unittest {

class TestSets : public CppUnit::TestFixture
{
public:
    void setUp();
    void tearDown();

    /// Test that adding Geometries to the GeometrySet works properly.
    void testAddGeometry();

    /// Test that removing Geometries from the GeometrySet works properly.
    void testRemoveGeometry();

    /// Test that the GeometrySet clear() method works properly.
    void testClearGeometry();

    /// Test that the GeometrySet isStatic() method works properly.
    void testStaticGeometry();

    /// Test that adding Lights to the LightSet works properly.
    void testAddLight();

    /// Test that removing Lights from the LightSet works properly.
    void testRemoveLight();

    /// Test that the LightSet clear() method works properly.
    void testClearLight();

    CPPUNIT_TEST_SUITE(TestSets);
    CPPUNIT_TEST(testAddGeometry);
    CPPUNIT_TEST(testRemoveGeometry);
    CPPUNIT_TEST(testClearGeometry);
    CPPUNIT_TEST(testStaticGeometry);
    CPPUNIT_TEST(testAddLight);
    CPPUNIT_TEST(testRemoveLight);
    CPPUNIT_TEST(testClearLight);
    CPPUNIT_TEST_SUITE_END();

private:
    std::unique_ptr<SceneContext> mContext;
};

} // namespace unittest
} // namespace rdl2
} // namespace scene_rdl2

