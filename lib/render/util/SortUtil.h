// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#pragma once
#include "Arena.h"
#include "BitUtils.h"
#include <algorithm>

#define EXTRACT_KEY32(x, offset)    (((const uint32_t *)(&(x)))[offset >> 2])
#define EXTRACT_KEY22(x, offset)    ((((const uint32_t *)(&(x)))[offset >> 2]) & 0x3fffff)
#define EXTRACT_KEY11(x, offset)    ((((const uint32_t *)(&(x)))[offset >> 2]) & 0x7ff)
#define EXTRACT_KEY_MSB8(x, offset) ((((const uint32_t *)(&(x)))[offset >> 2]) >> 24)
#define EXTRACT_LOW_KEY(x)          ((x) & 0x7ff)
#define EXTRACT_MID_KEY(x)          (((x) >> 11) & 0x7ff)
#define EXTRACT_HIGH_KEY(x)         ((x) >> 22 )

namespace scene_rdl2 {
namespace util {

//-----------------------------------------------------------------------------

template<typename T>
bool
isSorted(unsigned numElems, const T *elems)
{
    if (numElems) {
        const T *prevElem = &elems[0];
        for (unsigned i = 1; i < numElems; ++i) {
            const T *currElem = &elems[i];
            if (*prevElem > *currElem) {
                return false;
            }
            prevElem = currElem;
        }
    }
    return true;
}

template<typename T, unsigned SORT_KEY_OFFSET>
bool
isSorted32(unsigned numElems, const T *elems)
{
    if (numElems) {
        const T *prevElem = &elems[0];
        for (unsigned i = 1; i < numElems; ++i) {
            const T *currElem = &elems[i];
            if (EXTRACT_KEY32(*prevElem, SORT_KEY_OFFSET) > EXTRACT_KEY32(*currElem, SORT_KEY_OFFSET)) {
                return false;
            }
            prevElem = currElem;
        }
    }
    return true;
}

template<typename T>
bool
isSortedAndUnique(unsigned numElems, const T *elems)
{
    if (numElems) {
        const T *prevElem = &elems[0];
        for (unsigned i = 1; i < numElems; ++i) {
            const T *currElem = &elems[i];
            if (*prevElem >= *currElem) {
                return false;
            }
            prevElem = currElem;
        }
    }
    return true;
}

template<typename T, unsigned SORT_KEY_OFFSET>
bool
isSortedAndUnique32(unsigned numElems, const T *elems)
{
    if (numElems) {
        const T *prevElem = &elems[0];
        for (unsigned i = 1; i < numElems; ++i) {
            const T *currElem = &elems[i];
            if (EXTRACT_KEY32(*prevElem, SORT_KEY_OFFSET) >= EXTRACT_KEY32(*currElem, SORT_KEY_OFFSET)) {
                return false;
            }
            prevElem = currElem;
        }
    }
    return true;
}

//-----------------------------------------------------------------------------

//
// Radix sort functions.
//

template<typename T, unsigned SORT_KEY_OFFSET = 0>
inline void
inPlaceRadixSort32(unsigned numElems, T *elems, alloc::Arena *arena)
{
    MNRY_ASSERT(arena);

    // Least significant "digit" radix sort, where a 32-bit value is composed
    // of 3 11-bit keys (radix 11).

    const uint32_t radix = 11;
    const uint32_t numBuckets = 1 << radix;
    const uint32_t histogramBufSize = numBuckets * 3 * sizeof(uint32_t);
    const uint32_t scratchBufSize = alignUp<uint32_t>(sizeof(T) * numElems, CACHE_LINE_SIZE);
    MNRY_ASSERT((histogramBufSize % CACHE_LINE_SIZE) == 0);

    SCOPED_MEM(arena);
    unsigned bufSize = histogramBufSize + scratchBufSize * 2;
    uint8_t *buf = arena->alloc(bufSize, CACHE_LINE_SIZE);

    T *scratch1 = (T *)(buf);
    T *scratch2 = (T *)(buf + scratchBufSize);

    uint32_t *histrogramBuf = (uint32_t *)(buf + scratchBufSize * 2);
    memset(histrogramBuf, 0, histogramBufSize);

    uint32_t *histLow  = histrogramBuf;
    uint32_t *histMid  = histrogramBuf + numBuckets;
    uint32_t *histHigh = histrogramBuf + numBuckets * 2;

    MNRY_ASSERT((uint8_t *)(histHigh + numBuckets) <= buf + bufSize);

    // Fill histograms for 3 passes.
    for (unsigned i = 0; i < numElems; ++i) {
        uint32_t key = EXTRACT_KEY32(elems[i], SORT_KEY_OFFSET);
        ++histLow [EXTRACT_LOW_KEY (key)];
        ++histMid [EXTRACT_MID_KEY (key)];
        ++histHigh[EXTRACT_HIGH_KEY(key)];
    }

    // Accumulate histogram buckets to get start insertion point for each value.
    uint32_t accLow = 0, accMid = 0, accHigh = 0;
    for (unsigned i = 0; i < numBuckets; ++i) {
        uint32_t a = histLow[i] + accLow;
        uint32_t b = histMid[i] + accMid;
        uint32_t c = histHigh[i] + accHigh;

        histLow[i] = accLow;
        histMid[i] = accMid;
        histHigh[i] = accHigh;

        accLow = a;
        accMid = b;
        accHigh = c;
    }

    for (unsigned i = 0; i < numElems; ++i) {
        uint32_t key = EXTRACT_KEY32(elems[i], SORT_KEY_OFFSET);
        uint32_t bucket = EXTRACT_LOW_KEY(key);
        uint32_t dstIdx = histLow[bucket]++;
        scratch1[dstIdx] = elems[i];
    }

    for (unsigned i = 0; i < numElems; ++i) {
        uint32_t key = EXTRACT_KEY32(scratch1[i], SORT_KEY_OFFSET);
        uint32_t bucket = EXTRACT_MID_KEY(key);
        uint32_t dstIdx = histMid[bucket]++;
        scratch2[dstIdx] = scratch1[i];
    }

    for (unsigned i = 0; i < numElems; ++i) {
        uint32_t key = EXTRACT_KEY32(scratch2[i], SORT_KEY_OFFSET);
        uint32_t bucket = EXTRACT_HIGH_KEY(key);
        uint32_t dstIdx = histHigh[bucket]++;
        elems[dstIdx] = scratch2[i];
    }
}

// Same as inPlaceRadixSort32 but faster if sort key only uses least significant 22 bits
template<typename T, unsigned SORT_KEY_OFFSET = 0>
inline void
inPlaceRadixSort22(unsigned numElems, T *elems, alloc::Arena *arena)
{
    MNRY_ASSERT(arena);

    const uint32_t radix = 11;
    const uint32_t numBuckets = 1 << radix;
    const uint32_t histogramBufSize = numBuckets * 2 * sizeof(uint32_t);
    const uint32_t scratchBufSize = alignUp<uint32_t>(sizeof(T) * numElems, CACHE_LINE_SIZE);
    MNRY_ASSERT((histogramBufSize % CACHE_LINE_SIZE) == 0);

    SCOPED_MEM(arena);
    unsigned bufSize = histogramBufSize + scratchBufSize;
    uint8_t *buf = arena->alloc(bufSize, CACHE_LINE_SIZE);

    T *scratch = (T *)(buf);

    uint32_t *histrogramBuf = (uint32_t *)(buf + scratchBufSize);
    memset(histrogramBuf, 0, histogramBufSize);

    uint32_t *histLow  = histrogramBuf;
    uint32_t *histMid  = histrogramBuf + numBuckets;

    MNRY_ASSERT((uint8_t *)(histMid + numBuckets) <= buf + bufSize);

    // Fill histograms for 2 passes.
    for (unsigned i = 0; i < numElems; ++i) {
        uint32_t key = EXTRACT_KEY32(elems[i], SORT_KEY_OFFSET);    // Deliberately calling EXTRACT_KEY32 here.
        ++histLow[EXTRACT_LOW_KEY(key)];
        ++histMid[EXTRACT_MID_KEY(key)];
    }

    // Accumulate histogram buckets to get start insertion point for each value.
    uint32_t accLow = 0, accMid = 0;
    for (unsigned i = 0; i < numBuckets; ++i) {
        uint32_t a = histLow[i] + accLow;
        uint32_t b = histMid[i] + accMid;

        histLow[i] = accLow;
        histMid[i] = accMid;

        accLow = a;
        accMid = b;
    }

    for (unsigned i = 0; i < numElems; ++i) {
        uint32_t key = EXTRACT_KEY32(elems[i], SORT_KEY_OFFSET);    // Deliberately calling EXTRACT_KEY32 here.
        uint32_t bucket = EXTRACT_LOW_KEY(key);
        uint32_t dstIdx = histLow[bucket]++;
        scratch[dstIdx] = elems[i];
    }

    for (unsigned i = 0; i < numElems; ++i) {
        uint32_t key = EXTRACT_KEY32(scratch[i], SORT_KEY_OFFSET);  // Deliberately calling EXTRACT_KEY32 here.
        uint32_t bucket = EXTRACT_MID_KEY(key);
        uint32_t dstIdx = histMid[bucket]++;
        elems[dstIdx] = scratch[i];
    }
}

// Radix sort using only least significant 11 bits of sort key.
// Doesn't support in place sorting so caller must pass in destination buffer
// This destination buffer may not overlap the input elems buffer.
template<typename T, unsigned SORT_KEY_OFFSET = 0>
inline void
outOfPlaceRadixSort11(unsigned numElems, T *elems, T *dst, alloc::Arena *arena)
{
    MNRY_ASSERT(arena);

    const uint32_t radix = 11;
    const uint32_t numBuckets = 1 << radix;
    const uint32_t histogramBufSize = numBuckets * sizeof(uint32_t);
    MNRY_ASSERT((histogramBufSize % CACHE_LINE_SIZE) == 0);

    SCOPED_MEM(arena);
    uint32_t *histogram = (uint32_t *)arena->alloc(histogramBufSize, CACHE_LINE_SIZE);
    memset(histogram, 0, histogramBufSize);

    // Fill histogram.
    for (unsigned i = 0; i < numElems; ++i) {
        ++histogram[EXTRACT_KEY11(elems[i], SORT_KEY_OFFSET)];
    }

    // Accumulate histogram buckets to get start insertion point for each value.
    uint32_t acc = 0;
    for (unsigned i = 0; i < numBuckets; ++i) {
        uint32_t a = histogram[i] + acc;
        histogram[i] = acc;
        acc = a;
    }

    for (unsigned i = 0; i < numElems; ++i) {
        uint32_t key = EXTRACT_KEY11(elems[i], SORT_KEY_OFFSET);
        uint32_t dstIdx = histogram[key]++;
        dst[dstIdx] = elems[i];
    }
}

// Radix sort using most significant 8 bits of a 32-bit sort key.
// Doesn't support in place sorting so caller must pass in destination buffer
// This destination buffer may not overlap the input elems buffer.
template<typename T, unsigned SORT_KEY_OFFSET = 0>
inline void
outOfPlaceRadixSortMSB8(unsigned numElems, T *elems, T *dst, alloc::Arena *arena)
{
    MNRY_ASSERT(arena);

    const uint32_t radix = 8;
    const uint32_t numBuckets = 1 << radix;
    const uint32_t histogramBufSize = numBuckets * sizeof(uint32_t);
    MNRY_ASSERT((histogramBufSize % CACHE_LINE_SIZE) == 0);

    SCOPED_MEM(arena);
    uint32_t *histogram = (uint32_t *)arena->alloc(histogramBufSize, CACHE_LINE_SIZE);
    memset(histogram, 0, histogramBufSize);

    // Fill histogram.
    for (unsigned i = 0; i < numElems; ++i) {
        ++histogram[EXTRACT_KEY_MSB8(elems[i], SORT_KEY_OFFSET)];
    }

    // Accumulate histogram buckets to get start insertion point for each value.
    uint32_t acc = 0;
    for (unsigned i = 0; i < numBuckets; ++i) {
        uint32_t a = histogram[i] + acc;
        histogram[i] = acc;
        acc = a;
    }

    for (unsigned i = 0; i < numElems; ++i) {
        uint32_t key = EXTRACT_KEY_MSB8(elems[i], SORT_KEY_OFFSET);
        uint32_t dstIdx = histogram[key]++;
        dst[dstIdx] = elems[i];
    }
}

//-----------------------------------------------------------------------------

//
// Functions which will pick the best sort based on a user supplied heuristic.
//
// Notes:
// - If numElems < STD_SORT_CUTOFF then sort may not be stable, otherwise it is
//   guaranteed stable.
// - arena is only needed when numElems >= STD_SORT_CUTOFF.
//
// Perf:
// - Here are some sample runs on a gray box for various entry sizes. This should
//   help inform a reasonable value for STD_SORT_CUTOFF.
//
//                                std::sort              Radix
//  8 bytes entries:
//
//  Ticks for      16 elements =        714              13784
//  Ticks for      32 elements =       1937              14903
//  Ticks for      64 elements =       4356              16103
//  Ticks for     128 elements =      10216              17815
//  Ticks for     256 elements =      22556              20201
//  Ticks for     512 elements =      51265              25240
//  Ticks for    1024 elements =     111949              37259
//  Ticks for    2048 elements =     246403              61894
//  Ticks for    4096 elements =     528086             123483
//  Ticks for    8192 elements =    1130594             289954
//  Ticks for   16384 elements =    2412991             660591
//  Ticks for   32768 elements =    5116575            1660582
//  Ticks for   65536 elements =   10904247            3252503
//
//
//  32 bytes entries:
//
//  Ticks for      16 elements =       1995              12496
//  Ticks for      32 elements =       4651              13586
//  Ticks for      64 elements =      10637              15815
//  Ticks for     128 elements =      23858              19487
//  Ticks for     256 elements =      52235              27090
//  Ticks for     512 elements =     112857              41765
//  Ticks for    1024 elements =     246537              73444
//  Ticks for    2048 elements =     533994             139041
//  Ticks for    4096 elements =    1155521             283210
//  Ticks for    8192 elements =    2470578             734233
//  Ticks for   16384 elements =    5270934            1453990
//  Ticks for   32768 elements =   11249908            2862388
//  Ticks for   65536 elements =   23830368            5685731
//
//
//  64 bytes entries:
//
//  Ticks for      16 elements =       3166              13163
//  Ticks for      32 elements =       7065              14850
//  Ticks for      64 elements =      15020              17559
//  Ticks for     128 elements =      33392              23750
//  Ticks for     256 elements =      72918              36329
//  Ticks for     512 elements =     157226              61537
//  Ticks for    1024 elements =     336158             110050
//  Ticks for    2048 elements =     722558             221688
//  Ticks for    4096 elements =    1528237             527303
//  Ticks for    8192 elements =    3261946            1044045
//  Ticks for   16384 elements =    6897424            2075020
//  Ticks for   32768 elements =   14565107            4092514
//  Ticks for   65536 elements =   30712297            8237223
//
//
//  128 bytes entries:
//
//  Ticks for      16 elements =       4149              13640
//  Ticks for      32 elements =       9453              15911
//  Ticks for      64 elements =      20034              19562
//  Ticks for     128 elements =      42786              27269
//  Ticks for     256 elements =      96023              43744
//  Ticks for     512 elements =     211231              76603
//  Ticks for    1024 elements =     452082             154203
//  Ticks for    2048 elements =     977070             333822
//  Ticks for    4096 elements =    2091859             661812
//  Ticks for    8192 elements =    4468643            1311778
//  Ticks for   16384 elements =    9520722            2577644
//  Ticks for   32768 elements =   20213986            5164359
//  Ticks for   65536 elements =   42888268           59245181 <-- we've jumped off some cliff here
//
template<typename T, unsigned SORT_KEY_OFFSET = 0, unsigned STD_SORT_CUTOFF = 200>
inline void
inPlaceSort32(unsigned numElems, T *elems, alloc::Arena *arena)
{
    if (numElems < STD_SORT_CUTOFF) {

        // std::sort is great when low number of values, but starts slowing down
        // around the 200 mark.
        std::sort(elems, elems + numElems, [](const T &a, const T &b) -> bool
        {
            return EXTRACT_KEY32(a, SORT_KEY_OFFSET) < EXTRACT_KEY32(b, SORT_KEY_OFFSET);
        });

    } else {

        inPlaceRadixSort32<T, SORT_KEY_OFFSET>(numElems, elems, arena);
    }
}

// Like as inPlaceSort32 but faster when the sort key is only composed of the
// least significant 22 bits.
template<typename T, unsigned SORT_KEY_OFFSET = 0, unsigned STD_SORT_CUTOFF = 200>
inline void
inPlaceSort22(unsigned numElems, T *elems, alloc::Arena *arena)
{
    if (numElems < STD_SORT_CUTOFF) {

        std::sort(elems, elems + numElems, [](const T &a, const T &b) -> bool
        {
            return EXTRACT_KEY22(a, SORT_KEY_OFFSET) < EXTRACT_KEY22(b, SORT_KEY_OFFSET);
        });

    } else {

        inPlaceRadixSort22<T, SORT_KEY_OFFSET>(numElems, elems, arena);
    }
}

//-----------------------------------------------------------------------------

// This function sorts based on up to a 32-bit sort key, less if possible.
// The function returns a pointer to the newly sorted data. This pointer may or
// may not be the same as the original elems pointer passed in.
template<typename T, unsigned SORT_KEY_OFFSET = 0, unsigned STD_SORT_CUTOFF = 200>
inline T *
smartSort32(unsigned numElems, T *elems, uint32_t maxSortKey, alloc::Arena *arena)
{
    // std::sort is great with a low number of values, but starts slowing down
    // around the 200 mark (assuming 8 byte entries).
    if (numElems < STD_SORT_CUTOFF) {

        std::sort(elems, elems + numElems, [](const T a, const T b) -> bool
        {
            return EXTRACT_KEY32(a, SORT_KEY_OFFSET) < EXTRACT_KEY32(b, SORT_KEY_OFFSET);
        });

    } else {

        // Radix sort path.
        if (maxSortKey < (1 << 22)) {
            if (maxSortKey < (1 << 11)) {
                T *dst = arena->allocArray<T>(numElems, CACHE_LINE_SIZE);
                outOfPlaceRadixSort11<T, SORT_KEY_OFFSET>(numElems, elems, dst, arena);
                elems = dst;
            } else {
                inPlaceRadixSort22<T, SORT_KEY_OFFSET>(numElems, elems, arena);
            }
        } else {
            inPlaceRadixSort32<T, SORT_KEY_OFFSET>(numElems, elems, arena);
        }
    }

    MNRY_ASSERT( (isSorted32<T, SORT_KEY_OFFSET>(numElems, elems)) );

    return elems;
}

//-----------------------------------------------------------------------------

} // namespace util
} // namespace scene_rdl2


