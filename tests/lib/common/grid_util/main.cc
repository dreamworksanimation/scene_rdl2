// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "TestArg.h"
#include "TestParser.h"
#include "TestSha1.h"

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <scene_rdl2/pdevunit/pdevunit.h>

int
main(int ac, char **av)
{
    using namespace scene_rdl2::grid_util::unittest;

    CPPUNIT_TEST_SUITE_REGISTRATION(TestArg);
    CPPUNIT_TEST_SUITE_REGISTRATION(TestParser);
    CPPUNIT_TEST_SUITE_REGISTRATION(TestSha1);

    return pdevunit::run(ac, av);
}

