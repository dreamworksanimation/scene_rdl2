// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "TestSnapshotUtil.h"
#include <scene_rdl2/common/fb_util/SnapshotUtil.h>

#include <scene_rdl2/common/rec_time/RecTime.h>
#include <scene_rdl2/render/util/StrUtil.h>

#include <algorithm>
#include <functional>
#include <random>
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
    int updatePixTotal = 4096;

    std::vector<double> orgV(w * h);
    std::vector<float> orgW(w * h);
    setupBuff(orgV);
    setupBuff(orgW);

    std::vector<double> dstV(orgV);
    std::vector<float> dstW(orgW);
    std::vector<double> srcV(orgV);
    std::vector<float> srcW(orgW);
    std::vector<int> updateList;
    updateBuff(updatePixTotal, w, h,
               [&](const int offset) { // updatePixelFunc
                   srcV[offset] *= 0.5;
                   srcW[offset] *= 0.5f;
               },
               updateList);

#ifdef TIMING_TEST
    int timingTestLoopMax = 128; // for performance test
    bool doCompare = true;       // for performance test
#else // else TIMING_TEST
    int timingTestLoopMax = 1;
    bool doCompare = false;
#endif // end !TIMING_TEST
    snapshotTimingCompare
        (w, h, timingTestLoopMax, doCompare,
         [&]() { // resetDataFunc
            dstV = orgV;
            dstW = orgW;
         },
         [&](int offsetItem) -> uint64_t { // snapshotTileFuncA
            return fb_util::SnapshotUtil::snapshotTileHeatMapWeight
                ((uint64_t *)((uintptr_t)(&dstV[0]) + offsetItem * sizeof(double)),
                 (uint32_t *)((uintptr_t)(&dstW[0]) + offsetItem * sizeof(float)),
                 (uint64_t *)((uintptr_t)(&srcV[0]) + offsetItem * sizeof(double)),
                 (uint32_t *)((uintptr_t)(&srcW[0]) + offsetItem * sizeof(float)));
         },
         [&](int offsetItem) -> uint64_t { // snapshotTileFuncB
            return fb_util::SnapshotUtil::snapshotTileHeatMapWeight_SISD
                ((uint64_t *)((uintptr_t)(&dstV[0]) + offsetItem * sizeof(double)),
                 (uint32_t *)((uintptr_t)(&dstW[0]) + offsetItem * sizeof(float)),
                 (uint64_t *)((uintptr_t)(&srcV[0]) + offsetItem * sizeof(double)),
                 (uint32_t *)((uintptr_t)(&srcW[0]) + offsetItem * sizeof(float)));
         },
         [&](std::vector<uint64_t> &pixMaskBuff) -> bool { // verifyFunc
             return verifyPixMask(updateList, pixMaskBuff) && (dstV == srcV) && (dstW == srcW);
         });
}

void
TestSnapshotUtil::testWeight()
{
    int w = sTileReso * 240; // = 1920
    int h = sTileReso * 135; // = 1080
    int updatePixTotal = 4096;

    std::vector<float> orgW(w * h);
    setupBuff(orgW);

    std::vector<float> dstW(orgW);
    std::vector<float> srcW(orgW);
    std::vector<int> updateList;
    updateBuff(updatePixTotal, w, h,
               [&](const int offset) { // updatePixelFunc
                   srcW[offset] *= 0.5f;
               },
               updateList);

#ifdef TIMING_TEST
    int timingTestLoopMax = 128; // for performance test
    bool doCompare = true;       // for performance test
#else // else TIMING_TEST
    int timingTestLoopMax = 1;
    bool doCompare = false;
#endif // end !TIMING_TEST
    snapshotTimingCompare
        (w, h, timingTestLoopMax, doCompare,
         [&]() { // resetDataFunc
            dstW = orgW;
         },
         [&](int offsetItem) -> uint64_t { // snapshotTileFuncA
            return fb_util::SnapshotUtil::snapshotTileWeightBuffer
                ((uint32_t *)((uintptr_t)(&dstW[0]) + offsetItem * sizeof(float)),
                 (uint32_t *)((uintptr_t)(&srcW[0]) + offsetItem * sizeof(float)));
         },
         [&](int offsetItem) -> uint64_t { // snapshotTileFuncB
            return fb_util::SnapshotUtil::snapshotTileWeightBuffer_SISD
                ((uint32_t *)((uintptr_t)(&dstW[0]) + offsetItem * sizeof(float)),
                 (uint32_t *)((uintptr_t)(&srcW[0]) + offsetItem * sizeof(float)));
         },
         [&](std::vector<uint64_t> &pixMaskBuff) -> bool { // verifyFunc
             return verifyPixMask(updateList, pixMaskBuff) && (dstW == srcW);
         });
}

