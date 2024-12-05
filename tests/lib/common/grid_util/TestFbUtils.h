// Copyright 2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <scene_rdl2/common/fb_util/VariablePixelBuffer.h>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>

namespace scene_rdl2 {
namespace grid_util {
namespace unittest {

class TestFbUtils : public CppUnit::TestFixture
{
public:
    void setUp() {}
    void tearDown() {}

    void testUntileSinglePixelLoop();

    CPPUNIT_TEST_SUITE(TestFbUtils);
    CPPUNIT_TEST(testUntileSinglePixelLoop);
    CPPUNIT_TEST_SUITE_END();

private:

    bool testUntileSinglePixelLoopMain() const;
    bool runTestUntileSinglePixel(const unsigned width, const unsigned height, const bool top2Btm,
                                  const bool roiFlag,
                                  const unsigned minX, const unsigned minY,
                                  const unsigned maxX, const unsigned maxY) const;

    fb_util::VariablePixelBuffer setupDummyTiledBuffer(const unsigned width, const unsigned height,
                                                       const bool roiFlag,
                                                       const unsigned minX, const unsigned minY,
                                                       const unsigned maxX, const unsigned maxY) const;
    bool verifyUntileSinglePixel(const fb_util::VariablePixelBuffer& buffTiled,
                                 const bool top2Btm,
                                 const bool roiFlag,
                                 const unsigned minX, const unsigned minY,
                                 const unsigned maxX, const unsigned maxY) const;
};

} // namespace unittest
} // namespace grid_util
} // namespace scene_rdl2
