// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#pragma once

//
// -- High speed tile extrapolation by lookup table --
//
//    United States Patent : 10,970,894 : Apr/06/2021
//
// This file contents tile extrapolation related APIs.
// We don't need to consider pixel computation order inside tile. This tile
// extrapolation logic can handle any order of pixel computation inside one
// tile. Imprementation is done by table lookup and it's prety fast.
// See TileExtrapolation.cc for more detail of extrapolation logic itself.
//
// Call TileExtrapolation::searchActiveNearestPixel() to do tile extrapolation.
//

#include <scene_rdl2/common/platform/Intrinsics.h>

#include <string>
#include <vector>

namespace scene_rdl2 {
namespace fb_util {

//------------------------------------------------------------------------------
//
// TileExtrapolationPix and TileExtrapolationTile are used to create precomputed
// pixelMask data as c++ code (See TileExtrapolation.cc)
//
class TileExtrapolationPix
{
public:
    TileExtrapolationPix() : mId(0), mDistanceSquared(0) {}

    void init(const unsigned i) { mId = i; mDistanceSquared = 0; }
    void setDistanceSquared(const unsigned d2) { mDistanceSquared = d2; }

    unsigned getId() const { return mId; }
    unsigned getDistanceSquared() const { return mDistanceSquared; }

    std::string show(const std::string &hd) const;

protected:
    unsigned mId;
    unsigned mDistanceSquared;
}; // TileExtrapolationPix

class TileExtrapolationTile
{
public:
    TileExtrapolationTile() { mPixels.resize(64); initPixels(); }


    std::string makePrecomputeMaskTableCppHeader();

    bool isActivePix(const uint64_t activePixelMask, const unsigned x, const unsigned y) const {
        return ((activePixelMask >> pixId(x, y)) & 0x1)? true: false;
    }

protected:
    void initPixels() { for (unsigned i = 0; i < 64; ++i) mPixels[i].init(i); }
    void resetDistanceSquared() { for (unsigned i = 0; i < 64; ++i) mPixels[i].setDistanceSquared(0); }

    unsigned pixId(const unsigned x, const unsigned y) const { return (y << 3) + x; }

    void calcPrecomputeMaskTablePixel(const unsigned x, const unsigned y, std::vector<uint64_t> &maskTbl);

    std::string showPixels(const std::string &hd) const;

    //------------------------------

    std::vector<TileExtrapolationPix> mPixels;
}; // TileExtrapolationTile

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

class TileExtrapolationPhase
{
public:
    int mStartMaskId;
    int mEndMaskId;

    uint64_t mPhaseMask;

    std::string show(const std::string &hd) const;
}; // TileExtrapolationPhase

class TileExtrapolationPhaseManager
{
public:
    TileExtrapolationPhaseManager() : mPixId(0), mBundleMaskTotal(0) {}

    void init(const int pixId, const int bundleMaskTotal);

    finline int search_maskBundle2(const uint64_t activePixelMask, const uint64_t pixelSearchMaskPixId[]) const;
    finline int search_maskBundle3(const uint64_t activePixelMask, const uint64_t pixelSearchMaskPixId[]) const;
    finline int search_maskBundle4(const uint64_t activePixelMask, const uint64_t pixelSearchMaskPixId[]) const;
    finline int search_maskBundle5(const uint64_t activePixelMask, const uint64_t pixelSearchMaskPixId[]) const;
    finline int search_maskBundle6(const uint64_t activePixelMask, const uint64_t pixelSearchMaskPixId[]) const;
    finline int search_maskBundle7(const uint64_t activePixelMask, const uint64_t pixelSearchMaskPixId[]) const;
    finline int search_maskBundle8(const uint64_t activePixelMask, const uint64_t pixelSearchMaskPixId[]) const;

    int getBundleMaskTotal() const { return mBundleMaskTotal; }

    uint64_t test(const uint64_t mask64) const { return onOffSwitchMask(mask64); }

    std::string show(const std::string &hd) const;

protected:
    int mPixId;
    int mBundleMaskTotal;
    
    std::vector<TileExtrapolationPhase> mPhases;

    //------------------------------

    finline uint64_t onOffSwitchMask(const uint64_t mask64) const {
        // This is equivalent to (mask64)? (uint64_t)0xffffffffffffffff: (uint64_t)0x0;
        return static_cast<uint64_t>(((static_cast<int64_t>(mask64) & static_cast<int64_t>(0x8000000000000000)) |
                                      (static_cast<int64_t>(0x0) - static_cast<int64_t>(mask64))) >> 63);
    }

