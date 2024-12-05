// Copyright 2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <scene_rdl2/common/grid_util/ShmFb.h>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>

#include <functional>

namespace scene_rdl2 {
namespace grid_util {
namespace unittest {

class TestShmFb : public CppUnit::TestFixture
{
public:
    void setUp() {}
    void tearDown() {}

    void testFbDataSize();
    void testFb();
    void testFbCtrlDataSize();
    void testFbCtrl();
    void testFbH16();
    void testFbOutput();
    
    CPPUNIT_TEST_SUITE(TestShmFb);
    CPPUNIT_TEST(testFbDataSize);
    CPPUNIT_TEST(testFb);
    CPPUNIT_TEST(testFbCtrlDataSize);
    CPPUNIT_TEST(testFbCtrl);
    CPPUNIT_TEST(testFbH16);
    CPPUNIT_TEST(testFbOutput);
    CPPUNIT_TEST_SUITE_END();

protected:

    bool testFbMain(unsigned width, unsigned height, unsigned chanTotal, ShmFb::ChanMode chanMode) const;
    bool verifyFb(const ShmFb& fb, unsigned width, unsigned height,
                  unsigned chanTotal, ShmFb::ChanMode chanMode) const;

    bool testFbCtrlMain() const;
    bool verifyFbCtrl(const ShmFbCtrl& fbCtrl, const unsigned shmId) const;

    bool testFbH16Main() const;
    bool testFbH16Single(const float f) const;

    bool testFbOutputMain() const;
    bool testFbOutputSingle(const unsigned width,
                            const unsigned height,
                            const unsigned inChanTotal,
                            const ShmFb::ChanMode inChanMode,
                            const bool inTop2BtmFlag,
                            const unsigned outChanTotal,
                            const ShmFb::ChanMode outChanMode,
                            const bool outTop2BtmFlag,
                            const bool expectedResult) const;
};

} // namespace unittest
} // namespace grid_util
} // namespace scene_rdl2
