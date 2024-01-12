// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "TestPixelBuffer.h"
#include "TestRunningStats.h"
#include "TestSnapshotUtil.h"

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <scene_rdl2/pdevunit/pdevunit.h>

int
main(int argc, char* argv[])
{
    using namespace scene_rdl2::fb_util::unittest;

    CPPUNIT_TEST_SUITE_REGISTRATION(TestPixelBuffer);
    CPPUNIT_TEST_SUITE_REGISTRATION(TestRunningStats);
    CPPUNIT_TEST_SUITE_REGISTRATION(TestSnapshotUtil);

    return pdevunit::run(argc, argv);
}
