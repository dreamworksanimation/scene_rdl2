// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "Dso.h"

#include "SceneClass.h"
#include "SceneObject.h"
#include "Types.h"

#include <scene_rdl2/common/platform/Platform.h>
#include <scene_rdl2/common/except/exceptions.h>
#include <scene_rdl2/render/util/Files.h>
#include <scene_rdl2/render/util/Strings.h>

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <exception>
#include <memory>
#include <sstream>
#include <string>
#include <dlfcn.h>
#include <libgen.h>
#include <unistd.h>

namespace scene_rdl2 {
namespace rdl2 {

namespace internal {

std::string
classNameFromFileName(const std::string& baseName,
                      const std::string& expectedExtension)
{
    // Base name must be at least expectedExtension + 1 chars ("a<extension>").
    if (baseName.size() < expectedExtension.size() + 1) {
        return "";
    }

    // Final characters must match expectedExtension.
    std::string extension =
        baseName.substr(baseName.size() - expectedExtension.size(), std::string::npos);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    if (extension != expectedExtension) {
        return "";
    }

    // Class name is the file name without the extension.
    return baseName.substr(0, baseName.size() - expectedExtension.size());
}

} // namespace internal

/* static */
std::string
Dso::classNameFromFileName(const std::string& filePath)
{
    char* dirStr = strdup(filePath.c_str());
    std::string directory(dirname(dirStr));
    free(dirStr);
    char* baseStr = strdup(filePath.c_str());
    std::string baseName(basename(baseStr));
    free(baseStr);

    // Bail early if we can't determine the class name.

    std::string className(internal::classNameFromFileName(baseName, ".so.proxy"));
    if (className.empty()) {
        className = internal::classNameFromFileName(baseName, ".so");
    }
    return className;
}


Dso::Dso(const std::string& className, const std::string& searchPath, bool proxyModeEnabled) :
    mHandle(nullptr),
    mDeclareFunc(nullptr),
    mCreateFunc(nullptr),
    mDestroyFunc(nullptr)
{
    MNRY_ASSERT(!className.empty(), "Dso must be constructed with a non-empty SceneClass name.");
    mFilePath = className + ".so";

    if (proxyModeEnabled) {
        mFilePath += ".proxy";
    }

    // If they explicitly specified a search path, attempt to find the DSO.
    if (!searchPath.empty()) {
        mFilePath = util::findFile(mFilePath, searchPath);
    }

    // If the file path is empty, we couldn't find it.
    if (mFilePath.empty()) {
        throw except::IoError(util::buildString("Couldn't find DSO for '",
                className, "' in search path '", searchPath, "'."));
    }

    // Attempt to open the DSO.
    mHandle = dlopen(mFilePath.c_str(), RTLD_LAZY);
    if (!mHandle) {
        std::stringstream errMsg;
        errMsg << "Found RDL2 DSO '" << mFilePath << "', but failed to"
            " dlopen() it";
        char* error = dlerror();
        if (error) {
            errMsg << ": " << error;
        } else {
            errMsg << ".";
        }
        throw except::RuntimeError(errMsg.str());
    }
}

Dso::~Dso()
{
    if (mHandle) {
        dlclose(mHandle);
    }
}

ClassDeclareFunc
Dso::getDeclare()
{
    // Return cached function pointer if we already looked it up.
    if (mDeclareFunc) {
        return mDeclareFunc;
    }

    // Clear errors.
    dlerror();

    // Attempt to load the rdl_declare symbol.
    MNRY_ASSERT(mHandle, "Tried to load symbol from bad DSO handle.");
    void* declareSymbol = dlsym(mHandle, "rdl2_declare");
    if (!declareSymbol) {
        // Technically null symbols are valid, but a null function pointer
        // is useless to us, so that's really a failure case, too.
        std::stringstream errMsg;
        errMsg << "Failed to load symbol 'rdl2_declare' from RDL2 DSO '" <<
            mFilePath << "'";
        char* error = dlerror();
        if (error) {
            errMsg << ": " << error;
        } else {
            errMsg << ".";
        }
        throw except::RuntimeError(errMsg.str());
    }

    // Cache the function pointer.
    mDeclareFunc = (ClassDeclareFunc)declareSymbol;

    return mDeclareFunc;
}

ObjectCreateFunc
Dso::getCreate()
{
    // Return cached function pointer if we already looked it up.
    if (mCreateFunc) {
        return mCreateFunc;
    }

    // Clear errors.
    dlerror();

    // Attempt to load the rdl_create symbol.
    MNRY_ASSERT(mHandle, "Tried to load symbol from bad DSO handle.");
    void* createSymbol = dlsym(mHandle, "rdl2_create");
    if (!createSymbol) {
        // Technically null symbols are valid, but a null function pointer
        // is useless to us, so that's really a failure case, too.
        std::stringstream errMsg;
        errMsg << "Failed to load symbol 'rdl2_create' from RDL2 DSO '" <<
            mFilePath << "'";
        char* error = dlerror();
        if (error) {
            errMsg << ": " << error;
        } else {
            errMsg << ".";
        }
        throw except::RuntimeError(errMsg.str());
    }

    // Cache the function pointer.
    mCreateFunc = (ObjectCreateFunc)createSymbol;

    return mCreateFunc;
}

ObjectDestroyFunc
Dso::getDestroy()
{
    // Return the cached function pointer if we already looked it up.
    if (mDestroyFunc) {
        return mDestroyFunc;
    }

    // Clear errors.
    dlerror();

    // Attempt to load the rdl_destroy symbol.
    MNRY_ASSERT(mHandle, "Tried to load symbol from bad DSO handle.");
    void* destroySymbol = dlsym(mHandle, "rdl2_destroy");
    if (!destroySymbol) {
        // Technically null symbols are valid, but a null function pointer
        // is useless to us, so that's really a failure case, too.
        std::stringstream errMsg;
        errMsg << "Failed to load symbol 'rdl2_destroy' from RDL2 DSO '" <<
            mFilePath << "'";
        char* error = dlerror();
        if (error) {
            errMsg << ": " << error;
        } else {
            errMsg << ".";
        }
        throw except::RuntimeError(errMsg.str());
    }

    // Cache the function pointer.
    mDestroyFunc = (ObjectDestroyFunc)destroySymbol;

    return mDestroyFunc;
}

bool
Dso::isValidDso(const std::string& filePath, bool proxyModeEnabled)
{
    // Break the path into directory and basename components. Painfully, both
    // dirname() and basename() may do just about anything with your pointers,
    // so its safest to make a copy first.
    char* dirStr = strdup(filePath.c_str());
    std::string directory(dirname(dirStr));
    free(dirStr);
    char* baseStr = strdup(filePath.c_str());
    std::string baseName(basename(baseStr));
    free(baseStr);

    // Bail early if we can't determine the class name.
    const char* extension = (proxyModeEnabled) ? ".so.proxy" : ".so";
    std::string className = internal::classNameFromFileName(baseName, extension);
    if (className.empty()) {
        return false;
    }

    // Attempt to load it as an RDL Dso object and get the expected function
    // pointers.
    try {
        std::unique_ptr<Dso> dso(new Dso(className, directory, proxyModeEnabled));
        dso->getDeclare();
        if (!proxyModeEnabled) {
            dso->getCreate();
            dso->getDestroy();
        }
    } catch (...) {
        return false;
    }

    // It's a valid DSO.
    return true;
}

} // namespace rdl2
} // namespace scene_rdl2

