// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "test_math.h"

#include <scene_rdl2/pdevunit/pdevunit.h>

#include "test_math.h"
#include "test_math_Color.h"
#include "TestColorSpace.h"
#include "test_math_Mat3.h"
#include "test_math_Mat4.h"
#include "test_math_ReferenceFrame.h"
#include "test_math_Quaternion.h"
#include "test_math_Vec4.h"
#include "test_math_Xform.h"
#include "TestViewport.h"

int
main(int argc, char *argv[])
{
    CPPUNIT_TEST_SUITE_REGISTRATION(TestCommonMath);
    CPPUNIT_TEST_SUITE_REGISTRATION(TestCommonMathMat3);
    CPPUNIT_TEST_SUITE_REGISTRATION(TestCommonMathMat4);
    CPPUNIT_TEST_SUITE_REGISTRATION(TestCommonMathReferenceFrame);
    CPPUNIT_TEST_SUITE_REGISTRATION(TestCommonMathQuaternion);
    CPPUNIT_TEST_SUITE_REGISTRATION(TestCommonMathVec4);
    CPPUNIT_TEST_SUITE_REGISTRATION(TestCommonMathXform);
    CPPUNIT_TEST_SUITE_REGISTRATION(TestViewport);
    CPPUNIT_TEST_SUITE_REGISTRATION(TestCommonMathColor);
    CPPUNIT_TEST_SUITE_REGISTRATION(TestCommonColorSpace);
    return pdevunit::run(argc, argv);    
}


