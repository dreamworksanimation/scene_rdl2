// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "AffinityMapTool.h"

#include <cstdlib> // EXIT_SUCCESS, EXIT_FAILURE
#include <iostream>

int
main(int ac, char** av)
{
    scene_rdl2::grid_util::AffinityMapTool affinityMapTool;
    if (!affinityMapTool.main(ac, av)) {
        std::cerr << "ERROR : affinityMapTool.main() failed\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
