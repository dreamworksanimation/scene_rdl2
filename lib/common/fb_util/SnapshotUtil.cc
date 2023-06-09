// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#include <scene_rdl2/common/fb_util/ispc/SnapshotUtil_ispc_stubs.h>

#include "SnapshotUtil.h"

#include <iomanip>
#include <iostream>
#include <sstream>

//#define AVX2_TEST // experimental code for AVX2 instruction
#ifdef AVX2_TEST
#include <immintrin.h>          // AVX
#endif // end AVX2_TEST

namespace scene_rdl2 {
namespace fb_util {

//------------------------------------------------------------------------------
//
// beauty buffer
//
// static function
uint64_t
SnapshotUtil::snapshotTileColorWeight(uint32_t *dstC,
                                      uint32_t *dstW,
                                      const uint32_t *srcC,
                                      const uint32_t *srcW)
//
// make snapshot for color + weight data
// update destination buffers and return active pixel mask for this tile
//
// This is an ISPC version
//
// dstC : destination tile start address of color  data : color  buffer (r,g,b,a) = 16byte * 8 * 8
// dstW : destination tile start address of weight data : weight buffer (w)       =  4byte * 8 * 8
// srcC :      source tile start address of color  data : color  buffer (r,g,b,a) = 16byte * 8 * 8
// srcW :      source tile start address of weight data : weight buffer (w)       =  4byte * 8 * 8
//
{
    return ispc::snapshotTileFloat4Weight((int *)dstC, (int *)dstW,
                                          const_cast<int *>((const int *)srcC),
                                          const_cast<int *>((const int *)srcW));
}

#ifdef AVX2_TEST
// static function
uint64_t
SnapshotUtil::snapshotTileColorWeight_AVX2(uint32_t *dstCPtr,
                                           uint32_t *dstWPtr,
                                           const uint32_t *srcCPtr,
                                           const uint32_t *srcWPtr)
//
// make snapshot for color + weight data
// update destination buffers and return active pixel mask for this tile
//
// This is AVX2 version (experimental)
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
SnapshotUtil::snapshotTileColorNumSample(uint32_t *dstC,
                                         uint32_t *dstN,
                                         const uint64_t dstTileMask,
                                         const uint32_t *srcC,
                                         const uint32_t *srcN,
                                         const uint64_t srcTileMask)
{
    return ispc::snapshotTileFloat4NumSample((int *)dstC, (int *)dstN, dstTileMask,
                                             const_cast<int *>((const int *)srcC),
                                             const_cast<int *>((const int *)srcN),
                                             srcTileMask);
}

//------------------------------------------------------------------------------
//
// heatMap
//
// static function
uint64_t
SnapshotUtil::snapshotTileHeatMapWeight(uint64_t *dstV,
                                        uint32_t *dstW,
                                        const uint64_t *srcV,
                                        const uint32_t *srcW)
{
    /*
      I switched back to SISD version due to this ispc function crashing under the following reflplat.
      variant=3 : refplat-houdini165.1
      variant=6 : refplat-maya2018.1
      variant=7 : refplat-vfx2018.2 icc-17.0
      Other refplat is fine. Toshi (Oct/24/2020)
      
    return ispc::snapshotTileHeatMapWeight((int64_t *)dstV, (int *)dstW,
                                           const_cast<int64_t*>((const int64_t*)srcV),
                                           const_cast<int *>((const int *)srcW));
    */
    return snapshotTileHeatMapWeight_SISD(dstV, dstW, srcV, srcW);
}
    
// static function
uint64_t
SnapshotUtil::snapshotTileHeatMapWeight_SISD(uint64_t *dstV,
                                             uint32_t *dstW,
                                             const uint64_t *srcV,
                                             const uint32_t *srcW)
{
    uint64_t activePixelMask = static_cast<uint64_t>(0x0);
    for (unsigned y = 0; y < 8; ++y) {
        unsigned offset = (y << 3);
        const uint64_t *currSrcVPtr = srcV + offset;
        const uint32_t *currSrcWPtr = srcW + offset;
        uint64_t       *currDstVPtr = dstV + offset;
        uint32_t       *currDstWPtr = dstW + offset;

        for (unsigned x = 0; x < 8; ++x) {
            //
            // Compare bit pattern between previous and current
            //
            uint64_t activeMask = ((currSrcVPtr[0] - currDstVPtr[0]) |
                                   static_cast<uint64_t>(currSrcWPtr[0] - currDstWPtr[0])); // cast to 64bit
            if (activeMask) {
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
SnapshotUtil::snapshotTileHeatMapNumSample(uint32_t *dstV,
                                           uint32_t *dstN,
                                           const uint64_t dstTileMask,
                                           const uint32_t *srcV,
                                           const uint32_t *srcN,
                                           const uint64_t srcTileMask)
{
    return ispc::snapshotTileFloatNumSample((int *)dstV, (int *)dstN, dstTileMask,
                                            const_cast<int *>((const int *)srcV),
                                            const_cast<int *>((const int *)srcN),
                                            srcTileMask);
}

//------------------------------------------------------------------------------
//
// weight buffer
//
// static function
uint64_t
SnapshotUtil::snapshotTileWeightBuffer(uint32_t *dst,
                                       const uint32_t *src)
{
    /*
      I switched back to SISD version due to this ispc function crashing under the following reflplat.
      variant=3 : refplat-houdini165.1
      variant=6 : refplat-maya2018.1
      variant=7 : refplat-vfx2018.2 icc-17.0
      Other refplat is fine. Toshi (Oct/24/2020)

    return ispc::snapshotTileWeightBuffer((int *)dst,
                                          const_cast<int *>((const int *)src));
    */
    return snapshotTileWeightBuffer_SISD(dst, src);
}
    
// static function
uint64_t
SnapshotUtil::snapshotTileWeightBuffer_SISD(uint32_t *dst,
                                            const uint32_t *src)
{
    uint64_t activePixelMask = static_cast<uint64_t>(0x0);
    for (unsigned y = 0; y < 8; ++y) {
        unsigned offset = (y << 3);
        const uint32_t *currSrc = src + offset;
        uint32_t       *currDst = dst + offset;

        for (unsigned x = 0; x < 8; ++x) {
            //
            // Compare bit pattern between previous and current
            //
            uint32_t activeMask = (currSrc[0] - currDst[0]);
            if (activeMask) {
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

//------------------------------------------------------------------------------
//
// renderOutput
//

// static function
uint64_t
SnapshotUtil::snapshotTileFloatWeight(uint32_t *dstV,
                                      uint32_t *dstW,
                                      const uint32_t *srcV,
                                      const uint32_t *srcW)
{
    /*
      I switched back to SISD version due to this ispc function crashing under the following reflplat.
      variant=3 : refplat-houdini165.1
      variant=6 : refplat-maya2018.1
      variant=7 : refplat-vfx2018.2 icc-17.0
      Other refplat is fine. Toshi (Oct/24/2020)

    return ispc::snapshotTileFloatWeight((int *)dstV, (int *)dstW,
                                         const_cast<int *>((const int *)srcV),
                                         const_cast<int *>((const int *)srcW));
    */
    return snapshotTileFloatWeight_SISD(dstV, dstW, srcV, srcW);
}

// static function
uint64_t
SnapshotUtil::snapshotTileFloatWeight_SISD(uint32_t *dstV,
                                           uint32_t *dstW,
                                           const uint32_t *srcV,
                                           const uint32_t *srcW)
{
    uint64_t activePixelMask = static_cast<uint64_t>(0x0);
    for (unsigned y = 0; y < 8; ++y) {
        unsigned offset = (y << 3);
        const uint32_t *currSrcV = srcV + offset;
        const uint32_t *currSrcW = srcW + offset;
        uint32_t *currDstV = dstV + offset;
        uint32_t *currDstW = dstW + offset;

        for (unsigned x = 0; x < 8; ++x) {
            //
            // Compare bit pattern between previous and current
            //
            uint32_t activeMask = ((currSrcV[0] - currDstV[0]) |
                                   (currSrcW[0] - currDstW[0]));
            if (activeMask) {
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
SnapshotUtil::snapshotTileFloatNumSample(uint32_t *dstV,
                                         uint32_t *dstN,
                                         const uint64_t dstTileMask,
                                         const uint32_t *srcV,
                                         const uint32_t *srcN,
                                         const uint64_t srcTileMask)
{
    return ispc::snapshotTileFloatNumSample((int *)dstV, (int *)dstN, dstTileMask,
                                            const_cast<int *>((const int *)srcV),
                                            const_cast<int *>((const int *)srcN),
                                            srcTileMask);
}

// static function
uint64_t
SnapshotUtil::snapshotTileFloatNumSample_SISD(uint32_t *dstV,
                                              uint32_t *dstN,
                                              const uint64_t dstTileMask,
                                              const uint32_t *srcV,
                                              const uint32_t *srcN,
                                              const uint64_t srcTileMask)
{
    if (!srcTileMask) return 0x0;

    uint64_t activePixelMask = static_cast<uint64_t>(0x0);
    for (unsigned y = 0; y < 8; ++y) {
        unsigned offset = (y << 3); // y * 8

        uint64_t currSrcTileMask = srcTileMask >> offset;
        if (!currSrcTileMask) break; // early exit : rest of them are all empty
        uint64_t currDstTileMask = dstTileMask >> offset;

        const uint32_t *currSrcV = srcV + offset;
        uint32_t       *currDstV = dstV + offset;
        const uint32_t *currSrcN = srcN + offset;
        uint32_t       *currDstN = dstN + offset;

        uint64_t currSrcTileScanlineMask = currSrcTileMask & static_cast<uint64_t>(0xff); // get one scanline mask
        uint64_t currDstTileScanlineMask = currDstTileMask & static_cast<uint64_t>(0xff); // get one scanline mask
        for (unsigned x = 0; x < 8; ++x) {
            if (!currSrcTileScanlineMask) break; // early exit for scanline

            if (currSrcTileScanlineMask & static_cast<uint64_t>(0x1)) {
                //
                // Compare bit pattern between previous and current
                //
                uint32_t freshPixelCondition = (currDstTileScanlineMask & static_cast<uint64_t>(0x1))? 0x0: 0xffffffff;
                uint32_t activeMask = ((currSrcV[0] - currDstV[0]) |
                                       (currSrcN[0] - currDstN[0]) |
                                       freshPixelCondition);
                if (activeMask) {
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
SnapshotUtil::snapshotTileFloat2Weight(uint32_t *dstV,
                                       uint32_t *dstW,
                                       const uint32_t *srcV,
                                       const uint32_t *srcW)
{
    /*
      I switched back to SISD version due to this ispc function crashing under the following reflplat.
      variant=3 : refplat-houdini165.1
      variant=6 : refplat-maya2018.1
      variant=7 : refplat-vfx2018.2 icc-17.0
      Other refplat is fine. Toshi (Oct/24/2020)

    return ispc::snapshotTileFloat2Weight((int *)dstV, (int64_t *)dstW,
                                          const_cast<int *>((const int *)srcV),
                                          const_cast<int64_t *>((const int64_t *)srcW));
    */
    return snapshotTileFloat2Weight_SISD(dstV, dstW, srcV, srcW);
}

// static function
uint64_t
SnapshotUtil::snapshotTileFloat2Weight_SISD(uint32_t *dstV,
                                            uint32_t *dstW,
                                            const uint32_t *srcV,
                                            const uint32_t *srcW)
{
    uint64_t activePixelMask = static_cast<uint64_t>(0x0);
    for (unsigned y = 0; y < 8; ++y) {
        unsigned offset = (y << 3);
        const uint32_t *currSrcV = srcV + offset * 2;
        const uint32_t *currSrcW = srcW + offset;
        uint32_t *currDstV = dstV + offset * 2;
        uint32_t *currDstW = dstW + offset;

        for (unsigned x = 0; x < 8; ++x) {
            //
            // Compare bit pattern between previous and current
            //
            uint32_t activeMask = ((currSrcV[0] - currDstV[0]) |
                                   (currSrcV[1] - currDstV[1]) |
                                   (currSrcW[0] - currDstW[0]));
            if (activeMask) {
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
SnapshotUtil::snapshotTileFloat2NumSample(uint32_t *dstV,
                                          uint32_t *dstN,
                                          const uint64_t dstTileMask,
                                          const uint32_t *srcV,
                                          const uint32_t *srcN,
                                          const uint64_t srcTileMask)
{
    return ispc::snapshotTileFloat2NumSample((int *)dstV, (int64_t *)dstN, dstTileMask,
                                             const_cast<int *>((const int *)srcV),
                                             const_cast<int64_t *>((const int64_t *)srcN),
                                             srcTileMask);
}

// static function
uint64_t
SnapshotUtil::snapshotTileFloat2NumSample_SISD(uint32_t *dstV,
                                               uint32_t *dstN,
                                               const uint64_t dstTileMask,
                                               const uint32_t *srcV,
                                               const uint32_t *srcN,
                                               const uint64_t srcTileMask)
{
    if (!srcTileMask) return 0x0;

    uint64_t activePixelMask = static_cast<uint64_t>(0x0);
    for (unsigned y = 0; y < 8; ++y) {
        unsigned offset = (y << 3); // y * 8

        uint64_t currSrcTileMask = srcTileMask >> offset;
        if (!currSrcTileMask) break; // early exit : rest of them are all empty
        uint64_t currDstTileMask = dstTileMask >> offset;

        const uint32_t *currSrcV = srcV + offset * 2;
        uint32_t       *currDstV = dstV + offset * 2;
        const uint32_t *currSrcN = srcN + offset;
        uint32_t       *currDstN = dstN + offset;

        uint64_t currSrcTileScanlineMask = currSrcTileMask & static_cast<uint64_t>(0xff); // get one scanline mask
        uint64_t currDstTileScanlineMask = currDstTileMask & static_cast<uint64_t>(0xff); // get one scanline mask
        for (unsigned x = 0; x < 8; ++x) {
            if (!currSrcTileScanlineMask) break; // early exit for scanline

            if (currSrcTileScanlineMask & static_cast<uint64_t>(0x1)) {
                //
                // Compare bit pattern between previous and current
                //
                uint32_t freshPixelCondition = (currDstTileScanlineMask & static_cast<uint64_t>(0x1))? 0x0: 0xffffffff;
                uint32_t activeMask = ((currSrcV[0] - currDstV[0]) |
                                       (currSrcV[1] - currDstV[1]) |
                                       (currSrcN[0] - currDstN[0]) |
                                       freshPixelCondition);
                if (activeMask) {
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
SnapshotUtil::snapshotTileFloat3Weight(uint32_t *dstV,
                                       uint32_t *dstW,
                                       const uint32_t *srcV,
                                       const uint32_t *srcW)
{
    /*
      I switched back to SISD version due to this ispc function crashing under the following reflplat.
      variant=3 : refplat-houdini165.1
      variant=6 : refplat-maya2018.1
      variant=7 : refplat-vfx2018.2 icc-17.0
      Other refplat is fine. Toshi (Oct/24/2020)

    return ispc::snapshotTileFloat3Weight((int *)dstV, (int *)dstW,
                                          const_cast<int *>((const int *)srcV),
                                          const_cast<int *>((const int *)srcW));
    */
    return snapshotTileFloat3Weight_SISD(dstV, dstW, srcV, srcW);
}

// static function
uint64_t
SnapshotUtil::snapshotTileFloat3Weight_SISD(uint32_t *dstV,
                                            uint32_t *dstW,
                                            const uint32_t *srcV,
                                            const uint32_t *srcW)
{
    uint64_t activePixelMask = static_cast<uint64_t>(0x0);
    for (unsigned y = 0; y < 8; ++y) {
        unsigned offset = (y << 3);
        const uint32_t *currSrcV = srcV + offset * 3;
        const uint32_t *currSrcW = srcW + offset;
        uint32_t *currDstV = dstV + offset * 3;
        uint32_t *currDstW = dstW + offset;

        for (unsigned x = 0; x < 8; ++x) {
            //
            // Compare bit pattern between previous and current
            //
            uint32_t activeMask = ((currSrcV[0] - currDstV[0]) |
                                   (currSrcV[1] - currDstV[1]) |
                                   (currSrcV[2] - currDstV[2]) |
                                   (currSrcW[0] - currDstW[0]));
            if (activeMask) {
                // update data
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
SnapshotUtil::snapshotTileFloat3NumSample(uint32_t *dstV,
                                          uint32_t *dstN,
                                          const uint64_t dstTileMask,
                                          const uint32_t *srcV,
                                          const uint32_t *srcN,
                                          const uint64_t srcTileMask)
{
    return ispc::snapshotTileFloat3NumSample((int *)dstV, (int *)dstN, dstTileMask,
                                             const_cast<int *>((const int *)srcV),
                                             const_cast<int *>((const int *)srcN),
                                             srcTileMask);
}
    
// static function
uint64_t
SnapshotUtil::snapshotTileFloat3NumSample_SISD(uint32_t *dstV,
                                               uint32_t *dstN,
                                               const uint64_t dstTileMask,
                                               const uint32_t *srcV,
                                               const uint32_t *srcN,
                                               const uint64_t srcTileMask)
{
    if (!srcTileMask) return 0x0;

    uint64_t activePixelMask = static_cast<uint64_t>(0x0);
    for (unsigned y = 0; y < 8; ++y) {
        unsigned offset = (y << 3); // y * 8

        uint64_t currSrcTileMask = srcTileMask >> offset;
        if (!currSrcTileMask) break; // early exit : rest of them are all empty
        uint64_t currDstTileMask = dstTileMask >> offset;

        const uint32_t *currSrcV = srcV + offset * 3;
        uint32_t       *currDstV = dstV + offset * 3;
        const uint32_t *currSrcN = srcN + offset;
        uint32_t       *currDstN = dstN + offset;

        uint64_t currSrcTileScanlineMask = currSrcTileMask & static_cast<uint64_t>(0xff); // get one scanline mask
        uint64_t currDstTileScanlineMask = currDstTileMask & static_cast<uint64_t>(0xff); // get one scanline mask
        for (unsigned x = 0; x < 8; ++x) {
            if (!currSrcTileScanlineMask) break; // early exit for scanline

            if (currSrcTileScanlineMask & static_cast<uint64_t>(0x1)) {
                //
                // Compare bit pattern between previous and current
                //
                uint32_t freshPixelCondition = (currDstTileScanlineMask & static_cast<uint64_t>(0x1))? 0x0: 0xffffffff;
                uint32_t activeMask = ((currSrcV[0] - currDstV[0]) |
                                       (currSrcV[1] - currDstV[1]) |
                                       (currSrcV[2] - currDstV[2]) |
                                       (currSrcN[0] - currDstN[0]) |
                                       freshPixelCondition);
                if (activeMask) {
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
SnapshotUtil::snapshotTileFloat4Weight(uint32_t *dstV,
                                       uint32_t *dstW,
                                       const uint32_t *srcV,
                                       const uint32_t *srcW)
{
    /*
      I switched back to SISD version due to this ispc function crashing under the following reflplat.
      variant=3 : refplat-houdini165.1
      variant=6 : refplat-maya2018.1
      variant=7 : refplat-vfx2018.2 icc-17.0
      Other refplat is fine. Toshi (Oct/24/2020)

    return ispc::snapshotTileFloat4Weight((int *)dstV, (int *)dstW,
                                          const_cast<int *>((const int *)srcV),
                                          const_cast<int *>((const int *)srcW));
    */
    return snapshotTileFloat4Weight_SISD(dstV, dstW, srcV, srcW);
}

// static function
uint64_t
SnapshotUtil::snapshotTileFloat4Weight_SISD(uint32_t *dstV,
                                            uint32_t *dstW,
                                            const uint32_t *srcV,
                                            const uint32_t *srcW)
{
    uint64_t activePixelMask = static_cast<uint64_t>(0x0);
    for (unsigned y = 0; y < 8; ++y) {
        unsigned offset = (y << 3);
        const uint32_t *currSrcV = srcV + offset * 4;
        const uint32_t *currSrcW = srcW + offset;
        uint32_t *currDstV = dstV + offset * 4;
        uint32_t *currDstW = dstW + offset;

        for (unsigned x = 0; x < 8; ++x) {
            //
            // Compare bit pattern between previous and current
            //
            uint32_t activeMask = ((currSrcV[0] - currDstV[0]) |
                                   (currSrcV[1] - currDstV[1]) |
                                   (currSrcV[2] - currDstV[2]) |
                                   (currSrcV[3] - currDstV[3]) |
                                   (currSrcW[0] - currDstW[0]));
            if (activeMask) {
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

uint64_t
SnapshotUtil::snapshotTileFloat4NumSample(uint32_t *dstV,
                                          uint32_t *dstN,
                                          const uint64_t dstTileMask,
                                          const uint32_t *srcV,
                                          const uint32_t *srcN,
                                          const uint64_t srcTileMask)
{
    return ispc::snapshotTileFloat4NumSample((int *)dstV, (int *)dstN, dstTileMask,
                                             const_cast<int *>((const int *)srcV),
                                             const_cast<int *>((const int *)srcN),
                                             srcTileMask);
}
    
// static function
uint64_t
SnapshotUtil::snapshotTileFloat4NumSample_SISD(uint32_t *dstV,
                                               uint32_t *dstN,
                                               const uint64_t dstTileMask,
                                               const uint32_t *srcV,
                                               const uint32_t *srcN,
                                               const uint64_t srcTileMask)
{
    if (!srcTileMask) return 0x0;

    uint64_t activePixelMask = static_cast<uint64_t>(0x0);
    for (unsigned y = 0; y < 8; ++y) {
        unsigned offset = (y << 3); // y * 8

        uint64_t currSrcTileMask = srcTileMask >> offset;
        if (!currSrcTileMask) break; // early exit : rest of them are all empty
        uint64_t currDstTileMask = dstTileMask >> offset;

        const uint32_t *currSrcV = srcV + offset * 4;
        uint32_t       *currDstV = dstV + offset * 4;
        const uint32_t *currSrcN = srcN + offset;
        uint32_t       *currDstN = dstN + offset;

        uint64_t currSrcTileScanlineMask = currSrcTileMask & static_cast<uint64_t>(0xff); // get one scanline mask
        uint64_t currDstTileScanlineMask = currDstTileMask & static_cast<uint64_t>(0xff); // get one scanline mask
        for (unsigned x = 0; x < 8; ++x) {
            if (!currSrcTileScanlineMask) break; // early exit for scanline

            if (currSrcTileScanlineMask & static_cast<uint64_t>(0x1)) {
                //
                // Compare bit pattern between previous and current
                //
                uint32_t freshPixelCondition = (currDstTileScanlineMask & static_cast<uint64_t>(0x1))? 0x0: 0xffffffff;
                uint32_t activeMask = ((currSrcV[0] - currDstV[0]) |
                                       (currSrcV[1] - currDstV[1]) |
                                       (currSrcV[2] - currDstV[2]) |
                                       (currSrcV[3] - currDstV[3]) |
                                       (currSrcN[0] - currDstN[0]) |
                                       freshPixelCondition);
                if (activeMask) {
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

//------------------------------------------------------------------------------

uint64_t
SnapshotUtil::snapshotTileUInt32WithMask(uint32_t *dst,
                                         const uint64_t dstTileMask,
                                         const uint32_t *src,
                                         const uint64_t srcTileMask)
{
    return ispc::snapshotTileUInt32WithMask((int *)dst, dstTileMask,
                                            const_cast<int *>((const int *)src),
                                            srcTileMask);
}

// static function
uint64_t
SnapshotUtil::snapshotTileUInt32WithMask_SISD(uint32_t *dst,
                                              const uint64_t dstTileMask,
                                              const uint32_t *src,
                                              const uint64_t srcTileMask)
{
    if (!srcTileMask) return 0x0;

    uint64_t activePixelMask = static_cast<uint64_t>(0x0);
    for (unsigned y = 0; y < 8; ++y) {
        unsigned offset = (y << 3); // y * 8

        uint64_t currSrcTileMask = srcTileMask >> offset;
        if (!currSrcTileMask) break; // early exit : rest of them are all empty
        uint64_t currDstTileMask = dstTileMask >> offset;

        const uint32_t *currSrc = src + offset;
        uint32_t       *currDst = dst + offset;

        uint64_t currSrcTileScanlineMask = currSrcTileMask & static_cast<uint64_t>(0xff); // get one scanline mask
        uint64_t currDstTileMaskScanline = currDstTileMask & static_cast<uint64_t>(0xff); // get one scanline mask
        for (unsigned x = 0; x < 8; ++x) {
            if (!currSrcTileScanlineMask) break; // early exit for scanline

            if (currSrcTileScanlineMask & static_cast<uint64_t>(0x1)) {
                uint32_t freshPixelCondition = (currDstTileMaskScanline & static_cast<uint64_t>(0x1))? 0x0: 0xffffffff;
                uint32_t activeMask = ((currSrc[0] - currDst[0]) |
                                       freshPixelCondition);
                if (activeMask) {
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