    finline uint64_t countRightZeroBit(uint64_t mask64) const {
        //
        // bit scan forward : This function does not work when mask64 == 0 because bsf return undefined value.
        //
        uint64_t result;
        asm volatile("bsfq %1, %0": "=r"(result): "r"(mask64));
        return result;
    }
}; // TileExtrapolationPhaseManager

finline int
TileExtrapolationPhaseManager::search_maskBundle2(const uint64_t activePixelMask, const uint64_t pixelSearchMaskPixId[]) const
//
// bundle mask total 2
//
{
    for (size_t phaseId = 0; phaseId < mPhases.size(); ++phaseId) {
        const TileExtrapolationPhase &currPhase = mPhases[phaseId];
            
        if (activePixelMask & currPhase.mPhaseMask) {
            uint64_t currMaskA = activePixelMask & pixelSearchMaskPixId[currPhase.mStartMaskId];
            uint64_t currMaskB = activePixelMask & pixelSearchMaskPixId[currPhase.mStartMaskId + 1];

            uint64_t switchMaskA = onOffSwitchMask(currMaskA);

            uint64_t resultMask = currMaskB;
            resultMask = currMaskA | (~switchMaskA & resultMask);

            return static_cast<int>(countRightZeroBit(resultMask));
        }
    }
    return -1;
}

finline int
TileExtrapolationPhaseManager::search_maskBundle3(const uint64_t activePixelMask, const uint64_t pixelSearchMaskPixId[]) const
//
// bundle mask total 3
//
{
    for (size_t phaseId = 0; phaseId < mPhases.size(); ++phaseId) {
        const TileExtrapolationPhase &currPhase = mPhases[phaseId];
            
        if (activePixelMask & currPhase.mPhaseMask) {
            uint64_t currMaskA = activePixelMask & pixelSearchMaskPixId[currPhase.mStartMaskId];
            uint64_t currMaskB = activePixelMask & pixelSearchMaskPixId[currPhase.mStartMaskId + 1];
            uint64_t currMaskC = activePixelMask & pixelSearchMaskPixId[currPhase.mStartMaskId + 2];

            uint64_t switchMaskA = onOffSwitchMask(currMaskA);
            uint64_t switchMaskB = onOffSwitchMask(currMaskB);

            uint64_t resultMask = currMaskC;
            resultMask = currMaskB | (~switchMaskB & resultMask);
            resultMask = currMaskA | (~switchMaskA & resultMask);

            return static_cast<int>(countRightZeroBit(resultMask));
        }
    }
    return -1;
}

finline int
TileExtrapolationPhaseManager::search_maskBundle4(const uint64_t activePixelMask, const uint64_t pixelSearchMaskPixId[]) const
//
// bundle mask total 4
//
{
    for (size_t phaseId = 0; phaseId < mPhases.size(); ++phaseId) {
        const TileExtrapolationPhase &currPhase = mPhases[phaseId];
            
        if (activePixelMask & currPhase.mPhaseMask) {
            uint64_t currMaskA = activePixelMask & pixelSearchMaskPixId[currPhase.mStartMaskId];
            uint64_t currMaskB = activePixelMask & pixelSearchMaskPixId[currPhase.mStartMaskId + 1];
            uint64_t currMaskC = activePixelMask & pixelSearchMaskPixId[currPhase.mStartMaskId + 2];
            uint64_t currMaskD = activePixelMask & pixelSearchMaskPixId[currPhase.mStartMaskId + 3];

            uint64_t switchMaskA = onOffSwitchMask(currMaskA);
            uint64_t switchMaskB = onOffSwitchMask(currMaskB);
            uint64_t switchMaskC = onOffSwitchMask(currMaskC);

            uint64_t resultMask = currMaskD;
            resultMask = currMaskC | (~switchMaskC & resultMask);
            resultMask = currMaskB | (~switchMaskB & resultMask);
            resultMask = currMaskA | (~switchMaskA & resultMask);

            return static_cast<int>(countRightZeroBit(resultMask));
        }
    }
    return -1;
}

finline int
TileExtrapolationPhaseManager::search_maskBundle5(const uint64_t activePixelMask, const uint64_t pixelSearchMaskPixId[]) const
//
// bundle mask total 5
//
{
    for (size_t phaseId = 0; phaseId < mPhases.size(); ++phaseId) {
        const TileExtrapolationPhase &currPhase = mPhases[phaseId];
            
        if (activePixelMask & currPhase.mPhaseMask) {
            uint64_t currMaskA = activePixelMask & pixelSearchMaskPixId[currPhase.mStartMaskId];
            uint64_t currMaskB = activePixelMask & pixelSearchMaskPixId[currPhase.mStartMaskId + 1];
            uint64_t currMaskC = activePixelMask & pixelSearchMaskPixId[currPhase.mStartMaskId + 2];
            uint64_t currMaskD = activePixelMask & pixelSearchMaskPixId[currPhase.mStartMaskId + 3];
            uint64_t currMaskE = activePixelMask & pixelSearchMaskPixId[currPhase.mStartMaskId + 4];

            uint64_t switchMaskA = onOffSwitchMask(currMaskA);
            uint64_t switchMaskB = onOffSwitchMask(currMaskB);
            uint64_t switchMaskC = onOffSwitchMask(currMaskC);
            uint64_t switchMaskD = onOffSwitchMask(currMaskD);

            uint64_t resultMask = currMaskE;
            resultMask = currMaskD | (~switchMaskD & resultMask);
            resultMask = currMaskC | (~switchMaskC & resultMask);
            resultMask = currMaskB | (~switchMaskB & resultMask);
            resultMask = currMaskA | (~switchMaskA & resultMask);

            return static_cast<int>(countRightZeroBit(resultMask));
        }
    }
    return -1;
}

finline int
TileExtrapolationPhaseManager::search_maskBundle6(const uint64_t activePixelMask, const uint64_t pixelSearchMaskPixId[]) const
//
// bundle mask total 6
//
{
    for (size_t phaseId = 0; phaseId < mPhases.size(); ++phaseId) {
        const TileExtrapolationPhase &currPhase = mPhases[phaseId];
            
        if (activePixelMask & currPhase.mPhaseMask) {
            uint64_t currMaskA = activePixelMask & pixelSearchMaskPixId[currPhase.mStartMaskId];
            uint64_t currMaskB = activePixelMask & pixelSearchMaskPixId[currPhase.mStartMaskId + 1];
            uint64_t currMaskC = activePixelMask & pixelSearchMaskPixId[currPhase.mStartMaskId + 2];
            uint64_t currMaskD = activePixelMask & pixelSearchMaskPixId[currPhase.mStartMaskId + 3];
            uint64_t currMaskE = activePixelMask & pixelSearchMaskPixId[currPhase.mStartMaskId + 4];
            uint64_t currMaskF = activePixelMask & pixelSearchMaskPixId[currPhase.mStartMaskId + 5];

            uint64_t switchMaskA = onOffSwitchMask(currMaskA);
            uint64_t switchMaskB = onOffSwitchMask(currMaskB);
            uint64_t switchMaskC = onOffSwitchMask(currMaskC);
            uint64_t switchMaskD = onOffSwitchMask(currMaskD);
            uint64_t switchMaskE = onOffSwitchMask(currMaskE);

            uint64_t resultMask = currMaskF;
            resultMask = currMaskE | (~switchMaskE & resultMask);
            resultMask = currMaskD | (~switchMaskD & resultMask);
            resultMask = currMaskC | (~switchMaskC & resultMask);
            resultMask = currMaskB | (~switchMaskB & resultMask);
            resultMask = currMaskA | (~switchMaskA & resultMask);

            return static_cast<int>(countRightZeroBit(resultMask));
        }
    }
    return -1;
}

finline int
TileExtrapolationPhaseManager::search_maskBundle7(const uint64_t activePixelMask, const uint64_t pixelSearchMaskPixId[]) const
//
// bundle mask total 7
//
{
    for (size_t phaseId = 0; phaseId < mPhases.size(); ++phaseId) {
        const TileExtrapolationPhase &currPhase = mPhases[phaseId];
            
        if (activePixelMask & currPhase.mPhaseMask) {
            uint64_t currMaskA = activePixelMask & pixelSearchMaskPixId[currPhase.mStartMaskId];
            uint64_t currMaskB = activePixelMask & pixelSearchMaskPixId[currPhase.mStartMaskId + 1];
            uint64_t currMaskC = activePixelMask & pixelSearchMaskPixId[currPhase.mStartMaskId + 2];
            uint64_t currMaskD = activePixelMask & pixelSearchMaskPixId[currPhase.mStartMaskId + 3];
            uint64_t currMaskE = activePixelMask & pixelSearchMaskPixId[currPhase.mStartMaskId + 4];
            uint64_t currMaskF = activePixelMask & pixelSearchMaskPixId[currPhase.mStartMaskId + 5];
            uint64_t currMaskG = activePixelMask & pixelSearchMaskPixId[currPhase.mStartMaskId + 6];

            uint64_t switchMaskA = onOffSwitchMask(currMaskA);
            uint64_t switchMaskB = onOffSwitchMask(currMaskB);
            uint64_t switchMaskC = onOffSwitchMask(currMaskC);
            uint64_t switchMaskD = onOffSwitchMask(currMaskD);
            uint64_t switchMaskE = onOffSwitchMask(currMaskE);
            uint64_t switchMaskF = onOffSwitchMask(currMaskF);

            uint64_t resultMask = currMaskG;
            resultMask = currMaskF | (~switchMaskF & resultMask);
            resultMask = currMaskE | (~switchMaskE & resultMask);
            resultMask = currMaskD | (~switchMaskD & resultMask);
            resultMask = currMaskC | (~switchMaskC & resultMask);
            resultMask = currMaskB | (~switchMaskB & resultMask);
            resultMask = currMaskA | (~switchMaskA & resultMask);

            return static_cast<int>(countRightZeroBit(resultMask));
        }
    }
    return -1;
}

finline int
TileExtrapolationPhaseManager::search_maskBundle8(const uint64_t activePixelMask, const uint64_t pixelSearchMaskPixId[]) const
//
// bundle mask total 8
//
{
    for (size_t phaseId = 0; phaseId < mPhases.size(); ++phaseId) {
        const TileExtrapolationPhase &currPhase = mPhases[phaseId];
            
        if (activePixelMask & currPhase.mPhaseMask) {
            uint64_t currMaskA = activePixelMask & pixelSearchMaskPixId[currPhase.mStartMaskId];
            uint64_t currMaskB = activePixelMask & pixelSearchMaskPixId[currPhase.mStartMaskId + 1];
            uint64_t currMaskC = activePixelMask & pixelSearchMaskPixId[currPhase.mStartMaskId + 2];
            uint64_t currMaskD = activePixelMask & pixelSearchMaskPixId[currPhase.mStartMaskId + 3];
            uint64_t currMaskE = activePixelMask & pixelSearchMaskPixId[currPhase.mStartMaskId + 4];
            uint64_t currMaskF = activePixelMask & pixelSearchMaskPixId[currPhase.mStartMaskId + 5];
            uint64_t currMaskG = activePixelMask & pixelSearchMaskPixId[currPhase.mStartMaskId + 6];
            uint64_t currMaskH = activePixelMask & pixelSearchMaskPixId[currPhase.mStartMaskId + 7];    

            uint64_t switchMaskA = onOffSwitchMask(currMaskA);
            uint64_t switchMaskB = onOffSwitchMask(currMaskB);
            uint64_t switchMaskC = onOffSwitchMask(currMaskC);
            uint64_t switchMaskD = onOffSwitchMask(currMaskD);
            uint64_t switchMaskE = onOffSwitchMask(currMaskE);
            uint64_t switchMaskF = onOffSwitchMask(currMaskF);
            uint64_t switchMaskG = onOffSwitchMask(currMaskG);

            uint64_t resultMask = currMaskH;
            resultMask = currMaskG | (~switchMaskG & resultMask);
            resultMask = currMaskF | (~switchMaskF & resultMask);
            resultMask = currMaskE | (~switchMaskE & resultMask);
            resultMask = currMaskD | (~switchMaskD & resultMask);
            resultMask = currMaskC | (~switchMaskC & resultMask);
            resultMask = currMaskB | (~switchMaskB & resultMask);
            resultMask = currMaskA | (~switchMaskA & resultMask);

            return static_cast<int>(countRightZeroBit(resultMask));
        }
    }
    return -1;
}

//------------------------------------------------------------------------------

class TileExtrapolation
{
public:
    TileExtrapolation();

