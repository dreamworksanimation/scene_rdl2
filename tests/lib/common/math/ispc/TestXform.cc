// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file TestXform.cc

#include "TestXform.h"

#include "TestXform_ispc_stubs.h"

using namespace scene_rdl2;
using scene_rdl2::common::math::ispc::unittest::TestXform;

void
TestXform::testCreate()
{
    CPPUNIT_ASSERT(::ispc::Test_Xform_ctor() == 0);
}

void
TestXform::testInverse()
{
    CPPUNIT_ASSERT(::ispc::Test_Xform_inverse() == 0);
}

void
TestXform::testRow()
{
    CPPUNIT_ASSERT(::ispc::Test_Xform_row() == 0);
}

void
TestXform::testIdentity()
{
    CPPUNIT_ASSERT(::ispc::Test_Xform_identity() == 0);
}

void
TestXform::testTranslation()
{
    CPPUNIT_ASSERT(::ispc::Test_Xform_translation() == 0);
}

void
TestXform::testRotation()
{
    CPPUNIT_ASSERT(::ispc::Test_Xform_rotation() == 0);
}

void
TestXform::testScale()
{
    CPPUNIT_ASSERT(::ispc::Test_Xform_scale() == 0);
}

void
TestXform::testLookAtPoint()
{
    CPPUNIT_ASSERT(::ispc::Test_Xform_lookAtPoint() == 0);
}

void
TestXform::testAdd()
{
    CPPUNIT_ASSERT(::ispc::Test_Xform_add() == 0);
}

void
TestXform::testMinus()
{
    CPPUNIT_ASSERT(::ispc::Test_Xform_minus() == 0);
}

void
TestXform::testSMultXform()
{
    CPPUNIT_ASSERT(::ispc::Test_Xform_sMultXform() == 0);
}

void
TestXform::testXformMultXform()
{
    CPPUNIT_ASSERT(::ispc::Test_Xform_xformMultXform() == 0);
}

void
TestXform::testTransformPoint()
{
    CPPUNIT_ASSERT(::ispc::Test_Xform_transformPoint() == 0);
}

void
TestXform::testTransformVector()
{
    CPPUNIT_ASSERT(::ispc::Test_Xform_transformVector() == 0);
}

void
TestXform::testTransformNormal()
{
    CPPUNIT_ASSERT(::ispc::Test_Xform_transformNormal() == 0);
}

void
TestXform::testIsEqual()
{
    CPPUNIT_ASSERT(::ispc::Test_Xform_isEqual() == 0);
}

void
TestXform::testIsEqualFixedEps()
{
    CPPUNIT_ASSERT(::ispc::Test_Xform_isEqualFixedEps() == 0);
}

void
TestXform::testSlerp()
{
    CPPUNIT_ASSERT(::ispc::Test_Xform_slerp() == 0);
}

