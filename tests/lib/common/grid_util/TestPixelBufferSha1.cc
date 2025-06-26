// Copyright 2023-2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "TestPixelBufferSha1.h"
#include "TimeOutput.h"

#include <scene_rdl2/render/util/StrUtil.h>

#include <algorithm>
#include <tbb/parallel_for.h>

// This directive disable multi-thread execution for debugging purposes.
// This should be disabled and always use MT version for release.
//#define SINGLE_THREAD

//#define DEBUG_MSG

namespace scene_rdl2 {
namespace grid_util {
namespace unittest {

//
// Template specialization for TestPixelBufferSha1::randomPix<T>()
//
template <> fb_util::ByteColor TestPixelBufferSha1::randomPix<fb_util::ByteColor>() { return randByteColor(); }
template <> fb_util::ByteColor4 TestPixelBufferSha1::randomPix<fb_util::ByteColor4>() { return randByteColor4(); }
template <> int64_t TestPixelBufferSha1::randomPix<int64_t>() { return randInt64bit(); }
template <> fb_util::PixelInfo TestPixelBufferSha1::randomPix<fb_util::PixelInfo>() { return randPixelInfo(); }
template <> math::Vec2f TestPixelBufferSha1::randomPix<math::Vec2f>() { return randV2(); }
template <> math::Vec3f TestPixelBufferSha1::randomPix<math::Vec3f>() { return randV3(); }
template <> math::Vec4f TestPixelBufferSha1::randomPix<math::Vec4f>() { return randV4(); }
// We don't need RenderColor because RenderColor is equivalent with Vec4f
//template <> fb_util::RenderColor TestPixelBufferSha1::randomPix<fb_util::RenderColor>() { return randV4(); }

//------------------------------------------------------------------------------------------

void
TestPixelBufferSha1::testSingleRegion()
{
    TIME_START;

    // randomPixTest(); // useful debug message

#   ifdef SINGLE_THREAD
    constexpr int testTotal = 8;
#   else // else SINGLE_THREAD
    constexpr int testTotal = 256;
#   endif // end else SINGLE_THREAD

    singleRegionTestMain<fb_util::ByteColor>(testTotal);
    singleRegionTestMain<fb_util::ByteColor4>(testTotal);
    singleRegionTestMain<int64_t>(testTotal);
    singleRegionTestMain<fb_util::PixelInfo>(testTotal);
    singleRegionTestMain<math::Vec2f>(testTotal);
    singleRegionTestMain<math::Vec3f>(testTotal);
    singleRegionTestMain<fb_util::RenderColor>(testTotal);

    TIME_END;
}

void
TestPixelBufferSha1::testDualRegion()
{
    TIME_START;

#   ifdef SINGLE_THREAD
    constexpr int testTotal = 8;
#   else // else SINGLE_THREAD
    constexpr int testTotal = 256;
#   endif // end else SINGLE_THREAD

    dualRegionTestMain<fb_util::ByteColor>(testTotal);
    dualRegionTestMain<fb_util::ByteColor4>(testTotal);
    dualRegionTestMain<int64_t>(testTotal);
    dualRegionTestMain<fb_util::PixelInfo>(testTotal);
    dualRegionTestMain<math::Vec2f>(testTotal);
    dualRegionTestMain<math::Vec3f>(testTotal);
    dualRegionTestMain<fb_util::RenderColor>(testTotal);

    TIME_END;
}

//------------------------------------------------------------------------------------------

template <typename T>
void
TestPixelBufferSha1::singleRegionTestMain(int testTotal)
{
    //
    // generate paramTbl
    //
    TestRunParamTbl paramTbl; // input parameters for each test
    paramTbl.emplace_back(~0, ~0); // special case, non partialmerge

    for (int testId = 0; testId < testTotal; ++testId) {
        unsigned tileIdStart, tileIdEnd;
        if (testId == 0) { // full active tiles test
            tileIdStart = 0;
            tileIdEnd = mTileTotal - 1;
        } else if (testId == 1) { // no active tile test
            tileIdStart = 1;
            tileIdEnd = 0;
        } else {
            setupMinMaxTileId(tileIdStart, tileIdEnd);
        }
        paramTbl.emplace_back(tileIdStart, tileIdEnd); // partialmerge param
    }

    //
    // run paramTbl
    //
    execTestMainLoop<T>("singleRegionTest",
                        paramTbl,
                        [&](const TestRunParam& param, fb_util::PixelBuffer<T>& buff) {
                            singleRegionTestRun(param, buff);
                        });
}

template <typename T>
void
TestPixelBufferSha1::singleRegionTestRun(const TestRunParam& currParam,
                                         fb_util::PixelBuffer<T>& buff)
{
    if (currParam.mTileIdA == ~0) {
        //
        // special case, no partialmerge information
        //
#       ifdef DEBUG_MSG
        std::cerr << ">> TestFbSha1.cc singleRegionTestRun() no partialmerge\n";
#       endif // end DEBUG_MSG

        PixelBufferSha1Hash fbHash;
        fbHash.calcHash(nullptr, buff);

        const unsigned tileIdEnd = mTileTotal - 1;

        PixelBufferSha1Hash::Hash verifyHash;
        bool verifyResult;
        const bool verifyActive = fbHash.calcHashForVerify(0, tileIdEnd, buff, verifyHash, verifyResult);
        CPPUNIT_ASSERT("singleRegionTestRun-A0" && verifyResult);
        CPPUNIT_ASSERT("singleRegionTestRun-A1" &&
                       verifyPrimaryResultOnly<T>(fbHash, verifyActive, verifyHash, 0, tileIdEnd));
    } else {
        //
        // partialmerge
        //
        const unsigned tileIdStart = currParam.mTileIdA;
        const unsigned tileIdEnd = currParam.mTileIdB;
#       ifdef DEBUG_MSG
        std::cerr << ">> TestFbSha1.cc singleRegionTestRun()"
                  << " tileIdStart:" << tileIdStart << " tileIdEnd:" << tileIdEnd << "\n";
#       endif // end DEBUG_MSG

        const PartialMergeTilesTbl tileTbl = setupSingleRegion(tileIdStart, tileIdEnd);
        // useful debug message
        // std::cerr << "TestFbSha1.cc " << PixelBufferSha1Hash::showPartialMergeTilesTbl(tileTbl) << '\n';

        PixelBufferSha1Hash fbHash;
        fbHash.calcHash(&tileTbl, buff);

        PixelBufferSha1Hash::Hash verifyHash;
        bool verifyResult;
        const bool verifyActive = fbHash.calcHashForVerify(tileIdStart, tileIdEnd, buff, verifyHash, verifyResult);
        CPPUNIT_ASSERT("singleRegionTestRun-B0" && verifyResult);
        CPPUNIT_ASSERT("singleRegionTestRun-B1" &&
                       verifyPrimaryResultOnly<T>(fbHash, verifyActive, verifyHash, tileIdStart, tileIdEnd));
    }
}

template <typename T>
void
TestPixelBufferSha1::dualRegionTestMain(int testTotal)
{
    //
    // generate paramTbl
    //
    TestRunParamTbl paramTbl;

    for (int testId = 0; testId < testTotal; ++testId) {
        unsigned tileIdA, tileIdB;
        while (true) {
            setupMinMaxTileId(tileIdA, tileIdB);
            if ((tileIdB - tileIdA) > 1) { // We need at least 1 tile gap.
                break;
            }
        }

        paramTbl.emplace_back(tileIdA, tileIdB);
    }    

    //
    // run paramTbl
    //
    execTestMainLoop<T>("dualRegionTest",
                        paramTbl,
                        [&](const TestRunParam& param, fb_util::PixelBuffer<T>& buff) {
                            dualRegionTestRun(param, buff);
                        });
}

template <typename T>
void
TestPixelBufferSha1::dualRegionTestRun(const TestRunParam& currParam,
                                       fb_util::PixelBuffer<T>& buff)
{
    const unsigned tileIdA = currParam.mTileIdA;
    const unsigned tileIdB = currParam.mTileIdB;
#   ifdef DEBUG_MSG
    std::cerr << ">> TestFbSha1.cc dualRegionTestRun() tileIdA:" << tileIdA << " tileIdB:" << tileIdB << "\n";
#   endif // end DEBUG_MSG

    const PartialMergeTilesTbl tileTbl = setupDualRegion(tileIdA, tileIdB);
    // useful debug message
    // std::cerr << "TestFbSha1.cc " << PixelBufferSha1Hash::showPartialMergeTilesTbl(tileTbl) << '\n';

    PixelBufferSha1Hash fbHash;
    fbHash.calcHash(&tileTbl, buff);

    PixelBufferSha1Hash::Hash verifyHashA, verifyHashB;
    bool verifyResultA, verifyResultB;
    const bool verifyActiveA = fbHash.calcHashForVerify(0, tileIdA, buff, verifyHashA, verifyResultA);
    const bool verifyActiveB = fbHash.calcHashForVerify(tileIdB, mTileTotal - 1, buff, verifyHashB, verifyResultB);
    CPPUNIT_ASSERT("dualRegionTestRun-A" && verifyResultA);
    CPPUNIT_ASSERT("dualRegionTestRun-B" && verifyResultB);
    CPPUNIT_ASSERT("dualRegionTestRun-C" &&
                   verifyResult<T>(fbHash,
                                   verifyActiveA, verifyHashA, 0, tileIdA,
                                   verifyActiveB, verifyHashB, tileIdB, mTileTotal - 1));
}

template <typename T>
void
TestPixelBufferSha1::execTestMainLoop(const std::string& title,
                                      const TestRunParamTbl& paramTbl,
                                      const std::function<void(const TestRunParam& param,
                                                               fb_util::PixelBuffer<T>& buff)>& runFunc)
{
    fb_util::PixelBuffer<T> buff;
    setupBuff(buff);

#   ifdef SINGLE_THREAD
    std::cerr << title << " execTestMainLoop singleThread"
              << " typeid:" << typeid(T).name() << " total:" << paramTbl.size() << '\n';

    for (const auto& itr : paramTbl) {
        runFunc(itr, buff);
    }
#   else // else SINGLE_THREAD
    std::cerr << title << " execTestMainLoop multiThread"
              << " typeid:" << typeid(T).name() << " total:" << paramTbl.size() << '\n';
    
    tbb::blocked_range<size_t> range(0, paramTbl.size(), 1);
    tbb::parallel_for(range, [&](const tbb::blocked_range<size_t>& r) {
            for (size_t i = r.begin(); i < r.end(); ++i) {
                runFunc(paramTbl[i], buff);
            }
        });
#   endif // end else SINGLE_THREAD
}

template <typename T>
void
TestPixelBufferSha1::setupBuff(fb_util::PixelBuffer<T>& buff)
{
    buff.init(mTileAlignedWidth, mTileAlignedHeight);
    buff.clear();

    // fill all pixels with random value
    for (unsigned y = 0; y < mHeight; ++y) {
        for (unsigned x = 0; x < mWidth; ++x) {
            buff.setPixel(x, y, randomPix<T>());
        }
    }
}

void
TestPixelBufferSha1::setupMinMaxTileId(unsigned& tileIdMin, unsigned& tileIdMax)
{
    tileIdMin = randTileId();
    tileIdMax = randTileId();
    if (tileIdMax < tileIdMin) {
        unsigned tmp = tileIdMin;
        tileIdMin = tileIdMax;
        tileIdMax = tmp;
    }
}

TestPixelBufferSha1::PartialMergeTilesTbl
TestPixelBufferSha1::setupSingleRegion(unsigned tileIdStart, unsigned tileIdEnd)
{
    PartialMergeTilesTbl tbl(mTileTotal);
    resetTileIdTbl(tbl);
    if (tileIdStart <= tileIdEnd) {
        fillTileIdTbl(tbl, tileIdStart, tileIdEnd);
    }
    return tbl;
}

TestPixelBufferSha1::PartialMergeTilesTbl
TestPixelBufferSha1::setupDualRegion(unsigned tileIdA, unsigned tileIdB)
{
    PartialMergeTilesTbl tbl(mTileTotal);
    resetTileIdTbl(tbl);
    fillTileIdTbl(tbl, 0, tileIdA); // regionA
    fillTileIdTbl(tbl, tileIdB, mTileTotal - 1); // regionB
    return tbl;
}

void
TestPixelBufferSha1::resetTileIdTbl(PartialMergeTilesTbl& tbl) const
{
    std::fill(tbl.begin(), tbl.end(), static_cast<char>(false));
}

void
TestPixelBufferSha1::fillTileIdTbl(PartialMergeTilesTbl& tbl, unsigned startId, unsigned endId) const
{
    MNRY_ASSERT(startId < tbl.size() && endId < tbl.size() && startId <= endId);
    std::fill(tbl.begin() + startId, tbl.begin() + endId + 1, static_cast<char>(true));
}

template <typename T>
bool
TestPixelBufferSha1::verifyPrimaryResultOnly(const PixelBufferSha1Hash& fbHash,
                                             bool verifyActive,
                                             PixelBufferSha1Hash::Hash& verifyHash,
                                             unsigned tileIdStart, unsigned tileIdEnd) const
{
    if (!fbHash.getPrimaryActive() && !verifyActive) {
        return true;
    }

    if (fbHash.getPrimaryActive() == verifyActive &&
        fbHash.getPrimaryHash() == verifyHash &&
        fbHash.getPrimaryStartTileId() == static_cast<size_t>(tileIdStart) &&
        fbHash.getPrimaryEndTileId() == static_cast<size_t>(tileIdEnd)) {
        return true;
    }

    std::ostringstream ostr;
    ostr << "VerifyResult Failed (typeId:" << typeid(T).name() << ") {\n"
         << "  mSeed:" << mSeed << '\n'
         << str_util::addIndent(fbHash.show()) << '\n'
         << str_util::addIndent(showVerifyInfo("verifyInfo",
                                               verifyActive, verifyHash, tileIdStart, tileIdEnd)) << '\n'
         << '}';
    std::cerr << ostr.str() << '\n';

    return false;
}

template <typename T>
bool
TestPixelBufferSha1::verifyResult(const PixelBufferSha1Hash& fbHash,
                                  bool verifyActiveA,
                                  PixelBufferSha1Hash::Hash& verifyHashA,
                                  unsigned tileIdStartA,
                                  unsigned tileIdEndA,
                                  bool verifyActiveB,
                                  PixelBufferSha1Hash::Hash& verifyHashB,
                                  unsigned tileIdStartB,
                                  unsigned tileIdEndB) const
{
    if (fbHash.getPrimaryActive() == verifyActiveA &&
        fbHash.getSecondaryActive() == verifyActiveB &&
        fbHash.getPrimaryHash() == verifyHashA &&
        fbHash.getPrimaryStartTileId() == static_cast<size_t>(tileIdStartA) &&
        fbHash.getPrimaryEndTileId() == static_cast<size_t>(tileIdEndA) &&
        fbHash.getSecondaryHash() == verifyHashB &&
        fbHash.getSecondaryStartTileId() == static_cast<size_t>(tileIdStartB) &&
        fbHash.getSecondaryEndTileId() == static_cast<size_t>(tileIdEndB)) {
        return true;
    }

    std::ostringstream ostr;
    ostr << "VerifyResult Failed (typeId:" << typeid(T).name() << ") {\n"
         << "  mSeed:" << mSeed << '\n'
         << str_util::addIndent(fbHash.show()) << '\n'
         << str_util::addIndent(showVerifyInfo("verifyInfoA",
                                               verifyActiveA, verifyHashA, tileIdStartA, tileIdEndA)) << '\n'
         << str_util::addIndent(showVerifyInfo("verifyInfoB",
                                               verifyActiveB, verifyHashB, tileIdStartB, tileIdEndB)) << '\n'
         << "}";
    std::cerr << ostr.str() << '\n';

    return false;
}

// static function
std::string
TestPixelBufferSha1::showVerifyInfo(const std::string& title,
                                    bool verifyActive,
                                    const PixelBufferSha1Hash::Hash& verifyHash,
                                    unsigned tileIdStart,
                                    unsigned tileIdEnd)
{
    std::ostringstream ostr;
    ostr << "verifyInfo (" << title << ") {\n";
    if (verifyActive) {
        ostr << "  verifyActive:ON\n"
             << str_util::addIndent(Sha1Util::show(verifyHash)) << '\n'
             << "  tileIdStart:" << tileIdStart << '\n'
             << "  tileIdEnd:" << tileIdEnd << '\n';
    } else {
        ostr << "  verifyActive:OFF\n";
    }
    ostr << "}";
    return ostr.str();
}

void    
TestPixelBufferSha1::randomPixTest()
{
    fb_util::ByteColor pix = randByteColor();
    fb_util::ByteColor4 pix2 = randByteColor4();
    fb_util::PixelInfo pix3 = randPixelInfo();

    std::cerr << "byteColor:(" << (int)pix.r << ' ' << (int)pix.g << ' ' << (int)pix.b << ")\n";
    std::cerr << "byteColor4:(" << (int)pix2.r << ' ' << (int)pix2.g << ' ' << (int)pix2.b << ' ' << (int)pix2.a << ")\n";
    std::cerr << "pixelInfo:(" << pix3.depth << ")\n";
    std::cerr << "vec2:" << randV2() << '\n'
              << "vec3:" << randV3() << '\n'
              << "vec4:" << randV4() << '\n';
    std::cerr << "renderColor:" << randRenderColor() << '\n';

    fb_util::ByteColor pixB = randomPix<fb_util::ByteColor>();
    fb_util::ByteColor4 pix2B = randomPix<fb_util::ByteColor4>();
    fb_util::PixelInfo pix3B = randomPix<fb_util::PixelInfo>();

    std::cerr << "byteColorB:(" << (int)pixB.r << ' ' << (int)pixB.g << ' ' << (int)pixB.b << ")\n";
    std::cerr << "byteColor4B:(" << (int)pix2B.r << ' ' << (int)pix2B.g << ' ' << (int)pix2B.b << ' ' << (int)pix2B.a << ")\n";
    std::cerr << "pixelInfoB:(" << pix3B.depth << ")\n";
    std::cerr << "vec2B:" << randomPix<math::Vec2f>() << '\n'
              << "vec3B:" << randomPix<math::Vec3f>() << '\n'
              << "vec4B:" << randomPix<math::Vec4f>() << '\n';
    std::cerr << "renderColorB:" << randomPix<fb_util::RenderColor>() << '\n';    
}

} // namespace unittest
} // namespace grid_util
} // namespace scene_rdl2
