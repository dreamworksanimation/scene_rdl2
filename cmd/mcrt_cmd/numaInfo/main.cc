// Copyright 2024-2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "NumaInfo.h"

#include <cstdlib> // EXIT_SUCCESS, EXIT_FAILURE
#include <iostream>

int
main(int ac, char** av)
{
#ifdef PLATFORM_APPLE
    std::cerr << "ERROR : numaInfo is not supported on Mac\n";
    return EXIT_FAILURE;
#else // else PLATFORM_APPLE    
    scene_rdl2::grid_util::NumaInfo numaInfo;
    if (!numaInfo.main(ac, av)) {
        std::cerr << "ERROR : numaInfo.main() failed\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
#endif // end of Non PLATFORM_APPLE
}
