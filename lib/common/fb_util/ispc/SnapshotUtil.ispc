// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#include <scene_rdl2/common/platform/Platform.isph>

export uniform uint64_t
snapshotTileHeatMapWeight(uniform int64 dstVBuff[],
                          uniform int   dstWBuff[],
                          uniform int64 srcVBuff[],
                          uniform int   srcWBuff[])
// This is an ISPC version of scene_rdl2::fb_util::SnapshotUtil::snapshotTileHeatMapWeight_SISD().
// Speed is 1.09x~1.10x faster than snapshotTileHeatMapWeight_SISD() (Toshi Oct/20/2020)
{
    uniform int loopMax = 64 / programCount;
    uniform uint64_t mask = (uint64_t)0x0;
    for (uniform int loopId = 0; loopId < loopMax; ++loopId) {
        int offset = loopId * programCount + programIndex;

        int64 dstV = dstVBuff[offset];
        int   dstW = dstWBuff[offset];
        int64 srcV = srcVBuff[offset];
        int   srcW = srcWBuff[offset];

        if (((srcW - dstW) != 0x0) || ((srcV - dstV) != (int64)0x0)) {
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
// Speed is 1.50x~1.56x faster than snapshotTileWeightBuffer_SISD() (Toshi Oct/20/2020)
{
    uniform int loopMax = 64 / programCount;
    uniform uint64_t mask = (uint64_t)0x0;
    for (uniform int loopId = 0; loopId < loopMax; ++loopId) {
        int offset = loopId * programCount + programIndex;

        int dst = dstBuff[offset];
        int src = srcBuff[offset];

        if ((src - dst) != 0x0) {
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
// Speed is 1.31x~1.49x faster than snapshotTileFloatWeight_SISD() (Toshi Oct/20/2020)
{
    uniform int loopMax = 64 / programCount;
    uniform uint64_t mask = (uint64_t)0x0;
    for (uniform int loopId = 0; loopId < loopMax; ++loopId) {
        int offset = loopId * programCount + programIndex;

        int dstV = dstVBuff[offset];
        int dstW = dstWBuff[offset];
        int srcV = srcVBuff[offset];
        int srcW = srcWBuff[offset];

        if (((srcV - dstV) != 0x0) || ((srcW - dstW) != 0x0)) {        
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
// Speed is 2.80x~3.02x faster than snapshotTileFloatNumSample_SISD() (Toshi Oct/20/2020)
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
                int activeMask = (srcV - dstV) | (srcN - dstN) | freshPixCondition;
                if (activeMask) {
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

        int activeMask = (srcVa - dstVa) | (srcVb - dstVb) | (srcW - dstW);
        if (activeMask) {
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
// Speed is 1.06x~1.08x faster than snapshotTileFloat2Weight_SISD() (Toshi Oct/20/2020)
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
        if ((srcR - dstR) | (srcG - dstG) | (srcWa - dstWa)) {
            mask |= (uint64_t)0x1 << ((offset + programIndex) * 2);
            active = true;
        }
        if ((srcB - dstB) | (srcA - dstA) | (srcWb - dstWb)) {
            mask |= (uint64_t)0x1 << ((offset + programIndex) * 2 + 1);
            active = true;
        }
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
// Speed is 1.22~1.39 faster than snapshotTileFloat2NumSample_SISD() (Toshi Oct/21/2020)
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
                if ((srcR - dstR) | (srcG - dstG) | (srcNa - dstNa)) {
                    mask |= (uint64_t)0x1 << ((offset + programIndex) * 2);
                    active = true;
                }
                if ((srcB - dstB) | (srcA - dstA) | (srcNb - dstNb)) {
                    mask |= (uint64_t)0x1 << ((offset + programIndex) * 2 + 1);
                    active = true;
                }
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
// Speed is 1.15x~1.31x faster than snapshotTileFloat3Weight_SISD() (Toshi Oct/20/2020)
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

        int activeMask = (srcR - dstR) | (srcG - dstG) | (srcB - dstB) | (srcW - dstW);
        if (activeMask) {
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
// Speed is 1.49x~1.54x faster than snapshotTileFloat3NumSample_SISD() (Toshi Oct/20/2020)
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
                int activeMask = (srcR - dstR) | (srcG - dstG) | (srcB - dstB) | (srcN - dstN) | freshPixCondition;
                if (activeMask) {
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
// Speed is 1.23x~1.26x faster than snapshotTileFloat4Weight_SISD() (Toshi Oct/20/2020)
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

        int activeMask = (srcR - dstR) | (srcG - dstG) | (srcB - dstB) | (srcA - dstA) | (srcW - dstW);
        if (activeMask) {
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
// Speed is 1.31x~1.57x faster than snapshotTileFloat4NumSample_SISD() (Toshi Oct/20/2020)
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
                int activeMask =
                    ((srcR - dstR) | (srcG - dstG) | (srcB - dstB) | (srcA - dstA) | (srcN - dstN) |
                     freshPixCondition);

                if (activeMask) {
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
                int activeMask = (srcV - dstV) | freshPixCondition;
                if (activeMask) {
                    dstVBuff[offset + programIndex] = srcV;

                    mask |= ((uint64_t)(lanemask()) << offset);
                }
            }
        }
    }
    return mask;
}

