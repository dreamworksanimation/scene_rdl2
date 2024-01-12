// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

// Include this before any other includes!
#include <scene_rdl2/common/platform/Platform.h>

#include "Types.h"
#include <string>

namespace scene_rdl2 {
namespace rdl2 {

// Forward declarations necessary for unit tests.
namespace unittest {
    class TestDso;
}

/**
 * A Dso represents a dynamically loaded shared library that contains the
 * definition of a new RDL object type that derives from the existing RDL
 * object hierarchy. It is the point of runtime customization for objects in
 * the scene.
 *
 * RDL DSOs don't use this class directly. Rather, this class represents a
 * DSO that has been loaded that RDL wishes to use. It loads symbols lazily
 * to avoid dragging in big library dependencies from custom types.
 *
 * The search path for DSOs is a colon separated list of directory paths,
 * similar to the $PATH variable in your shell.
 *
 * Thread Safety:
 *  - The Dso class makes no attempt to keep a registry of loaded DSOs or
 *      make the process of opening DSOs thread safe. Synchronization of these
 *      Dso objects should happen at a higher level.
 *  - In the context of RDL, this synchronization happens in the SceneContext.
 *      The context has a concurrent hash map of SceneClasses, and we only
 *      construct (and thus, open) a DSO if we have a write accessor to the
 *      given class name key. This effectively gives us a lock on DSOs with
 *      that given name, and gets around any thread safety issues.
 */
class Dso
{
public:
    /**
     * Attempts to locate, open, and extract the rdl_declare() symbol for an
     * RDL SceneClass with the given name and potential search path. The name
     * should be the class name. The ".so" extension is appended to the class
     * name when searching for the DSO.
     *
     * @param   className   The name of the SceneClass you are trying to load.
     * @param   searchPath  Colon separated list of potential paths on the file
     *                      system.
     * @param   proxyModeEnabled    If true, it searches for the DSO with a
     *                              ".proxy" extension.
     * @throw   except::IoError     If the DSO could not be found in the search
     *                              path, or it could not be dlopen()'d.
     * @throw   except::RuntimeError    If the rdl_declare() symbol could not
     *                                  be loaded from the DSO.
     */
    Dso(const std::string& className, const std::string& searchPath,
        bool proxyModeEnabled = false);

    /**
     * Closes the DSO if it was successfully opened.
     */
    ~Dso();

    /**
     * Extracts the rdl2_declare() function from the DSO and returns a function
     * pointer to it. If extraction fails, a RuntimeError is thrown.
     *
     * @return  A function pointer to the rdl2_declare() function.
     * @throw   except::RuntimeError    If the "rdl2_declare" symbol cannot be
     *                                  extracted from the DSO.
     */
    ClassDeclareFunc getDeclare();

    /**
     * Extracts the rdl2_create() function from the DSO and returns a function
     * pointer to it. If extraction fails, a RuntimeError is thrown.
     *
     * @return  A function pointer to the rdl2_create() function.
     * @throw   except::RuntimeError    If the "rdl2_create" symbol cannot be
     *                                  extracted from the DSO.
     */
    ObjectCreateFunc getCreate();

    /**
     * Extracts the rdl2_destroy() function from the DSO and returns a function
     * pointer to it. If extraction fails, a RuntimeError is thrown.
     *
     * @return  A function pointer to the rdl2_destroy() function.
     * @throw   except::RuntimeError    If the "rdl2_destroy" symbol cannot be
     *                                  extracted from the DSO.
     */
    ObjectDestroyFunc getDestroy();

    /// The full path to the DSO which we found on the filesystem.
    finline const std::string& getFilePath() const;

    /**
     * Takes a file path and attempts to open it as an RDL DSO. If this
     * succeeds, the DSO is closed and we return true. If any part of this
     * fails, we catch the exceptions and return false.
     *
     * This only verifies that the DSO correctly exports the rdl2_declare()
     * symbol. It's still possible that it has broken rdl2_create() and
     * rdl2_destroy() functions. This guarantees that, at a minimum, we can use
     * the DSO with proxy objects.
     *
     * @param   filePath    Path to a file that we'll attempt to open as an RDL DSO.
     * @param   proxyModeEnabled    True if we're testing proxy DSOs.
     * @return  True if the file appears to be a valid RDL DSO.
     */
    static bool isValidDso(const std::string& filePath, bool proxyModeEnabled = false);

    /**
     * Takes a file path and returns the class name of the DSO
     *
     * @param   filePath    Path to a file that we'll attempt to open as an RDL DSO.
     * @return  The name of the DSO Class, or empty string on failure
     */
    static std::string classNameFromFileName(const std::string& filePath);



private:
    // Non-copyable.
    Dso(const Dso&);
    const Dso& operator=(const Dso&);

    // The full path to the file we found on the filesystem.
    std::string mFilePath;

    // The DSO handle.
    void* mHandle;

    // The rdl_declare() function pointer extracted from the DSO.
    ClassDeclareFunc mDeclareFunc;

    // The rdl_create() function pointer extracted from the DSO.
    ObjectCreateFunc mCreateFunc;

    // The rdl_destroy() function pointer extracted from the DSO.
    ObjectDestroyFunc mDestroyFunc;

    // Classes which need access for unit testing purposes.
    friend class unittest::TestDso;
};

const std::string&
Dso::getFilePath() const
{
    return mFilePath;
}

} // namespace rdl2
} // namespace scene_rdl2

