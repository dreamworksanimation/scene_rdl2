// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "Files.h"

#include "Strings.h"

#include <scene_rdl2/common/platform/Platform.h>
#include <scene_rdl2/common/except/exceptions.h>

#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <cstring>
#include <filesystem>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <iostream>

// TODO: are all of these actually still necessary
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#if defined(__APPLE__)
     #include <sys/types.h>
     #include <sys/socket.h>
     #include <sys/uio.h>
#else
#include <sys/sendfile.h>
#endif
#include <unistd.h>

namespace scene_rdl2 {
namespace util {

namespace {

struct FreeDeleter
{
    void operator()(void* ptr) { free(ptr); }
};

} // namespace

std::pair<std::string, std::string>
splitPath(const std::string& filePath)
{
    std::filesystem::path p(filePath);
    std::string directory(p.parent_path().string());
    std::string filename(p.filename().string());

    // Handle special case of filename only with no directory.
    if (directory.empty()) {
        directory = ".";
    }

    return std::make_pair(std::move(directory), std::move(filename));
}

std::string
lowerCaseExtension(const std::string& filePath)
{
    const auto split = splitPath(filePath);
    const std::string& fileName = split.second;

    auto pos = fileName.rfind('.');
    if (pos == std::string::npos || pos == fileName.size() - 1) {
        return "";
    }

    auto ext = fileName.substr(pos + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext;
}

bool
fileExists(const std::string& filePath)
{
    if (access(filePath.c_str(), F_OK) == 0) {
        return true;
    } else {
        return false;
    }
}

bool
writeTest(const std::string& filePath, bool createDirectories)
{
    // If the file exists, we're done.
    if (access(filePath.c_str(), W_OK) == 0) {
        return true;
    }

    // Try to create the file.
    int fd = creat(filePath.c_str(), 0666);
    if (fd != -1) {
        // Success. Close and remove it.
        close(fd);
        unlink(filePath.c_str());
        return true;
    }

    // The only recoverable error is ENOENT, which indicates that we need
    // to construct the parent directories.
    if (errno != ENOENT || !createDirectories) {
        return false;
    }

    for (std::size_t separatorPos = 0; separatorPos != std::string::npos;
            separatorPos = filePath.find('/', separatorPos + 1)) {
        auto leadingPath(filePath.substr(0, separatorPos));
        if (!leadingPath.empty() && access(leadingPath.c_str(), F_OK) != 0) {
            // If the problem isn't ENOENT, there's nothing we can do.
            if (errno != ENOENT) {
                return false;
            }

            // Does not exist, try to create directory.
            if (mkdir(leadingPath.c_str(), 0777) != 0) {
                return false;
            }
        }
    }

    // No need to try to create the file again, since by this point we created
    // the directory hierarchy.
    return true;
}

std::string
findFile(const std::string& name, const std::string& searchPath)
{
    // Prepend colon-separated entries from the searchPath until we succeed or
    // run out of entries.
    std::string remaining = searchPath;
    while (!remaining.empty()) {
        // Grab the next path entry.
        std::size_t colonPos = remaining.find_first_of(':');
        std::string directory = remaining.substr(0, colonPos);

        // Concatenate it with the filename and check for existence.
        std::string path = directory + '/' + name;
        if (access(path.c_str(), R_OK) != -1) {
            // We found it, return the full path.
            return path;
        }

        // Move to the next path entry.
        if (colonPos != std::string::npos) {
            remaining = remaining.substr(colonPos + 1, std::string::npos);
        } else {
            remaining = "";
        }
    }

    // We exhausted every path entry in the search path and didn't find it.
    return std::string();
}

// TODO: Use std::filesystem::copy()
// Is there a performance benefit from rolling our own custom copy() ?

void
copyFile(const std::string& src, const std::string& dst)
{
#if 0
    #if defined(__APPLE__)
    // No custom copy file implementation for macOS.
    // TODO: If we go with this std::filesystem::copy() solution, should we modify
    // our unit test to expect a std::filesystem::filesystem_error so we don't need
    // to wrap it in a scene_rdl2::except::IoError ?
    try {
        // May throw std::filesystem::filesystem_error
        std::filesystem::copy(src, dst);
    } catch (const std::filesystem::filesystem_error &e) {
        throw except::IoError(e.what());
    }
    #else
    // pre-C++17 low-level kernel-space copy implementation for linux.
    // Keeping this handy because we may need it if production encounters
    // any issues with std::filesystemcopy.  This copyFile() function is
    // used by ImageWriteCache and it's a potentially critical/sensitive
    // operation in the production case where there are many AOVs.  If
    // std::filesystem::copy isn't up to the task, we may need to use this
    // older custom file copy implementation instead.

    // Copy the file in kernel space (zero-copy, woo!).
    // Note: A single call to sendfile will copy at most 2,147,479,552 bytes,
    //       so files larger than this require multiple calls.

    size_t bytesToCopy = numBytes;
    size_t bytesCopied = 0;
    off_t offset = 0;

    while (bytesCopied < numBytes) {
        ssize_t result = sendfile(out.fd, in.fd, &offset, bytesToCopy);

        if (result == -1) {
            throw except::IoError(util::buildString("sendfile() failed: ",
                    std::strerror(errno)));
        }
        bytesToCopy -= result;
        bytesCopied += result;
    }

    MNRY_ASSERT_REQUIRE(bytesCopied == numBytes);
    #endif
#else
    // TODO: If we go with this std::filesystem::copy() solution, should we modify
    // our unit test to expect a std::filesystem::filesystem_error so we don't need
    // to wrap it in a scene_rdl2::except::IoError ?
    try {
        // May throw std::filesystem::filesystem_error
        std::filesystem::copy(src, dst);
    } catch (const std::filesystem::filesystem_error &e) {
        throw except::IoError(e.what());
    }
#endif
}

std::string
absolutePath(const std::string& filePath, std::string relativeToPath)
{
    // If already absolute, nothing to do.
    if (isAbsolute(filePath)) {
        return filePath;
    }

    // If no relative hint specified, assume current working directory.
    if (relativeToPath.empty()) {
        relativeToPath = currentWorkingDirectory();
    }

    // TODO: Use this when we can drop support for icc13.
    // return util::buildString(relativeToPath, '/', filePath);
    std::ostringstream result;
    result << relativeToPath << '/' << filePath;
    return result.str();
}

std::string
currentWorkingDirectory()
{
    #if defined(__APPLE__)
    char* cwd = getcwd (NULL, 0);
    #else
    char* cwd = get_current_dir_name();
    #endif
    std::string path(cwd);
    free(cwd);
    return path;
}

std::string
simplifyPath(const std::string& path)
{
    std::vector<std::string> pathStack;
    pathStack.reserve(10); // Should cover most paths with a single allocation.

    // Split the path into components, skipping components to clean up "." and
    // "..".
    std::size_t begin = 0;
    std::size_t len = path.size();
    while (begin < len) {
        auto end = path.find('/', begin);
        end = (end == std::string::npos) ? len : end;
        auto component = path.substr(begin, end - begin);
        if (component == ".") {
            // Ignore.
        } else if (component == "..") {
            // Remove previous component (leading .. is illegal).
            MNRY_ASSERT(pathStack.size() > 0);
            pathStack.pop_back();
        } else {
            pathStack.push_back(std::move(component));
        }
        begin = end + 1;
    }

    // Join the components back together.
    std::string joinedPath;
    if (pathStack.size() > 0) {
        joinedPath.append(pathStack[0]);
        for (std::size_t i = 1; i < pathStack.size(); ++i) {
            joinedPath.append("/");
            joinedPath.append(pathStack[i]);
        }
    }

    return joinedPath;
}

bool
createDirectories(const std::string& path)
{
    size_t pos = 0;
    while ((pos = path.find_first_of("/\\", pos + 1)) != std::string::npos) {
        std::string dir = path.substr(0, pos);
        #if defined(_WIN32)
            if (_mkdir(dir.c_str()) != 0 && errno != EEXIST) {
        #else
            if (mkdir(dir.c_str(), 0777) != 0 && errno != EEXIST) {
        #endif
                std::cerr << "Failed to create directory: " << dir << std::endl;
                return false;
            }
    }
    return true;
}

} // namespace util
} // namespace scene_rdl2

