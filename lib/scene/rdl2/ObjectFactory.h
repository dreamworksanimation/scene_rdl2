// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "Dso.h"
#include "Types.h"

#include <memory>
#include <string>

namespace scene_rdl2 {
namespace rdl2 {

/**
 * The ObjectFactory manages function pointers to the critical functions for
 * declaring, creating, and destroying objects of a particular type.
 *
 * You create an ObjectFactory through one of the static creation functions,
 * depending on how you want the objects created. At the moment we support
 * built in types, DSO types, and proxy objects (which use DSOs for their
 * attribute declarations, but are constructed from built in proxy types).
 *
 * The ObjectFactory also takes ownership of a Dso object, if loading symbols
 * from any DSO is required.
 *
 * Thread Safety:
 *  - Creating ObjectFactories for the same SceneClass from different threads
 *      simultaneously is not thread safe, because we don't enforce thread
 *      safety of any DSO operations.
 *  - Creating and manipulating ObjectFactories for different SceneClasses
 *      from multiple threads should be safe, as there is no global shared data
 *      to synchronize.
 */
class ObjectFactory
{
public:
    /**
     * Invoke the declare function, no matter where it came from. This
     * effectively forwards the call to the underlying function pointer.
     *
     * @param   sceneClass  The SceneClass that will be filled in with attribute
     *                      declarations.
     * @return  The SceneObjectInterface this SceneClass intends to implement.
     */
    finline SceneObjectInterface declare(SceneClass& sceneClass);

    /**
     * Invoke the create function, no matter where it came from. This
     * effectively forwards the call to the underlying function pointer.
     *
     * @param   sceneClass  The SceneClass that this new SceneObject is to
     *                      belong to.
     * @param   name        The name of the new SceneObject.
     * @return  A pointer to the newly allocated SceneObject. Ownership of this
     *          pointer is transferred to the caller.
     */
    finline SceneObject* create(const SceneClass& sceneClass, const std::string& name);

    /**
     * Invoke the destroy function, no matter where it came from. This
     * effectively forwards the call to the underlying function pointer.
     *
     * @param   sceneObject The SceneObject that we wish to destroy.
     */
    finline void destroy(SceneObject* sceneObject);

    /**
     * Returns the path to where this SceneClass came from. If the factory is a
     * DsoFactory or a ProxyFactory, it returns the file system path of the DSO
     * (or proxy DSO) that was loaded. If the factory is a built-in factory, it
     * returns an empty string.
     *
     * @return  File system path to the source of this SceneClass or an empty
     *          string if it's built-in.
     */
    std::string getSourcePath() const;

    /**
     * Create an ObjectFactory for the built in type specified by the template
     * parameter.
     *
     * @return  An ObjectFactory that can declare, create, and destroy these
     *          DSO objects.
     */
    template <typename T>
    static std::unique_ptr<ObjectFactory> createBuiltInFactory();

    /**
     * Create an ObjectFactory that sources its function pointers from a DSO.
     * The DSO should be named "className.so" and be in the dsoPath.
     *
     * @param   className   The SceneClass name. The DSO is expected to be
     *                      named "className.so".
     * @param   dsoPath     A colon seperated path to search for DSOs.
     * @return  An ObjectFactory that can declare, create, and destroy these
     *          DSO objects.
     */
    static std::unique_ptr<ObjectFactory> createDsoFactory(const std::string& className, const std::string& dsoPath);

    /**
     * Create an ObjectFactory that sources its declare function pointer from
     * a DSO, but creates and destroys objects through built in proxy objects.
     *
     * @param   className   The SceneClass name. The DSO is expected to be
     *                      named "className.so".
     * @param   dsoPath     A colon seperated path to search for DSOs.
     * @return  An ObjectFactory that can declare, create, and destroy these
     *          DSO objects.
     */
    static std::unique_ptr<ObjectFactory> createProxyFactory(const std::string& className, const std::string& dsoPath);

private:
    ObjectFactory(ClassDeclareFunc declareFunc, ObjectCreateFunc createFunc,
                  ObjectDestroyFunc destroyFunc, std::unique_ptr<Dso> dso = nullptr);

    std::unique_ptr<Dso> mDso;
    ClassDeclareFunc mDeclareFunc;
    ObjectCreateFunc mCreateFunc;
    ObjectDestroyFunc mDestroyFunc;
};

SceneObjectInterface
ObjectFactory::declare(SceneClass& sceneClass)
{
    return mDeclareFunc(sceneClass);
}

SceneObject*
ObjectFactory::create(const SceneClass& sceneClass, const std::string& name)
{
    return mCreateFunc(sceneClass, name);
}

void
ObjectFactory::destroy(SceneObject* sceneObject)
{
    mDestroyFunc(sceneObject);
}

} // namespace rdl2
} // namespace scene_rdl2

