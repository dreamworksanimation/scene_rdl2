// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include <scene_rdl2/common/grid_util/ShmFootmark.h>

#include <iostream>

int
main(int ac, char** av)
{
    if (ac < 2) {
        std::cerr << "Usage : " << av[0] << " <shMemId>\n"
                  << "Dump shared memory information created by ShmFootmark.\n";
        return 0;
    }

    int shMemId = atoi(av[1]);
    std::cerr << "shMemId:" << shMemId << '\n';

    scene_rdl2::grid_util::ShmFootmarkView footmarkView(shMemId);
    std::cerr << "[" << footmarkView.getAll() << "]\n";

    return 0;
}
