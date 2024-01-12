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

class TestAttribute : public CppUnit::TestFixture
{
public:
    void setUp();
    void tearDown();

    /// Test that types which aren't blurrable throw an exception if
    /// constructed with the blurrable flag.
    void testConstructBlurrable();

    // Test construction of an attribute with a default value.
    void testConstructWithDefault();

    /// Test the name getter.
    void testGetName();

    /// Test the type getter.
    void testGetType();

    /// Test the flags getter.
    void testGetFlags();

    /// Test the default value getter.
    void testGetDefaultValue();

    /// Test that the index is set correctly.
    void testIndex();

    /// Test that the offset is set correctly.
    void testOffset();

    /// Test that the isBindable() function works.
    void testIsBindable();

    /// Test that the isBlurrable() function works.
    void testIsBlurrable();

    /// Test that the isEnumerable() function works.
    void testIsEnumerable();

    /// Test that the isFilename() function works.
    void testIsFilename();

    /// Test that metadata can be set without throwing.
    void testSetMetadata();

    /// Test that metadata can be retrieved.
    void testGetMetadata();

    /// Test that we can search for metadata key existence.
    void testMetadataExists();

    /// Test that metadata iteration works.
    void testIterateMetadata();

    /// Test that enum values can be set without throwing.
    void testSetEnumValue();

    /// Test that enum value descriptions can be retrieved.
    void testGetEnumDescription();

    /// Test that we can verify the validity of enum values.
    void testIsValidEnumValue();

    /// Test that we can iterate over enum values properly.
    void testIterateEnumValues();

    /// Test that we can convert from the C++ static type system to the correct
    /// runtime type.
    void testAttributeType();

    CPPUNIT_TEST_SUITE(TestAttribute);
    CPPUNIT_TEST(testConstructBlurrable);
    CPPUNIT_TEST(testConstructWithDefault);
    CPPUNIT_TEST(testGetName);
    CPPUNIT_TEST(testGetType);
    CPPUNIT_TEST(testGetFlags);
    CPPUNIT_TEST(testGetDefaultValue);
    CPPUNIT_TEST(testIndex);
    CPPUNIT_TEST(testOffset);
    CPPUNIT_TEST(testIsBindable);
    CPPUNIT_TEST(testIsBlurrable);
    CPPUNIT_TEST(testIsEnumerable);
    CPPUNIT_TEST(testIsFilename);
    CPPUNIT_TEST(testSetMetadata);
    CPPUNIT_TEST(testGetMetadata);
    CPPUNIT_TEST(testMetadataExists);
    CPPUNIT_TEST(testIterateMetadata);
    CPPUNIT_TEST(testSetEnumValue);
    CPPUNIT_TEST(testGetEnumDescription);
    CPPUNIT_TEST(testIsValidEnumValue);
    CPPUNIT_TEST(testIterateEnumValues);
    CPPUNIT_TEST(testAttributeType);
    CPPUNIT_TEST_SUITE_END();

private:
    std::unique_ptr<Attribute> mConstant;
    std::unique_ptr<Attribute> mBindable;
    std::unique_ptr<Attribute> mBlurrable;
    std::unique_ptr<Attribute> mBoth;
    std::unique_ptr<Attribute> mEnumerable;
    std::unique_ptr<Attribute> mFilename;
};

} // namespace unittest
} // namespace rdl2
} // namespace scene_rdl2

