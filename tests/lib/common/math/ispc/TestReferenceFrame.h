// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file TestReferenceFrame.h
#pragma once

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

namespace scene_rdl2 {
namespace common {
namespace math {
namespace ispc {
namespace unittest {

class TestReferenceFrame : public CppUnit::TestFixture
{
public:
    void ctor();
    void getN();
    void xform();

    CPPUNIT_TEST_SUITE(TestReferenceFrame);
    CPPUNIT_TEST(ctor);
    CPPUNIT_TEST(getN);
    CPPUNIT_TEST(xform);
    CPPUNIT_TEST_SUITE_END();
};

} // namespace scene_rdl2
} // namespace common
} // namespace math
} // namespace ispc
} // namespace unittest