void
TestSnapshotUtil::testWeightMask()
{
    int w = sTileReso * 240; // = 1920
    int h = sTileReso * 135; // = 1080
    int tileTotal = w / sTileReso * h / sTileReso;

    std::vector<float> orgV(w * h);
    std::vector<uint64_t> dstPixMaskBuff(tileTotal);
    setupBuff(orgV);
    setupPixMaskBuff(0.2, // 20% empty
                     0.2, // 20% full
                     dstPixMaskBuff);

    std::vector<float> dstV(orgV);

    std::vector<float> srcV(orgV);
    std::vector<uint64_t> srcPixMaskBuff(tileTotal);
    setupPixMaskBuff(0.3, // 30% empty
                     0.1, // 10% full
                     srcPixMaskBuff);
    std::vector<int> updateList;
    updateBuff(srcPixMaskBuff,
               [&](const int offset) {
                   srcV[offset] *= 0.5f;
               },
               updateList);

#ifdef TIMING_TEST
    int timingTestLoopMax = 128; // for performance test
    bool doCompare = true;       // for performance test
#else // else TIMING_TEST
    int timingTestLoopMax = 1;
    bool doCompare = false;
#endif // end !TIMING_TEST
    snapshotTimingCompare
        (w, h, timingTestLoopMax, doCompare,
         [&]() {
            dstV = orgV;
         },
         [&](int offsetItem) -> uint64_t { // snapshotTileFuncA
            int tileId = offsetItem / (sTileReso * sTileReso);
            return fb_util::SnapshotUtil::snapshotTileWeightBuffer
                ((uint32_t *)((uintptr_t)&dstV[0] + offsetItem * sizeof(float)),
                 dstPixMaskBuff[tileId],
                 (uint32_t *)((uintptr_t)&srcV[0] + offsetItem * sizeof(float)),
                 srcPixMaskBuff[tileId]);
         },
         [&](int offsetItem) -> uint64_t { // snapshotTileFuncB
            int tileId = offsetItem / (sTileReso * sTileReso);
            return fb_util::SnapshotUtil::snapshotTileWeightBuffer_SISD
                ((uint32_t *)((uintptr_t)&dstV[0] + offsetItem * sizeof(float)),
                 dstPixMaskBuff[tileId],
                 (uint32_t *)((uintptr_t)&srcV[0] + offsetItem * sizeof(float)),
                 srcPixMaskBuff[tileId]);
         },
         [&](std::vector<uint64_t> &pixMaskBuff) -> bool { // verifyFunc
             return verifyPixMask(updateList, pixMaskBuff) && (dstV == srcV);
         });
}

