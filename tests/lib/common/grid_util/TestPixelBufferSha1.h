// Copyright 2023-2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <scene_rdl2/common/fb_util/FbTypes.h>
#include <scene_rdl2/common/grid_util/PixelBufferSha1Hash.h>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>

#include <random>

namespace scene_rdl2 {
namespace grid_util {
namespace unittest {

class TestRunParam
{
public:
    TestRunParam(unsigned idA, unsigned idB) : mTileIdA {idA} , mTileIdB {idB} {}

    unsigned mTileIdA {~static_cast<unsigned>(0)};
    unsigned mTileIdB {~static_cast<unsigned>(0)};
};

class TestPixelBufferSha1 : public CppUnit::TestFixture
{
public:
    void setUp() {}
    void tearDown() {}

    void testSingleRegion();
    void testDualRegion();

    CPPUNIT_TEST_SUITE(TestPixelBufferSha1);
    CPPUNIT_TEST(testSingleRegion);
    CPPUNIT_TEST(testDualRegion);
    CPPUNIT_TEST_SUITE_END();

protected:
    using PartialMergeTilesTbl = std::vector<char>;
    using TestRunParamTbl = std::vector<TestRunParam>;

    template <typename T> void singleRegionTestMain(int testMax);
    template <typename T> void singleRegionTestRun(const TestRunParam& currParam,
                                                   fb_util::PixelBuffer<T>& buff);
    template <typename T> void dualRegionTestMain(int testMax);
    template <typename T> void dualRegionTestRun(const TestRunParam& currParam,
                                                 fb_util::PixelBuffer<T>& buff);
    template <typename T>
    void execTestMainLoop(const std::string& title,
                          const TestRunParamTbl& paramTbl,
                          const std::function<void(const TestRunParam& param,
                                                   fb_util::PixelBuffer<T>& buff)>& runFunc);

    template <typename T> void setupBuff(fb_util::PixelBuffer<T>& buff);
    void setupMinMaxTileId(unsigned& tileIdMin, unsigned& tileIdMax);
    PartialMergeTilesTbl setupSingleRegion(unsigned tileIdStart, unsigned tileIdEnd);
    PartialMergeTilesTbl setupDualRegion(unsigned tileIdStart, unsigned tileIdEnd);
    void resetTileIdTbl(PartialMergeTilesTbl& tbl) const;
    void fillTileIdTbl(PartialMergeTilesTbl& tbl, unsigned startId, unsigned endId) const;

    //------------------------------
    //
    // verify test results
    //
    template <typename T>
    bool verifyPrimaryResultOnly(const PixelBufferSha1Hash& fbHash,
                                 bool verifyActive,
                                 PixelBufferSha1Hash::Hash& verifyHash,
                                 unsigned tileIdStart, unsigned tileIdEnd) const;
    template <typename T>
    bool verifyResult(const PixelBufferSha1Hash& fbHash,
                      bool verifyActiveA,
                      PixelBufferSha1Hash::Hash& verifyHashA,
                      unsigned tileIdStartA, unsigned tileIdEndA,
                      bool verifyActiveB,
                      PixelBufferSha1Hash::Hash& verifyHashB,
                      unsigned tileIdStartB, unsigned tileIdEndB) const;

    static std::string showVerifyInfo(const std::string& title,
                                      bool verifyActive,
                                      const PixelBufferSha1Hash::Hash& verifyHash,
                                      unsigned tileIdStart,
                                      unsigned tileIdEnd);

    //------------------------------
    //
    // random value related
    //
    void randomPixTest();

    // The actual functionality is implemented by a template specialization in TestPixelBufferSha1.cc.
    // This function body is never used.
    template <typename T> T randomPix() { return T(); }

    fb_util::ByteColor randByteColor()
    {
        fb_util::ByteColor b;
        b.r = rand0255();
        b.g = rand0255();
        b.b = rand0255();
        return b;
    }
    fb_util::ByteColor4 randByteColor4()
    {
        fb_util::ByteColor4 b4;
        b4.r = rand0255();
        b4.g = rand0255();
        b4.b = rand0255();
        b4.a = rand0255();
        return b4;
    }
    fb_util::PixelInfo randPixelInfo() { return fb_util::PixelInfo(randF01()); }
    math::Vec2f randV2() { return math::Vec2f(randF01(), randF01()); }
    math::Vec3f randV3() { return math::Vec3f(randF01(), randF01(), randF01()); }
    math::Vec4f randV4() { return math::Vec4f(randF01(), randF01(), randF01(), randF01()); }
    fb_util::RenderColor randRenderColor() { return randV4(); }
    float randF01() { return mFloat01(mMt); }
    int rand0255() { return mInt0255(mMt); }
    int64_t randInt64bit() { return mInt64bit(mMt); }
    unsigned randTileId() { return mRandTileId(mMt); }

    constexpr static unsigned mWidth {1918}; // non tile aligned size on purpose
    constexpr static unsigned mHeight {1078}; // non tile aligned size on purpose
    constexpr static unsigned mTileAlignedWidth {(mWidth + 7) & ~7};
    constexpr static unsigned mTileAlignedHeight {(mHeight + 7) & ~7};
    constexpr static unsigned mTileTotalX {mTileAlignedWidth / 8};
    constexpr static unsigned mTileTotalY {mTileAlignedHeight / 8};
    constexpr static unsigned mTileTotal {mTileTotalX * mTileTotalY};
    
    unsigned mSeed {std::random_device{}()};

    std::mt19937 mMt {mSeed};
    std::uniform_real_distribution<float> mFloat01 {0.0f, 1.0f};
    std::uniform_int_distribution<int> mInt0255 {0, 255};
    std::uniform_int_distribution<int64_t> mInt64bit {0, static_cast<int64_t>(0xffffffffffffffff)};
    std::uniform_int_distribution<unsigned> mRandTileId {0, mTileTotal - 1};
};

} // namespace unittest
} // namespace grid_util
} // namespace scene_rdl2
