// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "ObjectFactory.h"

#include "Camera.h"
#include "Displacement.h"
#include "Dso.h"
#include "GeometrySet.h"
#include "RenderOutput.h"
#include "Layer.h"
#include "LightFilterSet.h"
#include "LightSet.h"
#include "Metadata.h"
#include "Proxies.h"
#include "SceneClass.h"
#include "SceneObject.h"
#include "SceneVariables.h"
#include "ShadowReceiverSet.h"
#include "ShadowSet.h"
#include "TraceSet.h"
#include "Types.h"
#include "UserData.h"

#include <scene_rdl2/common/except/exceptions.h>
#include <scene_rdl2/common/platform/Platform.h>

#include <memory>
#include <sstream>
#include <string>
#include <utility>

namespace scene_rdl2 {
namespace rdl2 {

namespace {

template <typename T>
SceneObjectInterface
builtInDeclare(SceneClass& sceneClass)
{
    return T::declare(sceneClass);
}

template <typename T>
SceneObject*
builtInCreate(const SceneClass& sceneClass, const std::string& name)
{
    return new T(sceneClass, name);
}

void
builtInDestroy(SceneObject* sceneObject)
{
    delete sceneObject;
}

SceneObject*
proxyCreate(const SceneClass& sceneClass, const std::string& name)
{
    auto interface = sceneClass.getDeclaredInterface();

    // Create the proper proxy class based on the declared interface of the
    // DSO type.
    if (interface & INTERFACE_CAMERA) {
        return new CameraProxy(sceneClass, name);
    } else if (interface & INTERFACE_ENVMAP) {
        return new EnvMapProxy(sceneClass, name);
    } else if (interface & INTERFACE_GEOMETRY) {
        return new GeometryProxy(sceneClass, name);
    } else if (interface & INTERFACE_LIGHT) {
        return new LightProxy(sceneClass, name);
    } else if (interface & INTERFACE_MAP) {
        return new MapProxy(sceneClass, name);
    } else if (interface & INTERFACE_NORMALMAP) {
        return new NormalMapProxy(sceneClass, name);
    } else if (interface & INTERFACE_MATERIAL) {
        if (interface & INTERFACE_DWABASELAYERABLE) {
            return new DwaBaseLayerableProxy(sceneClass, name);
        } else if (interface & INTERFACE_DWABASEHAIRLAYERABLE) {
            return new DwaBaseHairLayerableProxy(sceneClass, name);
        } else if (interface & INTERFACE_DWABASE) {
            return new DwaBaseProxy(sceneClass, name);
        } else {
            return new MaterialProxy(sceneClass, name);
        }
    } else if (interface & INTERFACE_DISPLACEMENT) {
        return new DisplacementProxy(sceneClass, name);
    } else if (interface & INTERFACE_VOLUMESHADER) {
        return new VolumeShaderProxy(sceneClass, name);
    } else if (interface & INTERFACE_LIGHTFILTER) {
        return new LightFilterProxy(sceneClass, name);
    } else if (interface & INTERFACE_DISPLAYFILTER) {
        return new DisplayFilterProxy(sceneClass, name);
    } else {
        std::string errStr ="Undefined Scene Object Interface: " + name + "\n";
        MNRY_ASSERT(false, errStr.c_str());
    }


    // If none of these interfaces match, we can't get any more specific than a
    // generic SceneObject.
    return new SceneObjectProxy(sceneClass, name);
}

void
proxyDestroy(SceneObject* sceneObject)
{
    delete sceneObject;
}

} // namespace

ObjectFactory::ObjectFactory(ClassDeclareFunc declareFunc, ObjectCreateFunc createFunc,
                             ObjectDestroyFunc destroyFunc, std::unique_ptr<Dso> dso) :
    mDso(std::move(dso)),
    mDeclareFunc(declareFunc),
    mCreateFunc(createFunc),
    mDestroyFunc(destroyFunc)
{
    MNRY_ASSERT(mDeclareFunc, "ObjectFactory must have a declare function pointer!");
    MNRY_ASSERT(mCreateFunc, "ObjectFactory must have a create function pointer!");
    MNRY_ASSERT(mDestroyFunc, "ObjectFactory must have a destroy function pointer!");
}

std::string
ObjectFactory::getSourcePath() const
{
    return (mDso) ? mDso->getFilePath() : std::string();
}

template <typename T>
std::unique_ptr<ObjectFactory>
ObjectFactory::createBuiltInFactory()
{
    // Create the ObjectFactory.
    return std::unique_ptr<ObjectFactory>(
        new ObjectFactory(builtInDeclare<T>, builtInCreate<T>, builtInDestroy));
}

// Explicit instantiations of all the built in types you can create. Add more
// if you create any new built in types.
template std::unique_ptr<ObjectFactory> ObjectFactory::createBuiltInFactory<GeometrySet>();
template std::unique_ptr<ObjectFactory> ObjectFactory::createBuiltInFactory<Joint>();
template std::unique_ptr<ObjectFactory> ObjectFactory::createBuiltInFactory<TraceSet>();
template std::unique_ptr<ObjectFactory> ObjectFactory::createBuiltInFactory<Layer>();
template std::unique_ptr<ObjectFactory> ObjectFactory::createBuiltInFactory<LightFilterSet>();
template std::unique_ptr<ObjectFactory> ObjectFactory::createBuiltInFactory<LightSet>();
template std::unique_ptr<ObjectFactory> ObjectFactory::createBuiltInFactory<RenderOutput>();
template std::unique_ptr<ObjectFactory> ObjectFactory::createBuiltInFactory<SceneVariables>();
template std::unique_ptr<ObjectFactory> ObjectFactory::createBuiltInFactory<ShadowSet>();
template std::unique_ptr<ObjectFactory> ObjectFactory::createBuiltInFactory<UserData>();
template std::unique_ptr<ObjectFactory> ObjectFactory::createBuiltInFactory<Metadata>();
template std::unique_ptr<ObjectFactory> ObjectFactory::createBuiltInFactory<ShadowReceiverSet>();

std::unique_ptr<ObjectFactory>
ObjectFactory::createDsoFactory(const std::string& className, const std::string& dsoPath)
{
    // Open the DSO.
    std::unique_ptr<Dso> dso(new Dso(className, dsoPath, false));

    // Extract the declare, create, and destroy function pointers.
    ClassDeclareFunc declarer = dso->getDeclare();
    ObjectCreateFunc creator = dso->getCreate();
    ObjectDestroyFunc destroyer = dso->getDestroy();

    // Create the ObjectFactory.
    return std::unique_ptr<ObjectFactory>(
        new ObjectFactory(declarer, creator, destroyer, std::move(dso)));
}

std::unique_ptr<ObjectFactory>
ObjectFactory::createProxyFactory(const std::string& className, const std::string& dsoPath)
{
    // Open the DSO.
    std::unique_ptr<Dso> dso(new Dso(className, dsoPath, true));

    // Extract just the declare function.
    ClassDeclareFunc declarer = dso->getDeclare();

    // Create the ObjectFactory.
    return std::unique_ptr<ObjectFactory>(
        new ObjectFactory(declarer, proxyCreate, proxyDestroy, std::move(dso)));
}

} // namespace rdl2
} // namespace scene_rdl2

