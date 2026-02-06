// Copyright 2026 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "ShmUtilTool.h"

#include <cstdlib> // EXIT_SUCCESS, EXIT_FAILURE
#include <iostream>

int
main(int ac, char** av)
{
    scene_rdl2::grid_util::ShmUtilTool shmUtilTool;
    if (!shmUtilTool.main(ac, av)) {
        std::cerr << "ERROR : shmUtilTool.main() failed\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
