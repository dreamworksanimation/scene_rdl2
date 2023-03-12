// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

///
/// @file TestAtomicFloat.h
/// $Id$
///

#pragma once

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>

namespace scene_rdl2 {
namespace pbr {


//----------------------------------------------------------------------------

///
/// @class TestAtomicFloat TestAtomicFloat.h <pbr/TestAtomicFloat.h>
/// @brief This class tests the functionality of our specialization of
/// std::atomic<*FLOATING POINT TYPE*>
/// 
class TestAtomicFloat : public CppUnit::TestFixture
{
public:
    CPPUNIT_TEST_SUITE(TestAtomicFloat);
    CPPUNIT_TEST(testAtomicFloat);
    CPPUNIT_TEST_SUITE_END();

    void testAtomicFloat();
};


//----------------------------------------------------------------------------

} // namespace pbr
} // namespace scene_rdl2


