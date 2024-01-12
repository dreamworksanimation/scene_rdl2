// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include <scene_rdl2/common/platform/Platform.isph>

//
// We have 3 different types of implementations for ISPC version. All the same results but different
// performances. IMPL_FULLBITOP is the best result so far but we keep all code for future testing on
// different compiler environment.
//
// We hove chosen IMPL_FULLBITOP based on several different timing test run on
// Intel Xeon Gold 6140 2.3GHz by GCC9.3, ISPC1.20 @ Sep/15/2023
//    
//#define IMPL_HYBRID // original
#define IMPL_FULLBITOP // full bit operation
//#define IMPL_NAIVELOGICAL // naive logical

export uniform uint64_t
snapshotTileHeatMapWeight(uniform int64 dstVBuff[],
                          uniform int   dstWBuff[],
                          uniform int64 srcVBuff[],
                          uniform int   srcWBuff[])
// This is an ISPC version of scene_rdl2::fb_util::SnapshotUtil::snapshotTileHeatMapWeight_SISD().
{
    uniform int loopMax = 64 / programCount;
    uniform uint64_t mask = (uint64_t)0x0;
    for (uniform int loopId = 0; loopId < loopMax; ++loopId) {
        int offset = loopId * programCount + programIndex;

        int64 dstV = dstVBuff[offset];
        int   dstW = dstWBuff[offset];
        int64 srcV = srcVBuff[offset];
        int   srcW = srcWBuff[offset];

#ifdef IMPL_HYBRID
        int64 activeMask = ((int64)(srcW - dstW) | (srcV - dstV)) && (int64)srcW;
        if (activeMask) {
#endif // end IMPL_HYBRID
#ifdef IMPL_FULLBITOP
        int64 srcWFlag = (srcW) ? ~(int64)0x0 : (int64)0x0;
        int64 activeMask = ((int64)(srcW - dstW) | (srcV - dstV)) & srcWFlag;
        if (activeMask) {
#endif // end IMPL_FULLBITOP
#ifdef IMPL_NAIVELOGICAL
        bool activeFlag = (((srcW - dstW) != 0x0) || ((srcV - dstV) != (int64)0x0)) && srcW; 
        if (activeFlag) {
#endif // end IMPL_NAIVELOGICAL
            dstVBuff[offset] = srcV;
            dstWBuff[offset] = srcW;

            mask |= ((uint64_t)(lanemask()) << (loopId * programCount));
        }
    }
    return mask;
}

export uniform uint64_t
snapshotTileWeightBuffer(uniform int dstBuff[],
                         uniform int srcBuff[])
// This is an ISPC version of scene_rdl2::fb_util::SnapshotUtil::snapshotTileWeightBuffer_SISD().
{
    uniform int loopMax = 64 / programCount;
    uniform uint64_t mask = (uint64_t)0x0;
    for (uniform int loopId = 0; loopId < loopMax; ++loopId) {
        int offset = loopId * programCount + programIndex;

        int dst = dstBuff[offset];
        int src = srcBuff[offset];

#ifdef IMPL_HYBRID
        int activeMask = (src - dst) && src;
        if (activeMask) {
#endif // end IMPL_HYBRID
#ifdef IMPL_FULLBITOP
        int activeMask = (src - dst) & ((src) ? ~0x0 : 0x0);
        if (activeMask) {
#endif // end IMPL_FULLBITOP
#ifdef IMPL_NAIVELOGICAL
        bool activeFlag = ((src - dst) != 0x0) && src;
        if (activeFlag) {
#endif // end IMPL_NAIVELOGICAL
            dstBuff[offset] = src;

            mask |= ((uint64_t)(lanemask()) << (loopId * programCount));
        }
    }
    return mask;
}

export uniform uint64_t
snapshotTileFloatWeight(uniform int dstVBuff[],
                        uniform int dstWBuff[],
                        uniform int srcVBuff[],
                        uniform int srcWBuff[])
// This is an ISPC version of scene_rdl2::fb_util::SnapshotUtil::snapshotTileFloatWeight_SISD().
{
    uniform int loopMax = 64 / programCount;
    uniform uint64_t mask = (uint64_t)0x0;
    for (uniform int loopId = 0; loopId < loopMax; ++loopId) {
        int offset = loopId * programCount + programIndex;

        int dstV = dstVBuff[offset];
        int dstW = dstWBuff[offset];
        int srcV = srcVBuff[offset];
        int srcW = srcWBuff[offset];

#ifdef IMPL_HYBRID
        int activeMask = ((srcV - dstV) | (srcW - dstW)) && srcW;
        if (activeMask) {
#endif // end IMPL_HYBRID
#ifdef IMPL_FULLBITOP
        int activeMask = ((srcV - dstV) | (srcW - dstW)) & ((srcW) ? ~0x0 : 0x0);
        if (activeMask) {
#endif // end IMPL_FULLBITOP
#ifdef IMPL_NAIVELOGICAL
        bool activeFlag = (((srcV - dstV) != 0x0) || ((srcW - dstW) != 0x0)) && srcW;
        if (activeFlag) {
#endif // end IMPL_NAIVELOGICAL
            dstVBuff[offset] = srcV;
            dstWBuff[offset] = srcW;

            mask |= ((uint64_t)(lanemask()) << (loopId * programCount));
        }
    }
    return mask;
}


export uniform uint64_t
snapshotTileFloatNumSample(uniform int dstVBuff[],
                           uniform int dstNBuff[],
                           uniform uint64_t dstTileMask,
                           uniform int srcVBuff[],
                           uniform int srcNBuff[],
                           uniform uint64_t srcTileMask)
// This is an ISPC version of scene_rdl2::fb_util::SnapshotUtil::snapshotTileFloatNumSample_SISD().
{
    if (!srcTileMask) return (uint64_t)0x0;

    uniform int fullGangMask = lanemask();
    MNRY_ASSERT(fullGangMask & (1 << programIndex));

    uniform int loopMax = 64 / programCount;
    uniform uint64_t mask = (uint64_t)0x0;
    for (uniform int loopId = 0; loopId < loopMax; ++loopId) {
        uniform int offset = loopId * programCount;

        uniform uint32_t currSrcMask = (uint32_t)((uint64_t)(srcTileMask >> offset) & fullGangMask);
        if (currSrcMask) {
            uniform uint32_t currDstMask = (uint32_t)((uint64_t)(dstTileMask >> offset) & fullGangMask);
            if (currSrcMask & (uint32_t)(0x1 << programIndex)) {
                int dstV = dstVBuff[offset + programIndex];
                int dstN = dstNBuff[offset + programIndex];

                int srcV = srcVBuff[offset + programIndex];
                int srcN = srcNBuff[offset + programIndex];
                            
                int freshPixCondition = !(currDstMask & (int)(0x1 << programIndex));
#ifdef IMPL_HYBRID
                int activeMask = ((srcV - dstV) | (srcN - dstN) | freshPixCondition) && srcN;
                if (activeMask) {
#endif // end IMPL_HYBRID
#ifdef IMPL_FULLBITOP
                int activeMask = ((srcV - dstV) | (srcN - dstN) | freshPixCondition) & ((srcN) ? ~0x0 : 0x0);
                if (activeMask) {
#endif // end IMPL_FULLBITOP
#ifdef IMPL_NAIVELOGICAL
                bool activeFlag = ((srcV - dstV) != 0x0 || (srcN - dstN) != 0x0 || freshPixCondition) && srcN;
                if (activeFlag) {
#endif // end IMPL_NAIVELOGICAL
                    dstVBuff[offset + programIndex] = srcV;
                    dstNBuff[offset + programIndex] = srcN;

                    mask |= ((uint64_t)(lanemask()) << offset);
                }
            }
        }
    }
    return mask;
}

/* This is experimental code for snapshotTileFloat2Weight.
   There is no aos_to_soa2() inside ISPC lib, so I created soa input data before calling this function.
   However it's slower than original non SIMD version. Toshi
export uniform uint64_t
snapshotTileFloat2Weight(uniform int dstVaBuff[],
                         uniform int dstVbBuff[],
                         uniform int dstWBuff[],
                         uniform int srcVaBuff[],
                         uniform int srcVbBuff[],
                         uniform int srcWBuff[])
{
    uniform int loopMax = 64 / programCount;
    uint64_t mask = (uint64_t)0x0;
    for (uniform int loopId = 0; loopId < loopMax; ++loopId) {
        int offset = loopId * programCount + programIndex;

        int dstVa = dstVaBuff[offset];
        int dstVb = dstVbBuff[offset];
        int dstW  = dstWBuff[offset];
        int srcVa = srcVaBuff[offset];
        int srcVb = srcVbBuff[offset];
        int srcW  = srcWBuff[offset];

#ifdef IMPL_HYBRID
        int activeMask = ((srcVa - dstVa) | (srcVb - dstVb) | (srcW - dstW)) && srcW;
        if (activeMask) {
#endif // end IMPL_HYBRID
#ifdef IMPL_FULLBITOP
        int activeMask = ((srcVa - dstVa) | (srcVb - dstVb) | (srcW - dstW)) & ((srcW) ? ~0x0 : 0x0);
        if (activeMask) {
#endif // end IMPL_FULLBITOP
#ifdef IMPL_NAIVELOGICAL
        bool activeFlag = ((srcVa - dstVa) != 0x0 || (srcVb - dstVb) != 0x0 || (srcW - dstW) != 0x0) && srcW;
        if (activeFlag) {
#endif // end IMPL_NAIVELOGICAL
            mask |= (uint64_t)0x1 << offset;

            dstWBuff[offset] = srcWBuff[offset];
        }
    }
    return extract(mask, programCount - 1) | extract(exclusive_scan_or(mask), programCount - 1);
}
*/

export uniform uint64_t
snapshotTileFloat2Weight(uniform int   dstVBuff[],
                         uniform int64 dstWBuff[],
                         uniform int   srcVBuff[],
                         uniform int64 srcWBuff[])
// This is an ISPC version of scene_rdl2::fb_util::SnapshotUtil::snapshotTileFloat2Weight_SISD().
// We don't have aos_to_soa2() inside ISPC lib, so I used aos_to_soa4() and processed 2 pixels at once
// The downside is that this function updates the destination buffer by 2 pixels at once when we found
// active pixels. We need a more clever way to optimize.
{
    uniform int loopMax = 64 / programCount / 2;
    uint64_t mask = (uint64_t)0x0;
    for (uniform int loopId = 0; loopId < loopMax; ++loopId) {
        uniform int offset = loopId * programCount;
        uniform int offsetV = offset * 4;

        int dstR, dstG, dstB, dstA;
        aos_to_soa4(&dstVBuff[offsetV], &dstR, &dstG, &dstB, &dstA);
        int64 dstW = dstWBuff[offset + programIndex];
        /* int32 is better
        int64 dstWa = dstW & (int64)0x00000000ffffffff;
        int64 dstWb = dstW & (int64)0xffffffff00000000;
        */
        int dstWa = dstW;
        int dstWb = dstW >> 32;

        int srcR, srcG, srcB, srcA;
        aos_to_soa4(&srcVBuff[offsetV], &srcR, &srcG, &srcB, &srcA);
        int64 srcW = srcWBuff[offset + programIndex];
        /* int32 is better
        int64 srcWa = srcW & (int64)0x00000000ffffffff;
        int64 srcWb = srcW & (int64)0xffffffff00000000;
        */
        int srcWa = srcW;
        int srcWb = srcW >> 32;

        bool active = false;
#ifdef IMPL_HYBRID
        if (((srcR - dstR) | (srcG - dstG) | (srcWa - dstWa)) && srcWa) {
            mask |= (uint64_t)0x1 << ((offset + programIndex) * 2);
            active = true;
        }
        if (((srcB - dstB) | (srcA - dstA) | (srcWb - dstWb)) && srcWb) {
            mask |= (uint64_t)0x1 << ((offset + programIndex) * 2 + 1);
            active = true;
        }
#endif // end IMPL_HYBRID
#ifdef IMPL_FULLBITOP
        if (((srcR - dstR) | (srcG - dstG) | (srcWa - dstWa)) & ((srcWa) ? ~0x0 : 0x0)) {
            mask |= (uint64_t)0x1 << ((offset + programIndex) * 2);
            active = true;
        }
        if (((srcB - dstB) | (srcA - dstA) | (srcWb - dstWb)) & ((srcWb) ? ~0x0 : 0x0)) {
            mask |= (uint64_t)0x1 << ((offset + programIndex) * 2 + 1);
            active = true;
        }
#endif // end IMPL_FULLBITOP
#ifdef IMPL_NAIVELOGICAL
        if (((srcR - dstR) != 0x0 || (srcG - dstG) != 0x0 | (srcWa - dstWa) != 0x0) && srcWa) {
            mask |= (uint64_t)0x1 << ((offset + programIndex) * 2);
            active = true;
        }
        if (((srcB - dstB) != 0x0 || (srcA - dstA) != 0x0 | (srcWb - dstWb) != 0x0) && srcWb) {
            mask |= (uint64_t)0x1 << ((offset + programIndex) * 2 + 1);
            active = true;
        }
#endif // end IMPL_NAIVELOGICAL
        if (active) {
            soa_to_aos4(srcR, srcG, srcB, srcA, &dstVBuff[offsetV]);
            dstWBuff[offset + programIndex] = srcW;
        }
    }
    return extract(mask, programCount - 1) | extract(exclusive_scan_or(mask), programCount - 1);
}

export uniform uint64_t
snapshotTileFloat2NumSample(uniform int   dstVBuff[],
                            uniform int64 dstNBuff[],
                            uniform uint64_t dstTileMask,
                            uniform int   srcVBuff[],
                            uniform int64 srcNBuff[],
                            uniform uint64_t srcTileMask)
// This is an ISPC version of scene_rdl2::fb_util::SnapshotUtil::snapshotTileFloat2NumSample_SISD().
// We don't have aos_to_soa2() inside ISPC lib, so I used aos_to_soa4() and processed 2 pixels at once
// The downside is that this function updates the destination buffer by 2 pixels at once when we found
// active pixels. We need a more clever way to optimize.
{
    if (!srcTileMask) return (uint64_t)0x0;

    uniform int fullGangMask = lanemask();
    MNRY_ASSERT(fullGangMask & (1 << programIndex));
    uniform int fullGangMask2 = (fullGangMask << programCount) | fullGangMask;

    uniform int loopMax = 64 / programCount / 2;
    uint64_t mask = (uint64_t)0x0;
    for (uniform int loopId = 0; loopId < loopMax; ++loopId) {
        uniform int offset = loopId * programCount;
        uniform int offsetV = offset * 4;
        uniform int maskShift2 = offset * 2;

        uniform uint32_t currSrcMask2 = (uint32_t)((uint64_t)(srcTileMask >> maskShift2) & fullGangMask2);
        if (currSrcMask2) {
            uniform uint32_t currDstMask2 = (uint32_t)((uint64_t)(dstTileMask >> maskShift2) & fullGangMask2);
            if (currSrcMask2 & (uint32_t)(0x3 << (programIndex * 2))) { // test 2 adjoint pixels together
                int dstR, dstG, dstB, dstA;
                aos_to_soa4(&dstVBuff[offsetV], &dstR, &dstG, &dstB, &dstA);
                int64 dstN = dstNBuff[offset + programIndex];
                /* int32 is better
                int64 dstNa = dstN & (int64)0x00000000ffffffff;
                int64 dstNb = dstN & (int64)0xffffffff00000000;
                */
                int dstNa = dstN;
                int dstNb = dstN >> 32;

                int srcR, srcG, srcB, srcA;
                aos_to_soa4(&srcVBuff[offsetV], &srcR, &srcG, &srcB, &srcA);
                int64 srcN = srcNBuff[offset + programIndex];
                /* int32 is better
                int64 srcNa = srcN & (int64)0x00000000ffffffff;
                int64 srcNb = srcN & (int64)0xffffffff00000000;
                */
                int srcNa = srcN;
                int srcNb = srcN >> 32;

                bool active = false;
#ifdef IMPL_HYBRID
                if (((srcR - dstR) | (srcG - dstG) | (srcNa - dstNa)) && srcNa) {
                    mask |= (uint64_t)0x1 << ((offset + programIndex) * 2);
                    active = true;
                }
                if (((srcB - dstB) | (srcA - dstA) | (srcNb - dstNb)) && srcNb) {
                    mask |= (uint64_t)0x1 << ((offset + programIndex) * 2 + 1);
                    active = true;
                }
#endif // end IMPL_HYBRID
#ifdef IMPL_FULLBITOP
                if (((srcR - dstR) | (srcG - dstG) | (srcNa - dstNa)) & ((srcNa) ? ~0x0 : 0x0)) {
                    mask |= (uint64_t)0x1 << ((offset + programIndex) * 2);
                    active = true;
                }
                if (((srcB - dstB) | (srcA - dstA) | (srcNb - dstNb)) & ((srcNb) ? ~0x0 : 0x0)) {
                    mask |= (uint64_t)0x1 << ((offset + programIndex) * 2 + 1);
                    active = true;
                }
#endif // end IMPL_FULLBITOP
#ifdef IMPL_NAIVELOGICAL
                if (((srcR - dstR) != 0x0 || (srcG - dstG) != 0x0 || (srcNa - dstNa) != 0x0) && srcNa) {
                    mask |= (uint64_t)0x1 << ((offset + programIndex) * 2);
                    active = true;
                }
                if (((srcB - dstB) != 0x0 || (srcA - dstA) != 0x0 || (srcNb - dstNb) != 0x0) && srcNb) {
                    mask |= (uint64_t)0x1 << ((offset + programIndex) * 2 + 1);
                    active = true;
                }
#endif // end IMPL_NAIVELOGICAL
                if (active) {
                    soa_to_aos4(srcR, srcG, srcB, srcA, &dstVBuff[offsetV]);
                    dstNBuff[offset + programIndex] = srcN;
                }
            }
        }
    }
    return extract(mask, programCount - 1) | extract(exclusive_scan_or(mask), programCount - 1);
}

export uniform uint64_t
snapshotTileFloat3Weight(uniform int dstCBuff[],
                         uniform int dstWBuff[],
                         uniform int srcCBuff[],
                         uniform int srcWBuff[])
// This is an ISPC version of scene_rdl2::fb_util::SnapshotUtil::snapshotTileFloat3Weight_SISD().
{
    uniform int loopMax = 64 / programCount;
    uniform uint64_t mask = (uint64_t)0x0;
    for (uniform int loopId = 0; loopId < loopMax; ++loopId) {
        uniform int offsetC = loopId * programCount * 3;
        uniform int offsetW = loopId * programCount;

        int dstR, dstG, dstB;
        aos_to_soa3(&dstCBuff[offsetC], &dstR, &dstG, &dstB);
        int dstW = dstWBuff[offsetW + programIndex];

        int srcR, srcG, srcB;
        aos_to_soa3(&srcCBuff[offsetC], &srcR, &srcG, &srcB);
        int srcW = srcWBuff[offsetW + programIndex];

#ifdef IMPL_HYBRID
        int activeMask = ((srcR - dstR) | (srcG - dstG) | (srcB - dstB) | (srcW - dstW)) && srcW;
        if (activeMask) {
#endif // end IMPL_HYBRID
#ifdef IMPL_FULLBITOP
        int activeMask = ((srcR - dstR) | (srcG - dstG) | (srcB - dstB) | (srcW - dstW)) & ((srcW) ? ~0x0 : 0x0);
        if (activeMask) {
#endif // end IMPL_FULLBITOP
#ifdef IMPL_NAIVELOGICAL
        bool activeFlag = ((srcR - dstR) != 0x0 || (srcG - dstG) != 0x0 || (srcB - dstB) != 0x0 || (srcW - dstW) != 0x0) && srcW;
        if (activeFlag) {
#endif // end IMPL_NAIVELOGICAL
            soa_to_aos3(srcR, srcG, srcB, &dstCBuff[offsetC]);
            dstWBuff[offsetW + programIndex] = srcW;

            mask |= ((uint64_t)(lanemask()) << offsetW);
        }
    }
    return mask;
}

export uniform uint64_t
snapshotTileFloat3NumSample(uniform int dstVBuff[],
                            uniform int dstNBuff[],
                            uniform uint64_t dstTileMask,
                            uniform int srcVBuff[],
                            uniform int srcNBuff[],
                            uniform uint64_t srcTileMask)
// This is an ISPC version of scene_rdl2::fb_util::SnapshotUtil::snapshotTileFloat3NumSample_SISD().
{
    if (!srcTileMask) return (uint64_t)0x0;

    uniform int fullGangMask = lanemask();
    MNRY_ASSERT(fullGangMask & (1 << programIndex));

    uniform int loopMax = 64 / programCount;
    uniform uint64_t mask = (uint64_t)0x0;
    for (uniform int loopId = 0; loopId < loopMax; ++loopId) {
        uniform int offsetN = loopId * programCount;
        uniform int offsetV = offsetN * 3;

        uniform uint32_t currSrcMask = (uint32_t)((uint64_t)(srcTileMask >> offsetN) & fullGangMask);
        if (currSrcMask) {
            uniform uint32_t currDstMask = (uint32_t)((uint64_t)(dstTileMask >> offsetN) & fullGangMask);
            if (currSrcMask & (uint32_t)(0x1 << programIndex)) {
                int dstR, dstG, dstB;
                aos_to_soa3(&dstVBuff[offsetV], &dstR, &dstG, &dstB);
                int dstN = dstNBuff[offsetN + programIndex];

                int srcR, srcG, srcB;
                aos_to_soa3(&srcVBuff[offsetV], &srcR, &srcG, &srcB);
                int srcN = srcNBuff[offsetN + programIndex];
                            
                int freshPixCondition = !(currDstMask & (int)(0x1 << programIndex));
#ifdef IMPL_HYBRID
                int activeMask = ((srcR - dstR) | (srcG - dstG) | (srcB - dstB) | (srcN - dstN) | freshPixCondition) && srcN;
                if (activeMask) {
#endif // end IMPL_HYBRID
#ifdef IMPL_FULLBITOP
                int activeMask = ((srcR - dstR) | (srcG - dstG) | (srcB - dstB) | (srcN - dstN) | freshPixCondition) & ((srcN) ? ~0x0 : 0x0);
                if (activeMask) {
#endif // end IMPL_FULLBITOP
#ifdef IMPL_NAIVELOGICAL
                bool activeFlag = ((srcR - dstR) != 0x0 || (srcG - dstG) != 0x0 ||
                                   (srcB - dstB) != 0x0 || (srcN - dstN) != 0x0 || freshPixCondition) && srcN;
                if (activeFlag) {
#endif // end IMPL_NAIVELOGICAL
                    soa_to_aos3(srcR, srcG, srcB, &dstVBuff[offsetV]);
                    dstNBuff[offsetN + programIndex] = srcN;

                    mask |= ((uint64_t)(lanemask()) << offsetN);
                }
            }
        }
    }
    return mask;
}

export uniform uint64_t
snapshotTileFloat4Weight(uniform int dstVBuff[],
                         uniform int dstWBuff[],
                         uniform int srcVBuff[],
                         uniform int srcWBuff[])
// This is an ISPC version of scene_rdl2::fb_util::SnapshotUtil::snapshotTileFloat4Weight_SISD().
{
    uniform int loopMax = 64 / programCount;
    uniform uint64_t mask = (uint64_t)0x0;
    for (uniform int loopId = 0; loopId < loopMax; ++loopId) {
        uniform int offsetV = loopId * programCount * 4;
        uniform int offsetW = loopId * programCount;

        int dstR, dstG, dstB, dstA;
        aos_to_soa4(&dstVBuff[offsetV], &dstR, &dstG, &dstB, &dstA);
        int dstW = dstWBuff[offsetW + programIndex];

        int srcR, srcG, srcB, srcA;
        aos_to_soa4(&srcVBuff[offsetV], &srcR, &srcG, &srcB, &srcA);
        int srcW = srcWBuff[offsetW + programIndex];

#ifdef IMPL_HYBRID
        int activeMask = ((srcR - dstR) | (srcG - dstG) | (srcB - dstB) | (srcA - dstA) | (srcW - dstW)) && srcW;
        if (activeMask) {
#endif // end IMPL_HYBRID
#ifdef IMPL_FULLBITOP
        int activeMask = ((srcR - dstR) | (srcG - dstG) | (srcB - dstB) | (srcA - dstA) | (srcW - dstW)) & ((srcW) ? ~0x0 : 0x0);
        if (activeMask) {
#endif // end IMPL_FULLBITOP
#ifdef IMPL_NAIVELOGICAL
        bool activeFlag = ((srcR - dstR) != 0x0 || (srcG - dstG) != 0x0 ||
                           (srcB - dstB) != 0x0 || (srcA - dstA) != 0x0 ||
                           (srcW - dstW) != 0x0) && srcW;
        if (activeFlag) {
#endif // end IMPL_NAIVELOGICAL
            soa_to_aos4(srcR, srcG, srcB, srcA, &dstVBuff[offsetV]);
            dstWBuff[offsetW + programIndex] = srcW;

            mask |= ((uint64_t)(lanemask()) << offsetW);
        }
    }
    return mask;
}

export uniform uint64_t
snapshotTileFloat4NumSample(uniform int dstVBuff[],
                            uniform int dstNBuff[],
                            uniform uint64_t dstTileMask,
                            uniform int srcVBuff[],
                            uniform int srcNBuff[],
                            uniform uint64_t srcTileMask)
// This is an ISPC version of scene_rdl2::fb_util::SnapshotUtil::snapshotTileFloat4NumSample_SISD().
{
    if (!srcTileMask) return (uint64_t)0x0;

    uniform int fullGangMask = lanemask();
    MNRY_ASSERT(fullGangMask & (1 << programIndex));

    uniform int loopMax = 64 / programCount;
    uniform uint64_t mask = (uint64_t)0x0;
    for (uniform int loopId = 0; loopId < loopMax; ++loopId) {
        uniform int offsetN = loopId * programCount;
        uniform int offsetV = offsetN * 4;

        uniform uint32_t currSrcMask = (uint32_t)((uint64_t)(srcTileMask >> offsetN) & fullGangMask);
        if (currSrcMask) {
            uniform uint32_t currDstMask = (uint32_t)((uint64_t)(dstTileMask >> offsetN) & fullGangMask);
            if (currSrcMask & (uint32_t)(0x1 << programIndex)) {
                int dstR, dstG, dstB, dstA;
                aos_to_soa4(&dstVBuff[offsetV], &dstR, &dstG, &dstB, &dstA);
                int dstN = dstNBuff[offsetN + programIndex];

                int srcR, srcG, srcB, srcA;
                aos_to_soa4(&srcVBuff[offsetV], &srcR, &srcG, &srcB, &srcA);
                int srcN = srcNBuff[offsetN + programIndex];
                            
                int freshPixCondition = !(currDstMask & (int)(0x1 << programIndex));
#ifdef IMPL_HYBRID
                int activeMask =
                    ((srcR - dstR) | (srcG - dstG) | (srcB - dstB) | (srcA - dstA) | (srcN - dstN) |
                     freshPixCondition) && srcN;
                if (activeMask) {
#endif // end IMPL_HYBRID
#ifdef IMPL_FULLBITOP
                int activeMask =
                    ((srcR - dstR) | (srcG - dstG) | (srcB - dstB) | (srcA - dstA) | (srcN - dstN) |
                     freshPixCondition) & ((srcN) ? ~0x0 : 0x0);
                if (activeMask) {
#endif // end IMPL_FULLBITOP
#ifdef IMPL_NAIVELOGICAL
                bool activeFlag =
                    ((srcR - dstR) != 0x0 || (srcG - dstG) != 0x0 ||
                     (srcB - dstB) != 0x0 || (srcA - dstA) != 0x0 || (srcN - dstN) != 0x0 ||
                     freshPixCondition) && srcN;
                if (activeFlag) {
#endif // end IMPL_NAIVELOGICAL
                    soa_to_aos4(srcR, srcG, srcB, srcA, &dstVBuff[offsetV]);
                    dstNBuff[offsetN + programIndex] = srcN;

                    mask |= ((uint64_t)(lanemask()) << offsetN);
                }
            }
        }
    }
    return mask;
}

export uniform uint64_t
snapshotTileUInt32WithMask(uniform int dstVBuff[],
                           uniform uint64_t dstTileMask,
                           uniform int srcVBuff[],
                           uniform uint64_t srcTileMask)
// This is an ISPC version of scene_rdl2::fb_util::SnapshotUtil::snapshotTileUInt32WithMask_SISD().
// Speed is 3.44x~3.49x faster than snapshotTileUInt32WithMask_SISD() (Toshi Oct/21/2020)
{
    if (!srcTileMask) return (uint64_t)0x0;

    uniform int fullGangMask = lanemask();
    MNRY_ASSERT(fullGangMask & (1 << programIndex));

    uniform int loopMax = 64 / programCount;
    uniform uint64_t mask = (uint64_t)0x0;
    for (uniform int loopId = 0; loopId < loopMax; ++loopId) {
        uniform int offset = loopId * programCount;

        uniform uint32_t currSrcMask = (uint32_t)((uint64_t)(srcTileMask >> offset) & fullGangMask);
        if (currSrcMask) {
            uniform uint32_t currDstMask = (uint32_t)((uint64_t)(dstTileMask >> offset) & fullGangMask);
            if (currSrcMask & (uint32_t)(0x1 << programIndex)) {
                int dstV = dstVBuff[offset + programIndex];
                int srcV = srcVBuff[offset + programIndex];

                int freshPixCondition = !(currDstMask & (int)(0x1 << programIndex));
#ifdef IMPL_HYBRID
                int activeMask = ((srcV - dstV) | freshPixCondition) && srcV;
                if (activeMask) {
#endif // end IMPL_HYBRID
#ifdef IMPL_FULLBITOP
                int activeMask = ((srcV - dstV) | freshPixCondition) & ((srcV) ? ~0x0 : 0x0);
                if (activeMask) {
#endif // end IMPL_FULLBITOP
#ifdef IMPL_NAIVELOGICAL
                bool activeFlag = ((srcV - dstV) != 0x0 || freshPixCondition) && srcV;
                if (activeFlag) {
#endif // end IMPL_NAIVELOGICAL
                    dstVBuff[offset + programIndex] = srcV;

                    mask |= ((uint64_t)(lanemask()) << offset);
                }
            }
        }
    }
    return mask;
}
