// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#include <scene_rdl2/common/grid_util/PackTilesTest.h>

#include <cstdlib> // EXIT_SUCCESS
#include <iostream>

int main(int ac, char **av)
//
// This program shows pack-tile version1 and version 2 related performance analyze result based on
// the given file which already recorded as snapshotDeltaDump file.
// This program is only used by performance analyzing purpose only.
//    
// Typical method to create this snapshotDeltaDump file is to use debug console command of progmcrt_dispatch.
// Followings are related progmcrt_dispatch debug console commands.
//   snapshotDeltaRecStart     : start snapshotDelta rec
//   snapshotDeltaRecStop      : stop snapshotDelta rec
//   snapshotDeltaRecReset     : reset and clear previous snapshotDelta rec info and status
//   snapshotDeltaRecDump file : output snapshotDelta rec info to the file. required "stop" first.
//
{
    if (ac != 2) {
        std::cerr << "Usage : " << av[0] << " snapshotDeltaDumpFile" << std::endl;
    } else {
        scene_rdl2::grid_util::PackTilesTest::replaySnapshotDelta(av[1]);
    }

    return EXIT_SUCCESS;
}

