// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file TestRenderOutput.h

#pragma once

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <memory>

namespace scene_rdl2 {
namespace rdl2 {

class SceneContext;

namespace unittest {

class TestRenderOutput: public CppUnit::TestFixture
{
public:
    void setUp() override;
    void tearDown() override;

    void testSetup();
    void testAscii();
    void testBinary();

    CPPUNIT_TEST_SUITE(TestRenderOutput);
    CPPUNIT_TEST(testSetup);
    CPPUNIT_TEST(testAscii);
    CPPUNIT_TEST(testBinary);
    CPPUNIT_TEST_SUITE_END();

private:
    void compare(SceneContext const &a, SceneContext const &b) const;

    std::unique_ptr<SceneContext> mContext;
};

} // namespace unittest
} // namespace rdl2
} // namespace scene_rdl2