void
TestSnapshotUtil::testFloatWeight()
{
    int w = sTileReso * 240; // = 1920
    int h = sTileReso * 135; // = 1080
    int updatePixTotal = 4096;

    std::vector<float> orgV(w * h), orgW(w * h);
    setupBuff(orgV);
    setupBuff(orgW);

    std::vector<float> dstV(orgV), dstW(orgW);
    std::vector<float> srcV(dstV), srcW(orgW);
    std::vector<int> updateList;
    updateBuff(updatePixTotal, w, h,
               [&](const int offset) { // updatePixelFunc
                   srcV[offset] *= 0.5f;
                   srcW[offset] *= 0.5f;
               },
               updateList);

#ifdef TIMING_TEST
    int timingTestLoopMax = 128; // for performance test
    bool doCompare = true;       // for performance test
#else // else TIMING_TEST
    int timingTestLoopMax = 1;
    bool doCompare = false;
#endif // end !TIMING_TEST
    snapshotTimingCompare
        (w, h, timingTestLoopMax, doCompare,
         [&]() { // resetDataFunc
            dstV = orgV;
            dstW = orgW;
         },
         [&](int offsetItem) -> uint64_t { // snapshotTileFuncA
            return fb_util::SnapshotUtil::snapshotTileFloatWeight
                ((uint32_t *)((uintptr_t)(&dstV[0]) + offsetItem * sizeof(float)),
                 (uint32_t *)((uintptr_t)(&dstW[0]) + offsetItem * sizeof(float)),
                 (uint32_t *)((uintptr_t)(&srcV[0]) + offsetItem * sizeof(float)),
                 (uint32_t *)((uintptr_t)(&srcW[0]) + offsetItem * sizeof(float)));
         },
         [&](int offsetItem) -> uint64_t { // snapshotTileFuncB
            return fb_util::SnapshotUtil::snapshotTileFloatWeight_SISD
                ((uint32_t *)((uintptr_t)(&dstV[0]) + offsetItem * sizeof(float)),
                 (uint32_t *)((uintptr_t)(&dstW[0]) + offsetItem * sizeof(float)),
                 (uint32_t *)((uintptr_t)(&srcV[0]) + offsetItem * sizeof(float)),
                 (uint32_t *)((uintptr_t)(&srcW[0]) + offsetItem * sizeof(float)));
         },
         [&](std::vector<uint64_t> &pixMaskBuff) -> bool { // verifyFunc
             return verifyPixMask(updateList, pixMaskBuff) && (dstV == srcV) && (dstW == srcW);
         });
}
void
TestSnapshotUtil::testFloatNumSample()
{
    int w = sTileReso * 240; // = 1920
    int h = sTileReso * 135; // = 1080
    int tileTotal = w / sTileReso * h / sTileReso;

    std::vector<float> orgV(w * h);
    std::vector<unsigned int> orgN(w * h);
    std::vector<uint64_t> dstPixMaskBuff(tileTotal);
    setupBuff(orgV);
    setupNumBuff(orgN);
    setupPixMaskBuff(0.2, // 20% empty
                     0.2, // 20% full
                     dstPixMaskBuff);

    std::vector<float> dstV(orgV);
    std::vector<unsigned int> dstN(orgN);

    std::vector<float> srcV(orgV);
    std::vector<unsigned int> srcN(orgN);
    std::vector<uint64_t> srcPixMaskBuff(tileTotal);
    setupPixMaskBuff(0.3, // 30% empty
                     0.1, // 10% full
                     srcPixMaskBuff);
    std::vector<int> updateList;
    updateBuff(srcPixMaskBuff,
               [&](const int offset) {
                   srcV[offset] *= 0.5f;
                   srcN[offset] += 123;
               },
               updateList);

#ifdef TIMING_TEST
    int timingTestLoopMax = 128; // for performance test
    bool doCompare = true;       // for performance test
#else // else TIMING_TEST
    int timingTestLoopMax = 1;
    bool doCompare = false;
#endif // end !TIMING_TEST
    snapshotTimingCompare
        (w, h, timingTestLoopMax, doCompare,
         [&]() {
            dstV = orgV;
            dstN = orgN;
         },
         [&](int offsetItem) -> uint64_t { // snapshotTileFuncA
            int tileId = offsetItem / (sTileReso * sTileReso);
            return fb_util::SnapshotUtil::snapshotTileFloatNumSample
                ((uint32_t *)((uintptr_t)&dstV[0] + offsetItem * sizeof(float)),
                 (uint32_t *)((uintptr_t)&dstN[0] + offsetItem * sizeof(unsigned int)),
                 dstPixMaskBuff[tileId],
                 (uint32_t *)((uintptr_t)&srcV[0] + offsetItem * sizeof(float)),
                 (uint32_t *)((uintptr_t)&srcN[0] + offsetItem * sizeof(unsigned int)),
                 srcPixMaskBuff[tileId]);
         },
         [&](int offsetItem) -> uint64_t { // snapshotTileFuncB
            int tileId = offsetItem / (sTileReso * sTileReso);
            return fb_util::SnapshotUtil::snapshotTileFloatNumSample_SISD
                ((uint32_t *)((uintptr_t)&dstV[0] + offsetItem * sizeof(float)),
                 (uint32_t *)((uintptr_t)&dstN[0] + offsetItem * sizeof(unsigned int)),
                 dstPixMaskBuff[tileId],
                 (uint32_t *)((uintptr_t)&srcV[0] + offsetItem * sizeof(float)),
                 (uint32_t *)((uintptr_t)&srcN[0] + offsetItem * sizeof(unsigned int)),
                 srcPixMaskBuff[tileId]);
         },
         [&](std::vector<uint64_t> &pixMaskBuff) -> bool { // verifyFunc
             return verifyPixMask(updateList, pixMaskBuff) && (dstV == srcV) && (dstN == srcN);
         });
}

