// Copyright 2023-2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "TestAffinityMapTable.h"
#include "TestArg.h"
#include "TestBinPacketDictionary.h"
#include "TestCpuSocketUtil.h"
#include "TestFbUtils.h"
#include "TestParser.h"
#include "TestPixelBufferSha1.h"
#include "TestSha1.h"
#include "TestShmFb.h"
#include "TestShmAffInfo.h"
#include "TestVectorPacket.h"

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>
#include <scene_rdl2/pdevunit/pdevunit.h>

int
main(int ac, char** av)
{
    using namespace scene_rdl2::grid_util::unittest;

    CPPUNIT_TEST_SUITE_REGISTRATION(TestAffinityMapTable);
    CPPUNIT_TEST_SUITE_REGISTRATION(TestArg);
    CPPUNIT_TEST_SUITE_REGISTRATION(TestBinPacketDictionary);
    CPPUNIT_TEST_SUITE_REGISTRATION(TestCpuSocketUtil);
    CPPUNIT_TEST_SUITE_REGISTRATION(TestFbUtils);
    CPPUNIT_TEST_SUITE_REGISTRATION(TestParser);
    CPPUNIT_TEST_SUITE_REGISTRATION(TestPixelBufferSha1);
    CPPUNIT_TEST_SUITE_REGISTRATION(TestSha1);
    CPPUNIT_TEST_SUITE_REGISTRATION(TestShmAffInfo);
    CPPUNIT_TEST_SUITE_REGISTRATION(TestShmFb);
    CPPUNIT_TEST_SUITE_REGISTRATION(TestVectorPacket);
    
    return pdevunit::run(ac, av);
}
