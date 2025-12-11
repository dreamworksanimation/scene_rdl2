// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

///
/// @file TestFiles.h
/// $Id$
///

#pragma once

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>

namespace scene_rdl2 {
namespace util {
namespace unittest {

//----------------------------------------------------------------------------

///
/// @class TestFiles TestFiles.h <util/TestFiles.h>
/// @brief This class tests the functionality of Files utility functions.
/// 
class TestFiles : public CppUnit::TestFixture
{
public:
    void setUp() override;
    void tearDown() override;

    CPPUNIT_TEST_SUITE(TestFiles);
    CPPUNIT_TEST(testSplitPath);
    CPPUNIT_TEST(testLowerCaseExtension);
    CPPUNIT_TEST(testFileExists);
    CPPUNIT_TEST(testWriteTest);
    CPPUNIT_TEST(testFindFile);
    CPPUNIT_TEST(testCopyFile);
    CPPUNIT_TEST(testIsAbsolute);
    CPPUNIT_TEST(testAbsolutePath);
    CPPUNIT_TEST(testCurrentWorkingDirectory);
    CPPUNIT_TEST(testSimplifyPath);
    CPPUNIT_TEST(testCreateDirectories);
    CPPUNIT_TEST_SUITE_END();

    void testSplitPath();
    void testLowerCaseExtension();
    void testFileExists();
    void testWriteTest();
    void testFindFile();
    void testCopyFile();
    void testIsAbsolute();
    void testAbsolutePath();
    void testCurrentWorkingDirectory();
    void testSimplifyPath();
    void testCreateDirectories();

private:
    std::string mTempDir;
};

} // namespace unittest
} // namespace util
} // namespace scene_rdl2