void
TestSnapshotUtil::testFloat2Weight()
{
    int w = sTileReso * 240; // = 1920
    int h = sTileReso * 135; // = 1080
    int updatePixTotal = 4096;

    std::vector<float> orgV(w * h * 2), orgW(w * h);
    setupBuff(orgV);
    setupBuff(orgW);

    std::vector<float> dstV(orgV), dstW(orgW);
    std::vector<float> srcV(orgV), srcW(orgW);
    std::vector<int> updateList;
    updateBuff(updatePixTotal, w, h,
               [&](const int offset) { // updatePixelFunc
                   float *vPtr = &srcV[offset * 2];
                   vPtr[0] *= 0.5f; vPtr[1] *= 0.5f;
                   srcW[offset] *= 0.5f;
               },
               updateList);

#ifdef TIMING_TEST
    int timingTestLoopMax = 128; // for performance test
    bool doCompare = true;       // for performance test
#else // else TIMING_TEST
    int timingTestLoopMax = 1;
    bool doCompare = false;
#endif // end !TIMING_TEST
    snapshotTimingCompare
        (w, h, timingTestLoopMax, doCompare,
         [&]() { // resetDataFunc
            dstV = orgV;
            dstW = orgW;
         },
         [&](int offsetItem) -> uint64_t { // snapshotTileFuncA
            return fb_util::SnapshotUtil::snapshotTileFloat2Weight
                ((uint32_t *)((uintptr_t)(&dstV[0]) + offsetItem * sizeof(float) * 2),
                 (uint32_t *)((uintptr_t)(&dstW[0]) + offsetItem * sizeof(float)),
                 (uint32_t *)((uintptr_t)(&srcV[0]) + offsetItem * sizeof(float) * 2),
                 (uint32_t *)((uintptr_t)(&srcW[0]) + offsetItem * sizeof(float)));
         },
         [&](int offsetItem) -> uint64_t { // snapshotTileFuncB
            return fb_util::SnapshotUtil::snapshotTileFloat2Weight_SISD
                ((uint32_t *)((uintptr_t)(&dstV[0]) + offsetItem * sizeof(float) * 2),
                 (uint32_t *)((uintptr_t)(&dstW[0]) + offsetItem * sizeof(float)),
                 (uint32_t *)((uintptr_t)(&srcV[0]) + offsetItem * sizeof(float) * 2),
                 (uint32_t *)((uintptr_t)(&srcW[0]) + offsetItem * sizeof(float)));
         },
         [&](std::vector<uint64_t> &pixMaskBuff) -> bool { // verifyFunc
             return verifyPixMask(updateList, pixMaskBuff) && (dstV == srcV) && (dstW == srcW);
         });
}

void
TestSnapshotUtil::testFloat2NumSample()
{
    int w = sTileReso * 240; // = 1920
    int h = sTileReso * 135; // = 1080
    int tileTotal = w / sTileReso * h / sTileReso;

    std::vector<float> orgV(w * h * 2);
    std::vector<unsigned int> orgN(w * h);
    std::vector<uint64_t> dstPixMaskBuff(tileTotal);
    setupBuff(orgV);
    setupNumBuff(orgN);
    setupPixMaskBuff(0.2, // 20% empty
                     0.2, // 20% full
                     dstPixMaskBuff);

    std::vector<float> dstV(orgV);
    std::vector<unsigned int> dstN(orgN);
        
    std::vector<float> srcV(orgV);
    std::vector<unsigned int> srcN(orgN);
    std::vector<uint64_t> srcPixMaskBuff(tileTotal);
    setupPixMaskBuff(0.3, // 30% empty
                     0.1, // 10% full
                     srcPixMaskBuff);
    std::vector<int> updateList;
    updateBuff(srcPixMaskBuff,
               [&](const int offset) {
                   float *vPtr = &srcV[offset * 2];
                   vPtr[0] *= 0.5f; vPtr[1] *= 0.5f;
                   srcN[offset] += 123;
               },
               updateList);

#ifdef TIMING_TEST
    int timingTestLoopMax = 128; // for performance test
    bool doCompare = true;       // for performance test
#else // else TIMING_TEST
    int timingTestLoopMax = 1;
    bool doCompare = false;
#endif // end !TIMING_TEST
    snapshotTimingCompare
        (w, h, timingTestLoopMax, doCompare,
         [&]() { // resetDataFunc
            dstV = orgV;
            dstN = orgN;
         },
         [&](int offsetItem) -> uint64_t { // snapshotTileFuncA
            int tileId = offsetItem / (sTileReso * sTileReso);
            return fb_util::SnapshotUtil::snapshotTileFloat2NumSample
                ((uint32_t *)((uintptr_t)&dstV[0] + offsetItem * sizeof(float) * 2),
                 (uint32_t *)((uintptr_t)&dstN[0] + offsetItem * sizeof(unsigned int)),
                 dstPixMaskBuff[tileId],
                 (uint32_t *)((uintptr_t)&srcV[0] + offsetItem * sizeof(float) * 2),
                 (uint32_t *)((uintptr_t)&srcN[0] + offsetItem * sizeof(unsigned int)),
                 srcPixMaskBuff[tileId]);
         },
         [&](int offsetItem) -> uint64_t { // snapshotTileFuncB
            int tileId = offsetItem / (sTileReso * sTileReso);
            return fb_util::SnapshotUtil::snapshotTileFloat2NumSample_SISD
                ((uint32_t *)((uintptr_t)&dstV[0] + offsetItem * sizeof(float) * 2),
                 (uint32_t *)((uintptr_t)&dstN[0] + offsetItem * sizeof(unsigned int)),
                 dstPixMaskBuff[tileId],
                 (uint32_t *)((uintptr_t)&srcV[0] + offsetItem * sizeof(float) * 2),
                 (uint32_t *)((uintptr_t)&srcN[0] + offsetItem * sizeof(unsigned int)),
                 srcPixMaskBuff[tileId]);
         },
         [&](std::vector<uint64_t> &pixMaskBuff) -> bool { // verifyFunc
             return verifyPixMask(updateList, pixMaskBuff) && (dstV == srcV) && (dstN == srcN);
         });
}

