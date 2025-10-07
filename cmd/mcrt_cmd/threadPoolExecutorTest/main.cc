// Copyright 2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include <scene_rdl2/render/util/ThreadPoolExecutor.h>

#include <iostream>
#include <thread>

void
testLoop(const size_t threadTotal, const int loopCount)
{
    for (size_t loopId = 0; loopId < loopCount; ++loopId) {
        std::cerr << "loopId:" << loopId << " start ";
        scene_rdl2::ThreadPoolExecutor pool(threadTotal, [](size_t id) { return id; });
        std::cerr << (pool.testBootShutdown() ? "OK" : "NG") << '\n';
    }
}

int
main(int argc, char** argv)
//
// This program is designed for the endurance test of ThreadPoolExecutor and executes a user-defined
// loop count without any runtime duration limit.
// The test body is the same as unitTest (scene_rdl2/tests/lib/render/util/TestTHreadPoolExecutor.{h,cc}).
//        
{
    if (argc < 2) {
        std::cerr << "Usage : " << argv[0] << " <loop-count>\n";
        return 0;
    }

    int loopCount = atoi(argv[1]);
    std::cerr << "loopCount:" << loopCount << '\n';

    size_t threadTotal = std::thread::hardware_concurrency();
    std::cerr << "threadTotal:" << threadTotal << '\n';

    testLoop(threadTotal, loopCount);

    return 0;
}
