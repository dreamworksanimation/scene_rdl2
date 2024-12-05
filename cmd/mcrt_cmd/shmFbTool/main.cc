// Copyright 2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "ShmFbTool.h"

#include <iostream>

int
main(int ac, char** av)
{
    scene_rdl2::grid_util::ShmFbTool shmFbTool;
    if (!shmFbTool.main(ac, av)) {
        std::cerr << "ERROR : shmFbTool.main() failed\n";
        return 0;
    }
    return 1;
}
