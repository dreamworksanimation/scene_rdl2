// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#include "PackActiveTiles.h"

#include "ActiveBitTable.h"
#include "RunLenBitTable.h"

#include <scene_rdl2/scene/rdl2/ValueContainerDeq.h>
#include <scene_rdl2/scene/rdl2/ValueContainerEnq.h>

#include <iomanip>
#include <random>

//#define DEBUG_MSG_ENQ
//#define DEBUG_MSG_ENQ_DETAIL_INFO

namespace scene_rdl2 {
namespace grid_util {

//
// PackAcitveTiles:enqTileMaskBlock() returns encode data type by unsigned char and
// this unsigned char status consists of 2 DumpMode (tileMode and pixMaskMode)
//

static constexpr unsigned char
combineMode(const ActiveBitTables::DumpMode tileMode,
            const RunLenBitTable::DumpMode pixMaskMode)
{
    return static_cast<unsigned char>(pixMaskMode) | static_cast<unsigned char>(tileMode);
}

static void
retrieveMode(unsigned char dumpMode,
             ActiveBitTables::DumpMode &tileMode,
             RunLenBitTable::DumpMode &pixMaskMode)
{
    tileMode = static_cast<ActiveBitTables::DumpMode>(dumpMode & ActiveBitTables::DUMPMODE_MASK);
    pixMaskMode = static_cast<RunLenBitTable::DumpMode>(dumpMode & RunLenBitTable::DUMPMODE_MASK);
}

//---------------------------------------------------------------------------------------------------------------

// conbined condition definition about both of ActiveBitTables and RunLenBitTable have DumpMode=SKIP_DUMP
static constexpr unsigned char COMBINED_CONDITION_ALL_SKIP =
    combineMode(ActiveBitTables::DumpMode::SKIP_DUMP, RunLenBitTable::DumpMode::SKIP_DUMP);

// static function
unsigned char    
PackActiveTiles::getAllSkipCondition()
//
// return condition definition about both of ActiveBitTables and RunLenBitTable have DumpMode=SKIP_DUMP
//
{
    return COMBINED_CONDITION_ALL_SKIP;
}

// static function
unsigned char    
PackActiveTiles::enqTileMaskBlock(const ActivePixels &activePixels,
                                  VContainerEnq &vContainerEnq, // output
                                  int64_t *sizeInfo) // output for debug
//
// int64_t *sizeInfo :
//     statistical information for tracking detail purpose.
//     If you don't need this information, you should set nullptr.
//     There is no computational overhead if sizeInfo is set to nullptr.
//     sizeInfo[0] : PackActiveTiles encoded data size (we call this as version 2)
//     sizeInfo[1] : Size difference between version2 and version1. sizeInfo[1] = sizeInfo[0] - ver1Size
//                   i.e. ver1Size = sizeInfo[0] - sizeInfo[1]
//
{
    //
    // Encoded data consists of 3 parts
    // 1) DumpMode
    //    Indicates dump mode of following active tile/pixel position blocks
    // 2) Active tile position block
    //    Focused on encoding active tile position information.
    //    We only encode active tile position and skip all empty tiles.
    //    There are several different dump mode options and automatically pick best mode.
    // 3) Active pixel position block
    //    Focused on encoding active pixel position information.
    //    We only encode active pixel position and skip all empty pixels.
    //    There are several different dump mode options and automatically pick best mode.
    // 

    unsigned numActiveTiles = activePixels.getActiveTileTotal();
    if (numActiveTiles == 0) {
        //
        // Empty active tile/pixel information. 
        //
        vContainerEnq.enqUChar(COMBINED_CONDITION_ALL_SKIP);
#       ifdef DEBUG_MSG_ENQ
        std::cerr << ">> PackActiveTiles.cc no active tiles" << std::endl;
#       endif // end DEBUG_MSG_ENQ 
        return COMBINED_CONDITION_ALL_SKIP;
    }

    unsigned numTiles = activePixels.getNumTiles();
    if (numTiles == numActiveTiles) {
        //
        // SKIP_DUMP mode : we don't need to output active tile position information
        //                  because all tiles are active.
        //
        RunLenBitTable pixMaskInfo(numTiles);
        crawlAllActivePixelsTile(activePixels, [&](const unsigned tileId) {
                pixMaskInfo.set(tileId, activePixels.getTileMask(tileId)); // all tiles are active
            });

        ActiveBitTables::DumpMode tileMode = ActiveBitTables::DumpMode::SKIP_DUMP;
        RunLenBitTable::DumpMode pixMaskMode = pixMaskInfo.finalize();
        unsigned char dumpMode = combineMode(tileMode, pixMaskMode);

        vContainerEnq.enqUChar(dumpMode); // enqueue dumpMode first

        // Skip enqueue operation about active tile position info here because we know all tiles are active
        // and dumpMode = SKIP_DUMP indicate enough this condition.

        enqPixMaskInfo(pixMaskMode, pixMaskInfo, vContainerEnq); // enqueue pixelMask info 

        if (sizeInfo) {
            // Compute data size of format version 1 for debug/analyze purpose
            size_t ver1Size = numTiles * (sizeof(unsigned int) + sizeof(uint64_t)); // old data size
            size_t ver2Size = 1 + pixMaskInfo.getDataSize(); // PackActiveTiles encoded data size
            int deltaSize = (int)ver2Size - (int)ver1Size;
            // if (deltaSize > 0) std::cerr << "ver2 is BIGGER!!" << std::endl; // useful for test 

            sizeInfo[0] = (int64_t)ver2Size; // byte
            sizeInfo[1] = (int64_t)deltaSize; // byte

#           ifdef DEBUG_MSG_ENQ
            float ratio = (float)ver2Size / (float)ver1Size;
#           ifdef DEBUG_MSG_ENQ_DETAIL_INFO
            std::cerr << ">> PackActiveTiles.cc " << showDumpMode(dumpMode) << '\n'
                      << " total:" << ver2Size
                      << " (ver1:" << deltaSize << ' '
                      << std::setw(5) << std::fixed << std::setprecision(3) << ratio << ')'
                      << std::endl;
#           else // else DEBUG_MSG_ENQ_DETAIL_INFO
            std::cerr << ">> PackActiveTiles.cc " << showDumpMode(dumpMode)
                      << " size:" << ver2Size
                      << " %:" << std::setw(5) << std::fixed << std::setprecision(3) << ratio
                      << " diff:" << deltaSize << std::endl;
#           endif // end !DEBUG_MSG_ENQ_DETAIL_INFO
#           endif // end DEBUG_MSG_ENQ
        }
        
        return dumpMode;
    }

    //
    // setup active tile condition
    //
    ActiveBitTables tilesInfo(numTiles);
    RunLenBitTable pixMaskInfo(numActiveTiles);
    {
        unsigned activeTileId = 0;
        crawlAllActivePixelsTile(activePixels,
                                 [&](const unsigned tileId) {
                                     if (uint64_t currMask = activePixels.getTileMask(tileId)) {
                                         tilesInfo.setOn(tileId);
                                         pixMaskInfo.set(activeTileId++, currMask);
                                     }
                                 });
    }

    //
    // pick up proper method to dump tilesInfo and pixMaskInfo
    //
    ActiveBitTables::DumpMode tileMode = tilesInfo.finalize();
    RunLenBitTable::DumpMode pixMaskMode = pixMaskInfo.finalize();
    unsigned char dumpMode = combineMode(tileMode, pixMaskMode);

    vContainerEnq.enqUChar(dumpMode); // enqueue dumpMode first

    // enqueue operation about active tile position info. We have several options to enqueue.
    // FULL_DUMP is used under debug purpose only. Because FULL_DELTA_DUMP performance is always
    // better and there is no reason to use FULL_DUMP.
    switch (tileMode) {
    case ActiveBitTables::DumpMode::FULL_DUMP:       tilesInfo.enqFullDump(vContainerEnq);      break;
    case ActiveBitTables::DumpMode::FULL_DELTA_DUMP: tilesInfo.enqFullDeltaDump(vContainerEnq); break;
    case ActiveBitTables::DumpMode::TABLE_DUMP:      tilesInfo.enqTblDump(vContainerEnq);       break;
    case ActiveBitTables::DumpMode::LEAF_TABLE_DUMP: tilesInfo.enqTblDump(vContainerEnq);       break;
    default : break; // never happen
    }

    enqPixMaskInfo(pixMaskMode, pixMaskInfo, vContainerEnq); // enqueue pixelMask info

    if (sizeInfo) {
        // Compute data size of format version 1 for debug/analyze purpose
        size_t ver1Size = numActiveTiles * (sizeof(unsigned int) + sizeof(uint64_t)); // old data size
        size_t fullDump, fullDeltaDump, tblDump;
        size_t tileInfoSize = tilesInfo.debugGetSizeInfo(fullDump, fullDeltaDump, tblDump);
        size_t pixMaskInfoSize = pixMaskInfo.getDataSize();
        size_t ver2Size = 1 + tileInfoSize + pixMaskInfoSize; // PackActiveTiles encoded data size
        int deltaSize = (int)ver2Size - (int)ver1Size;
        // if (deltaSize > 0) std::cerr << "ver2 is BIGGER!!" << std::endl; // useful for test

        sizeInfo[0] = (int64_t)ver2Size; // byte
        sizeInfo[1] = (int64_t)deltaSize; // byte

#       ifdef DEBUG_MSG_ENQ
        float ratio = (float)ver2Size / (float)ver1Size;
#       ifdef DEBUG_MSG_ENQ_DETAIL_INFO
        std::cerr << ">> PackActiveTiles.cc enqTileMaskBlock() {\n"
                  << "  " << showDumpMode(dumpMode) << '\n'
                  << "  tileInfo(full:" << fullDump
                  << " delta:" << fullDeltaDump
                  << " tbl:" << tblDump << ")=" << tileInfoSize << '\n'
                  << "  pixMaskInfo=" << pixMaskInfoSize << '\n'
                  << "  total:" << ver2Size
                  << " (ver1:" << ver1Size << " delta:" << deltaSize << " ratio:"
                  << std::setw(5) << std::fixed << std::setprecision(3) << ratio << ")\n"
                  << '}'
                  << std::endl;
#       else // else DEBUG_MSG_ENQ_DETAIL_INFO
        std::cerr << ">> PackActiveTiles.cc " << showDumpMode(dumpMode)
                  << " size:" << ver2Size
                  << " %:" << std::setw(5) << std::fixed << std::setprecision(3) << ratio
                  << " diff:" << deltaSize << std::endl;
#       endif // end !DEBUG_MSG_ENQ_DETAIL_INFO        
#       endif // end DEBUG_MSG_ENQ
    }

    return dumpMode;
}

// static function
bool
PackActiveTiles::deqTileMaskBlock(VContainerDeq &vContainerDeq,
                                  const unsigned activeTileTotal,
                                  ActivePixels &activePixels)
{
    unsigned char dumpMode;
    vContainerDeq.deqUChar(dumpMode);
    ActiveBitTables::DumpMode tileMode;
    RunLenBitTable::DumpMode pixMaskMode;
    retrieveMode(dumpMode, tileMode, pixMaskMode);

    //
    // reconstruct tilesInfo from data
    //
    ActiveBitTables tilesInfo(activePixels.getNumTiles());
    switch(tileMode) {
    case ActiveBitTables::DumpMode::SKIP_DUMP:
        break;
    case ActiveBitTables::DumpMode::FULL_DUMP:
        tilesInfo.deqFullDump(vContainerDeq, activeTileTotal);
        break;
    case ActiveBitTables::DumpMode::FULL_DELTA_DUMP:
        tilesInfo.deqFullDeltaDump(vContainerDeq, activeTileTotal);
        break;
    case ActiveBitTables::DumpMode::TABLE_DUMP:
        tilesInfo.deqTblDump(vContainerDeq, false);
        break;
    case ActiveBitTables::DumpMode::LEAF_TABLE_DUMP:
        tilesInfo.deqTblDump(vContainerDeq, true);
        break;
    default:
        return false;           // unknown tileMode -> no data
    }

    //
    // reconstruct pixMaskInfo from data
    //
    RunLenBitTable pixMaskInfo(activeTileTotal);
    switch (pixMaskMode) {
    case RunLenBitTable::DumpMode::SKIP_DUMP:                                           break;
    case RunLenBitTable::DumpMode::ALLMASK_DUMP: pixMaskInfo.deqAllMask(vContainerDeq); break;
    case RunLenBitTable::DumpMode::ALLID_DUMP:   pixMaskInfo.deqAllId(vContainerDeq);   break;
    case RunLenBitTable::DumpMode::RUNLEN_DUMP:  pixMaskInfo.deqRunLen(vContainerDeq);  break;
    default: return false;      // unknown pixMaskMode -> no data
    }

    //
    // reconstruct activePixels from tilesInfo and pixMaskInfo
    //
    if (tileMode == ActiveBitTables::DumpMode::SKIP_DUMP) {
        if (pixMaskMode == RunLenBitTable::DumpMode::SKIP_DUMP) {
            return false;       // no data
        } else {
            crawlAllActivePixelsTile(activePixels, [&](const unsigned tileId) {
                    activePixels.setTileMask(tileId, pixMaskInfo.get(tileId));
                });
        }
    } else {
        unsigned activeTileId = 0;
        tilesInfo.crawlActiveTblItem([&](unsigned tileId) {
                activePixels.setTileMask(tileId, pixMaskInfo.get(activeTileId));
                activeTileId++;
            });
    }

    return true;
}

// static function
void
PackActiveTiles::randomActivePixels(ActivePixels &activePixels, const unsigned totalActivePixels)
// for debug
{
    unsigned totalPixels = activePixels.getWidth() * activePixels.getHeight();

    std::random_device rnd;
    std::mt19937 mt(rnd());
    std::uniform_int_distribution<> randGen(0, totalPixels - 1);

    for (unsigned i = 0; i < totalActivePixels; ++i) {
        while (1) {
            unsigned pixId = randGen(mt);
            if (!getPix(activePixels, pixId)) {
                setPix(activePixels, pixId);
                break;
            }
        }
    }
}

// static function
bool
PackActiveTiles::codecVerify(const ActivePixels &activePixels)
{
    std::string data;
    VContainerEnq vContainerEnq(&data);
    unsigned char dumpMode = enqTileMaskBlock(activePixels, vContainerEnq);
    size_t dataSize = vContainerEnq.finalize();

    ActivePixels activePixels2;
    activePixels2.init(activePixels.getWidth(), activePixels.getHeight());
    activePixels2.reset();

    unsigned activeTileTotal = activePixels.getActiveTileTotal();

    VContainerDeq vContainerDeq(data.data(), dataSize);
    deqTileMaskBlock(vContainerDeq, activeTileTotal, activePixels2);

    if (!activePixels.compare(activePixels2)) {
        std::cerr << "codecVerifyError {\n";
        std::cerr << "  " << showDumpMode(dumpMode) << std::endl;
        std::cerr << activePixels.show("  in ") << std::endl;
        std::cerr << activePixels2.show("  out") << std::endl;
        std::cerr << activePixels.showFullInfo("  ") << std::endl;
        std::cerr << "}\n";
        return false;
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------

// static function
void
PackActiveTiles::enqPixMaskInfo(const RunLenBitTable::DumpMode pixMaskMode,
                                const RunLenBitTable &pixMaskInfo,
                                VContainerEnq &vContainerEnq)
{
    switch (pixMaskMode) {
    case RunLenBitTable::DumpMode::ALLMASK_DUMP: pixMaskInfo.enqAllMask(vContainerEnq); break;
    case RunLenBitTable::DumpMode::ALLID_DUMP  : pixMaskInfo.enqAllId(vContainerEnq);   break;
    case RunLenBitTable::DumpMode::RUNLEN_DUMP : pixMaskInfo.enqRunLen(vContainerEnq);  break;
    default : break; // never happened
    }
}

// static function
bool
PackActiveTiles::getPix(const ActivePixels &activePixels, const unsigned pixId)
// for debug
{
    if (pixId >= activePixels.getWidth() * activePixels.getHeight()) return false; // just in case

    bool pixStatus = false;;
    accessPixel(activePixels, pixId,
                [&](unsigned tileId, unsigned shift) {
                    const uint64_t &currMask = activePixels.getTileMask(tileId);
                    if (currMask & ((uint64_t)0x1 << shift)) pixStatus = true;
                });

    return pixStatus;
}

// static function
void
PackActiveTiles::setPix(ActivePixels &activePixels, const unsigned pixId)
// for debug
{
    accessPixel(activePixels, pixId,
                [&](unsigned tileId, unsigned shift) {
                    activePixels.orOp(tileId, ((uint64_t)0x1 << shift));
                });
}

// static function
std::string
PackActiveTiles::showDumpMode(const unsigned char dumpMode)
{
    std::ostringstream ostr;
    ostr << "dumpMode:0x" << std::hex << std::setw(2) << std::setfill('0') << (int)dumpMode << std::dec
         << "=(pixMask:" << RunLenBitTable::showDumpMode(dumpMode)
         << ",tile:" << ActiveBitTables::showDumpMode(dumpMode)
         << ')';
    return ostr.str();
}

} // namespace grid_util
} // namespace scene_rdl2