    void searchActiveNearestPixel(const uint64_t activePixelMask, int extrapolatePixIdArray[64],
                                  const int minX = 0, const int maxX = 8, const int minY = 0, const int maxY = 8 ) const {
        //
        // activePixelMask        : input : 8x8pixels = 64bit, each bit represented pixel is active(1) or not(0).
        // extrapolatePixIdArray  : output : result of pixel id which need to fill
        // minX, maxX, minY, maxY : active boundary of tile
        //
        // I run loop test and set following bundary activePixTotal number to switch search logic. Toshi (Oct/26/2017)
        //
        // Test is done like this.
        // 1) randomly generated active pixel pattern by loop
        // 2) for each pattern, try all of the logic which are bundle mask total from 2 to 8 and no bundle case.
        // 3) compare result and find best(fastest) logic for that pattern.
        // 4) try multiple times (i.e., run 2~3 multiple times) and find optimal logic for each activePixTotal
        //
        // Test run is done on tealpseudo (Intel(R) Xeon(R) CPU E5-2697 v3 @ 2.60GHz) at Oct/26/2017.
        //   activePixTotal :     1 : bundle 3
        //                     2~18 : bundle 2
        //                    19~64 : bundle 1
        //
        int activePixTotal = static_cast<int>(countBit64(activePixelMask));
        if (activePixTotal >= 19) {
            return searchActiveNearestPixel_maskBundle1(activePixelMask, extrapolatePixIdArray, minX, maxX, minY, maxY);
        } else if (activePixTotal >= 2) {
            return searchActiveNearestPixel_maskBundle2(activePixelMask, extrapolatePixIdArray, minX, maxX, minY, maxY);
        }
        return searchActiveNearestPixel_maskBundle3(activePixelMask, extrapolatePixIdArray, minX, maxX, minY, maxY);
    }

