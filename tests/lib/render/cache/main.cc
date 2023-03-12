// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "TestCacheUtil.h"

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <scene_rdl2/pdevunit/pdevunit.h>

int
main(int ac, char **av)
{
    using namespace scene_rdl2::cache::unittest;

    CPPUNIT_TEST_SUITE_REGISTRATION(TestCacheUtil);

    return pdevunit::run(ac, av);
}

