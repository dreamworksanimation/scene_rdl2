// Copyright 2026 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "ShmUtilTool.h"

#include <scene_rdl2/common/grid_util/ShmAffinityInfo.h>
#include <scene_rdl2/common/grid_util/ShmData.h>

namespace scene_rdl2 {
namespace grid_util {

std::string
ShmUtilTool::showLs() const
{
    return ShmDataManager::showAllShmList([](const int shmId, const std::string& userName) {
        return ShmAffinityInfoManager::showShmIdDetailedInfo(shmId, userName);
    });
}

void
ShmUtilTool::parserConfigure()
{
    mParser.description("ShmUtilTool command options");

    mParser.opt("-ls", "", "list of shared memory and semaphore",
                [&](Arg& arg) { return arg.msg(showLs() + '\n'); });
}

} // namespace grid_util
} // namespace scene_rdl2
    
