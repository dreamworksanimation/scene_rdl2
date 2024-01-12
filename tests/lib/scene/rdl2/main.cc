// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "TestAscii.h"
#include "TestAttribute.h"
#include "TestAttributeKey.h"
#include "TestBinary.h"
#include "TestDso.h"
#include "TestDsoFinder.h"
#include "TestJoint.h"
#include "TestLayer.h"
#include "TestProxies.h"
#include "TestRenderOutput.h"
#include "TestSceneClass.h"
#include "TestSceneContext.h"
#include "TestSceneObject.h"
#include "TestSets.h"
#include "TestSplit.h"
#include "TestTraceSet.h"
#include "TestTypes.h"
#include "TestUserData.h"
#include "TestValueContainer.h"

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <scene_rdl2/pdevunit/pdevunit.h>

int
main(int argc, char* argv[])
{
    using namespace scene_rdl2::rdl2::unittest;

    CPPUNIT_TEST_SUITE_REGISTRATION(TestAttribute);
    CPPUNIT_TEST_SUITE_REGISTRATION(TestAttributeKey);
    CPPUNIT_TEST_SUITE_REGISTRATION(TestDso);
    CPPUNIT_TEST_SUITE_REGISTRATION(TestDsoFinder);
    CPPUNIT_TEST_SUITE_REGISTRATION(TestSceneClass);
    CPPUNIT_TEST_SUITE_REGISTRATION(TestSceneObject);
    CPPUNIT_TEST_SUITE_REGISTRATION(TestSceneContext);
    CPPUNIT_TEST_SUITE_REGISTRATION(TestAscii);
    CPPUNIT_TEST_SUITE_REGISTRATION(TestBinary);
    CPPUNIT_TEST_SUITE_REGISTRATION(TestSplit);
    CPPUNIT_TEST_SUITE_REGISTRATION(TestSets);
    CPPUNIT_TEST_SUITE_REGISTRATION(TestJoint);
    CPPUNIT_TEST_SUITE_REGISTRATION(TestLayer);
    CPPUNIT_TEST_SUITE_REGISTRATION(TestProxies);
    CPPUNIT_TEST_SUITE_REGISTRATION(TestTraceSet);
    CPPUNIT_TEST_SUITE_REGISTRATION(TestTypes);
    CPPUNIT_TEST_SUITE_REGISTRATION(TestRenderOutput);
    CPPUNIT_TEST_SUITE_REGISTRATION(TestUserData);
    CPPUNIT_TEST_SUITE_REGISTRATION(TestValueContainer);

    return pdevunit::run(argc, argv);
}

