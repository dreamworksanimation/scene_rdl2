// Copyright 2023-2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#include "TestArray2D.h"
#include "TestAtomicFloat.h"
#include "TestMemPool.h"
#include "TestProcCpuAffinity.h"
#include "TestThreadPoolExecutor.h"
#include "test_util.h"

#include <scene_rdl2/pdevunit/pdevunit.h>

#include <iostream>

int
main(int argc, char *argv[])
{
    CPPUNIT_TEST_SUITE_REGISTRATION(scene_rdl2::pbr::TestArray2D);
    CPPUNIT_TEST_SUITE_REGISTRATION(scene_rdl2::pbr::TestAtomicFloat);
    CPPUNIT_TEST_SUITE_REGISTRATION(scene_rdl2::alloc::TestMemPool); // 21.0211 sec on cobaltcard @ Apr/28/2025
#ifndef PLATFORM_APPLE
    CPPUNIT_TEST_SUITE_REGISTRATION(scene_rdl2::affinity::unittest::TestProcCpuAffinity);
#endif // end of !PLATFORM_APPLE
    CPPUNIT_TEST_SUITE_REGISTRATION(scene_rdl2::threadPoolExecutor::unittest::TestThreadPoolExecutor);
    CPPUNIT_TEST_SUITE_REGISTRATION(TestCommonUtil); // 65.3025 sec on cobaltcard @ Apr/28/2025

    return pdevunit::run(argc, argv);    
}
