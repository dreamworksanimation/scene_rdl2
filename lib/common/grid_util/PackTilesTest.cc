// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#include "PackTilesTest.h"

#include "ActiveBitTable.h"
#include "ActivePixelsArray.h"
#include "PackTiles.h"
#include "PackActiveTiles.h"

#include <scene_rdl2/common/fb_util/ActivePixels.h>

#include <fstream>

namespace scene_rdl2 {
namespace grid_util {

static bool
readActivePixelsArray(const std::string &filename,
                      ActivePixelsArray &outActivePixelsArray)
{
    std::ifstream fin(filename, std::ios::in|std::ios::binary);
    if (!fin) {
        std::cerr << "read open failed. file:" << filename << std::endl;
        return false;
    }

    fin.seekg(0, std::ios_base::end);
    size_t fileSize = fin.tellg();
    fin.seekg(0, std::ios_base::beg);

    std::string data(fileSize, 0x0);
    fin.read(&data[0], fileSize);
    if (!fin) {
        std::cerr << "read data failed. file:" << filename << std::endl;
        return false;
    }

    fin.close();

    //------------------------------

    try {
        outActivePixelsArray.decode(data);
    }
    catch (...) {
        std::cerr << "decode activePixelsArray failed." << std::endl;
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------

// static function
void
PackTilesTest::activeBitTablesEncodedSizeTest(const unsigned tableSize,
                                              const unsigned minOnId,
                                              const unsigned maxOnId)
// ActiveBitTables serialize data size testing.
// Set active item by range from min to max.
{
    ActiveBitTables::encodeSizeTest(tableSize, minOnId, maxOnId);
}

// static function
void
PackTilesTest::packActiveTilesCodecVerifyTest(const unsigned width,
                                              const unsigned height,
                                              const unsigned totalActivePixels)
// PackActiveTiles codec verify test.
// Intentionally using std::cerr for debug purpose.
{
    fb_util::ActivePixels activePixels;
    activePixels.init(width, height);
    PackActiveTiles::randomActivePixels(activePixels, totalActivePixels);

    // std::cerr << activePixels.show("") << std::endl;
    std::cerr << "activePix:" << activePixels.getActivePixelTotal() << ' ';

    if (!PackActiveTiles::codecVerify(activePixels)) {
        std::cerr << "codecVerify() failed" << std::endl;
    } else {
        // std::cerr << "codecVerify() OK" << std::endl;
    }
}

// static function
void
PackTilesTest::runLenBitTableCodecVerifyTest(const std::vector<uint64_t> &testData)
// RunLenBitTable codec verify test
// Specify array of bitmask for input of encode data.
// Intentionally using std::cerr for debug purpose.
{
    size_t total = testData.size();
    scene_rdl2::grid_util::RunLenBitTable tbl(total);

    tbl.setTestData(testData);

    if (!RunLenBitTable::codecVerify(tbl)) {
        std::cerr << "codecVerify() failed" << std::endl;
        // std::cerr << tbl.show("") << std::endl;
    } else {
        std::cerr << "codecVerify() OK" << std::endl;
    }
}

// static function
void    
PackTilesTest::timingTestEnqTileMaskBlock(const unsigned width,
                                          const unsigned height,
                                          const unsigned totalActivePixels)
//
// EnqTileMaskBlock operation timing compare test between ver1 and ver2.
// All ActivePixels are procedurally generated.
//   ver1 : original naive activeTileId + activePixelMask
//   ver2 : PackActiveTiles encoding method
//
{
    PackTiles::timingTestEnqTileMaskBlock(width, height, totalActivePixels);
}

// static function
void
PackTilesTest::replaySnapshotDelta(const std::string &filename)
//
// EnqTimeMaskBlock ver1+ver2 timing test using already dumped ActivePixelsArray data
//   ver1 : original naive activeTileId + activePixelMask
//   ver2 : PackActiveTiles encoding method
//
// Typical method to create this snapshotDeltaDump file is to use debug console command of
// progmcrt_dispatch.
// Followings are related progmcrt_dispatch debug console commands.
//   snapshotDeltaRecStart     : start snapshotDelta rec
//   snapshotDeltaRecStop      : stop snapshotDelta rec
//   snapshotDeltaRecReset     : reset and clear previous snapshotDelta rec info and status
//   snapshotDeltaRecDump file : output snapshotDelta rec info to the file. required "stop" first.
//
{
    // Typically, cerr output from this function will be used by gnuplot.
    // So we output start by # symbol about comment information.

    std::cerr << "#>> PackTilestest.cc replaySnapshotDelta() filename:" << filename << " start" << std::endl;

    ActivePixelsArray activePixelsArray;
    if (!readActivePixelsArray(filename, activePixelsArray)) {
        std::cerr << "read activePixelsArray failed." << std::endl;
        return;
    }

    //------------------------------

    std::cerr << "# 1      2                 3        4        5        6        7"
              << " 8                 9" << std::endl;
    std::cerr << "# coarse totalActivePixels ver1Time ver2Time ver1Size ver2Size %"
              << " ver1PixPosInfoAve ver2PixPosInfoAve" << std::endl;

    for (size_t i = 0; i < activePixelsArray.size(); ++i) {
        const fb_util::ActivePixels &currActivePixels = activePixelsArray.get(i);
        bool currCoarsePass = activePixelsArray.getCoarsePass(i);

        std::cerr << currCoarsePass << ' ';

        PackTiles::PrecisionMode precisionMode =
            ((currCoarsePass)? PackTiles::PrecisionMode::H16: PackTiles::PrecisionMode::F32);
        PackTiles::timingAndSizeTest(currActivePixels, precisionMode);
    }

    std::cerr << "#>> PackTilestest.cc replaySnapshotDelta() filename:" << filename << " done" << std::endl;
}

// static function
void
PackTilesTest::replaySnapshotDelta_dumpActivePixPos(const std::string &filename,
                                                    unsigned snapshotId)
//
// Dump activePixel position info about particular snapshotId of already dumped ActivePixelsArray data
//
// Typical method to create this snapshotDeltaDump file is to use debug console command of
// progmcrt_dispatch.
// Followings are related progmcrt_dispatch debug console commands.
//   snapshotDeltaRecStart     : start snapshotDelta rec
//   snapshotDeltaRecStop      : stop snapshotDelta rec
//   snapshotDeltaRecReset     : reset and clear previous snapshotDelta rec info and status
//   snapshotDeltaRecDump file : output snapshotDelta rec info to the file. required "stop" first.
//
{
    // Typically, cerr output from this function will be used by gnuplot.
    // So we output start by # symbol about comment information.

    std::cerr << "#>> PackTilestest.cc replaySnapshotDelta_dumpActivePixPos()"
              << " filename:" << filename
              << " snapshotId:" << snapshotId
              << " start" << std::endl;

    ActivePixelsArray activePixelsArray;
    if (!readActivePixelsArray(filename, activePixelsArray)) {
        std::cerr << "read activePixelsArray failed." << std::endl;
        return;
    }

    //------------------------------

    std::cerr << "# totalSnapshotCount:" << activePixelsArray.size() << std::endl;

    snapshotId = std::min(snapshotId, (unsigned)activePixelsArray.size() - 1);
    const fb_util::ActivePixels &currActivePixels = activePixelsArray.get(snapshotId);

    std::cerr << "# activePixelTotal:" << currActivePixels.getActivePixelTotal() << std::endl;

    unsigned tileXTotal = currActivePixels.getNumTilesX();

    std::cerr << "# 1    2" << std::endl;
    std::cerr << "# posX posY" << std::endl;

    fb_util::ActivePixels::crawlAllActivePixels
        (currActivePixels,
         [&](unsigned currPixOffset) {
            unsigned tileId = currPixOffset / 64;
            unsigned tileBaseX = (tileId % tileXTotal) * 8;
            unsigned tileBaseY = (tileId / tileXTotal) * 8;
            unsigned pixLocalId = currPixOffset % 64;
            unsigned pixLocalX = pixLocalId % 8;
            unsigned pixLocalY = pixLocalId / 8;
            unsigned pixX = tileBaseX + pixLocalX;
            unsigned pixY = tileBaseY + pixLocalY;
            
            std::cerr << pixX << ' ' << pixY << std::endl;
        });

    std::cerr << "#>> PackTilestest.cc replaySnapshotDelta_dumpActivePixPos()"
              << " filename:" << filename
              << " snapshotId:" << snapshotId
              << " done" << std::endl;
}

} // namespace grid_util
} // namespace scene_rdl2

