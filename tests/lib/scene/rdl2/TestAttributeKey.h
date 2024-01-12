// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <scene_rdl2/scene/rdl2/rdl2.h>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <memory>

namespace scene_rdl2 {
namespace rdl2 {
namespace unittest {

class TestAttributeKey : public CppUnit::TestFixture
{
public:
    void setUp();
    void tearDown();

    /// Test that AttributeKeys have their index set properly.
    void testIndex();

    /// Test that AttributeKeys have their offset set properly.
    void testOffset();

    /// Test that AttributeKeys have their flags set properly.
    void testFlags();

    /// Test that AttributeKeys have their object type set properly.
    void testObjectType();

    /// Test that AttributeKeys can be compared for equality and inequality.
    void testEquality();

    /// Test that we correctly type check AttributeKeys at construction time.
    void testTypes();

    /// Test that we can correctly identify valid and invalid AttributeKeys.
    void testIsValid();

    /// Test that we can correctly identify bindable AttributeKeys.
    void testIsBindable();

    /// Test that we can correctly identify blurrable AttributeKeys.
    void testIsBlurrable();

    /// Test that we can correctly identify enumerable AttributeKeys.
    void testIsEnumerable();

    /// Test that we can correctly identify filename AttributeKeys.
    void testIsFilename();

    CPPUNIT_TEST_SUITE(TestAttributeKey);
    CPPUNIT_TEST(testIndex);
    CPPUNIT_TEST(testOffset);
    CPPUNIT_TEST(testFlags);
    CPPUNIT_TEST(testObjectType);
    CPPUNIT_TEST(testEquality);
    CPPUNIT_TEST(testTypes);
    CPPUNIT_TEST(testIsValid);
    CPPUNIT_TEST(testIsBindable);
    CPPUNIT_TEST(testIsBlurrable);
    CPPUNIT_TEST(testIsEnumerable);
    CPPUNIT_TEST(testIsFilename);
    CPPUNIT_TEST_SUITE_END();

private:
    std::unique_ptr<Attribute> mAttribute;
    std::unique_ptr<AttributeKey<scene_rdl2::rdl2::Float> > mKey;
};

} // namespace unittest
} // namespace rdl2
} // namespace scene_rdl2

