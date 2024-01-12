// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include <scene_rdl2/common/fb_util/ispc/SnapshotUtil_ispc_stubs.h>

#include "SnapshotUtil.h"

#include <iomanip>
#include <iostream>
#include <sstream>

//#define AVX2_TEST // experimental code for AVX2 instruction
#ifdef AVX2_TEST
#include <immintrin.h>          // AVX
#endif // end AVX2_TEST

//
// We have 3 different types of implementations for C++ APIs. All the same results but different
// performances. IMPL_FULLBITOP is the best result so far but we keep all code for future testing
// on different compiler environment.
// We hove chosen IMPL_FULLBITOP based on several different timing testrun on
// Intel Xeon Gold 6140 2.3GHz by GCC9.3, ISPC1.20 @ Sep/15/2023
//
//#define IMPL_HYBRID // original
#define IMPL_FULLBITOP // full bit operation
//#define IMPL_NAIVELOGICAL // naive logical 

namespace scene_rdl2 {
namespace fb_util {

//
// We have 2 different versions of all SnapshotUtil public APIs. They are C++ and ISPC.
// The following directives define which implementation we use.
// Profiling was done using unitTest (tests/lib/common/fb_util/TestSnapshotUtil.{h,cc}).
// See TestSnapshotUtil.cc for more detail.
//
// Basically, we have chosen all ISPC implementations for all APIs. This was decided based on the profiling
// result by GCC9.2 and ISPC1.20 on Intel Xeon Gold 6140 2.3 GHz @ Sep/15/2023. ISPC code was around
// 1.58x ~ 8.29x faster than C++.
//        
#define SNAPSHOTTILE_COL_WEIGHT_ISPC
#define SNAPSHOTTILE_COL_NUMSAMPLE_ISPC
#define SNAPSHOTTILE_HEAT_WEIGHT_ISPC
#define SNAPSHOTTILE_HEAT_NUMSAMPLE_ISPC
#define SNAPSHOTTILE_WEIGHT_ISPC

#define SNAPSHOTTILE_FLOAT_WEIGHT_ISPC
#define SNAPSHOTTILE_FLOAT_NUMSAMPLE_ISPC
#define SNAPSHOTTILE_FLOAT2_WEIGHT_ISPC
#define SNAPSHOTTILE_FLOAT2_NUMSAMPLE_ISPC
#define SNAPSHOTTILE_FLOAT3_WEIGHT_ISPC
#define SNAPSHOTTILE_FLOAT3_NUMSAMPLE_ISPC
#define SNAPSHOTTILE_FLOAT4_WEIGHT_ISPC
#define SNAPSHOTTILE_FLOAT4_NUMSAMPLE_ISPC

#define SNAPSHOTTILE_UINT32_MASK_ISPC

//------------------------------------------------------------------------------
//
// beauty buffer
//
// static function
uint64_t
SnapshotUtil::snapshotTileColorWeight(uint32_t* dstC,
                                      uint32_t* dstW,
                                      const uint32_t* srcC,
                                      const uint32_t* srcW)
//
// make snapshot for color + weight data
// update destination buffers and return active pixel mask for this tile
//
// dstC : destination tile start address of color  data : color  buffer (r,g,b,a) = 16byte * 8 * 8
// dstW : destination tile start address of weight data : weight buffer (w)       =  4byte * 8 * 8
// srcC :      source tile start address of color  data : color  buffer (r,g,b,a) = 16byte * 8 * 8
// srcW :      source tile start address of weight data : weight buffer (w)       =  4byte * 8 * 8
//
{
#ifdef SNAPSHOTTILE_COL_WEIGHT_ISPC
    return ispc::snapshotTileFloat4Weight(reinterpret_cast<int*>(dstC),
                                          reinterpret_cast<int*>(dstW),
                                          const_cast<int*>(reinterpret_cast<const int*>(srcC)),
                                          const_cast<int*>(reinterpret_cast<const int*>(srcW)));
#else // else SNAPSHOTTILE_COL_WEIGHT_ISPC
    return snapshotTileFloat4Weight_SISD(dstC, dstW, srcC, srcW);
#endif // end else SNAPSHOTTILE_COL_WEIGHT_ISPC
}

#ifdef AVX2_TEST
// static function
uint64_t
SnapshotUtil::snapshotTileColorWeight_AVX2(uint32_t* dstCPtr,
                                           uint32_t* dstWPtr,
                                           const uint32_t* srcCPtr,
                                           const uint32_t* srcWPtr)
//
// make snapshot for color + weight data
// update destination buffers and return active pixel mask for this tile
//
// This is AVX2 version (experimental)
// This code does not check source weight value is not 0.0f for active pixel decision yet due to
// experimental code. In order to finalize this code, we have to consider the weight value check.
//
{
    __m256i offsetMask;
    {
        int *i = reinterpret_cast<int *>(&offsetMask);
        i[0] = 3;
        i[1] = 7;
        i[2] = 2;
        i[3] = 6;
        i[4] = 1;
        i[5] = 5;
        i[6] = 0;
        i[7] = 4;
    }

    __m256i tmpA, tmpB;
    __m256i d01, d23, d45, d67, dw;
    __m256i m2031, m6475, m64207531, m01234567, mask;

    uint64_t activePixelMask = (uint64_t)0x0;
    for (unsigned y = 0; y < 8; ++y) {
        unsigned offset = (y << 3);
        const uint32_t *currSrcCPtr = srcCPtr + offset * 4;
        const uint32_t *currSrcWPtr = srcWPtr + offset;
        uint32_t *currDstCPtr = dstCPtr + offset * 4;
        uint32_t *currDstWPtr = dstWPtr + offset;

        //
        // Process one scanline (8 pixels)
        //

        // Store pix0(rgba), pix1(rgba) to YMM register
        tmpA = _mm256_load_si256(reinterpret_cast<const __m256i *>(currSrcCPtr)); // AVX
        tmpB = _mm256_load_si256(reinterpret_cast<const __m256i *>(currDstCPtr)); // AVX
        d01 = _mm256_sub_epi32(tmpB, tmpA); // AVX2

        // Store pix2(rgba), pix3(rgba) to YMM register
        tmpA = _mm256_load_si256(reinterpret_cast<const __m256i *>(currSrcCPtr + 8)); // AVX
        tmpB = _mm256_load_si256(reinterpret_cast<const __m256i *>(currDstCPtr + 8)); // AVX
        d23 = _mm256_sub_epi32(tmpB, tmpA); // AVX2

        // Store pix4(rgba), pix5(rgba) to YMM register
        tmpA = _mm256_load_si256(reinterpret_cast<const __m256i *>(currSrcCPtr + 16)); // AVX
        tmpB = _mm256_load_si256(reinterpret_cast<const __m256i *>(currDstCPtr + 16)); // AVX
        d45 = _mm256_sub_epi32(tmpB, tmpA); // AVX2

        // Store pix6(rgba), pix7(rgba) to YMM register
        tmpA = _mm256_load_si256(reinterpret_cast<const __m256i *>(currSrcCPtr + 24)); // AVX
        tmpB = _mm256_load_si256(reinterpret_cast<const __m256i *>(currDstCPtr + 24)); // AVX
        d67 = _mm256_sub_epi32(tmpB, tmpA); // AVX2

        // Store weight0~7 to YMM register
        tmpA = _mm256_load_si256(reinterpret_cast<const __m256i *>(currSrcWPtr)); // AVX
        tmpB = _mm256_load_si256(reinterpret_cast<const __m256i *>(currDstWPtr)); // AVX
        dw = _mm256_sub_epi32(tmpB, tmpA); // AVX2

        // First of all, test all data are zero or not. If zero, all data are same as
        // previous result and no need to update destination.
        if (!_mm256_testz_si256(d01, d01) || !_mm256_testz_si256(d23, d23) || // AVX
            !_mm256_testz_si256(d45, d45) || !_mm256_testz_si256(d67, d67) ||
            !_mm256_testz_si256(dw, dw)) {
            //
            // Found non zero case (there is some difference on this pixel), then test more deeply
            //            
            m2031 = _mm256_hadd_epi32(d23, d01); // AVX2
            m6475 = _mm256_hadd_epi32(d67, d45); // AVX2
            m64207531 = _mm256_hadd_epi32(m6475, m2031); // AVX2
            m01234567 = _mm256_permutevar8x32_epi32(m64207531, offsetMask); // shuffle : AVX2
            mask = _mm256_add_epi32(m01234567, dw); // combine with weight  : AVX2

            const int   *maskPtr  = reinterpret_cast<const int *>(&mask);
            const float *buffAPtr = reinterpret_cast<const float *>(currSrcCPtr);
            const float *wAPtr    = reinterpret_cast<const float *>(currSrcWPtr);
                  float *buffBPtr = reinterpret_cast<float *>(currDstCPtr);
                  float *wBPtr    = reinterpret_cast<float *>(currDstWPtr);
            for (int i = 0; i < 8; ++i) {
                if (maskPtr[i]) {
                    buffBPtr[0] = buffAPtr[0];
                    buffBPtr[1] = buffAPtr[1];
                    buffBPtr[2] = buffAPtr[2];
                    buffBPtr[3] = buffAPtr[3];
                    wBPtr[0] = wAPtr[0];
                    activePixelMask |= ((uint64_t)0x1 << (offset + i));
                }
                buffAPtr += 4;
                buffBPtr += 4;
                wAPtr++;
                wBPtr++;
            }
        }
    }

    return activePixelMask;
}
#endif // end AVX2_TEST

// static function
uint64_t
SnapshotUtil::snapshotTileColorNumSample(uint32_t* dstC,
                                         uint32_t* dstN,
                                         const uint64_t dstTileMask,
                                         const uint32_t* srcC,
                                         const uint32_t* srcN,
                                         const uint64_t srcTileMask)
{
#ifdef SNAPSHOTTILE_COL_NUMSAMPLE_ISPC
    return ispc::snapshotTileFloat4NumSample(reinterpret_cast<int*>(dstC),
                                             reinterpret_cast<int*>(dstN),
                                             dstTileMask,
                                             const_cast<int*>(reinterpret_cast<const int*>(srcC)),
                                             const_cast<int*>(reinterpret_cast<const int*>(srcN)),
                                             srcTileMask);
#else // else SNAPSHOTTILE_COL_NUMSAMPLE_ISPC
    return snapshotTileFloat4NumSample_SISD(dstC, dstN, dstTileMask, srcC, srcN, srcTileMask);
#endif // end else SNAPSHOTTILE_COL_NUMSAMPLE_ISPC
}

//------------------------------------------------------------------------------
//
// heatMap
//
// static function
uint64_t
SnapshotUtil::snapshotTileHeatMapWeight(uint64_t* dstV,
                                        uint32_t* dstW,
                                        const uint64_t* srcV,
                                        const uint32_t* srcW)
{
#ifdef SNAPSHOTTILE_HEAT_WEIGHT_ISPC
    return ispc::snapshotTileHeatMapWeight(reinterpret_cast<int64_t*>(dstV),
                                           reinterpret_cast<int*>(dstW),
                                           const_cast<int64_t*>(reinterpret_cast<const int64_t*>(srcV)),
                                           const_cast<int*>(reinterpret_cast<const int*>(srcW)));
#else // else SNAPSHOTTILE_HEAT_WEIGHT_ISPC
    return snapshotTileHeatMapWeight_SISD(dstV, dstW, srcV, srcW);
#endif // end else SNAPSHOTTILE_HEAT_WEIGHT_ISPC
}
    
// static function
uint64_t
SnapshotUtil::snapshotTileHeatMapWeight_SISD(uint64_t* dstV,
                                             uint32_t* dstW,
                                             const uint64_t* srcV,
                                             const uint32_t* srcW)
{
    uint64_t activePixelMask = static_cast<uint64_t>(0x0);
    for (unsigned y = 0; y < 8; ++y) {
        unsigned offset = (y << 3);
        const uint64_t* currSrcVPtr = srcV + offset;
        const uint32_t* currSrcWPtr = srcW + offset;
        uint64_t*       currDstVPtr = dstV + offset;
        uint32_t*       currDstWPtr = dstW + offset;

        for (unsigned x = 0; x < 8; ++x) {
            //
            // Compare bit pattern between previous and current
            // See more detail for snapshotTileFloat4Weight_SISD()'s comment
            //
#ifdef IMPL_HYBRID
            uint64_t activeMask =
                ((currSrcVPtr[0] - currDstVPtr[0]) |
                static_cast<uint64_t>(currSrcWPtr[0] - currDstWPtr[0])) &&
                static_cast<uint64_t>(currSrcWPtr[0]);
            if (activeMask) {
#endif // end IMPL_HYBRID
#ifdef IMPL_FULLBITOP
            uint64_t srcWFlag = (currSrcWPtr[0]) ? ~static_cast<uint64_t>(0) : static_cast<uint64_t>(0);
            uint64_t activeMask =
                ((currSrcVPtr[0] - currDstVPtr[0]) |
                 static_cast<uint64_t>(currSrcWPtr[0] - currDstWPtr[0])) & srcWFlag;
            if (activeMask) {
#endif // end IMPL_FULLBITOP
#ifdef IMPL_NAIVELOGICAL
            bool activeFlag =
                (((currSrcVPtr[0] - currDstVPtr[0]) != 0x0) ||
                 (static_cast<uint64_t>(currSrcWPtr[0] - currDstWPtr[0]) != static_cast<uint64_t>(0x0))) &&
                currSrcWPtr[0];
            if (activeFlag) {
#endif // end IMPL_NAIVELOGICAL
                // update data
                currDstVPtr[0] = currSrcVPtr[0];
                currDstWPtr[0] = currSrcWPtr[0];
                activePixelMask |= (static_cast<uint64_t>(0x1) << offset);
            }

            offset++;
            currSrcVPtr++;
            currSrcWPtr++;
            currDstVPtr++;
            currDstWPtr++;
        }
    }

    return activePixelMask;
}

// static function
uint64_t
SnapshotUtil::snapshotTileHeatMapWeight_SIMD(uint64_t* dstV,
                                             uint32_t* dstW,
                                             const uint64_t* srcV,
                                             const uint32_t* srcW)
{
    return ispc::snapshotTileHeatMapWeight(reinterpret_cast<int64_t*>(dstV),
                                           reinterpret_cast<int*>(dstW),
                                           const_cast<int64_t*>(reinterpret_cast<const int64_t*>(srcV)),
                                           const_cast<int*>(reinterpret_cast<const int*>(srcW)));
}

// static function
uint64_t
SnapshotUtil::snapshotTileHeatMapNumSample(uint32_t* dstV,
                                           uint32_t* dstN,
                                           const uint64_t dstTileMask,
                                           const uint32_t* srcV,
                                           const uint32_t* srcN,
                                           const uint64_t srcTileMask)
{
#ifdef SNAPSHOTTILE_HEAT_NUMSAMPLE_ISPC
    return ispc::snapshotTileFloatNumSample(reinterpret_cast<int *>(dstV),
                                            reinterpret_cast<int *>(dstN),
                                            dstTileMask,
                                            const_cast<int *>(reinterpret_cast<const int *>(srcV)),
                                            const_cast<int *>(reinterpret_cast<const int *>(srcN)),
                                            srcTileMask);
#else // else SNAPSHOTTILE_HEAT_NUMSAMPLE_ISPC
    return snapshotTileFloatNumSample_SISD(dstV, dstN, dstTileMask, srcV, srcN, srcTileMask);
#endif // end else SNAPSHOTTILE_HEAT_NUMSAMPLE_ISPC
}

//------------------------------------------------------------------------------
//
// weight buffer
//
// static function
uint64_t
SnapshotUtil::snapshotTileWeightBuffer(uint32_t* dst, const uint32_t* src)
{
#ifdef SNAPSHOTTILE_WEIGHT_ISPC
    return ispc::snapshotTileWeightBuffer(reinterpret_cast<int*>(dst),
                                          const_cast<int *>(reinterpret_cast<const int*>(src)));
#else // else SNAPSHOTTILE_WEIGHT_ISPC   
    return snapshotTileWeightBuffer_SISD(dst, src);
#endif // end else SNAPSHOTTILE_WEIGHT_ISPC
}
    
// static function
uint64_t
SnapshotUtil::snapshotTileWeightBuffer_SISD(uint32_t* dst, const uint32_t* src)
{
    uint64_t activePixelMask = static_cast<uint64_t>(0x0);
    for (unsigned y = 0; y < 8; ++y) {
        unsigned offset = (y << 3);
        const uint32_t* currSrc = src + offset;
        uint32_t*       currDst = dst + offset;

        for (unsigned x = 0; x < 8; ++x) {
            //
            // Compare bit pattern between previous and current
            // See more detail for snapshotTileFloat4Weight_SISD()'s comment
            //
#ifdef IMPL_HYBRID
            uint32_t activeMask = (currSrc[0] - currDst[0]) && currSrc[0];
            if (activeMask) {
#endif // end IMPL_HYBRID
#ifdef IMPL_FULLBITOP
            uint32_t activeMask = (currSrc[0] - currDst[0]) & ((currSrc[0]) ? ~0x0 : 0x0);
            if (activeMask) {
#endif // end IMPL_FULLBITOP
#ifdef IMPL_NAIVELOGICAL
            bool activeFlag =
                ((currSrc[0] - currDst[0]) != 0x0) && currSrc[0];
            if (activeFlag) {
#endif // end IMPL_NAIVELOGICAL
                // update data
                currDst[0] = currSrc[0];
                activePixelMask |= (static_cast<uint64_t>(0x1) << offset);
            }

            offset++;
            currSrc++;
            currDst++;
        }
    }

    return activePixelMask;
}

// static function
uint64_t
SnapshotUtil::snapshotTileWeightBuffer_SIMD(uint32_t* dst, const uint32_t* src)
{
    return ispc::snapshotTileWeightBuffer(reinterpret_cast<int*>(dst),
                                          const_cast<int *>(reinterpret_cast<const int*>(src)));
}

//------------------------------------------------------------------------------
//
// renderOutput
//

// static function
uint64_t
SnapshotUtil::snapshotTileFloatWeight(uint32_t* dstV,
                                      uint32_t* dstW,
                                      const uint32_t* srcV,
                                      const uint32_t* srcW)
{
#ifdef SNAPSHOTTILE_FLOAT_WEIGHT_ISPC
    return ispc::snapshotTileFloatWeight(reinterpret_cast<int*>(dstV),
                                         reinterpret_cast<int*>(dstW),
                                         const_cast<int *>(reinterpret_cast<const int*>(srcV)),
                                         const_cast<int *>(reinterpret_cast<const int*>(srcW)));
#else // else SNAPSHOTTILE_FLOAT_WEIGHT_ISPC
    return snapshotTileFloatWeight_SISD(dstV, dstW, srcV, srcW);
#endif // end else SNAPSHOTTILE_FLOAT_WEIGHT_ISPC
}

// static function
uint64_t
SnapshotUtil::snapshotTileFloatWeight_SISD(uint32_t* dstV,
                                           uint32_t* dstW,
                                           const uint32_t* srcV,
                                           const uint32_t* srcW)
{
    uint64_t activePixelMask = static_cast<uint64_t>(0x0);
    for (unsigned y = 0; y < 8; ++y) {
        unsigned offset = (y << 3);
        const uint32_t* currSrcV = srcV + offset;
        const uint32_t* currSrcW = srcW + offset;
        uint32_t*       currDstV = dstV + offset;
        uint32_t*       currDstW = dstW + offset;

        for (unsigned x = 0; x < 8; ++x) {
            //
            // Compare bit pattern between previous and current
            // See more detail for snapshotTileFloat4Weight_SISD()'s comment
            //
#ifdef IMPL_HYBRID
            uint32_t activeMask = ((currSrcV[0] - currDstV[0]) |
                                   (currSrcW[0] - currDstW[0])) && currSrcW[0];
            if (activeMask) {
#endif // end IMPL_HYBRID
#ifdef IMPL_FULLBITOP
            uint32_t activeMask = ((currSrcV[0] - currDstV[0]) |
                                   (currSrcW[0] - currDstW[0])) & ((currSrcW[0]) ? ~0x0 : 0x0);
            if (activeMask) {
#endif // end IMPL_FULLBITOP                
#ifdef IMPL_NAIVELOGICAL
            bool activeFlag =
                (((currSrcV[0] - currDstV[0]) != 0x0) || ((currSrcW[0] - currDstW[0]) != 0x0)) &&
                currSrcW[0];
            if (activeFlag) {
#endif // end IMPL_NAIVELOGICAL
                // update data
                currDstV[0] = currSrcV[0];
                currDstW[0] = currSrcW[0];
                activePixelMask |= (static_cast<uint64_t>(0x1) << offset);
            }

            offset++;
            currSrcV++;
            currSrcW++;
            currDstV++;
            currDstW++;
        }
    }

    return activePixelMask;
}

// static function
uint64_t
SnapshotUtil::snapshotTileFloatWeight_SIMD(uint32_t* dstV,
                                           uint32_t* dstW,
                                           const uint32_t* srcV,
                                           const uint32_t* srcW)
{
    return ispc::snapshotTileFloatWeight(reinterpret_cast<int*>(dstV),
                                         reinterpret_cast<int*>(dstW),
                                         const_cast<int *>(reinterpret_cast<const int*>(srcV)),
                                         const_cast<int *>(reinterpret_cast<const int*>(srcW)));
}

// static function
uint64_t
SnapshotUtil::snapshotTileFloatNumSample(uint32_t* dstV,
                                         uint32_t* dstN,
                                         const uint64_t dstTileMask,
                                         const uint32_t* srcV,
                                         const uint32_t* srcN,
                                         const uint64_t srcTileMask)
{
#ifdef SNAPSHOTTILE_FLOAT_NUMSAMPLE_ISPC
    return ispc::snapshotTileFloatNumSample(reinterpret_cast<int*>(dstV),
                                            reinterpret_cast<int*>(dstN),
                                            dstTileMask,
                                            const_cast<int*>(reinterpret_cast<const int*>(srcV)),
                                            const_cast<int*>(reinterpret_cast<const int*>(srcN)),
                                            srcTileMask);
#else // else SNAPSHOTTILE_FLOAT_NUMSAMPLE_ISPC
    return snapshotTileFloatNumSample_SISD(dstV, dstN, dstTileMask, srcV, srcN, srcTileMask);
#endif // end else SNAPSHOTTILE_FLOAT_NUMSAMPLE_ISPC
}

// static function
uint64_t
SnapshotUtil::snapshotTileFloatNumSample_SISD(uint32_t* dstV,
                                              uint32_t* dstN,
                                              const uint64_t dstTileMask,
                                              const uint32_t* srcV,
                                              const uint32_t* srcN,
                                              const uint64_t srcTileMask)
{
    if (!srcTileMask) return 0x0;

    uint64_t activePixelMask = static_cast<uint64_t>(0x0);
    for (unsigned y = 0; y < 8; ++y) {
        unsigned offset = (y << 3); // y * 8

        uint64_t currSrcTileMask = srcTileMask >> offset;
        if (!currSrcTileMask) break; // early exit : rest of them are all empty
        uint64_t currDstTileMask = dstTileMask >> offset;

        const uint32_t* currSrcV = srcV + offset;
        const uint32_t* currSrcN = srcN + offset;
        uint32_t*       currDstV = dstV + offset;
        uint32_t*       currDstN = dstN + offset;

        uint64_t currSrcTileScanlineMask = currSrcTileMask & static_cast<uint64_t>(0xff); // get one scanline mask
        uint64_t currDstTileScanlineMask = currDstTileMask & static_cast<uint64_t>(0xff); // get one scanline mask
        for (unsigned x = 0; x < 8; ++x) {
            if (!currSrcTileScanlineMask) break; // early exit for scanline

            if (currSrcTileScanlineMask & static_cast<uint64_t>(0x1)) {
                //
                // Compare bit pattern between previous and current
                // See more detail for snapshotTileFloat4Weight_SISD()'s comment
                //
#ifdef IMPL_HYBRID
                uint32_t freshPixelCondition = (currDstTileScanlineMask & static_cast<uint64_t>(0x1))? 0x0: 0xffffffff;
                uint32_t activeMask = ((currSrcV[0] - currDstV[0]) |
                                       (currSrcN[0] - currDstN[0]) |
                                       freshPixelCondition) && currSrcN[0];
                if (activeMask) {
#endif // end IMPL_HYBRID
#ifdef IMPL_FULLBITOP
                uint32_t freshPixelCondition = (currDstTileScanlineMask & static_cast<uint64_t>(0x1))? 0x0: 0xffffffff;
                uint32_t activeMask = ((currSrcV[0] - currDstV[0]) |
                                       (currSrcN[0] - currDstN[0]) |
                                       freshPixelCondition) & ((currSrcN[0]) ? ~0x0 : 0x0);
                if (activeMask) {
#endif // end IMPL_FULLBITOP
#ifdef IMPL_NAIVELOGICAL
                bool freshPixelCondition = (currDstTileScanlineMask & static_cast<uint64_t>(0x1)) ? false : true;
                bool activeFlag = (((currSrcV[0] - currDstV[0]) != 0x0) ||
                                   ((currSrcN[0] - currDstN[0]) != 0x0) ||
                                   freshPixelCondition) && currSrcN[0];
                if (activeFlag) {
#endif // end IMPL_NAIVELOGICAL
                    // updated pixel
                    currDstV[0] = currSrcV[0];
                    currDstN[0] = currSrcN[0];
                    activePixelMask |= (static_cast<uint64_t>(0x1) << offset);
                }
            }

            offset++;
            currSrcV++;
            currDstV++;
            currSrcN++;
            currDstN++;
            currSrcTileScanlineMask >>= 1;
            currDstTileScanlineMask >>= 1;
        }
    }

    return activePixelMask;
}

// static function
uint64_t
SnapshotUtil::snapshotTileFloatNumSample_SIMD(uint32_t* dstV,
                                              uint32_t* dstN,
                                              const uint64_t dstTileMask,
                                              const uint32_t* srcV,
                                              const uint32_t* srcN,
                                              const uint64_t srcTileMask)
{
    return ispc::snapshotTileFloatNumSample(reinterpret_cast<int*>(dstV),
                                            reinterpret_cast<int*>(dstN),
                                            dstTileMask,
                                            const_cast<int*>(reinterpret_cast<const int*>(srcV)),
                                            const_cast<int*>(reinterpret_cast<const int*>(srcN)),
                                            srcTileMask);
}

// static function
uint64_t
SnapshotUtil::snapshotTileFloat2Weight(uint32_t* dstV,
                                       uint32_t* dstW,
                                       const uint32_t* srcV,
                                       const uint32_t* srcW)
{
#ifdef SNAPSHOTTILE_FLOAT2_WEIGHT_ISPC
    return ispc::snapshotTileFloat2Weight(reinterpret_cast<int*>(dstV),
                                          reinterpret_cast<int64_t*>(dstW),
                                          const_cast<int*>(reinterpret_cast<const int*>(srcV)),
                                          const_cast<int64_t*>(reinterpret_cast<const int64_t*>(srcW)));
#else // else SNAPSHOTTILE_FLOAT2_WEIGHT_ISPC
    return snapshotTileFloat2Weight_SISD(dstV, dstW, srcV, srcW);
#endif // end else SNAPSHOTTILE_FLOAT2_WEIGHT_ISPC
}

// static function
uint64_t
SnapshotUtil::snapshotTileFloat2Weight_SISD(uint32_t* dstV,
                                            uint32_t* dstW,
                                            const uint32_t* srcV,
                                            const uint32_t* srcW)
{
    uint64_t activePixelMask = static_cast<uint64_t>(0x0);
    for (unsigned y = 0; y < 8; ++y) {
        unsigned offset = (y << 3);
        const uint32_t* currSrcV = srcV + offset * 2;
        const uint32_t* currSrcW = srcW + offset;
        uint32_t*       currDstV = dstV + offset * 2;
        uint32_t*       currDstW = dstW + offset;

        for (unsigned x = 0; x < 8; ++x) {
            //
            // Compare bit pattern between previous and current
            // See more detail for snapshotTileFloat4Weight_SISD()'s comment
            //
#ifdef IMPL_HYBRID
            uint32_t activeMask = ((currSrcV[0] - currDstV[0]) |
                                   (currSrcV[1] - currDstV[1]) |
                                   (currSrcW[0] - currDstW[0])) && currSrcW[0];
            if (activeMask) {
#endif // end IMPL_HYBRID
#ifdef IMPL_FULLBITOP
            uint32_t activeMask = ((currSrcV[0] - currDstV[0]) |
                                   (currSrcV[1] - currDstV[1]) |
                                   (currSrcW[0] - currDstW[0])) & ((currSrcW[0]) ? ~0x0 : 0x0);
            if (activeMask) {
#endif // end IMPL_FULLBITOP
#ifdef IMPL_NAIVELOGICAL
            bool activeFlag = (((currSrcV[0] - currDstV[0]) != 0x0) ||
                               ((currSrcV[1] - currDstV[1]) != 0x0) ||
                               ((currSrcW[0] - currDstW[0]) != 0x0)) && currSrcW[0];
            if (activeFlag) {
#endif // end IMPL_NAIVELOGICAL
                // update data
                currDstV[0] = currSrcV[0];
                currDstV[1] = currSrcV[1];
                currDstW[0] = currSrcW[0];
                activePixelMask |= (static_cast<uint64_t>(0x1) << offset);
            }

            offset++;
            currSrcV += 2;
            currSrcW++;
            currDstV += 2;
            currDstW++;
        }
    }

    return activePixelMask;
}

// static function
uint64_t
SnapshotUtil::snapshotTileFloat2Weight_SIMD(uint32_t* dstV,
                                            uint32_t* dstW,
                                            const uint32_t* srcV,
                                            const uint32_t* srcW)
{
    return ispc::snapshotTileFloat2Weight(reinterpret_cast<int*>(dstV),
                                          reinterpret_cast<int64_t*>(dstW),
                                          const_cast<int*>(reinterpret_cast<const int*>(srcV)),
                                          const_cast<int64_t*>(reinterpret_cast<const int64_t*>(srcW)));
}

// static function
uint64_t
SnapshotUtil::snapshotTileFloat2NumSample(uint32_t* dstV,
                                          uint32_t* dstN,
                                          const uint64_t dstTileMask,
                                          const uint32_t* srcV,
                                          const uint32_t* srcN,
                                          const uint64_t srcTileMask)
{
#ifdef SNAPSHOTTILE_FLOAT2_NUMSAMPLE_ISPC
    return ispc::snapshotTileFloat2NumSample(reinterpret_cast<int*>(dstV),
                                             reinterpret_cast<int64_t*>(dstN),
                                             dstTileMask,
                                             const_cast<int*>(reinterpret_cast<const int*>(srcV)),
                                             const_cast<int64_t*>(reinterpret_cast<const int64_t*>(srcN)),
                                             srcTileMask);
#else // else SNAPSHOTTILE_FLOAT2_NUMSAMPLE_ISPC
    return snapshotTileFloat2NumSample_SISD(dstV, dstN, dstTileMask, srcV, srcN, srcTileMask);
#endif // end else SNAPSHOTTILE_FLOAT2_NUMSAMPLE_ISPC
}

// static function
uint64_t
SnapshotUtil::snapshotTileFloat2NumSample_SISD(uint32_t* dstV,
                                               uint32_t* dstN,
                                               const uint64_t dstTileMask,
                                               const uint32_t* srcV,
                                               const uint32_t* srcN,
                                               const uint64_t srcTileMask)
{
    if (!srcTileMask) return 0x0;

    uint64_t activePixelMask = static_cast<uint64_t>(0x0);
    for (unsigned y = 0; y < 8; ++y) {
        unsigned offset = (y << 3); // y * 8

        uint64_t currSrcTileMask = srcTileMask >> offset;
        if (!currSrcTileMask) break; // early exit : rest of them are all empty
        uint64_t currDstTileMask = dstTileMask >> offset;

        const uint32_t* currSrcV = srcV + offset * 2;
        const uint32_t* currSrcN = srcN + offset;
        uint32_t*       currDstV = dstV + offset * 2;
        uint32_t*       currDstN = dstN + offset;

        uint64_t currSrcTileScanlineMask = currSrcTileMask & static_cast<uint64_t>(0xff); // get one scanline mask
        uint64_t currDstTileScanlineMask = currDstTileMask & static_cast<uint64_t>(0xff); // get one scanline mask
        for (unsigned x = 0; x < 8; ++x) {
            if (!currSrcTileScanlineMask) break; // early exit for scanline

            if (currSrcTileScanlineMask & static_cast<uint64_t>(0x1)) {
                //
                // Compare bit pattern between previous and current
                // See more detail for snapshotTileFloat4Weight_SISD()'s comment
                //
#ifdef IMPL_HYBRID
                uint32_t freshPixelCondition = (currDstTileScanlineMask & static_cast<uint64_t>(0x1))? 0x0: 0xffffffff;
                uint32_t activeMask = ((currSrcV[0] - currDstV[0]) |
                                       (currSrcV[1] - currDstV[1]) |
                                       (currSrcN[0] - currDstN[0]) |
                                       freshPixelCondition) && currSrcN[0];
                if (activeMask) {
#endif // end IMPL_HYBRID
#ifdef IMPL_FULLBITOP
                uint32_t freshPixelCondition = (currDstTileScanlineMask & static_cast<uint64_t>(0x1))? 0x0: 0xffffffff;
                uint32_t activeMask = ((currSrcV[0] - currDstV[0]) |
                                       (currSrcV[1] - currDstV[1]) |
                                       (currSrcN[0] - currDstN[0]) |
                                       freshPixelCondition) & ((currSrcN[0]) ? ~0x0 : 0x0);
                if (activeMask) {
#endif // end IMPL_FULLBITOP
#ifdef IMPL_NAIVELOGICAL
                bool freshPixelCondition = (currDstTileScanlineMask & static_cast<uint64_t>(0x1)) ? false : true;
                bool activeFlag = (((currSrcV[0] - currDstV[0]) != 0x0) ||
                                   ((currSrcV[1] - currDstV[1]) != 0x0) ||
                                   ((currSrcN[0] - currDstN[0]) != 0x0) ||
                                   freshPixelCondition) && currSrcN[0];
                if (activeFlag) {
#endif // end IMPL_NAIVELOGICAL
                    // updated pixel
                    currDstV[0] = currSrcV[0];
                    currDstV[1] = currSrcV[1];
                    currDstN[0] = currSrcN[0];
                    activePixelMask |= (static_cast<uint64_t>(0x1) << offset);
                }
            }

            offset++;
            currSrcV += 2;
            currDstV += 2;
            currSrcN++;
            currDstN++;
            currSrcTileScanlineMask >>= 1;
            currDstTileScanlineMask >>= 1;
        }
    }
    return activePixelMask;
}

// static function
uint64_t
SnapshotUtil::snapshotTileFloat2NumSample_SIMD(uint32_t* dstV,
                                               uint32_t* dstN,
                                               const uint64_t dstTileMask,
                                               const uint32_t* srcV,
                                               const uint32_t* srcN,
                                               const uint64_t srcTileMask)
{
    return ispc::snapshotTileFloat2NumSample(reinterpret_cast<int*>(dstV),
                                             reinterpret_cast<int64_t*>(dstN),
                                             dstTileMask,
                                             const_cast<int*>(reinterpret_cast<const int*>(srcV)),
                                             const_cast<int64_t*>(reinterpret_cast<const int64_t*>(srcN)),
                                             srcTileMask);
}

// static function
uint64_t
SnapshotUtil::snapshotTileFloat3Weight(uint32_t* dstV,
                                       uint32_t* dstW,
                                       const uint32_t* srcV,
                                       const uint32_t* srcW)
{
#ifdef SNAPSHOTTILE_FLOAT3_WEIGHT_ISPC
    return ispc::snapshotTileFloat3Weight(reinterpret_cast<int*>(dstV),
                                          reinterpret_cast<int*>(dstW),
                                          const_cast<int*>(reinterpret_cast<const int*>(srcV)),
                                          const_cast<int*>(reinterpret_cast<const int*>(srcW)));
#else // else SNAPSHOTTILE_FLOAT3_WEIGHT_ISPC
    return snapshotTileFloat3Weight_SISD(dstV, dstW, srcV, srcW);
#endif // end else SNAPSHOTTILE_FLOAT3_WEIGHT_ISPC
}

// static function
uint64_t
SnapshotUtil::snapshotTileFloat3Weight_SISD(uint32_t* dstV,
                                            uint32_t* dstW,
                                            const uint32_t* srcV,
                                            const uint32_t* srcW)
{
    uint64_t activePixelMask = static_cast<uint64_t>(0x0);
    for (unsigned y = 0; y < 8; ++y) {
        unsigned offset = (y << 3);
        const uint32_t* currSrcV = srcV + offset * 3;
        const uint32_t* currSrcW = srcW + offset;
        uint32_t*       currDstV = dstV + offset * 3;
        uint32_t*       currDstW = dstW + offset;

        for (unsigned x = 0; x < 8; ++x) {
            //
            // Compare bit pattern between previous and current
            // See more detail for snapshotTileFloat4Weight_SISD()'s comment
            //
#ifdef IMPL_HYBRID
            uint32_t activeMask = ((currSrcV[0] - currDstV[0]) |
                                   (currSrcV[1] - currDstV[1]) |
                                   (currSrcV[2] - currDstV[2]) |
                                   (currSrcW[0] - currDstW[0])) && currSrcW[0];
            if (activeMask) {
#endif // end IMPL_HYBRID
#ifdef IMPL_FULLBITOP
            uint32_t activeMask = ((currSrcV[0] - currDstV[0]) |
                                   (currSrcV[1] - currDstV[1]) |
                                   (currSrcV[2] - currDstV[2]) |
                                   (currSrcW[0] - currDstW[0])) & ((currSrcW[0]) ? ~0x0 : 0x0);
            if (activeMask) {
#endif // end IMPL_FULLBITOP
#ifdef IMPL_NAIVELOGICAL
            bool activeFlag = (((currSrcV[0] - currDstV[0]) != 0x0) ||
                               ((currSrcV[1] - currDstV[1]) != 0x0) ||
                               ((currSrcV[2] - currDstV[2]) != 0x0) ||
                               ((currSrcW[0] - currDstW[0]) != 0x0)) && currSrcW[0];
            if (activeFlag) {
#endif // end IMPL_NAIVELOGICAL
                currDstV[0] = currSrcV[0];
                currDstV[1] = currSrcV[1];
                currDstV[2] = currSrcV[2];
                currDstW[0] = currSrcW[0];
                activePixelMask |= (static_cast<uint64_t>(0x1) << offset);
            }

            offset++;
            currSrcV += 3;
            currSrcW++;
            currDstV += 3;
            currDstW++;
        }
    }

    return activePixelMask;
}

// static function
uint64_t
SnapshotUtil::snapshotTileFloat3Weight_SIMD(uint32_t* dstV,
                                            uint32_t* dstW,
                                            const uint32_t* srcV,
                                            const uint32_t* srcW)
{
    return ispc::snapshotTileFloat3Weight(reinterpret_cast<int*>(dstV),
                                          reinterpret_cast<int*>(dstW),
                                          const_cast<int*>(reinterpret_cast<const int*>(srcV)),
                                          const_cast<int*>(reinterpret_cast<const int*>(srcW)));
}

// static function
uint64_t
SnapshotUtil::snapshotTileFloat3NumSample(uint32_t* dstV,
                                          uint32_t* dstN,
                                          const uint64_t dstTileMask,
                                          const uint32_t* srcV,
                                          const uint32_t* srcN,
                                          const uint64_t srcTileMask)
{
#ifdef SNAPSHOTTILE_FLOAT3_NUMSAMPLE_ISPC
    return ispc::snapshotTileFloat3NumSample(reinterpret_cast<int*>(dstV),
                                             reinterpret_cast<int*>(dstN),
                                             dstTileMask,
                                             const_cast<int*>(reinterpret_cast<const int*>(srcV)),
                                             const_cast<int*>(reinterpret_cast<const int*>(srcN)),
                                             srcTileMask);
#else // else SNAPSHOTTILE_FLOAT3_NUMSAMPLE_ISPC
    return snapshotTileFloat3NumSample_SISD(dstV, dstN, dstTileMask, srcV, srcN, srcTileMask);
#endif // end else SNAPSHOTTILE_FLOAT3_NUMSAMPLE_ISPC    
}
    
// static function
uint64_t
SnapshotUtil::snapshotTileFloat3NumSample_SISD(uint32_t* dstV,
                                               uint32_t* dstN,
                                               const uint64_t dstTileMask,
                                               const uint32_t* srcV,
                                               const uint32_t* srcN,
                                               const uint64_t srcTileMask)
{
    if (!srcTileMask) return 0x0;

    uint64_t activePixelMask = static_cast<uint64_t>(0x0);
    for (unsigned y = 0; y < 8; ++y) {
        unsigned offset = (y << 3); // y * 8

        uint64_t currSrcTileMask = srcTileMask >> offset;
        if (!currSrcTileMask) break; // early exit : rest of them are all empty
        uint64_t currDstTileMask = dstTileMask >> offset;

        const uint32_t* currSrcV = srcV + offset * 3;
        const uint32_t* currSrcN = srcN + offset;
        uint32_t*       currDstV = dstV + offset * 3;
        uint32_t*       currDstN = dstN + offset;

        uint64_t currSrcTileScanlineMask = currSrcTileMask & static_cast<uint64_t>(0xff); // get one scanline mask
        uint64_t currDstTileScanlineMask = currDstTileMask & static_cast<uint64_t>(0xff); // get one scanline mask
        for (unsigned x = 0; x < 8; ++x) {
            if (!currSrcTileScanlineMask) break; // early exit for scanline

            if (currSrcTileScanlineMask & static_cast<uint64_t>(0x1)) {
                //
                // Compare bit pattern between previous and current
                // See more detail for snapshotTileFloat4Weight_SISD()'s comment
                //
#ifdef IMPL_HYBRID
                uint32_t freshPixelCondition = (currDstTileScanlineMask & static_cast<uint64_t>(0x1))? 0x0: 0xffffffff;
                uint32_t activeMask = ((currSrcV[0] - currDstV[0]) |
                                       (currSrcV[1] - currDstV[1]) |
                                       (currSrcV[2] - currDstV[2]) |
                                       (currSrcN[0] - currDstN[0]) |
                                       freshPixelCondition) && currSrcN[0];
                if (activeMask) {
#endif // end IMPL_HYBRID
#ifdef IMPL_FULLBITOP
                uint32_t freshPixelCondition = (currDstTileScanlineMask & static_cast<uint64_t>(0x1))? 0x0: 0xffffffff;
                uint32_t activeMask = ((currSrcV[0] - currDstV[0]) |
                                       (currSrcV[1] - currDstV[1]) |
                                       (currSrcV[2] - currDstV[2]) |
                                       (currSrcN[0] - currDstN[0]) |
                                       freshPixelCondition) & ((currSrcN[0]) ? ~0x0 : 0x0);
                if (activeMask) {
#endif // end IMPL_FULLBITOP
#ifdef IMPL_NAIVELOGICAL
                bool freshPixelCondition = (currDstTileScanlineMask & static_cast<uint64_t>(0x1)) ? false : true;
                bool activeFlag = (((currSrcV[0] - currDstV[0]) != 0x0) ||
                                   ((currSrcV[1] - currDstV[1]) != 0x0) ||
                                   ((currSrcV[2] - currDstV[2]) != 0x0) ||
                                   ((currSrcN[0] - currDstN[0]) != 0x0) ||
                                   freshPixelCondition) && currSrcN[0];
                if (activeFlag) {
#endif // end IMPL_NAIVELOGICAL
                    // updated pixel
                    currDstV[0] = currSrcV[0];
                    currDstV[1] = currSrcV[1];
                    currDstV[2] = currSrcV[2];
                    currDstN[0] = currSrcN[0];
                    activePixelMask |= (static_cast<uint64_t>(0x1) << offset);
                }
            }

            offset++;
            currSrcV += 3;
            currDstV += 3;
            currSrcN++;
            currDstN++;
            currSrcTileScanlineMask >>= 1;
            currDstTileScanlineMask >>= 1;
        }
    }
    return activePixelMask;
}

// static function
uint64_t
SnapshotUtil::snapshotTileFloat3NumSample_SIMD(uint32_t* dstV,
                                               uint32_t* dstN,
                                               const uint64_t dstTileMask,
                                               const uint32_t* srcV,
                                               const uint32_t* srcN,
                                               const uint64_t srcTileMask)
{
    return ispc::snapshotTileFloat3NumSample(reinterpret_cast<int*>(dstV),
                                             reinterpret_cast<int*>(dstN),
                                             dstTileMask,
                                             const_cast<int*>(reinterpret_cast<const int*>(srcV)),
                                             const_cast<int*>(reinterpret_cast<const int*>(srcN)),
                                             srcTileMask);
}

// static function
uint64_t
SnapshotUtil::snapshotTileFloat4Weight(uint32_t* dstV,
                                       uint32_t* dstW,
                                       const uint32_t* srcV,
                                       const uint32_t* srcW)
{
#ifdef SNAPSHOTTILE_FLOAT4_WEIGHT_ISPC
    return ispc::snapshotTileFloat4Weight(reinterpret_cast<int*>(dstV),
                                          reinterpret_cast<int*>(dstW),
                                          const_cast<int*>(reinterpret_cast<const int*>(srcV)),
                                          const_cast<int*>(reinterpret_cast<const int*>(srcW)));
#else // else SNAPSHOTTILE_FLOAT4_WEIGHT_ISPC
    return snapshotTileFloat4Weight_SISD(dstV, dstW, srcV, srcW);
#endif // end else SNAPSHOTTILE_FLOAT4_WEIGHT_ISPC
}

// static function
uint64_t
SnapshotUtil::snapshotTileFloat4Weight_SISD(uint32_t* dstV,
                                            uint32_t* dstW,
                                            const uint32_t* srcV,
                                            const uint32_t* srcW)
{
    uint64_t activePixelMask = static_cast<uint64_t>(0x0);
    for (unsigned y = 0; y < 8; ++y) {
        unsigned offset = (y << 3);
        const uint32_t* currSrcV = srcV + offset * 4;
        const uint32_t* currSrcW = srcW + offset;
        uint32_t*       currDstV = dstV + offset * 4;
        uint32_t*       currDstW = dstW + offset;

        for (unsigned x = 0; x < 8; ++x) {
            //
            // Compare bit pattern between previous and current
            //
            // Basically, activeMask is a representation of the bit pattern difference between the previous
            // and current pixel values. We only test this bit pattern difference when the current pixel
            // weight is not 0.0f. This is why we add "& ((currSrcW[0]) ? ~0x0 : 0x0)" at the end.
            // The following code explains the naive implementation of the same idea.
            //
            // uint32_t activeMask = ((currSrcV[0] - currDstV[0]) |
            //                        (currSrcV[1] - currDstV[1]) |
            //                        (currSrcV[2] - currDstV[2]) |
            //                        (currSrcV[3] - currDstV[3]) |
            //                        (currSrcW[0] - currDstW[0]));
            // float cW = *(float *)(&currSrcW[0]);
            // if (cW != 0.0f && activeMask) {
            //     ... update data ...
            // }
            //
#ifdef IMPL_HYBRID
            uint32_t activeMask = ((currSrcV[0] - currDstV[0]) |
                                   (currSrcV[1] - currDstV[1]) |
                                   (currSrcV[2] - currDstV[2]) |
                                   (currSrcV[3] - currDstV[3]) |
                                   (currSrcW[0] - currDstW[0])) && currSrcW[0];
            if (activeMask) {
#endif // end IMPL_HYBRID
#ifdef IMPL_FULLBITOP
            uint32_t activeMask = ((currSrcV[0] - currDstV[0]) |
                                   (currSrcV[1] - currDstV[1]) |
                                   (currSrcV[2] - currDstV[2]) |
                                   (currSrcV[3] - currDstV[3]) |
                                   (currSrcW[0] - currDstW[0])) & ((currSrcW[0]) ? ~0x0 : 0x0);
            if (activeMask) {
#endif // end IMPL_FULLBITOP
#ifdef IMPL_NAIVELOGICAL
            bool activeFlag = ((currSrcV[0] - currDstV[0]) != 0x0 ||
                               (currSrcV[1] - currDstV[1]) != 0x0 ||
                               (currSrcV[2] - currDstV[2]) != 0x0 ||
                               (currSrcV[3] - currDstV[3]) != 0x0 ||
                               (currSrcW[0] - currDstW[0]) != 0x0) && currSrcW[0];
            if (activeFlag) {
#endif // end IMPL_NAIVELOGICAL
                // update data
                currDstV[0] = currSrcV[0];
                currDstV[1] = currSrcV[1];
                currDstV[2] = currSrcV[2];
                currDstV[3] = currSrcV[3];
                currDstW[0] = currSrcW[0];
                activePixelMask |= (static_cast<uint64_t>(0x1) << offset);
            }

            offset++;
            currSrcV += 4;
            currSrcW++;
            currDstV += 4;
            currDstW++;
        }
    }

    return activePixelMask;
}

// static function
uint64_t
SnapshotUtil::snapshotTileFloat4Weight_SIMD(uint32_t* dstV,
                                            uint32_t* dstW,
                                            const uint32_t* srcV,
                                            const uint32_t* srcW)
{
    return ispc::snapshotTileFloat4Weight(reinterpret_cast<int*>(dstV),
                                          reinterpret_cast<int*>(dstW),
                                          const_cast<int*>(reinterpret_cast<const int*>(srcV)),
                                          const_cast<int*>(reinterpret_cast<const int*>(srcW)));
}

uint64_t
SnapshotUtil::snapshotTileFloat4NumSample(uint32_t* dstV,
                                          uint32_t* dstN,
                                          const uint64_t dstTileMask,
                                          const uint32_t* srcV,
                                          const uint32_t* srcN,
                                          const uint64_t srcTileMask)
{
#ifdef SNAPSHOTTILE_FLOAT4_NUMSAMPLE_ISPC
    return ispc::snapshotTileFloat4NumSample(reinterpret_cast<int*>(dstV),
                                             reinterpret_cast<int*>(dstN),
                                             dstTileMask,
                                             const_cast<int*>(reinterpret_cast<const int*>(srcV)),
                                             const_cast<int*>(reinterpret_cast<const int*>(srcN)),
                                             srcTileMask);
#else // else SNAPSHOTTILE_FLOAT4_NUMSAMPLE_ISPC
    return snapshotTileFloat4NumSample_SISD(dstV, dstN, dstTileMask, srcV, srcN, srcTileMask);
#endif // end else SNAPSHOTTILE_FLOAT4_NUMSAMPLE_ISPC    
}
    
// static function
uint64_t
SnapshotUtil::snapshotTileFloat4NumSample_SISD(uint32_t* dstV,
                                               uint32_t* dstN,
                                               const uint64_t dstTileMask,
                                               const uint32_t* srcV,
                                               const uint32_t* srcN,
                                               const uint64_t srcTileMask)
{
    if (!srcTileMask) return 0x0;

    uint64_t activePixelMask = static_cast<uint64_t>(0x0);
    for (unsigned y = 0; y < 8; ++y) {
        unsigned offset = (y << 3); // y * 8

        uint64_t currSrcTileMask = srcTileMask >> offset;
        if (!currSrcTileMask) break; // early exit : rest of them are all empty
        uint64_t currDstTileMask = dstTileMask >> offset;

        const uint32_t* currSrcV = srcV + offset * 4;
        const uint32_t* currSrcN = srcN + offset;
        uint32_t*       currDstV = dstV + offset * 4;
        uint32_t*       currDstN = dstN + offset;

        uint64_t currSrcTileScanlineMask = currSrcTileMask & static_cast<uint64_t>(0xff); // get one scanline mask
        uint64_t currDstTileScanlineMask = currDstTileMask & static_cast<uint64_t>(0xff); // get one scanline mask
        for (unsigned x = 0; x < 8; ++x) {
            if (!currSrcTileScanlineMask) break; // early exit for scanline

            if (currSrcTileScanlineMask & static_cast<uint64_t>(0x1)) {
                //
                // Compare bit pattern between previous and current
                // See more detail for snapshotTileFloat4Weight_SISD()'s comment
                //
                uint32_t freshPixelCondition = (currDstTileScanlineMask & static_cast<uint64_t>(0x1))? 0x0: 0xffffffff;
#ifdef IMPL_HYBRID
                uint32_t activeMask = ((currSrcV[0] - currDstV[0]) |
                                       (currSrcV[1] - currDstV[1]) |
                                       (currSrcV[2] - currDstV[2]) |
                                       (currSrcV[3] - currDstV[3]) |
                                       (currSrcN[0] - currDstN[0]) |
                                       freshPixelCondition) && currSrcN[0];
                if (activeMask) {
#endif // end IMPL_HYBRID
#ifdef IMPL_FULLBITOP
                uint32_t activeMask = ((currSrcV[0] - currDstV[0]) |
                                       (currSrcV[1] - currDstV[1]) |
                                       (currSrcV[2] - currDstV[2]) |
                                       (currSrcV[3] - currDstV[3]) |
                                       (currSrcN[0] - currDstN[0]) |
                                       freshPixelCondition) & ((currSrcN[0]) ? ~0x0 : 0x0);
                if (activeMask) {
#endif // end IMPL_FULLBITOP
#ifdef IMPL_NAIVELOGICAL
                bool activeFlag = ((currSrcV[0] - currDstV[0]) != 0x0 ||
                                   (currSrcV[1] - currDstV[1]) != 0x0 ||
                                   (currSrcV[2] - currDstV[2]) != 0x0 ||
                                   (currSrcV[3] - currDstV[3]) != 0x0 ||
                                   (currSrcN[0] - currDstN[0]) != 0x0 ||
                                   freshPixelCondition) && currSrcN[0];
                if (activeFlag) {
#endif // end IMPL_NAIVELOGICAL
                    // updated pixel
                    currDstV[0] = currSrcV[0];
                    currDstV[1] = currSrcV[1];
                    currDstV[2] = currSrcV[2];
                    currDstV[3] = currSrcV[3];
                    currDstN[0] = currSrcN[0];
                    activePixelMask |= (static_cast<uint64_t>(0x1) << offset);
                }
            }

            offset++;
            currSrcV += 4;
            currDstV += 4;
            currSrcN++;
            currDstN++;
            currSrcTileScanlineMask >>= 1;
            currDstTileScanlineMask >>= 1;
        }
    }
    return activePixelMask;
}

// static function
uint64_t
SnapshotUtil::snapshotTileFloat4NumSample_SIMD(uint32_t* dstV,
                                               uint32_t* dstN,
                                               const uint64_t dstTileMask,
                                               const uint32_t* srcV,
                                               const uint32_t* srcN,
                                               const uint64_t srcTileMask)
{
    return ispc::snapshotTileFloat4NumSample(reinterpret_cast<int*>(dstV),
                                             reinterpret_cast<int*>(dstN),
                                             dstTileMask,
                                             const_cast<int*>(reinterpret_cast<const int*>(srcV)),
                                             const_cast<int*>(reinterpret_cast<const int*>(srcN)),
                                             srcTileMask);
}

//------------------------------------------------------------------------------

uint64_t
SnapshotUtil::snapshotTileUInt32WithMask(uint32_t* dst,
                                         const uint64_t dstTileMask,
                                         const uint32_t* src,
                                         const uint64_t srcTileMask)
{
#ifdef SNAPSHOTTILE_UINT32_MASK_ISPC
    return ispc::snapshotTileUInt32WithMask(reinterpret_cast<int *>(dst),
                                            dstTileMask,
                                            const_cast<int *>(reinterpret_cast<const int *>(src)),
                                            srcTileMask);
#else // else SNAPSHOTTILE_UINT32_MASK_ISPC
    return snapshotTileUInt32WithMask_SISD(dst, dstTileMask, src, srcTileMask);
#endif // end else SNAPSHOTTILE_UINT32_MASK_ISPC    
}

// static function
uint64_t
SnapshotUtil::snapshotTileUInt32WithMask_SISD(uint32_t* dst,
                                              const uint64_t dstTileMask,
                                              const uint32_t* src,
                                              const uint64_t srcTileMask)
{
    if (!srcTileMask) return 0x0;

    uint64_t activePixelMask = static_cast<uint64_t>(0x0);
    for (unsigned y = 0; y < 8; ++y) {
        unsigned offset = (y << 3); // y * 8

        uint64_t currSrcTileMask = srcTileMask >> offset;
        if (!currSrcTileMask) break; // early exit : rest of them are all empty
        uint64_t currDstTileMask = dstTileMask >> offset;

        const uint32_t* currSrc = src + offset;
        uint32_t*       currDst = dst + offset;

        uint64_t currSrcTileScanlineMask = currSrcTileMask & static_cast<uint64_t>(0xff); // get one scanline mask
        uint64_t currDstTileMaskScanline = currDstTileMask & static_cast<uint64_t>(0xff); // get one scanline mask
        for (unsigned x = 0; x < 8; ++x) {
            if (!currSrcTileScanlineMask) break; // early exit for scanline

            if (currSrcTileScanlineMask & static_cast<uint64_t>(0x1)) {
                //
                // Compare bit pattern between previous and current
                // See more detail for snapshotTileFloat4Weight_SISD()'s comment                
                //
                uint32_t freshPixelCondition = (currDstTileMaskScanline & static_cast<uint64_t>(0x1))? 0x0: 0xffffffff;
#ifdef IMPL_HYBRID
                uint32_t activeMask = ((currSrc[0] - currDst[0]) |
                                       freshPixelCondition) && currSrc[0];
                if (activeMask) {
#endif // end IMPL_HYBRID
#ifdef IMPL_FULLBITOP
                uint32_t activeMask = ((currSrc[0] - currDst[0]) |
                                       freshPixelCondition) & ((currSrc[0]) ? ~0x0 : 0x0);
                if (activeMask) {
#endif // end IMPL_FULLBITOP
#ifdef IMPL_NAIVELOGICAL
                bool activeFlag = ((currSrc[0] - currDst[0]) != 0x0 ||
                                   freshPixelCondition) && currSrc[0];
                if (activeFlag) {
#endif // end IMPL_NAIVELOGICAL
                    // updated pixel
                    currDst[0] = currSrc[0];
                    activePixelMask |= (static_cast<uint64_t>(0x1) << offset);
                }
            }

            offset++;
            currSrc++;
            currDst++;
            currSrcTileScanlineMask >>= 1;
            currDstTileMaskScanline >>= 1;
        }
    }
    return activePixelMask;
}

// static function
uint64_t
SnapshotUtil::snapshotTileUInt32WithMask_SIMD(uint32_t* dst,
                                              const uint64_t dstTileMask,
                                              const uint32_t* src,
                                              const uint64_t srcTileMask)
{
    return ispc::snapshotTileUInt32WithMask(reinterpret_cast<int *>(dst),
                                            dstTileMask,
                                            const_cast<int *>(reinterpret_cast<const int *>(src)),
                                            srcTileMask);
}

// static function
std::string
SnapshotUtil::showMask(const uint64_t mask64)
{
    std::ostringstream ostr;
    ostr << "mask 0x" << std::setw(16) << std::setfill('0') << std::hex << mask64 << " {\n";
    for (int y = 7; y >= 0; --y) {
        ostr << "  ";
        for (int x = 0; x < 8; ++x) {
            if (mask64 & (static_cast<uint64_t>(0x1) << ((y << 3) + x))) {
                // ostr << "* ";
                ostr << std::setw(2) << std::oct << ((y << 3) + x) << ' ';
            } else {
                // ostr << ". ";
                ostr << " . ";
            }
        }
        ostr << '\n';
    }
    ostr << "}";
    return ostr.str();
}

} // namespace fb_util
} // namespace scene_rdl2
