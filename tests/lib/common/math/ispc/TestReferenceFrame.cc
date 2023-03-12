// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file TestReferenceFrame.cc

#include "TestReferenceFrame.h"

#include "TestReferenceFrame_ispc_stubs.h"

using namespace scene_rdl2;
using scene_rdl2::common::math::ispc::unittest::TestReferenceFrame;

void
TestReferenceFrame::ctor()
{
    CPPUNIT_ASSERT(::ispc::Test_ReferenceFrame_ctor() == 0);
}

void
TestReferenceFrame::getN()
{
    CPPUNIT_ASSERT(::ispc::Test_ReferenceFrame_getN() == 0);
}

void
TestReferenceFrame::xform()
{
    CPPUNIT_ASSERT(::ispc::Test_ReferenceFrame_xform() == 0);
}

