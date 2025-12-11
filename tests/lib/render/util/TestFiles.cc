// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

///
/// @file TestFiles.cc
/// $Id$
///

#include "TestFiles.h"
#include "TimeOutput.h"

#include <scene_rdl2/common/except/exceptions.h>
#include <scene_rdl2/render/util/Files.h>

#include <filesystem>
#include <fstream>
#include <system_error>
#include <thread>

namespace scene_rdl2 {
namespace util {
namespace unittest {

void
TestFiles::setUp()
{
    // Create a unique temporary directory for testing
    std::error_code ec;
    auto tempPath = std::filesystem::temp_directory_path(ec);
    CPPUNIT_ASSERT(!ec);

    // Create a unique subdirectory
    tempPath /= "test_files_" + std::to_string(std::hash<std::thread::id>{}(std::this_thread::get_id()));
    std::filesystem::create_directories(tempPath, ec);
    CPPUNIT_ASSERT(!ec);

    mTempDir = tempPath.string();
}

void
TestFiles::tearDown()
{
    // Clean up temporary directory and files
    if (!mTempDir.empty()) {
        std::error_code ec;
        std::filesystem::remove_all(mTempDir, ec);
        // Ignore errors during cleanup
    }
}

void
TestFiles::testSplitPath()
{
    TIME_START;

    // Test basic path splitting
    {
        auto result = splitPath("/some/file/path.txt");
        CPPUNIT_ASSERT_EQUAL(std::string("/some/file"), result.first);
        CPPUNIT_ASSERT_EQUAL(std::string("path.txt"), result.second);
    }

    // Test path with trailing slash
    {
        auto result = splitPath("/some/directory/");
        CPPUNIT_ASSERT_EQUAL(std::string("/some/directory"), result.first);
        CPPUNIT_ASSERT_EQUAL(std::string(""), result.second);
    }

    // Test root path
    {
        auto result = splitPath("/file.txt");
        CPPUNIT_ASSERT_EQUAL(std::string("/"), result.first);
        CPPUNIT_ASSERT_EQUAL(std::string("file.txt"), result.second);
    }

    // Test relative path
    {
        auto result = splitPath("dir/subdir/file.txt");
        CPPUNIT_ASSERT_EQUAL(std::string("dir/subdir"), result.first);
        CPPUNIT_ASSERT_EQUAL(std::string("file.txt"), result.second);
    }

    // Test current directory prefix
    {
        auto result = splitPath("./file.txt");
        CPPUNIT_ASSERT_EQUAL(std::string("."), result.first);
        CPPUNIT_ASSERT_EQUAL(std::string("file.txt"), result.second);
    }

    // Test bare filename (no directory)
    {
        auto result = splitPath("file.txt");
        CPPUNIT_ASSERT_EQUAL(std::string("."), result.first);
        CPPUNIT_ASSERT_EQUAL(std::string("file.txt"), result.second);
    }

    TIME_END;
}

void
TestFiles::testLowerCaseExtension()
{
    TIME_START;

    // Test uppercase extension
    CPPUNIT_ASSERT_EQUAL(std::string("txt"), lowerCaseExtension("some/file/path.TXT"));

    // Test mixed case extension
    CPPUNIT_ASSERT_EQUAL(std::string("jpeg"), lowerCaseExtension("image.JpEg"));

    // Test lowercase extension
    CPPUNIT_ASSERT_EQUAL(std::string("cc"), lowerCaseExtension("source.cc"));

    // Test no extension
    CPPUNIT_ASSERT_EQUAL(std::string(""), lowerCaseExtension("filename"));

    // Test trailing dot
    CPPUNIT_ASSERT_EQUAL(std::string(""), lowerCaseExtension("filename."));

    // Test multiple dots
    CPPUNIT_ASSERT_EQUAL(std::string("gz"), lowerCaseExtension("archive.tar.GZ"));

    // Test hidden file
    CPPUNIT_ASSERT_EQUAL(std::string("conf"), lowerCaseExtension(".vimrc.CONF"));

    TIME_END;
}

void
TestFiles::testFileExists()
{
    TIME_START;

    // Create a test file
    std::string testFile = mTempDir + "/test_file.txt";
    std::ofstream ofs(testFile);
    ofs << "test content";
    ofs.close();

    // Test existing file
    CPPUNIT_ASSERT(fileExists(testFile));

    // Test non-existing file
    CPPUNIT_ASSERT(!fileExists(mTempDir + "/nonexistent_file.txt"));

    // Test directory existence (directories are also files)
    CPPUNIT_ASSERT(fileExists(mTempDir));

    TIME_END;
}

void
TestFiles::testWriteTest()
{
    TIME_START;

    // Test writing to an existing directory without creating directories
    {
        std::string testPath = mTempDir + "/write_test.txt";
        CPPUNIT_ASSERT(writeTest(testPath, false));
    }

    // Test writing to a non-existing directory without creating directories
    {
        std::string testPath = mTempDir + "/nonexistent/write_test.txt";
        CPPUNIT_ASSERT(!writeTest(testPath, false));
    }

    // Test writing to a non-existing directory with creating directories
    {
        std::string testPath = mTempDir + "/new_dir/subdir/write_test.txt";
        CPPUNIT_ASSERT(writeTest(testPath, true));
        // Verify the directories were created
        CPPUNIT_ASSERT(fileExists(mTempDir + "/new_dir"));
        CPPUNIT_ASSERT(fileExists(mTempDir + "/new_dir/subdir"));
    }

    // Test writing to an existing file
    {
        std::string testFile = mTempDir + "/existing_file.txt";
        std::ofstream ofs(testFile);
        ofs << "content";
        ofs.close();
        CPPUNIT_ASSERT(writeTest(testFile, false));
    }

    TIME_END;
}

void
TestFiles::testFindFile()
{
    TIME_START;

    // Create test files in different directories
    std::string dir1 = mTempDir + "/dir1";
    std::string dir2 = mTempDir + "/dir2";
    std::string dir3 = mTempDir + "/dir3";

    std::filesystem::create_directory(dir1);
    std::filesystem::create_directory(dir2);
    std::filesystem::create_directory(dir3);

    std::string file1 = dir2 + "/test.txt";
    std::ofstream ofs1(file1);
    ofs1 << "content";
    ofs1.close();

    // Test finding file in search path
    {
        std::string searchPath = dir1 + ":" + dir2 + ":" + dir3;
        std::string result = findFile("test.txt", searchPath);
        CPPUNIT_ASSERT_EQUAL(file1, result);
    }

    // Test file not found
    {
        std::string searchPath = dir1 + ":" + dir3;
        std::string result = findFile("test.txt", searchPath);
        CPPUNIT_ASSERT_EQUAL(std::string(""), result);
    }

    // Test with single directory in path
    {
        std::string result = findFile("test.txt", dir2);
        CPPUNIT_ASSERT_EQUAL(file1, result);
    }

    TIME_END;
}

void
TestFiles::testCopyFile()
{
    TIME_START;

    // Create source file with content
    std::string srcFile = mTempDir + "/source.txt";
    std::string content = "This is test content for file copying.\n";
    std::ofstream ofs(srcFile);
    ofs << content;
    ofs.close();

    // Test successful copy
    {
        std::string dstFile = mTempDir + "/destination.txt";
        copyFile(srcFile, dstFile);

        // Verify file exists and has same content
        CPPUNIT_ASSERT(fileExists(dstFile));

        std::ifstream ifs(dstFile);
        std::string copiedContent((std::istreambuf_iterator<char>(ifs)),
                                   std::istreambuf_iterator<char>());
        CPPUNIT_ASSERT_EQUAL(content, copiedContent);
    }

    // Test exception when source doesn't exist
    {
        bool exceptionThrown = false;
        try {
            copyFile(mTempDir + "/nonexistent.txt", mTempDir + "/dest.txt");
        } catch (const except::IoError&) {
            exceptionThrown = true;
        }
        CPPUNIT_ASSERT(exceptionThrown);
    }

    // Test exception when destination path is invalid
    {
        bool exceptionThrown = false;
        try {
            copyFile(srcFile, "/invalid/path/that/does/not/exist/dest.txt");
        } catch (const except::IoError&) {
            exceptionThrown = true;
        }
        CPPUNIT_ASSERT(exceptionThrown);
    }

    TIME_END;
}

void
TestFiles::testIsAbsolute()
{
    TIME_START;

    // Test absolute paths
    CPPUNIT_ASSERT(isAbsolute("/"));
    CPPUNIT_ASSERT(isAbsolute("/usr/bin"));
    CPPUNIT_ASSERT(isAbsolute("/home/user/file.txt"));

    // Test relative paths
    CPPUNIT_ASSERT(!isAbsolute("relative/path"));
    CPPUNIT_ASSERT(!isAbsolute("file.txt"));
    CPPUNIT_ASSERT(!isAbsolute("./file.txt"));
    CPPUNIT_ASSERT(!isAbsolute("../parent"));

    // Test empty string
    CPPUNIT_ASSERT(!isAbsolute(""));

    TIME_END;
}

void
TestFiles::testAbsolutePath()
{
    TIME_START;

    // Test already absolute path
    {
        std::string absPath = "/usr/bin/test";
        CPPUNIT_ASSERT_EQUAL(absPath, absolutePath(absPath));
    }

    // Test relative path with explicit base
    {
        std::string result = absolutePath("subdir/file.txt", "/base/path");
        CPPUNIT_ASSERT_EQUAL(std::string("/base/path/subdir/file.txt"), result);
    }

    // Test relative path without base (uses current working directory)
    {
        std::string result = absolutePath("file.txt");
        CPPUNIT_ASSERT(isAbsolute(result));
        CPPUNIT_ASSERT(result.find("file.txt") != std::string::npos);
    }

    // Test with empty relative path hint
    {
        std::string result = absolutePath("test.txt", "");
        CPPUNIT_ASSERT(isAbsolute(result));
    }

    TIME_END;
}

void
TestFiles::testCurrentWorkingDirectory()
{
    TIME_START;

    std::string cwd = currentWorkingDirectory();

    // Should return an absolute path
    CPPUNIT_ASSERT(isAbsolute(cwd));

    // Should not be empty
    CPPUNIT_ASSERT(!cwd.empty());

    // Verify it matches what std::filesystem would return
    std::error_code ec;
    auto fsCwd = std::filesystem::current_path(ec);
    CPPUNIT_ASSERT(!ec);
    CPPUNIT_ASSERT_EQUAL(fsCwd.string(), cwd);

    TIME_END;
}

void
TestFiles::testSimplifyPath()
{
    TIME_START;

    // Test basic simplification
    {
        std::string result = simplifyPath("a/./b");
        CPPUNIT_ASSERT_EQUAL(std::string("a/b"), result);
    }

    // Test parent directory references
    {
        std::string result = simplifyPath("a/b/../c");
        CPPUNIT_ASSERT_EQUAL(std::string("a/c"), result);
    }

    // Test multiple dots
    {
        std::string result = simplifyPath("a/./b/./c");
        CPPUNIT_ASSERT_EQUAL(std::string("a/b/c"), result);
    }

    // Test complex path
    {
        std::string result = simplifyPath("a/b/c/../../d");
        CPPUNIT_ASSERT_EQUAL(std::string("a/d"), result);
    }

    // Test path with no simplification needed
    {
        std::string result = simplifyPath("a/b/c");
        CPPUNIT_ASSERT_EQUAL(std::string("a/b/c"), result);
    }

    // Test multiple consecutive parent references
    {
        std::string result = simplifyPath("a/b/c/../../d/e");
        CPPUNIT_ASSERT_EQUAL(std::string("a/d/e"), result);
    }

    TIME_END;
}

void
TestFiles::testCreateDirectories()
{
    TIME_START;

    // Test creating nested directories from file path
    {
        std::string filePath = mTempDir + "/level1/level2/level3/file.txt";
        CPPUNIT_ASSERT(createDirectories(filePath));

        // Verify directories were created (but not the file)
        CPPUNIT_ASSERT(fileExists(mTempDir + "/level1"));
        CPPUNIT_ASSERT(fileExists(mTempDir + "/level1/level2"));
        CPPUNIT_ASSERT(fileExists(mTempDir + "/level1/level2/level3"));
        CPPUNIT_ASSERT(!fileExists(filePath)); // file itself should not exist
    }

    // Test creating directories when some already exist
    {
        std::string existingPath = mTempDir + "/existing";
        std::filesystem::create_directory(existingPath);

        std::string filePath = mTempDir + "/existing/new_subdir/file.txt";
        CPPUNIT_ASSERT(createDirectories(filePath));
        CPPUNIT_ASSERT(fileExists(mTempDir + "/existing/new_subdir"));
    }

    // Test with single level
    {
        std::string filePath = mTempDir + "/single/file.txt";
        CPPUNIT_ASSERT(createDirectories(filePath));
        CPPUNIT_ASSERT(fileExists(mTempDir + "/single"));
    }

    TIME_END;
}

} // namespace unittest
} // namespace util
} // namespace scene_rdl2