    //
    // Why following APIs are public even we have searchActiveNearestPixel() is because test program needs to
    // access following logic specifically. This means following *_maskBundle?() functions should
    // not used by moonray and only used by test program.
    //
    void searchActiveNearestPixel_maskBundle1(const uint64_t activePixelMask, int extrapolatePixIdArray[64],
                                              const int minX = 0, const int maxX = 8,
                                              const int minY = 0, const int maxY = 8 ) const {
        for (int y = minY; y < maxY; ++y) {
            for (int x = minX; x < maxX; ++x) {
                unsigned pixId = (y << 3) + x;
                extrapolatePixIdArray[pixId] = searchActiveNearestPixelMain_maskBundle1(activePixelMask, pixId);
            }
        }
    }
    void searchActiveNearestPixel_maskBundle2(const uint64_t activePixelMask, int extrapolatePixIdArray[64],
                                              const int minX = 0, const int maxX = 8,
                                              const int minY = 0, const int maxY = 8 ) const {
        for (int y = minY; y < maxY; ++y) {
            for (int x = minX; x < maxX; ++x) {
                unsigned pixId = (y << 3) + x;
                extrapolatePixIdArray[pixId] = searchActiveNearestPixelMain_maskBundle2(activePixelMask, pixId);
            }
        }
    }
    void searchActiveNearestPixel_maskBundle3(const uint64_t activePixelMask, int extrapolatePixIdArray[64],
                                              const int minX = 0, const int maxX = 8,
                                              const int minY = 0, const int maxY = 8 ) const {
        for (int y = minY; y < maxY; ++y) {
            for (int x = minX; x < maxX; ++x) {
                unsigned pixId = (y << 3) + x;
                extrapolatePixIdArray[pixId] = searchActiveNearestPixelMain_maskBundle3(activePixelMask, pixId);
            }
        }
    }
    void searchActiveNearestPixel_maskBundle4(const uint64_t activePixelMask, int extrapolatePixIdArray[64],
                                              const int minX = 0, const int maxX = 8,
                                              const int minY = 0, const int maxY = 8 ) const {
        for (int y = minY; y < maxY; ++y) {
            for (int x = minX; x < maxX; ++x) {
                unsigned pixId = (y << 3) + x;
                extrapolatePixIdArray[pixId] = searchActiveNearestPixelMain_maskBundle4(activePixelMask, pixId);
            }
        }
    }
    void searchActiveNearestPixel_maskBundle5(const uint64_t activePixelMask, int extrapolatePixIdArray[64],
                                              const int minX = 0, const int maxX = 8,
                                              const int minY = 0, const int maxY = 8 ) const {
        for (int y = minY; y < maxY; ++y) {
            for (int x = minX; x < maxX; ++x) {
                unsigned pixId = (y << 3) + x;
                extrapolatePixIdArray[pixId] = searchActiveNearestPixelMain_maskBundle5(activePixelMask, pixId);
            }
        }
    }
    void searchActiveNearestPixel_maskBundle6(const uint64_t activePixelMask, int extrapolatePixIdArray[64],
                                              const int minX = 0, const int maxX = 8,
                                              const int minY = 0, const int maxY = 8 ) const {
        for (int y = minY; y < maxY; ++y) {
            for (int x = minX; x < maxX; ++x) {
                unsigned pixId = (y << 3) + x;
                extrapolatePixIdArray[pixId] = searchActiveNearestPixelMain_maskBundle6(activePixelMask, pixId);
            }
        }
    }
    void searchActiveNearestPixel_maskBundle7(const uint64_t activePixelMask, int extrapolatePixIdArray[64],
                                              const int minX = 0, const int maxX = 8,
                                              const int minY = 0, const int maxY = 8 ) const {
        for (int y = minY; y < maxY; ++y) {
            for (int x = minX; x < maxX; ++x) {
                unsigned pixId = (y << 3) + x;
                extrapolatePixIdArray[pixId] = searchActiveNearestPixelMain_maskBundle7(activePixelMask, pixId);
            }
        }
    }
    void searchActiveNearestPixel_maskBundle8(const uint64_t activePixelMask, int extrapolatePixIdArray[64],
                                              const int minX = 0, const int maxX = 8,
                                              const int minY = 0, const int maxY = 8 ) const {
        for (int y = minY; y < maxY; ++y) {
            for (int x = minX; x < maxX; ++x) {
                unsigned pixId = (y << 3) + x;
                extrapolatePixIdArray[pixId] = searchActiveNearestPixelMain_maskBundle8(activePixelMask, pixId);
            }
        }
    }

