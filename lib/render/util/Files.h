// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <string>
#include <utility>
#include <unistd.h>

namespace scene_rdl2 {
namespace util {

/**
 * RAII guard that wraps a file descriptor. The descriptor will always be
 * closed, no matter how the scope is exited.
 *
 * Example:
 *      {
 *          FileDescriptorGuard guard(open(...));
 *          write(guard.fd, ...);
 *      } // close(guard.fd) is automatically called
 */
struct FileDescriptorGuard
{
    FileDescriptorGuard(int descriptor) :
        fd(descriptor)
    {
    }

    ~FileDescriptorGuard()
    {
        close(fd);
    }

    const int fd;
};

/**
 * Splits a file path string into its basename and dirname components. Returns
 * a pair<string, string> where the first element is the dirname and the second
 * element is the basename.
 * 
 * This is a bit tricky since basename() and dirname() may or may not modify
 * their arguments and may or may not return pointers to statically allocated
 * space.
 *
 * Example:
 *      auto components = splitPath("some/file/path.txt");
 *      assert(components.first == "some/file");
 *      assert(components.second == "path.txt");
 */
std::pair<std::string, std::string> splitPath(const std::string& filePath);

/**
 * Extracts the file extension from the file path (if it has one), converts
 * it to lower case, and returns it. Returns an empty string if the file name
 * has no extension.
 *
 * Example:
 *      auto ext = lowerCaseExtension("some/file/path.TXT");
 *      assert(ext == "txt");
 */
std::string lowerCaseExtension(const std::string& filePath);

/**
 * Returns true if the file exists and false otherwise.
 */
bool fileExists(const std::string& filePath);

/**
 * Returns true if it's possible to create a file with the specified path.
 *
 * If the parent directories that would contain the file do not exist, the
 * createDirectories argument can be used to create them.
 *
 * Example:
 *      if (writeTest("/some/path/to/file.txt", true)) {
 *          // Directories were created and we can write the file.
 *          writeFile("/some/path/to/file.txt");
 *      }
 */
bool writeTest(const std::string& filePath, bool createDirectories);

/**
 * Locates a file with the given file name in the given (colon separated)
 * search path, returning the full path to the file.
 *
 * This is done by checking successive elements of the search path and
 * returning the first full path which matches an existing file.
 *
 * Full path means <part of the search path that matched> + '/' + name.
 *
 * If the search path is exhausted and the file still cannot be found, an
 * empty string is returned.
 *
 * Example:
 *      auto passwd = findFile("passwd", ".:..:/etc");
 *      // passwd is "/etc/passwd"
 */
std::string findFile(const std::string& name, const std::string& searchPath);

/**
 * Copies the file with the given source path to the given destination path.
 * The file copy is done entirely within the kernel, so it's more efficient
 * than copying the file through userspace buffers.
 *
 * Throws an except::IoError if any of the following occur:
 *  1) The source file cannot be opened for reading.
 *  2) The destination file cannot be created and opened for writing.
 *  3) The source file cannot be stat()'d to discover its size.
 *
 *  Example:
 *      copyFile("/etc/passwd", "/home/bsomers/passwd");
 */
void copyFile(const std::string& src, const std::string& dst);

/**
 * Returns true if the path is absolute on *nix.
 */
inline bool isAbsolute(const std::string& filePath)
{
    return !filePath.empty() && filePath[0] == '/';
}

/**
 * Takes an absolute or relative path to a file and returns the absolute path.
 * The relativeToPath argument is a hint as to the path the file may be
 * relative to. If relativeToPath is the empty string, the current working
 * directory is assumed to be the hint.
 */
std::string absolutePath(const std::string& filePath, std::string relativeToPath = "");

/**
 * Returns the current working directory as a std::string.
 */
std::string currentWorkingDirectory();

/**
 * Returns the given path with the effects of "." and ".." elements collapsed.
 *
 * Paths with a leading ".." or "/.." are illegal and the result is undefined.
 */
std::string simplifyPath(const std::string& path);


/**
 * Recursively creates subdirectories from file path if they don't exist.
 * Assumes the last part of the path is a filename and does not create
 * a directory for it
 */
bool createDirectories(const std::string& path);

} // namespace util
} // namespace scene_rdl2