void
TestSnapshotUtil::testFloat3Weight()
{
    int w = sTileReso * 240; // = 1920
    int h = sTileReso * 135; // = 1080
    int updatePixTotal = 4096;

    std::vector<float> orgV(w * h * 3), orgW(w * h);
    setupBuff(orgV);
    setupBuff(orgW);

    std::vector<float> dstV(orgV), dstW(orgW);
    std::vector<float> srcV(orgV), srcW(orgW);
    std::vector<int> updateList;
    updateBuff(updatePixTotal, w, h,
               [&](const int offset) { // updatePixelFunc
                   float *vPtr = &srcV[offset * 3];
                   vPtr[0] *= 0.5f; vPtr[1] *= 0.5f; vPtr[2] *= 0.5f;
                   srcW[offset] *= 0.5f;
               },
               updateList);

#ifdef TIMING_TEST
    int timingTestLoopMax = 128; // for performance test
    bool doCompare = true;       // for performance test
#else // else TIMING_TEST
    int timingTestLoopMax = 1;
    bool doCompare = false;
#endif // end !TIMING_TEST
    snapshotTimingCompare
        (w, h, timingTestLoopMax, doCompare,
         [&]() { // resetDataFunc
            dstV = orgV;
            dstW = orgW;
         },
         [&](int offsetItem) -> uint64_t { // snapshotTileFuncA
            return fb_util::SnapshotUtil::snapshotTileFloat3Weight
                ((uint32_t *)((uintptr_t)(&dstV[0]) + offsetItem * sizeof(float) * 3),
                 (uint32_t *)((uintptr_t)(&dstW[0]) + offsetItem * sizeof(float)),
                 (uint32_t *)((uintptr_t)(&srcV[0]) + offsetItem * sizeof(float) * 3),
                 (uint32_t *)((uintptr_t)(&srcW[0]) + offsetItem * sizeof(float)));
         },
         [&](int offsetItem) -> uint64_t { // snapshotTileFuncB
            return fb_util::SnapshotUtil::snapshotTileFloat3Weight_SISD
                ((uint32_t *)((uintptr_t)(&dstV[0]) + offsetItem * sizeof(float) * 3),
                 (uint32_t *)((uintptr_t)(&dstW[0]) + offsetItem * sizeof(float)),
                 (uint32_t *)((uintptr_t)(&srcV[0]) + offsetItem * sizeof(float) * 3),
                 (uint32_t *)((uintptr_t)(&srcW[0]) + offsetItem * sizeof(float)));
         },
         [&](std::vector<uint64_t> &pixMaskBuff) -> bool { // verifyFunc
             return verifyPixMask(updateList, pixMaskBuff) && (dstV == srcV) && (dstW == srcW);
         });
}

