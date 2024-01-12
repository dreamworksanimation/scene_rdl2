// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <scene_rdl2/scene/rdl2/rdl2.h>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

namespace scene_rdl2 {
namespace rdl2 {
namespace unittest {

class TestProxies : public CppUnit::TestFixture
{
public:
    void setUp();
    void tearDown();


    /// Test the creation of scenes with proxy Layer Materials
    void testProxyDwaBaseLayerable();

    /// Test the created of scenes with proxy Cameras.
    void testProxyCamera();

    /// Test the created of scenes with proxy DisplayFilters.
    void testProxyDisplayFilter();

    /// Test the creation of scenes with proxy EnvMaps.
    void testProxyEnvMap();

    /// Test the creation of scenes with proxy Geometries.
    void testProxyGeometry();

    /// Test the creation of scenes with proxy Lights.
    void testProxyLight();

    /// Test the creation of scenes with proxy LightFilters.
    void testProxyLightFilter();

    /// Test the creation of scenes with proxy Maps.
    void testProxyMap();

    /// Test the creation of scenes with proxy NormalMaps.
    void testProxyNormalMap();

    /// Test the creation of scenes with proxy Materials.
    void testProxyMaterial();

    CPPUNIT_TEST_SUITE(TestProxies);
    CPPUNIT_TEST(testProxyDwaBaseLayerable);
    CPPUNIT_TEST(testProxyCamera);
    CPPUNIT_TEST(testProxyDisplayFilter);
    CPPUNIT_TEST(testProxyEnvMap);
    CPPUNIT_TEST(testProxyGeometry);
    CPPUNIT_TEST(testProxyLight);
    CPPUNIT_TEST(testProxyLightFilter);
    CPPUNIT_TEST(testProxyMap);
    CPPUNIT_TEST(testProxyNormalMap);
    CPPUNIT_TEST(testProxyMaterial);
    CPPUNIT_TEST_SUITE_END();
};

} // namespace unittest
} // namespace rdl2
} // namespace scene_rdl2

