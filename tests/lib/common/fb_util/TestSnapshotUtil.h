// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <random>

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
    using TestSnapshotTileFunc = std::function<uint64_t(uint32_t* dstVPtr, uint32_t* dstWPtr,
                                                        uint32_t* srcVPtr, uint32_t* srcWPtr)>;
    using TestSnapshotTileFunc2 = std::function<uint64_t(uint32_t* dstVPtr, uint32_t* dstNPtr, uint64_t dstPixMask,
                                                         uint32_t* srcVPtr, uint32_t* srcNPtr, uint64_t srcPixMask)>;

    void testFloatNWeight(const int pixDim,
                          const TestSnapshotTileFunc& snapshotTileFuncA,
                          const TestSnapshotTileFunc& snapshotTileFuncB);
    void testFloatNNumSample(const int pixDim,
                             const TestSnapshotTileFunc2& snapshotTileFuncA,
                             const TestSnapshotTileFunc2& snapshotTileFuncB);

    template <typename T> void setupBuffRandom(std::vector<T>& buff) const; // setup random val buffer
    template <typename T> void setupBuffZero(std::vector<T>& buff,
                                             const int pixDim, const float blackPixFraction) const;

    template <typename T> void setupRealBuff(std::vector<T>& buff,
                                             const int pixDim, const float zeroWeightPixFraction) const;
    void setupWeightBuff(std::vector<float>& buff, const float zeroWeightPixFraction) const;
    void setupNumBuff(std::vector<unsigned int>& buff, const float zeroWeightPixFraction) const;
    void setupPixMaskBuff(float emptyMaskFraction, float fullMaskFraction, std::vector<uint64_t> &buff) const;

    void updateBuff(const float updatePixFraction,
                    const int w, // should be tile aligned resolution
                    const int h, // should be tile aligned resolution
                    const std::function<bool(const int pixOffset)>& updatePixelFunc,
                    const std::function<void(const int pixOffset)>& updateTargetFunc,
                    std::vector<int>& updatePixIdArray) const;
    void updateBuff2(std::vector<uint64_t>& pixMaskBuff,
                     const std::function<bool(const int pixOffset)>& updatePixelFunc,
                     const std::function<void(const int pixOffset)>& updateTargetFunc,
                     std::vector<int>& updatePixIdArray) const;
    template <typename T>
    bool updatePix(std::vector<T>& pixBuff,
                   std::vector<float>& wieghtBuff,
                   const int pixOffset,
                   const int pixDim);
    template <typename T>
    bool updatePix2(std::vector<T>& pixBuff,
                    std::vector<unsigned int>& numBuff,
                    const int pixOffset,
                    const int pixDim);

    template <typename T>
    void copyPix(std::vector<T>& destBuff,
                 std::vector<float>& destWeight,
                 const std::vector<T>& srcBuff,
                 const std::vector<float>& srcWeight,
                 const int pixOffset,
                 const int pixDim) const;
    template <typename T>
    void copyPix2(std::vector<T>& destBuff,
                  std::vector<unsigned int>& destNumBuff,
                  const std::vector<T>& srcBuff,
                  const std::vector<unsigned int>& srcNumBuff,
                  const int pixOffset,
                  const int pixDim) const;

    template <typename T>
    std::string analyzeBuff(const int pixDim,
                            const std::vector<T>& orgV,
                            const std::vector<float>& orgW,
                            const std::vector<T>& srcV,
                            const std::vector<float>& srcW) const;
    template <typename T>
    std::string analyzeBuff2(const int pixDim,
                             const std::vector<T>& orgV,
                             const std::vector<unsigned int>& orgN,
                             const std::vector<T>& srcV,
                             const std::vector<unsigned int>& srcN) const;
    std::string analyzeWeightBuff(const std::vector<float>& orgW,
                                  const std::vector<float>& srcW) const;

    void snapshotTileLoop(const int w, // should be tile aligned resolution
                          const int h, // should be tile aligned resolution
                          std::vector<uint64_t> &pixMaskBuff,
                          std::function<uint64_t(int offsetItem)> snapshotTileFunc) const;
    void snapshotTimingCompare(const int w, // should be tile aligned resolution
                               const int h, // should be tile aligned resolution
                               std::function<void()> resetDataFunc,
                               const std::function<uint64_t(int offsetItem)>& snapshotTileFuncA,
                               const std::function<uint64_t(int offsetItem)>& snapshotTileFuncB,
                               const std::function<bool(std::vector<uint64_t>&)>& verifyFunc) const;
    bool verifyPixMask(std::vector<int> &updatePixIdArray, std::vector<uint64_t> &pixMaskBuff) const;

    std::string showTile(int tileId, int offsetItem,
                         std::function<std::string(int offsetItem)> showItemFunction) const;
    std::string showBuff(int w, int h,
                         std::function<std::string(int offsetItem)> showItemFunction) const;
    std::string showBuffColWeight(int w, int h, const std::vector<float> &cBuff, const std::vector<float> &wBuff) const;
    std::string showPixMask(const uint64_t &pixMask) const;
    std::string showPixMaskBuff(const std::vector<uint64_t> &pixMaskBuff) const;
    std::string showUpdatePixIdArray(const std::vector<int> &updatePixIdArray) const;

    float getRandReal01() { return mRandReal01(mMt19937); }
    float getNon0RandReal01() {
        for (int i = 0; i < 10; ++i) { // try to find non zero random value 10 times
            float v = getRandReal01();
            if (v != 0.0f) return v;
        }
        return 1.0f; // unlikely to be here
    }
    int getRandInt04096() { return mRandInt04096(mMt19937); }

    //------------------------------

    std::random_device mRnd;
    std::mt19937 mMt19937 { mRnd() };
    std::uniform_real_distribution<> mRandReal01 {0.0, 1.0};
    std::uniform_int_distribution<> mRandInt04096 {0, 4096};
};

} // namespace unittest
} // namespace fb_util
} // namespace scene_rdl2