void
TestSnapshotUtil::testFloat3NumSample()
{
    int w = sTileReso * 240; // = 1920
    int h = sTileReso * 135; // = 1080
    int tileTotal = w / sTileReso * h / sTileReso;

    std::vector<float> orgV(w * h * 3);
    std::vector<unsigned int> orgN(w * h);
    std::vector<uint64_t> dstPixMaskBuff(tileTotal);
    setupBuff(orgV);
    setupNumBuff(orgN);
    setupPixMaskBuff(0.2, // 20% empty
                     0.2, // 20% full
                     dstPixMaskBuff);

    std::vector<float> dstV(orgV);
    std::vector<unsigned int> dstN(orgN);
        
    std::vector<float> srcV(orgV);
    std::vector<unsigned int> srcN(orgN);
    std::vector<uint64_t> srcPixMaskBuff(tileTotal);
    setupPixMaskBuff(0.3, // 30% empty
                     0.1, // 10% full
                     srcPixMaskBuff);
    std::vector<int> updateList;
    updateBuff(srcPixMaskBuff,
               [&](const int offset) {
                   float *vPtr = &srcV[offset * 3];
                   vPtr[0] *= 0.5f; vPtr[1] *= 0.5f; vPtr[2] *= 0.5f;
                   srcN[offset] += 123;
               },
               updateList);

#ifdef TIMING_TEST
    int timingTestLoopMax = 128; // for performance test
    bool doCompare = true;       // for performance test
#else // else TIMING_TEST
    int timingTestLoopMax = 1;
    bool doCompare = false;
#endif // end !TIMING_TEST
    snapshotTimingCompare
        (w, h, timingTestLoopMax, doCompare,
         [&]() { // resetDataFunc
            dstV = orgV;
            dstN = orgN;
         },
         [&](int offsetItem) -> uint64_t { // snapshotTileFuncA
            int tileId = offsetItem / (sTileReso * sTileReso);
            return fb_util::SnapshotUtil::snapshotTileFloat3NumSample
                ((uint32_t *)((uintptr_t)&dstV[0] + offsetItem * sizeof(float) * 3),
                 (uint32_t *)((uintptr_t)&dstN[0] + offsetItem * sizeof(unsigned int)),
                 dstPixMaskBuff[tileId],
                 (uint32_t *)((uintptr_t)&srcV[0] + offsetItem * sizeof(float) * 3),
                 (uint32_t *)((uintptr_t)&srcN[0] + offsetItem * sizeof(unsigned int)),
                 srcPixMaskBuff[tileId]);
         },
         [&](int offsetItem) -> uint64_t { // snapshotTileFuncB
            int tileId = offsetItem / (sTileReso * sTileReso);
            return fb_util::SnapshotUtil::snapshotTileFloat3NumSample_SISD
                ((uint32_t *)((uintptr_t)&dstV[0] + offsetItem * sizeof(float) * 3),
                 (uint32_t *)((uintptr_t)&dstN[0] + offsetItem * sizeof(unsigned int)),
                 dstPixMaskBuff[tileId],
                 (uint32_t *)((uintptr_t)&srcV[0] + offsetItem * sizeof(float) * 3),
                 (uint32_t *)((uintptr_t)&srcN[0] + offsetItem * sizeof(unsigned int)),
                 srcPixMaskBuff[tileId]);
         },
         [&](std::vector<uint64_t> &pixMaskBuff) -> bool { // verifyFunc
             return verifyPixMask(updateList, pixMaskBuff) && (dstV == srcV) && (dstN == srcN);
         });
}

void
TestSnapshotUtil::testFloat4Weight()
{
    int w = sTileReso * 240; // = 1920;
    int h = sTileReso * 135; // = 1080;
    int updatePixTotal = 4096;

    std::vector<float> orgV(w * h * 4), orgW(w * h);
    setupBuff(orgV);
    setupBuff(orgW);
    
    std::vector<float> dstV(orgV), dstW(orgW);
    std::vector<float> srcV(orgV), srcW(orgW);
    std::vector<int> updateList;
    updateBuff(updatePixTotal, w, h,
               [&](const int offset) { // updatePixelFunc
                   float *vPtr = &srcV[offset * 4];
                   vPtr[0] *= 0.5f; vPtr[1] *= 0.5f; vPtr[2] *= 0.5f; vPtr[3] *= 0.5f;
                   srcW[offset] *= 0.5f;
               },
               updateList);
    // std::cerr << showBuffColWeight(w, h, dstV, dstW) << std::endl; // useful for debug
    // std::cerr << "updateTotal:" << updateTotal << " " << showBuffColWeight(w, h, srcV, srcW) << std::endl;
    
#ifdef TIMING_TEST
    int timingTestLoopMax = 128; // for performance test
    bool doCompare = true;       // for performance test
#else // else TIMING_TEST
    int timingTestLoopMax = 1;
    bool doCompare = false;
#endif // end !TIMING_TEST
    snapshotTimingCompare
        (w, h, timingTestLoopMax, doCompare,
         [&]() { // resetDataFunc
            dstV = orgV;
            dstW = orgW;
         },
         [&](int offsetItem) -> uint64_t { // snapshotTileFuncA
            return fb_util::SnapshotUtil::snapshotTileFloat4Weight
                ((uint32_t *)((uintptr_t)(&dstV[0]) + offsetItem * sizeof(float) * 4), // RGBA
                 (uint32_t *)((uintptr_t)(&dstW[0]) + offsetItem * sizeof(float)),
                 (uint32_t *)((uintptr_t)(&srcV[0]) + offsetItem * sizeof(float) * 4), // RGBA
                 (uint32_t *)((uintptr_t)(&srcW[0]) + offsetItem * sizeof(float)));
         },
         [&](int offsetItem) -> uint64_t { // snapshotTileFuncB
             return fb_util::SnapshotUtil::snapshotTileFloat4Weight_SISD
                 ((uint32_t *)((uintptr_t)(&dstV[0]) + offsetItem * sizeof(float) * 4), // RGBA
                  (uint32_t *)((uintptr_t)(&dstW[0]) + offsetItem * sizeof(float)),
                  (uint32_t *)((uintptr_t)(&srcV[0]) + offsetItem * sizeof(float) * 4), // RGBA
                  (uint32_t *)((uintptr_t)(&srcW[0]) + offsetItem * sizeof(float)));
         },
         [&](std::vector<uint64_t> &pixMaskBuff) -> bool { // verifyFunc
             return verifyPixMask(updateList, pixMaskBuff) && (dstV == srcV) && (dstW == srcW);
         });
}

