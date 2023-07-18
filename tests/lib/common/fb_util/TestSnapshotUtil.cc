// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "TestSnapshotUtil.h"

#include <scene_rdl2/common/fb_util/SnapshotUtil.h>
#include <scene_rdl2/common/rec_time/RecTime.h>
#include <scene_rdl2/render/util/StrUtil.h>

#include <algorithm>
#include <functional>
#include <vector>

// If comment out following directive, all unitTest do timing test.
// Each unittest needs almost 128x longer execution cost.
//#define TIMING_TEST

namespace scene_rdl2 {
namespace fb_util {
namespace unittest {

static constexpr int sTileReso = 8; // we can not change tile resolution

void
TestSnapshotUtil::setUp()
{
}

void
TestSnapshotUtil::tearDown()
{
}

void
TestSnapshotUtil::testHeatMapWeight()
{
    int w = sTileReso * 240; // = 1920
    int h = sTileReso * 135; // = 1080

    std::vector<double> orgV(w * h);
    std::vector<float> orgW(w * h);
    setupRealBuff(orgV, 1, 0.3f); // 30% black pix, 70% random value
    setupWeightBuff(orgW, 0.3f); // 30% zero weight, 70% random value

    std::vector<double> dstV(orgV);
    std::vector<float> dstW(orgW);
    std::vector<double> srcV(orgV);
    std::vector<float> srcW(orgW);
    std::vector<double> tgtV(orgV);
    std::vector<float> tgtW(orgW);

    std::vector<int> updatePixIdArray;
    updateBuff(0.6f, // update 60% pix
               w,
               h,
               [&](const int pixOffset) { // updatePixelFunc
                   return updatePix(srcV, srcW, pixOffset, 1);
               },
               [&](const int pixOffset) { // updateTargetFunc
                   copyPix(tgtV, tgtW, srcV, srcW, pixOffset, 1);
               },
               updatePixIdArray);
    /*
    std::cerr << ">> TestSnapshotUtil.cc testHeatMapWeight() "
              << analyzeBuff(1, orgV, orgW, srcV, srcW) << '\n'; // useful debug dump message
    */

    snapshotTimingCompare
        (w,
         h,
         [&]() { // resetDataFunc
            dstV = orgV;
            dstW = orgW;
         },
         [&](int offsetItem) { // snapshotTileFuncA
            return fb_util::SnapshotUtil::snapshotTileHeatMapWeight
                ((uint64_t *)((uintptr_t)(&dstV[0]) + offsetItem * sizeof(double)),
                 (uint32_t *)((uintptr_t)(&dstW[0]) + offsetItem * sizeof(float)),
                 (uint64_t *)((uintptr_t)(&srcV[0]) + offsetItem * sizeof(double)),
                 (uint32_t *)((uintptr_t)(&srcW[0]) + offsetItem * sizeof(float)));
         },
         [&](int offsetItem) { // snapshotTileFuncB
            return fb_util::SnapshotUtil::snapshotTileHeatMapWeight_SISD
                ((uint64_t *)((uintptr_t)(&dstV[0]) + offsetItem * sizeof(double)),
                 (uint32_t *)((uintptr_t)(&dstW[0]) + offsetItem * sizeof(float)),
                 (uint64_t *)((uintptr_t)(&srcV[0]) + offsetItem * sizeof(double)),
                 (uint32_t *)((uintptr_t)(&srcW[0]) + offsetItem * sizeof(float)));
         },
         [&](std::vector<uint64_t>& pixMaskBuff) { // verifyFunc
             return verifyPixMask(updatePixIdArray, pixMaskBuff) && (dstV == tgtV) && (dstW == tgtW);
         });
}

void
TestSnapshotUtil::testWeight()
{
    int w = sTileReso * 240; // = 1920
    int h = sTileReso * 135; // = 1080

    std::vector<float> orgW(w * h);
    setupWeightBuff(orgW, 0.3f); // 30% zero weight 70% random value

    std::vector<float> dstW(orgW);
    std::vector<float> srcW(orgW);
    std::vector<float> tgtW(orgW);
    
    std::vector<int> updatePixIdArray;
    updateBuff(0.6f, // update 60% pix
               w,
               h,
               [&](const int pixOffset) { // updatePixelFunc
                   srcW[pixOffset] += getNon0RandReal01(); // weight value is only increased and never decreased
                   return true;
               },
               [&](const int pixOffset) { // updateTargetFunc 
                   tgtW[pixOffset] = srcW[pixOffset];
               },
               updatePixIdArray);
    /*
    std::cerr << ">> TestSnapshotUtil.cc testWeight() "
              << analyzeWeightBuff(orgW, srcW) << '\n'; // useful debug dump message
    */

    snapshotTimingCompare
        (w,
         h,
         [&]() { // resetDataFunc
            dstW = orgW;
         },
         [&](int offsetItem) { // snapshotTileFuncA
            return fb_util::SnapshotUtil::snapshotTileWeightBuffer
                ((uint32_t *)((uintptr_t)(&dstW[0]) + offsetItem * sizeof(float)),
                 (uint32_t *)((uintptr_t)(&srcW[0]) + offsetItem * sizeof(float)));
         },
         [&](int offsetItem) { // snapshotTileFuncB
            return fb_util::SnapshotUtil::snapshotTileWeightBuffer_SISD
                ((uint32_t *)((uintptr_t)(&dstW[0]) + offsetItem * sizeof(float)),
                 (uint32_t *)((uintptr_t)(&srcW[0]) + offsetItem * sizeof(float)));
         },
         [&](std::vector<uint64_t>& pixMaskBuff) { // verifyFunc
             return verifyPixMask(updatePixIdArray, pixMaskBuff) && (dstW == tgtW);
         });
}

void
TestSnapshotUtil::testWeightMask()
{
    int w = sTileReso * 240; // = 1920
    int h = sTileReso * 135; // = 1080

    std::vector<float> orgV(w * h);
    setupRealBuff(orgV, 1, 0.3f); // 30% black pix, 70% random value

    std::vector<float> dstV(orgV);
    std::vector<float> srcV(orgV);
    std::vector<float> tgtV(orgV);

    int tileTotal = (w / sTileReso) * (h / sTileReso);
    std::vector<uint64_t> dstPixMaskBuff(tileTotal);
    std::vector<uint64_t> srcPixMaskBuff(tileTotal);
    setupPixMaskBuff(0.2, // 20% empty
                     0.2, // 20% full
                     dstPixMaskBuff);
    setupPixMaskBuff(0.3, // 30% empty
                     0.1, // 10% full
                     srcPixMaskBuff);

    std::vector<int> updatePixIdArray;
    updateBuff2(srcPixMaskBuff,
                [&](const int pixOffset) { // updatePixelFunc
                    srcV[pixOffset] += getNon0RandReal01();
                    return true;
                },
                [&](const int pixOffset) { // updateTargetFunc
                    tgtV[pixOffset] = srcV[pixOffset];
                },
                updatePixIdArray);
    /*
    std::cerr << ">> TestSnapshotUtil.cc testWeightMask() "
              << analyzeWeightBuff(orgV, srcV) << '\n'; // useful debug dump message
    */

    snapshotTimingCompare
        (w,
         h,
         [&]() { // resetDataFunc
            dstV = orgV;
         },
         [&](int offsetItem) { // snapshotTileFuncA
            int tileId = offsetItem / (sTileReso * sTileReso);
            return fb_util::SnapshotUtil::snapshotTileWeightBuffer
                ((uint32_t *)((uintptr_t)&dstV[0] + offsetItem * sizeof(float)),
                 dstPixMaskBuff[tileId],
                 (uint32_t *)((uintptr_t)&srcV[0] + offsetItem * sizeof(float)),
                 srcPixMaskBuff[tileId]);
         },
         [&](int offsetItem) { // snapshotTileFuncB
            int tileId = offsetItem / (sTileReso * sTileReso);
            return fb_util::SnapshotUtil::snapshotTileWeightBuffer_SISD
                ((uint32_t *)((uintptr_t)&dstV[0] + offsetItem * sizeof(float)),
                 dstPixMaskBuff[tileId],
                 (uint32_t *)((uintptr_t)&srcV[0] + offsetItem * sizeof(float)),
                 srcPixMaskBuff[tileId]);
         },
         [&](std::vector<uint64_t> &pixMaskBuff) -> bool { // verifyFunc
             return verifyPixMask(updatePixIdArray, pixMaskBuff) && (dstV == tgtV);
         });
}

void
TestSnapshotUtil::testFloatWeight()
{
    testFloatNWeight(1,
                     [&](uint32_t* dstVPtr, uint32_t* dstWPtr, uint32_t* srcVPtr, uint32_t* srcWPtr) {
                         return fb_util::SnapshotUtil::snapshotTileFloatWeight
                             (dstVPtr, dstWPtr, srcVPtr, srcWPtr);
                     },
                     [&](uint32_t* dstVPtr, uint32_t* dstWPtr, uint32_t* srcVPtr, uint32_t* srcWPtr) {
                         return fb_util::SnapshotUtil::snapshotTileFloatWeight_SISD
                             (dstVPtr, dstWPtr, srcVPtr, srcWPtr);
                     });
}

void
TestSnapshotUtil::testFloatNumSample()
{
    testFloatNNumSample(1,
                        [&](uint32_t* dstVPtr, uint32_t* dstNPtr, uint64_t dstPixMask,
                            uint32_t* srcVPtr, uint32_t* srcNPtr, uint64_t srcPixMask) {
                            return fb_util::SnapshotUtil::snapshotTileFloatNumSample
                                (dstVPtr, dstNPtr, dstPixMask, srcVPtr, srcNPtr, srcPixMask);
                        },
                        [&](uint32_t* dstVPtr, uint32_t* dstNPtr, uint64_t dstPixMask,
                            uint32_t* srcVPtr, uint32_t* srcNPtr, uint64_t srcPixMask) {
                            return fb_util::SnapshotUtil::snapshotTileFloatNumSample_SISD
                                (dstVPtr, dstNPtr, dstPixMask, srcVPtr, srcNPtr, srcPixMask);
                        });
}

void
TestSnapshotUtil::testFloat2Weight()
{
    testFloatNWeight(2,
                     [&](uint32_t* dstVPtr, uint32_t* dstWPtr, uint32_t* srcVPtr, uint32_t* srcWPtr) {
                         return fb_util::SnapshotUtil::snapshotTileFloat2Weight
                             (dstVPtr, dstWPtr, srcVPtr, srcWPtr);
                     },
                     [&](uint32_t* dstVPtr, uint32_t* dstWPtr, uint32_t* srcVPtr, uint32_t* srcWPtr) {
                         return fb_util::SnapshotUtil::snapshotTileFloat2Weight_SISD
                             (dstVPtr, dstWPtr, srcVPtr, srcWPtr);
                     });
}

void
TestSnapshotUtil::testFloat2NumSample()
{
    testFloatNNumSample(2,
                        [&](uint32_t* dstVPtr, uint32_t* dstNPtr, uint64_t dstPixMask,
                            uint32_t* srcVPtr, uint32_t* srcNPtr, uint64_t srcPixMask) {
                            return fb_util::SnapshotUtil::snapshotTileFloat2NumSample
                                (dstVPtr, dstNPtr, dstPixMask, srcVPtr, srcNPtr, srcPixMask);
                        },
                        [&](uint32_t* dstVPtr, uint32_t* dstNPtr, uint64_t dstPixMask,
                            uint32_t* srcVPtr, uint32_t* srcNPtr, uint64_t srcPixMask) {
                            return fb_util::SnapshotUtil::snapshotTileFloat2NumSample_SISD
                                (dstVPtr, dstNPtr, dstPixMask, srcVPtr, srcNPtr, srcPixMask);
                        });
}
    
void
TestSnapshotUtil::testFloat3Weight()
{
    testFloatNWeight(3,
                     [&](uint32_t* dstVPtr, uint32_t* dstWPtr, uint32_t* srcVPtr, uint32_t* srcWPtr) {
                         return fb_util::SnapshotUtil::snapshotTileFloat3Weight
                             (dstVPtr, dstWPtr, srcVPtr, srcWPtr);
                     },
                     [&](uint32_t* dstVPtr, uint32_t* dstWPtr, uint32_t* srcVPtr, uint32_t* srcWPtr) {
                         return fb_util::SnapshotUtil::snapshotTileFloat3Weight_SISD
                             (dstVPtr, dstWPtr, srcVPtr, srcWPtr);
                     });
}

void
TestSnapshotUtil::testFloat3NumSample()
{
    testFloatNNumSample(3,
                        [&](uint32_t* dstVPtr, uint32_t* dstNPtr, uint64_t dstPixMask,
                            uint32_t* srcVPtr, uint32_t* srcNPtr, uint64_t srcPixMask) {
                            return fb_util::SnapshotUtil::snapshotTileFloat3NumSample
                                (dstVPtr, dstNPtr, dstPixMask, srcVPtr, srcNPtr, srcPixMask);
                        },
                        [&](uint32_t* dstVPtr, uint32_t* dstNPtr, uint64_t dstPixMask,
                            uint32_t* srcVPtr, uint32_t* srcNPtr, uint64_t srcPixMask) {
                            return fb_util::SnapshotUtil::snapshotTileFloat3NumSample_SISD
                                (dstVPtr, dstNPtr, dstPixMask, srcVPtr, srcNPtr, srcPixMask);
                        });
}

void
TestSnapshotUtil::testFloat4Weight()
{
    testFloatNWeight(4,
                     [&](uint32_t* dstVPtr, uint32_t* dstWPtr, uint32_t* srcVPtr, uint32_t* srcWPtr) {
                         return fb_util::SnapshotUtil::snapshotTileFloat4Weight
                             (dstVPtr, dstWPtr, srcVPtr, srcWPtr);
                     },
                     [&](uint32_t* dstVPtr, uint32_t* dstWPtr, uint32_t* srcVPtr, uint32_t* srcWPtr) {
                         return fb_util::SnapshotUtil::snapshotTileFloat4Weight_SISD
                             (dstVPtr, dstWPtr, srcVPtr, srcWPtr);
                     });
}

void
TestSnapshotUtil::testFloat4NumSample()
{
    testFloatNNumSample(4,
                        [&](uint32_t* dstVPtr, uint32_t* dstNPtr, uint64_t dstPixMask,
                            uint32_t* srcVPtr, uint32_t* srcNPtr, uint64_t srcPixMask) {
                            return fb_util::SnapshotUtil::snapshotTileFloat4NumSample
                                (dstVPtr, dstNPtr, dstPixMask, srcVPtr, srcNPtr, srcPixMask);
                        },
                        [&](uint32_t* dstVPtr, uint32_t* dstNPtr, uint64_t dstPixMask,
                            uint32_t* srcVPtr, uint32_t* srcNPtr, uint64_t srcPixMask) {
                            return fb_util::SnapshotUtil::snapshotTileFloat4NumSample_SISD
                                (dstVPtr, dstNPtr, dstPixMask, srcVPtr, srcNPtr, srcPixMask);
                        });
}
    
//------------------------------------------------------------------------------------------    

void
TestSnapshotUtil::testFloatNWeight(const int pixDim,
                                   const TestSnapshotTileFunc& snapshotTileFuncA,
                                   const TestSnapshotTileFunc& snapshotTileFuncB)
{
    int w = sTileReso * 240; // = 1920
    int h = sTileReso * 135; // = 1080

    std::vector<float> orgV(w * h * pixDim);
    std::vector<float> orgW(w * h);
    setupRealBuff(orgV, pixDim, 0.3f); // 30% black pix, 70% random value
    setupWeightBuff(orgW, 0.3f); // 30% zero weight, 70% random value

    std::vector<float> dstV(orgV);
    std::vector<float> dstW(orgW);
    std::vector<float> srcV(orgV);
    std::vector<float> srcW(orgW);
    std::vector<float> tgtV(orgV);
    std::vector<float> tgtW(orgW);

    std::vector<int> updatePixIdArray;
    updateBuff(0.6f, // update 60% pix
               w,
               h,
               [&](const int pixOffset) { // updatePixelFunc
                   return updatePix(srcV, srcW, pixOffset, pixDim);
               },
               [&](const int pixOffset) { // updateTargetFunc
                   copyPix(tgtV, tgtW, srcV, srcW, pixOffset, pixDim);
               },
               updatePixIdArray);
    /*
    std::cerr << ">> TestSnapshotUtil.cc testFloatNWeight() "
              << analyzeBuff(pixDim, orgV, orgW, srcV, srcW) << '\n'; // useful debug dump message
    */

    snapshotTimingCompare
        (w,
         h,
         [&]() { // resetDataFunc
            dstV = orgV;
            dstW = orgW;
         },
         [&](int offsetItem) { // snapshotTileFuncA
             uint32_t* dstVPtr = (uint32_t*)((uintptr_t)(&dstV[0]) + offsetItem * sizeof(float) * pixDim);
             uint32_t* dstWPtr = (uint32_t*)((uintptr_t)(&dstW[0]) + offsetItem * sizeof(float));
             uint32_t* srcVPtr = (uint32_t*)((uintptr_t)(&srcV[0]) + offsetItem * sizeof(float) * pixDim);
             uint32_t* srcWPtr = (uint32_t*)((uintptr_t)(&srcW[0]) + offsetItem * sizeof(float));
             return snapshotTileFuncA(dstVPtr, dstWPtr, srcVPtr, srcWPtr);
         },
         [&](int offsetItem) { // snapshotTileFuncB
             uint32_t* dstVPtr = (uint32_t*)((uintptr_t)(&dstV[0]) + offsetItem * sizeof(float) * pixDim);
             uint32_t* dstWPtr = (uint32_t*)((uintptr_t)(&dstW[0]) + offsetItem * sizeof(float));
             uint32_t* srcVPtr = (uint32_t*)((uintptr_t)(&srcV[0]) + offsetItem * sizeof(float) * pixDim);
             uint32_t* srcWPtr = (uint32_t*)((uintptr_t)(&srcW[0]) + offsetItem * sizeof(float));
             return snapshotTileFuncB(dstVPtr, dstWPtr, srcVPtr, srcWPtr);
         },
         [&](std::vector<uint64_t>& pixMaskBuff) { // verifyFunc
             return verifyPixMask(updatePixIdArray, pixMaskBuff) && (dstV == tgtV) && (dstW == tgtW);
         });
}

void
TestSnapshotUtil::testFloatNNumSample(const int pixDim,
                                      const TestSnapshotTileFunc2& snapshotTileFuncA,
                                      const TestSnapshotTileFunc2& snapshotTileFuncB)
{
    int w = sTileReso * 240; // = 1920
    int h = sTileReso * 135; // = 1080

    std::vector<float> orgV(w * h * pixDim);
    std::vector<unsigned int> orgN(w * h);
    setupRealBuff(orgV, pixDim, 0.3f); // 30% black pix, 70% random value
    setupNumBuff(orgN, 0.3f); // 30% zero weight, 70% random value

    std::vector<float> dstV(orgV);
    std::vector<unsigned int> dstN(orgN);
    std::vector<float> srcV(orgV);
    std::vector<unsigned int> srcN(orgN);
    std::vector<float> tgtV(orgV);
    std::vector<unsigned int> tgtN(orgN);

    int tileTotal = (w / sTileReso) * (h / sTileReso);
    std::vector<uint64_t> dstPixMaskBuff(tileTotal);
    std::vector<uint64_t> srcPixMaskBuff(tileTotal);
    setupPixMaskBuff(0.2, // 20% empty
                     0.2, // 20% full
                     dstPixMaskBuff);
    setupPixMaskBuff(0.3, // 30% empty
                     0.1, // 10% full
                     srcPixMaskBuff);

    std::vector<int> updatePixIdArray;
    updateBuff2(srcPixMaskBuff,
                [&](const int pixOffset) { // updatePixelFunc
                    return updatePix2(srcV, srcN, pixOffset, pixDim);
                },
                [&](const int pixOffset) { // updateTargetFunc
                    return copyPix2(tgtV, tgtN, srcV, srcN, pixOffset, pixDim);
                },
                updatePixIdArray);
    /*
    std::cerr << ">> TestSnapshotUtil.cc testFloatNNumSample() "
              << analyzeBuff2(pixDim, orgV, orgN, srcV, srcN) << '\n'; // useful debug dump message
    */

    snapshotTimingCompare
        (w,
         h,
         [&]() { // resetDataFunc
            dstV = orgV;
            dstN = orgN;
         },
         [&](int offsetItem) { // snapshotTileFuncA
            uint32_t* dstVPtr = (uint32_t*)((uintptr_t)&(dstV[0]) + offsetItem * sizeof(float) * pixDim);
            uint32_t* dstNPtr = (uint32_t*)((uintptr_t)&(dstN[0]) + offsetItem * sizeof(unsigned int));
            uint32_t* srcVPtr = (uint32_t*)((uintptr_t)&(srcV[0]) + offsetItem * sizeof(float) * pixDim);
            uint32_t* srcNPtr = (uint32_t*)((uintptr_t)&(srcN[0]) + offsetItem * sizeof(unsigned int));
            int tileId = offsetItem / (sTileReso * sTileReso);
            return snapshotTileFuncA(dstVPtr, dstNPtr, dstPixMaskBuff[tileId],
                                     srcVPtr, srcNPtr, srcPixMaskBuff[tileId]);
         },
         [&](int offsetItem) { // snapshotTileFuncB
            uint32_t* dstVPtr = (uint32_t*)((uintptr_t)&(dstV[0]) + offsetItem * sizeof(float) * pixDim);
            uint32_t* dstNPtr = (uint32_t*)((uintptr_t)&(dstN[0]) + offsetItem * sizeof(unsigned int));
            uint32_t* srcVPtr = (uint32_t*)((uintptr_t)&(srcV[0]) + offsetItem * sizeof(float) * pixDim);
            uint32_t* srcNPtr = (uint32_t*)((uintptr_t)&(srcN[0]) + offsetItem * sizeof(unsigned int));
            int tileId = offsetItem / (sTileReso * sTileReso);
            return snapshotTileFuncB(dstVPtr, dstNPtr, dstPixMaskBuff[tileId],
                                     srcVPtr, srcNPtr, srcPixMaskBuff[tileId]);
         },
         [&](std::vector<uint64_t> &pixMaskBuff) { // verifyFunc
             return verifyPixMask(updatePixIdArray, pixMaskBuff) && (dstV == tgtV) && (dstN == tgtN);
         });
}

template <typename T>
void
TestSnapshotUtil::setupBuffRandom(std::vector<T>& buff) const
//
// non zero random value
//
{
    std::random_device rnd;
    std::mt19937 mt(rnd());
    std::uniform_real_distribution<> randVal(0.001, 1.0);

    for (size_t i = 0; i < buff.size(); ++i) {
        buff[i] = static_cast<T>(randVal(mt));
    }
}

template <typename T>
void
TestSnapshotUtil::setupBuffZero(std::vector<T>& buff,
                                const int pixDim,
                                const float blackPixFraction) const
{
    std::random_device rnd;
    std::mt19937 mt(rnd());
    std::uniform_real_distribution<> randVal(0.0, 1.0);

    size_t pixTotal = buff.size() / static_cast<size_t>(pixDim);
    for (size_t pixId = 0; pixId < pixTotal; ++pixId) {
        if (randVal(mt) <= blackPixFraction) {
            size_t offset = pixId * pixDim;
            for (size_t i = 0; i < pixDim; ++i) {
                buff[offset + i] = static_cast<T>(0.0);
            }
        }
    }
}

template <typename T>
void
TestSnapshotUtil::setupRealBuff(std::vector<T>& buff,
                                const int pixDim,
                                const float zeroWeightPixFraction) const
{
    setupBuffRandom(buff); // set random weight value first
    setupBuffZero(buff, pixDim, zeroWeightPixFraction);
}

void
TestSnapshotUtil::setupWeightBuff(std::vector<float>& buff,
                                  const float zeroWeightPixFraction) const
{
    setupBuffRandom(buff); // set random weight value first
    setupBuffZero(buff, 1, zeroWeightPixFraction);
}

void
TestSnapshotUtil::setupNumBuff(std::vector<unsigned int>& buff,
                               const float zeroWeightPixFraction) const
{
    std::random_device rnd;
    std::mt19937 mt(rnd());
    std::uniform_int_distribution<> randN(1, 4096);
    std::uniform_real_distribution<> rand01(0.0, 1.0);

    for (size_t i = 0; i < buff.size(); ++i) {
        if (rand01(mt) <= zeroWeightPixFraction) {
            buff[i] = 0;
        } else {
            buff[i] = randN(mt);
        }
    }
}

void
TestSnapshotUtil::setupPixMaskBuff(float emptyMaskFraction,
                                   float fullMaskFraction,
                                   std::vector<uint64_t> &buff) const
{
    std::random_device rnd;
    std::mt19937 mt(rnd());
    std::uniform_real_distribution<> rand01(0.0f, 1.0f);
    std::uniform_int_distribution<> randPixTotal(1, 63);

    auto randomPixMask = [&](int totalActivePix) -> uint64_t {
        if (totalActivePix > 63) totalActivePix = 63;

        std::vector<int> pixIdBuff(64);
        std::iota(pixIdBuff.begin(), pixIdBuff.end(), 0);
        std::shuffle(pixIdBuff.begin(), pixIdBuff.end(), mt);

        uint64_t mask = 0x0;
        for (size_t i = 0; i < (size_t)totalActivePix; ++i) {
            int currPixId = pixIdBuff[i];
            mask |= ((uint64_t)0x1 << currPixId);
        }
        return mask;
    };


    for (size_t i = 0; i < buff.size(); ++i) {
        float r0 = rand01(mt);

        uint64_t currMask = 0x0;
        if (r0 < emptyMaskFraction) {
            currMask = 0x0;     // empty mask
        } else if (r0 < (emptyMaskFraction + fullMaskFraction)) {
            currMask = (uint64_t)0xffffffffffffffff; // full mask
        } else {
            currMask = randomPixMask(randPixTotal(mt));
        }
        buff[i] = currMask;
    }
}

void
TestSnapshotUtil::updateBuff(const float updatePixFraction,
                             const int w, // should be tile aligned resolution
                             const int h, // should be tile aligned resolution
                             const std::function<bool(const int pixOffset)>& updatePixelFunc,
                             const std::function<void(const int pixOffset)>& updateTargetFunc,
                             std::vector<int>& updatePixIdArray) const
{
    int totalPix = w * h;
    std::vector<int> pixOffsetArray(totalPix);
    for (size_t pixId = 0; pixId < totalPix; ++pixId) {
        pixOffsetArray[pixId] = pixId;
    }
    std::shuffle(pixOffsetArray.begin(), pixOffsetArray.end(), std::default_random_engine());

    /* useful debug dump
    for (int i = 0; i < 32; ++i) {
        int pixOff = pixOffsetArray[i];
        int tileId = pixOff / 64;
        int tileOff = pixOff % 64;
        int x = tileOff % 8;
        int y = tileOff / 8;
        std::cerr << "i:" << std::setw(2) << i
                  << " pixOff:" << std::setw(8) << pixOff
                  << " tileId:" << std::setw(5) << tileId
                  << " x:" << std::setw(1) << x
                  << " y:" << std::setw(1) << y
                  << '\n';
    }
    */

    int updateTotal = static_cast<int>(static_cast<float>(totalPix) * updatePixFraction);
    for (int id  = 0; id < updateTotal; ++id) {
        int pixOffset = pixOffsetArray[id];
        if (updatePixelFunc(pixOffset)) {
            updatePixIdArray.push_back(pixOffset); // updated pixel
            updateTargetFunc(pixOffset);     // update target buff and weight data
        }
    }

    if (!updatePixIdArray.empty()) {
        std::sort(updatePixIdArray.begin(), updatePixIdArray.end()); // sort updatePixIdArray for verify operation
    }
}

template <typename T>
bool
TestSnapshotUtil::updatePix(std::vector<T>& pixBuff, 
                            std::vector<float>& weightBuff,
                            const int pixOffset,
                            const int pixDim)
{
    auto getDataOffset = [&](int chanId) { return pixOffset * pixDim + chanId; };
    auto pixDimLoop = [&](const std::function<void(const size_t dataOffset)>& pixUpdateFunc) {
        for (int chanId = 0; chanId < pixDim; ++chanId) { pixUpdateFunc(getDataOffset(chanId)); }
    };
    auto updateVal = [&](const T origVal) {
        for (int i = 0; i < 10; ++i) { // try to find different values from the original 10 times
            T newVal = static_cast<T>(getRandReal01());
            if (newVal != origVal) return newVal;
        }
        // unlikely to be here
        return origVal + ((origVal > static_cast<T>(0.5)) ? static_cast<T>(-0.1) : static_cast<T>(0.1));
    };

    //
    // keep original condition
    //
    std::vector<T> origPix(pixDim);
    for (int chanId = 0; chanId < pixDim; ++chanId) { origPix[chanId] = pixBuff[getDataOffset(chanId)]; }
    float origWeight = weightBuff[pixOffset];

    //
    // update value : 33.333% of pixels : only update pixel value
    //                33.333% of pixels : update pixel value and weight
    //                33.333% of pixels : only update weight
    //
    //                     0.0      0.333     0.666      1.0
    //   update pix(el)  -> |<------->|<------->|         |
    //   update w(eight) -> |         |<------->|<------->|
    //                          pix     pix + w      w                   
    float val = getRandReal01();
    if (val < 0.66667f) {
        pixDimLoop([&](const size_t dataOffset) {
                pixBuff[dataOffset] = updateVal(pixBuff[dataOffset]);
            });
    }
    if (val > 0.33333f) {
        weightBuff[pixOffset] += getNon0RandReal01(); // weight value should be only increased and it never decreased
    }

    //
    // active pixel info update
    //
    auto activePixTest = [&]() {
        float currWeight = weightBuff[pixOffset];
        if (currWeight == 0.0f) return false; // early exit : non active pixel because weight is ZERO

        if (origWeight != currWeight) return true; // active pixel
        for (int chanId = 0; chanId < pixDim; ++chanId) {
            if (pixBuff[getDataOffset(chanId)] != origPix[chanId]) return true; // active pixel
        }
        return false; // non active pixel
    };
    return activePixTest();
}
          
template <typename T>
bool
TestSnapshotUtil::updatePix2(std::vector<T>& pixBuff,
                             std::vector<unsigned int>& numBuff,
                             const int pixOffset,
                             const int pixDim)
{
    auto getDataOffset = [&](int chanId) { return pixOffset * pixDim + chanId; };
    auto pixDimLoop = [&](const std::function<void(const size_t dataOffset)>& pixUpdateFunc) {
        for (int chanId = 0; chanId < pixDim; ++chanId) { pixUpdateFunc(getDataOffset(chanId)); }
    };
    auto updateVal = [&](const T origVal) {
        for (int i = 0; i < 10; ++i) { // try to find different values from the original 10 times
            T newVal = static_cast<T>(getRandReal01());
            if (newVal != origVal) return newVal;
        }
        // unlikely to be here
        return origVal + ((origVal > static_cast<T>(0.5)) ? static_cast<T>(-0.1) : static_cast<T>(0.1));
    };
    auto non0RandInt04096 = [&]() {
        for (int i = 0; i < 10; ++i) { // try to find non zero random value 10 times
            int n = getRandInt04096();
            if (n > 0) return n;
        }
        return 123; // unlikely to be here
    };

    //
    // keep original condition
    //
    std::vector<T> origPix(pixDim);
    for (int chanId = 0; chanId < pixDim; ++chanId) { origPix[chanId] = pixBuff[getDataOffset(chanId)]; }
    unsigned int origNumSample = numBuff[pixOffset];

    //
    // update value : 33.333% of pixels : only update pixel value
    //                33.333% of pixels : update pixel value and weight
    //                33.333% of pixels : only update weight
    //
    //                     0.0      0.333     0.666      1.0
    //   update pix(el)  -> |<------->|<------->|         |
    //   update w(eight) -> |         |<------->|<------->|
    //                          pix     pix + w      w                   
    float val = getRandReal01();
    if (val < 0.66667f) {
        pixDimLoop([&](const size_t dataOffset) {
                pixBuff[dataOffset] = updateVal(pixBuff[dataOffset]);
            });
    }
    if (val > 0.33333f) {
        numBuff[pixOffset] += non0RandInt04096(); // numSample value should be only increased and it never decreased
    }

    //
    // active pixel info update
    //
    auto activePixTest = [&]() {
        int currNumSample = numBuff[pixOffset];
        if (currNumSample == 0.0f) return false; // early exit : non active pixel because numSample is ZERO

        if (origNumSample != currNumSample) return true; // active pixel
        for (int chanId = 0; chanId < pixDim; ++chanId) {
            if (pixBuff[getDataOffset(chanId)] != origPix[chanId]) return true; // active pixel
        }
        return false; // non active pixel
    };
    return activePixTest();
}

template <typename T>
void
TestSnapshotUtil::copyPix(std::vector<T>& destBuff,
                          std::vector<float>& destWeight,
                          const std::vector<T>& srcBuff,
                          const std::vector<float>& srcWeight,
                          const int pixOffset,
                          const int pixDim) const
{
    size_t offset = pixOffset * pixDim;
    for (size_t chanId = 0; chanId < pixDim; ++chanId) {
        destBuff[offset] = srcBuff[offset];
        offset++;
    }
    destWeight[pixOffset] = srcWeight[pixOffset];
}

template <typename T>
void
TestSnapshotUtil::copyPix2(std::vector<T>& destBuff,
                           std::vector<unsigned int>& destNumBuff,
                           const std::vector<T>& srcBuff,
                           const std::vector<unsigned int>& srcNumBuff,
                           const int pixOffset,
                           const int pixDim) const
{
    size_t offset = pixOffset * pixDim;
    for (size_t chanId = 0; chanId < pixDim; ++chanId) {
        destBuff[offset] = srcBuff[offset];
        offset++;
    }
    destNumBuff[pixOffset] = srcNumBuff[pixOffset];
}

void
TestSnapshotUtil::updateBuff2(std::vector<uint64_t>& pixMaskBuff,
                              const std::function<bool(const int pixOffset)>& updatePixelFunc,
                              const std::function<void(const int pixOffset)>& updateTargetFunc,
                              std::vector<int>& updatePixIdArray) const
{
    for (size_t i = 0; i < pixMaskBuff.size(); ++i) {
        uint64_t currPixMask = pixMaskBuff[i];
        for (int j = 0; j < 64; ++j) {
            if ((currPixMask >> j) & (uint64_t)0x1) {
                int pixOffset = i * 64 + j;
                if (updatePixelFunc(pixOffset)) {
                    updatePixIdArray.push_back(pixOffset); // updated pixel
                    updateTargetFunc(pixOffset); // update target buff and weight data
                }
            }
        }
    }
}

template <typename T>
std::string
TestSnapshotUtil::analyzeBuff(const int pixDim,
                              const std::vector<T>& orgV,
                              const std::vector<float>& orgW,
                              const std::vector<T>& srcV,
                              const std::vector<float>& srcW) const
//
// show statistical information of generated test buffer data
//
{
    std::ostringstream ostr;

    if (orgV.size() != srcV.size() ||
        orgW.size() != srcW.size() ||
        (orgV.size() / pixDim) != orgW.size()) {
        ostr << "buffer size mismatch."
             << " pixDim:" << pixDim
             << " orgV.size():" << orgV.size()
             << " orgW.size():" << orgW.size()
             << " srcV.size():" << srcV.size()
             << " srcW.size():" << srcW.size() << '\n';
        return ostr.str();
    }

    auto isSameV = [&](const size_t pixId) {
        size_t pixOffset = pixId * pixDim;
        for (size_t chanId = 0; chanId < pixDim; ++chanId) {
            if (orgV[pixOffset] != srcV[pixOffset]) return false;
            pixOffset++;
        }
        return true;
    };
    auto isSameW = [&](const size_t pixId) {
        return (orgW[pixId] == srcW[pixId]);
    };

    size_t totalBothSame = 0;
    size_t totalOnlyVDiff = 0;
    size_t totalOnlyVDiffWZero = 0;
    size_t totalOnlyWDiff = 0;
    size_t totalBothDiff = 0;

    size_t pixTotal = orgV.size() / pixDim;
    for (size_t pixId = 0; pixId < pixTotal; ++pixId) {
        bool flagV = isSameV(pixId);
        bool flagW = isSameW(pixId);
        if (flagV && flagW) { totalBothSame++; }
        else if (!flagV && flagW) { if (srcW[pixId] == 0.0f) totalOnlyVDiffWZero++; else totalOnlyVDiff++; }
        else if (flagV && !flagW) { totalOnlyWDiff++; }
        else { totalBothDiff++; }
    }

    int w = scene_rdl2::str_util::getNumberOfDigits(pixTotal);

    auto showValAndPct = [&](const size_t v) {
        float pct = static_cast<float>(v) / static_cast<float>(pixTotal) * 100.0f;
        std::ostringstream ostr;
        ostr << std::setw(w) << v << ' ' << std::setw(10) << std::fixed << std::setprecision(5) << pct << '%';
        return ostr.str();
    };

    size_t total = totalBothSame + totalOnlyVDiff + totalOnlyVDiffWZero + totalOnlyWDiff + totalBothDiff;
    ostr << "analyzeBuff {\n"
         << "         pixDim      : " << std::setw(w) << pixDim << '\n'
         << "       pixTotal      : " << std::setw(w) << pixTotal << '\n'
         << "  both V&W Same      : " << showValAndPct(totalBothSame)  << '\n'
         << "    only V Diff(w!=0): " << showValAndPct(totalOnlyVDiff) << '\n'
         << "    only V Diff(w==0): " << showValAndPct(totalOnlyVDiffWZero) << '\n'
         << "    only W Diff      : " << showValAndPct(totalOnlyWDiff) << '\n'
         << "  both V&W Diff      : " << showValAndPct(totalBothDiff)  << '\n'
         << "          total      : " << showValAndPct(total) << '\n'
         << "}";  
    return ostr.str();
}

template <typename T>
std::string
TestSnapshotUtil::analyzeBuff2(const int pixDim,
                               const std::vector<T>& orgV,
                               const std::vector<unsigned int>& orgN,
                               const std::vector<T>& srcV,
                               const std::vector<unsigned int>& srcN) const
//
// show statistical information of generated test buffer data
//
{
    std::ostringstream ostr;

    if (orgV.size() != srcV.size() ||
        orgN.size() != srcN.size() ||
        (orgV.size() / pixDim) != orgN.size()) {
        ostr << "buffer size mismatch."
             << " pixDim:" << pixDim
             << " orgV.size():" << orgV.size()
             << " orgN.size():" << orgN.size()
             << " srcV.size():" << srcV.size()
             << " srcN.size():" << srcN.size() << '\n';
        return ostr.str();
    }

    auto isSameV = [&](const size_t pixId) {
        size_t pixOffset = pixId * pixDim;
        for (size_t chanId = 0; chanId < pixDim; ++chanId) {
            if (orgV[pixOffset] != srcV[pixOffset]) return false;
            pixOffset++;
        }
        return true;
    };
    auto isSameN = [&](const size_t pixId) {
        return (orgN[pixId] == srcN[pixId]);
    };

    size_t totalBothSame = 0;
    size_t totalOnlyVDiff = 0;
    size_t totalOnlyVDiffNZero = 0;
    size_t totalOnlyNDiff = 0;
    size_t totalBothDiff = 0;

    size_t pixTotal = orgV.size() / pixDim;
    for (size_t pixId = 0; pixId < pixTotal; ++pixId) {
        bool flagV = isSameV(pixId);
        bool flagN = isSameN(pixId);
        if (flagV && flagN) { totalBothSame++; }
        else if (!flagV && flagN) { if (srcN[pixId] == 0) totalOnlyVDiffNZero++; else totalOnlyVDiff++; }
        else if (flagV && !flagN) { totalOnlyNDiff++; }
        else { totalBothDiff++; }
    }

    int w = scene_rdl2::str_util::getNumberOfDigits(pixTotal);

    auto showValAndPct = [&](const size_t v) {
        float pct = static_cast<float>(v) / static_cast<float>(pixTotal) * 100.0f;
        std::ostringstream ostr;
        ostr << std::setw(w) << v << ' ' << std::setw(10) << std::fixed << std::setprecision(5) << pct << '%';
        return ostr.str();
    };

    size_t total = totalBothSame + totalOnlyVDiff + totalOnlyVDiffNZero + totalOnlyNDiff + totalBothDiff;
    ostr << "analyzeBuff2 {\n"
         << "         pixDim      : " << std::setw(w) << pixDim << '\n'
         << "       pixTotal      : " << std::setw(w) << pixTotal << '\n'
         << "  both V&N Same      : " << showValAndPct(totalBothSame)  << '\n'
         << "    only V Diff(n!=0): " << showValAndPct(totalOnlyVDiff) << '\n'
         << "    only V Diff(n==0): " << showValAndPct(totalOnlyVDiffNZero) << '\n'
         << "    only N Diff      : " << showValAndPct(totalOnlyNDiff) << '\n'
         << "  both V&N Diff      : " << showValAndPct(totalBothDiff)  << '\n'
         << "          total      : " << showValAndPct(total) << '\n'
         << "}";  
    return ostr.str();
}

std::string
TestSnapshotUtil::analyzeWeightBuff(const std::vector<float>& orgW,
                                    const std::vector<float>& srcW) const
{
    std::ostringstream ostr;

    if (orgW.size() != srcW.size()) {
        ostr << "buffer size mismatch."
             << " orgW.size():" << orgW.size()
             << " srcW.size():" << srcW.size() << '\n';
        return ostr.str();
    }

    auto isSameW = [&](const size_t pixId) {
        return (orgW[pixId] == srcW[pixId]);
    };

    size_t totalDiff = 0;
    size_t totalSame = 0;
    size_t totalSameWZero = 0;

    size_t pixTotal = orgW.size();
    for (size_t pixId = 0; pixId < pixTotal; ++pixId) {
        if (isSameW(pixId)) {
            if (srcW[pixId] == 0.0f) totalSameWZero++;
            else totalSame++;
        } else {
            totalDiff++;
        }
    }

    int w = scene_rdl2::str_util::getNumberOfDigits(pixTotal);

    auto showPct = [&](const size_t v) {
        float pct = static_cast<float>(v) / static_cast<float>(pixTotal) * 100.0f;
        std::ostringstream ostr;
        ostr << std::setw(w) << v << ' ' << std::setw(10) << std::fixed << std::setprecision(5) << pct << '%';
        return ostr.str();
    };

    ostr << "analyzeWeightBuff {\n"
         << "  pixTotal      : " << std::setw(w) << pixTotal << '\n'
         << "      diff      : " << showPct(totalDiff) << '\n'
         << "      same(w!=0): " << showPct(totalSame) << '\n'
         << "      same(w==0): " << showPct(totalSameWZero) << '\n'
         << "}";
    return ostr.str();
}

void
TestSnapshotUtil::snapshotTileLoop(const int w, // should be tile aligned resolution
                                   const int h, // should be tile aligned resolution
                                   std::vector<uint64_t> &pixMaskBuff,
                                   std::function<uint64_t(int offsetItem)> snapshotTileFunc) const
{
    for (int tileYId = 0; tileYId < h / sTileReso; ++tileYId) {
        for (int tileXId = 0; tileXId < w / sTileReso; ++tileXId) {
            int tileId = tileYId * w / sTileReso + tileXId;
            int offsetItem = tileId * sTileReso * sTileReso;
            pixMaskBuff[tileId] = snapshotTileFunc(offsetItem);
        }
    }
}

void
TestSnapshotUtil::snapshotTimingCompare(const int w, // should be tile aligned resolution
                                        const int h, // should be tile aligned resolution
                                        std::function<void()> resetDataFunc,
                                        const std::function<uint64_t(int offsetItem)>& snapshotTileFuncA,
                                        const std::function<uint64_t(int offsetItem)>& snapshotTileFuncB,
                                        const std::function<bool(std::vector<uint64_t> &)>& verifyFunc) const
{
#ifdef TIMING_TEST
    int timingTestLoopMax = 128; // for performance test
#else // else TIMING_TEST
    int timingTestLoopMax = 1;
#endif // end else TIMING_TEST

    int tileTotal = w / sTileReso * h / sTileReso;
    std::vector<uint64_t> pixMaskBuff(tileTotal, static_cast<uint64_t>(0x0));
    // std::cerr << showPixMaskBuff(pixMaskBuff) << std::endl; // useful for debug

    rec_time::RecTime recTime;

    float timeA = 0.0f;
    for (int i = 0; i < timingTestLoopMax; ++i) {
        resetDataFunc();
        recTime.start();
        {
            snapshotTileLoop(w, h, pixMaskBuff, snapshotTileFuncA);
        }
        timeA += recTime.end();
    }
    timeA /= static_cast<float>(timingTestLoopMax);
    CPPUNIT_ASSERT(verifyFunc(pixMaskBuff));

    pixMaskBuff.clear();
    pixMaskBuff.resize(tileTotal, static_cast<uint64_t>(0x0));
    float timeB = 0.0f;
    for (int i = 0; i < timingTestLoopMax; ++i) {
        resetDataFunc();
        recTime.start();
        {
            snapshotTileLoop(w, h, pixMaskBuff, snapshotTileFuncB);
        }
        timeB += recTime.end();
    }
    timeB /= static_cast<float>(timingTestLoopMax);
    CPPUNIT_ASSERT(verifyFunc(pixMaskBuff));

#ifdef TIMING_TEST  
    std::cerr << "timeA:" << timeA * 1000.0f << "ms (" << timeB / timeA << "x) "
              << "timeB:" << timeB * 1000.0f << "ms (" << timeA / timeB << "x)" << std::endl;
#endif // end TIMING_TEST
}

bool
TestSnapshotUtil::verifyPixMask(std::vector<int> &updateList, std::vector<uint64_t> &pixMaskBuff) const
{
    auto verifyMask = [](int tileId, const uint64_t &mask, std::vector<int> &updateList, int &total) -> bool {
        for (int yId = 0; yId < sTileReso; ++yId) {
            for (int xId = 0; xId < sTileReso; ++xId) {
                int shift = yId * sTileReso + xId;
                if (((uint64_t)(mask >> shift) & (uint64_t)0x1) == (uint64_t)0x1) {
                    int offset = tileId * sTileReso * sTileReso + yId * sTileReso + xId;
                    if (updateList[total] != offset) return false;
                    total++;
                }
            }
        }
        return true;
    };

    int total = 0;
    for (size_t tileId = 0; tileId < pixMaskBuff.size(); ++tileId) {
        if (pixMaskBuff[tileId]) {
            if (!verifyMask(tileId, pixMaskBuff[tileId], updateList, total)) {
                return false;
            }
        }
    }
    return true;
}

std::string
TestSnapshotUtil::showTile(int tileId, int offsetItem,
                           std::function<std::string(int offsetItem)> showItemFunction) const
{
    std::ostringstream ostr;
    ostr << "tileId:" << tileId << " {\n";
    for (int yId = sTileReso - 1; yId >= 0; --yId) {
        for (int xId = 0; xId < sTileReso; ++xId) {
            int offsetPix = yId * sTileReso + xId + offsetItem;
            ostr << "  xId:" << xId << " yId:" << yId << " " << showItemFunction(offsetPix) << '\n';
        }
    }
    ostr << "}";
    return ostr.str();
}

std::string
TestSnapshotUtil::showBuff(int w, // should be tile aligned resolution
                           int h, // should be tile aligned resolution
                           std::function<std::string(int offsetItem)> showItemFunction) const
{
    int tileTotal = w / sTileReso * h / sTileReso;

    std::ostringstream ostr;
    ostr << "showBuff {\n";
    for (int tileId = 0; tileId < tileTotal; ++tileId) {
        int offsetItem = tileId * sTileReso * sTileReso;
        ostr << str_util::addIndent(showTile(tileId, offsetItem, showItemFunction)) << '\n';
    }
    ostr << "}";
    return ostr.str();
}

std::string    
TestSnapshotUtil::showBuffColWeight(int w,
                                    int h,
                                    const std::vector<float> &cBuff,
                                    const std::vector<float> &wBuff) const
{
    return showBuff(w, h,
                    [&](unsigned offsetItem) -> std::string {
                        const float *cPtr = &cBuff[offsetItem * 4];
                        const float *wPtr = &wBuff[offsetItem];
                        std::ostringstream ostr;
                        ostr << "col("
                             << std::setw(10) << std::fixed << std::setprecision(8)
                             << cPtr[0] << ' ' << cPtr[1] << ' ' << cPtr[2] << ' ' << cPtr[3] << ") "
                             << "w:" << *wPtr;
                        return ostr.str();
                    });
}

std::string
TestSnapshotUtil::showPixMask(const uint64_t &pixMask) const
{
    std::ostringstream ostr;
    ostr << "pixMask {\n";
    for (int y = sTileReso - 1; y >= 0; --y) {
        ostr << "  " << y << ' ';
        for (int x = 0; x < sTileReso; ++x) {
            ostr << ((((pixMask >> (y * sTileReso + x)) & 0x1) == 0x1)? "* ": ". ");
        }
        ostr << '\n';
    }
    ostr << "   ";
    for (int x = 0; x < sTileReso; ++x) ostr << ' ' << x;
    ostr << "\n}";
    return ostr.str();
}

std::string
TestSnapshotUtil::showPixMaskBuff(const std::vector<uint64_t> &pixMaskBuff) const
{
    std::ostringstream ostr;
    ostr << "pixMask (total:" << pixMaskBuff.size() << ") {\n";
    for (unsigned i = 0; i < pixMaskBuff.size(); ++i) {
        ostr << "  i:" << i << " {\n"
             << str_util::addIndent(showPixMask(pixMaskBuff[i]), 2) << '\n'
             << "  }\n";
    }
    ostr << "}";
    return ostr.str();
}

std::string
TestSnapshotUtil::showUpdatePixIdArray(const std::vector<int> &updatePixIdArray) const
{
    std::ostringstream ostr;
    ostr << "updatePixIdArray (total:" << updatePixIdArray.size() << ") {\n";
    for (unsigned i = 0; i < updatePixIdArray.size(); ++i) {
        ostr << "  i:" << i << ' ' << updatePixIdArray[i] << '\n';
    }
    ostr << "}";
    return ostr.str();
}

} // namespace unittest
} // namespace fb_util
} // namespace scene_rdl2
