// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <scene_rdl2/scene/rdl2/rdl2.h>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

namespace scene_rdl2 {
namespace rdl2 {
namespace unittest {

class TestSceneContext : public CppUnit::TestFixture
{
public:
    void setUp();
    void tearDown();

    /// Test that we can set and get the DSO path.
    void testDsoPath();

    /// Test that we can create a new SceneClass and that creating it again is
    /// not a problem.
    void testCreateSceneClass();

    /// Test that we can get a SceneClass by name.
    void testGetSceneClass();

    /// Test that we can find if a SceneClass exists yet.
    void testSceneClassExists();

    /// Test that we can iterate over the SceneClasses.
    void testIterateSceneClasses();

    /// Test that we can create a new SceneObject, and that creating the same
    /// object again will return the existing object.
    void testCreateSceneObject();

    /// Test that we can get a SceneObject by name.
    void testGetSceneObject();

    /// Test that we can find if a SceneObject exists yet.
    void testSceneObjectExists();

    /// Test if we can iterate over the SceneObjects.
    void testIterateSceneObjects();

    /// Test if we can set attributes on a SceneObject and see their effect.
    void testSetSceneObject();

    /// Test that we can load all SceneClasses in our DSO path up front.
    void testLoadAllSceneClasses();

    /// Test that we can get and set the SceneVariables.
    void testSceneVariables();

    /// Test that we don't insert bad SceneClasses into the context when their
    /// creation fails.
    void testCreateClassFailure();

    /// Test that we don't insert bad SceneObjects into the context when their
    /// creation fails.
    void testCreateObjectFailure();

    CPPUNIT_TEST_SUITE(TestSceneContext);
    CPPUNIT_TEST(testDsoPath);
    CPPUNIT_TEST(testCreateSceneClass);
    CPPUNIT_TEST(testGetSceneClass);
    CPPUNIT_TEST(testSceneClassExists);
    CPPUNIT_TEST(testIterateSceneClasses);
    CPPUNIT_TEST(testCreateSceneObject);
    CPPUNIT_TEST(testGetSceneObject);
    CPPUNIT_TEST(testSceneObjectExists);
    CPPUNIT_TEST(testIterateSceneObjects);
    CPPUNIT_TEST(testSetSceneObject);
    CPPUNIT_TEST(testLoadAllSceneClasses);
    CPPUNIT_TEST(testSceneVariables);
    CPPUNIT_TEST(testCreateClassFailure);
    CPPUNIT_TEST(testCreateObjectFailure);
    CPPUNIT_TEST_SUITE_END();
};

} // namespace unittest
} // namespace rdl2
} // namespace scene_rdl2