void
TestSnapshotUtil::testFloat4NumSample()
{
    int w = sTileReso * 240; // = 1920
    int h = sTileReso * 135; // = 1080
    int tileTotal = w / sTileReso * h / sTileReso;

    std::vector<float> orgV(w * h * 4);
    std::vector<unsigned int> orgN(w * h);
    std::vector<uint64_t> dstPixMaskBuff(tileTotal);
    setupBuff(orgV);
    setupNumBuff(orgN);
    setupPixMaskBuff(0.2, // 20% empty
                     0.2, // 20% full
                     dstPixMaskBuff);

    std::vector<float> dstV(orgV);
    std::vector<unsigned int> dstN(orgN);
        
    std::vector<float> srcV(orgV);
    std::vector<unsigned int> srcN(orgN);
    std::vector<uint64_t> srcPixMaskBuff(tileTotal);
    setupPixMaskBuff(0.3, // 30% empty
                     0.1, // 10% full
                     srcPixMaskBuff);
    std::vector<int> updateList;
    updateBuff(srcPixMaskBuff,
               [&](const int offset) {
                   float *vPtr = &srcV[offset * 4];
                   vPtr[0] *= 0.5f; vPtr[1] *= 0.5f; vPtr[2] *= 0.5f; vPtr[3] *= 0.5f;
                   srcN[offset] += 123;
               },
               updateList);

#ifdef TIMING_TEST
    int timingTestLoopMax = 128; // for performance test
    bool doCompare = true;       // for performance test
#else // else TIMING_TEST
    int timingTestLoopMax = 1;
    bool doCompare = false;
#endif // end !TIMING_TEST
    snapshotTimingCompare
        (w, h, timingTestLoopMax, doCompare,
         [&]() {
            dstV = orgV;
            dstN = orgN;
         },
         [&](int offsetItem) -> uint64_t { // snapshotTileFuncA
            int tileId = offsetItem / (sTileReso * sTileReso);
            return fb_util::SnapshotUtil::snapshotTileFloat4NumSample
                ((uint32_t *)((uintptr_t)&dstV[0] + offsetItem * sizeof(float) * 4),
                 (uint32_t *)((uintptr_t)&dstN[0] + offsetItem * sizeof(unsigned int)),
                 dstPixMaskBuff[tileId],
                 (uint32_t *)((uintptr_t)&srcV[0] + offsetItem * sizeof(float) * 4),
                 (uint32_t *)((uintptr_t)&srcN[0] + offsetItem * sizeof(unsigned int)),
                 srcPixMaskBuff[tileId]);
         },
         [&](int offsetItem) -> uint64_t { // snapshotTileFuncB
            int tileId = offsetItem / (sTileReso * sTileReso);
            return fb_util::SnapshotUtil::snapshotTileFloat4NumSample_SISD
                ((uint32_t *)((uintptr_t)&dstV[0] + offsetItem * sizeof(float) * 4),
                 (uint32_t *)((uintptr_t)&dstN[0] + offsetItem * sizeof(unsigned int)),
                 dstPixMaskBuff[tileId],
                 (uint32_t *)((uintptr_t)&srcV[0] + offsetItem * sizeof(float) * 4),
                 (uint32_t *)((uintptr_t)&srcN[0] + offsetItem * sizeof(unsigned int)),
                 srcPixMaskBuff[tileId]);
         },
         [&](std::vector<uint64_t> &pixMaskBuff) -> bool { // verifyFunc
             return verifyPixMask(updateList, pixMaskBuff) && (dstV == srcV) && (dstN == srcN);
         });
}

//------------------------------------------------------------------------------------------

