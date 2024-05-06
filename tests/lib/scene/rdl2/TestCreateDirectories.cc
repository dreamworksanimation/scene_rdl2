// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "TestCreateDirectories.h"

#include <scene_rdl2/render/util/Files.h>
#include <sys/stat.h>

namespace scene_rdl2 {
namespace rdl2 {
namespace unittest {

void
TestCreateDirectories::setUp()
{
}

void
TestCreateDirectories::tearDown()
{
}

void
TestCreateDirectories::testCreateDirectories()
{
    const std::string testPath = "test_directory_1/test_subdirectory_1";
    const std::string testFile = testPath + "/some_file";
    CPPUNIT_ASSERT(util::createDirectories(testFile) == true);

    // Check if the directory exists
    #if defined(_WIN32)
        CPPUNIT_ASSERT(_mkdir(testPath.c_str()) != 0 && errno == EEXIST);
    #else
        CPPUNIT_ASSERT(mkdir(testPath.c_str(), 0777) != 0 && errno == EEXIST);
    #endif
}

} // namespace unittest
} // namespace rdl2
} // namespace scene_rdl2

