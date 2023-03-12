// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

namespace scene_rdl2 {
namespace fb_util {
namespace unittest {

class TestSnapshotUtil : public CppUnit::TestFixture
{
public:
    void setUp();
    void tearDown();

    void testHeatMapWeight();
    void testWeight();
    void testWeightMask();
    void testFloatWeight();
    void testFloatNumSample();
    void testFloat2Weight();
    void testFloat2NumSample();
    void testFloat3Weight();
    void testFloat3NumSample();
    void testFloat4Weight();
    void testFloat4NumSample();
    
    CPPUNIT_TEST_SUITE(TestSnapshotUtil);
    CPPUNIT_TEST(testHeatMapWeight);
    CPPUNIT_TEST(testWeight);
    CPPUNIT_TEST(testWeightMask);
    CPPUNIT_TEST(testFloatWeight);
    CPPUNIT_TEST(testFloatNumSample);
    CPPUNIT_TEST(testFloat2Weight);
    CPPUNIT_TEST(testFloat2NumSample);
    CPPUNIT_TEST(testFloat3Weight);
    CPPUNIT_TEST(testFloat3NumSample);
    CPPUNIT_TEST(testFloat4Weight);
    CPPUNIT_TEST(testFloat4NumSample);
    CPPUNIT_TEST_SUITE_END();

private:
    template <typename T> void setupBuff(std::vector<T> &buff) const; // setup test buffer by random number
    void setupNumBuff(std::vector<unsigned int> &nBuff) const; // setup test numBuffer by random number
    void setupPixMaskBuff(float emptyMaskFraction, float fullMaskFraction,
                          std::vector<uint64_t> &buff) const;
    void updateBuff(const int updateTotal,
                    const int w, // should be tile aligned resolution
                    const int h, // should be tile aligned resolution
                    std::function<void(const int offset)> updatePixelFunc,
                    std::vector<int> &updateList) const;
    void updateBuff(std::vector<uint64_t> &pixMaskBuff,
                    std::function<void(const int offset)> updatePixelFunc,
                    std::vector<int> &updateList) const;
    void snapshotTileLoop(const int w, // should be tile aligned resolution
                          const int h, // should be tile aligned resolution
                          std::vector<uint64_t> &pixMaskBuff,
                          std::function<uint64_t(int offsetItem)> snapshotTileFunc) const;
    void snapshotTimingCompare(const int w, // should be tile aligned resolution
                               const int h, // should be tile aligned resolution
                               const int timingTestLoopMax,
                               const bool doCompare,
                               std::function<void()> resetDataFunc,
                               std::function<uint64_t(int offsetItem)> snapshotTileFuncA,
                               std::function<uint64_t(int offsetItem)> snapshotTileFuncB,
                               std::function<bool(std::vector<uint64_t> &)> verifyFunc) const;
    bool verifyPixMask(std::vector<int> &updateList, std::vector<uint64_t> &pixMaskBuff) const;

    std::string showTile(int tileId, int offsetItem,
                         std::function<std::string(int offsetItem)> showItemFunction) const;
    std::string showBuff(int w, int h,
                         std::function<std::string(int offsetItem)> showItemFunction) const;
    std::string showBuffColWeight(int w, int h, const std::vector<float> &cBuff, const std::vector<float> &wBuff) const;
    std::string showPixMask(const uint64_t &pixMask) const;
    std::string showPixMaskBuff(const std::vector<uint64_t> &pixMaskBuff) const;
    std::string showUpdateList(const std::vector<int> &updateList) const;
};

} // namespace unittest
} // namespace fb_util
} // namespace scene_rdl2