template <typename T>
void
TestSnapshotUtil::setupBuff(std::vector<T> &buff) const
{
    std::random_device rnd;
    std::mt19937 mt(rnd());
    std::uniform_real_distribution<> rand01(0.001f, 0.999f);

    for (size_t i = 0; i < buff.size(); ++i) {
        buff[i] = (T)rand01(mt);
    }
}

void
TestSnapshotUtil::setupNumBuff(std::vector<unsigned int> &nBuff) const
{
    std::random_device rnd;
    std::mt19937 mt(rnd());
    std::uniform_int_distribution<> randNum(0, 4096);

    for (size_t i = 0; i < nBuff.size(); ++i) {
        nBuff[i] = randNum(mt);
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
TestSnapshotUtil::updateBuff(const int updateTotal,
                             const int w, // should be tile aligned resolution
                             const int h, // should be tile aligned resolution
                             std::function<void(const int offset)> updatePixelFunc,
                             std::vector<int> &updateList) const
{
    int totalTile = w / sTileReso * h / sTileReso;

    std::random_device rnd;
    std::mt19937 mt(rnd());
    std::uniform_int_distribution<> randTileId(0, totalTile - 1);
    std::uniform_int_distribution<> randXY(0, sTileReso - 1);

    auto randomPixel = [&]() -> int {
        // randomly pick single pixel position
        int offset = 0;
        while (1) { // ignore duplicate pixel position
            int tileId = randTileId(mt); // tileId
            int xPixId = randXY(mt);     // pixel X position inside tile
            int yPixId = randXY(mt);     // pixel Y position inside tile

            offset = tileId * sTileReso * sTileReso + yPixId * sTileReso + xPixId;
            if (std::find(updateList.begin(), updateList.end(), offset) == updateList.end()) break;
        }
        updateList.push_back(offset); // store offset value inside updateList for duplicate check and verify logic 
        return offset;
    };

    for (int i = 0; i < updateTotal; ++i) {
        if (updateList.size() >= (size_t)(w * h)) break; // escape if all the pixels are updated
        updatePixelFunc(randomPixel());
    }

    std::sort(updateList.begin(), updateList.end()); // sort updateList for verify operation
}

void
TestSnapshotUtil::updateBuff(std::vector<uint64_t> &pixMaskBuff,
                             std::function<void(const int offset)> updatePixelFunc,
                             std::vector<int> &updateList) const
{
    for (size_t i = 0; i < pixMaskBuff.size(); ++i) {
        uint64_t currPixMask = pixMaskBuff[i];
        for (int j = 0; j < 64; ++j) {
            if ((currPixMask >> j) & (uint64_t)0x1) {
                int offset = i * 64 + j;
                updatePixelFunc(offset);
                updateList.push_back(offset);
            }
        }
    }
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
                                        const int timingTestLoopMax,
                                        const bool doCompare,
                                        std::function<void()> resetDataFunc,
                                        std::function<uint64_t(int offsetItem)> snapshotTileFuncA,
                                        std::function<uint64_t(int offsetItem)> snapshotTileFuncB,
                                        std::function<bool(std::vector<uint64_t> &)> verifyFunc) const
{
    int tileTotal = w / sTileReso * h / sTileReso;
    std::vector<uint64_t> pixMaskBuff(tileTotal, (uint64_t)0x0);
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
    timeA /= (float)timingTestLoopMax;
    CPPUNIT_ASSERT(verifyFunc(pixMaskBuff));

    if (!doCompare) return;

    pixMaskBuff.clear();
    pixMaskBuff.resize(tileTotal, (uint64_t)0x0);
    float timeB = 0.0f;
    for (int i = 0; i < timingTestLoopMax; ++i) {
        resetDataFunc();
        recTime.start();
        {
            snapshotTileLoop(w, h, pixMaskBuff, snapshotTileFuncB);
        }
        timeB += recTime.end();
    }
    timeB /= (float)timingTestLoopMax;
    CPPUNIT_ASSERT(verifyFunc(pixMaskBuff));

    std::cerr << "timeA:" << timeA * 1000.0f << "ms (" << timeB / timeA << "x) "
              << "timeB:" << timeB * 1000.0f << "ms (" << timeA / timeB << "x)" << std::endl;
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
TestSnapshotUtil::showUpdateList(const std::vector<int> &updateList) const
{
    std::ostringstream ostr;
    ostr << "updateList (total:" << updateList.size() << ") {\n";
    for (unsigned i = 0; i < updateList.size(); ++i) {
        ostr << "  i:" << i << ' ' << updateList[i] << '\n';
    }
    ostr << "}";
    return ostr.str();
}

} // namespace unittest
} // namespace fb_util
} // namespace scene_rdl2

