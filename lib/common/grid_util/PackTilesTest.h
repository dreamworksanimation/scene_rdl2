// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#pragma once

//
// -- PackTile related testing APIs --
//
// This class only used for testing and debugging purpose for pack-tile codec
//

#include <cstdint>              // uint64_t
#include <string>
#include <vector>

namespace scene_rdl2 {
namespace grid_util {

class PackTilesTest
{
public:
    // ActiveBitTables serialize data size testing.
    // Set active item by range from min to max.
    static void activeBitTablesEncodedSizeTest(const unsigned tableSize,
                                               const unsigned minOnId,
                                               const unsigned maxOnId);

    // PackActiveTiles codec verify test
    static void packActiveTilesCodecVerifyTest(const unsigned width,
                                               const unsigned height,
                                               const unsigned totalActivePixels);

    // RunLenBitTable codec verify test
    // Specify array of bitmask for input of encode data.
    static void runLenBitTableCodecVerifyTest(const std::vector<uint64_t> &testData);

    // EnqTileMaskBlock operation timing compare test between ver1 and ver2.
    // All ActivePixels are procedurally generated.
    //   ver1 : original naive activeTileId + activePixelMask
    //   ver2 : PackActiveTiles encoding method
    static void timingTestEnqTileMaskBlock(const unsigned width,
                                           const unsigned height,
                                           const unsigned totalActivePixels);

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
    static void replaySnapshotDelta(const std::string &filename);

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
    static void replaySnapshotDelta_dumpActivePixPos(const std::string &filename, const unsigned snapshotId);
};

} // namespace grid_util
} // namespace scene_rdl2

