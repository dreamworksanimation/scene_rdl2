// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

///
/// @file TestArray2D.h
/// $Id$
///

#pragma once

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>

namespace scene_rdl2 {
namespace pbr {


//----------------------------------------------------------------------------

///
/// @class TestArray2D TestArray2D.h <pbr/TestArray2D.h>
/// @brief This class tests the functionality of Array2D.
/// 
class TestArray2D : public CppUnit::TestFixture
{
public:
    CPPUNIT_TEST_SUITE(TestArray2D);
    CPPUNIT_TEST(testStatic);
    CPPUNIT_TEST(testConstruction);
    CPPUNIT_TEST(testRandomInput);
    CPPUNIT_TEST(testIteratorConstructionC);
    CPPUNIT_TEST(testIteratorConstruction);
    CPPUNIT_TEST(testIteratorValueC);
    CPPUNIT_TEST(testIteratorValue);
    CPPUNIT_TEST(testCopy);
    CPPUNIT_TEST(testMove);
    CPPUNIT_TEST(testExceptions);
    CPPUNIT_TEST_SUITE_END();

    void testStatic();
    void testConstruction();
    void testRandomInput();
    void testIteratorConstructionC();
    void testIteratorConstruction();
    void testIteratorValueC();
    void testIteratorValue();
    void testCopy();
    void testMove();
    void testExceptions();
};


//----------------------------------------------------------------------------

} // namespace pbr
} // namespace scene_rdl2