    static std::string showMask(const std::string &hd, const uint64_t mask);
    static std::string showPixIdArray(const std::string &hd, const int extrapolatePixIdArray[64]);

    const TileExtrapolationPhaseManager &getPhaseManager(const int maskBundleTotal, const int pixId) const; // for debug
    static uint64_t getPixelSearchMask(const int x, const int y, const int maskId); // for debug

protected:
    int searchActiveNearestPixelMain_maskBundle1(const uint64_t activePixelMask, const unsigned pixId) const;
    int searchActiveNearestPixelMain_maskBundle2(const uint64_t activePixelMask, const unsigned pixId) const;
    int searchActiveNearestPixelMain_maskBundle3(const uint64_t activePixelMask, const unsigned pixId) const;
    int searchActiveNearestPixelMain_maskBundle4(const uint64_t activePixelMask, const unsigned pixId) const;
    int searchActiveNearestPixelMain_maskBundle5(const uint64_t activePixelMask, const unsigned pixId) const;
    int searchActiveNearestPixelMain_maskBundle6(const uint64_t activePixelMask, const unsigned pixId) const;
    int searchActiveNearestPixelMain_maskBundle7(const uint64_t activePixelMask, const unsigned pixId) const;
    int searchActiveNearestPixelMain_maskBundle8(const uint64_t activePixelMask, const unsigned pixId) const;

