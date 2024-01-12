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

class TestTraceSet : public CppUnit::TestFixture
{
public:
    void setUp();
    void tearDown();

    void testSerialize();

    CPPUNIT_TEST_SUITE(TestTraceSet);
    CPPUNIT_TEST(testSerialize);
    CPPUNIT_TEST_SUITE_END();

private:
    std::unique_ptr<SceneContext> mContext;
};

} // namespace unittest
} // namespace rdl2
} // namespace scene_rdl2