    finline uint64_t countBit64(uint64_t mask64) const;
    finline uint64_t countRightZeroBit(uint64_t mask64) const;

    //------------------------------

    //
    // This is phase manager to find out nearest active pixel search based on bundle mask total number.
    // This information is constructed by init() stage and accessed const way from search stage.
    // We keep several different mask bundle total versions independently.
    //
    TileExtrapolationPhaseManager mExtrapolationPhaseManager_bundle2[64];
    TileExtrapolationPhaseManager mExtrapolationPhaseManager_bundle3[64];
    TileExtrapolationPhaseManager mExtrapolationPhaseManager_bundle4[64];
    TileExtrapolationPhaseManager mExtrapolationPhaseManager_bundle5[64];
    TileExtrapolationPhaseManager mExtrapolationPhaseManager_bundle6[64];
    TileExtrapolationPhaseManager mExtrapolationPhaseManager_bundle7[64];
    TileExtrapolationPhaseManager mExtrapolationPhaseManager_bundle8[64];
}; // TileExtrapolation

finline uint64_t
TileExtrapolation::countBit64(uint64_t mask64) const
{
    //
    // population count : how many bits are on ?
    //
    return _mm_popcnt_u64(mask64);
}

finline uint64_t
TileExtrapolation::countRightZeroBit(uint64_t mask64) const
//
// This function does not work when mask64 == 0 because bsf return undefined value.
//
{
    //
    // bit scan forward : counting zero bits on right side
    //
    uint64_t result;
    asm volatile("bsfq %1, %0": "=r"(result): "r"(mask64));
    return result;
}

} // namespace fb_util
} // namespace scene_rdl2

