// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "TestProxies.h"

#include <scene_rdl2/scene/rdl2/rdl2.h>

#include <cppunit/extensions/HelperMacros.h>

#include <string>

namespace scene_rdl2 {
namespace rdl2 {
namespace unittest {

void
TestProxies::setUp()
{
}

void
TestProxies::tearDown()
{
}


void
TestProxies::testProxyDwaBaseLayerable()
{
    // This tests the ability of the INTERFACE_DWABASELAYERABLE flag to
    // work properly in proxy mode (see MOONRAY-2047).

    // Create the context, class, and object in proxy mode.
    SceneContext context;
    context.setProxyModeEnabled(true);
    const SceneClass* sc = context.createSceneClass("LibLadenDwaBaseLayerable");
    Material *mat = context.createSceneObject("LibLadenDwaBaseLayerable", "/seq/shot/layerMat")->asA<Material>();
    Material *matA = context.createSceneObject("LibLadenDwaBaseLayerable", "/seq/shot/layerMatA")->asA<Material>();

    // Set an attribute specific to the proxied type.
    AttributeKey<SceneObject *> key = sc->getAttributeKey<rdl2::SceneObject *>("mat A");
    mat->beginUpdate();
    mat->set(key, matA);
    mat->endUpdate();

    // Write it out.
    BinaryWriter writer(context);
    writer.toFile("proxyDwaBaseLayerable.rdlb");

    // Read it back in.
    SceneContext verifyContext;
    verifyContext.setProxyModeEnabled(true);
    BinaryReader reader(verifyContext);
    reader.fromFile("proxyDwaBaseLayerable.rdlb");

    // Verify the attribute.
    Material *verifyMat = static_cast<rdl2::Material *>
        (verifyContext.getSceneObject("/seq/shot/layerMat")->asA<Material>());
    CPPUNIT_ASSERT(verifyMat);
    Material *verifyMatA = static_cast<rdl2::Material *>
        (verifyContext.getSceneObject("/seq/shot/layerMatA")->asA<Material>());
    CPPUNIT_ASSERT(verifyMatA);
    sc = verifyContext.getSceneClass("LibLadenDwaBaseLayerable");
    key = sc->getAttributeKey<rdl2::SceneObject *>("mat A");
    CPPUNIT_ASSERT(verifyMat->get(key) == verifyMatA);
}

void
TestProxies::testProxyCamera()
{
    // Create the context, class, and object in proxy mode.
    SceneContext context;
    context.setProxyModeEnabled(true);
    const SceneClass* sc = context.createSceneClass("LibLadenCamera");
    Camera* cam = context.createSceneObject("LibLadenCamera", "/seq/shot/camera")->asA<Camera>();

    // Set an attribute specific to the proxied type.
    AttributeKey<Int> key = sc->getAttributeKey<Int>("library ladenness");
    cam->beginUpdate();
    cam->set(key, 42);
    cam->endUpdate();

    // Write it out.
    BinaryWriter writer(context);
    writer.toFile("proxyCamera.rdlb");

    // Read it back in.
    SceneContext verifyContext;
    verifyContext.setProxyModeEnabled(true);
    BinaryReader reader(verifyContext);
    reader.fromFile("proxyCamera.rdlb");

    // Verify the attribute.
    Camera* verifyCam = verifyContext.getSceneObject("/seq/shot/camera")->asA<Camera>();
    CPPUNIT_ASSERT(verifyCam->get(key) == 42);
}

void
TestProxies::testProxyDisplayFilter()
{
    // Create the context, class, and object in proxy mode.
    SceneContext context;
    context.setProxyModeEnabled(true);
    const SceneClass* sc = context.createSceneClass("LibLadenDisplayFilter");
    DisplayFilter* displayFilter =
        context.createSceneObject("LibLadenDisplayFilter", "/seq/shot/displayfilter")->asA<DisplayFilter>();

    // Set an attribute specific to the proxied type.
    AttributeKey<Int> key = sc->getAttributeKey<Int>("library ladenness");
    displayFilter->beginUpdate();
    displayFilter->set(key, 42);
    displayFilter->endUpdate();

    // Write it out.
    BinaryWriter writer(context);
    writer.toFile("proxyDisplayFilter.rdlb");

    // Read it back in.
    SceneContext verifyContext;
    verifyContext.setProxyModeEnabled(true);
    BinaryReader reader(verifyContext);
    reader.fromFile("proxyDisplayFilter.rdlb");

    // Verify the attribute.
    DisplayFilter* verifyDisplayFilter =
        verifyContext.getSceneObject("/seq/shot/displayfilter")->asA<DisplayFilter>();
    CPPUNIT_ASSERT(verifyDisplayFilter->get(key) == 42);
}

void
TestProxies::testProxyEnvMap()
{
    // Create the context, class, and object in proxy mode.
    SceneContext context;
    context.setProxyModeEnabled(true);
    const SceneClass* sc = context.createSceneClass("LibLadenEnvMap");
    EnvMap* envMap = context.createSceneObject("LibLadenEnvMap", "/seq/shot/envmap")->asA<EnvMap>();

    // Set an attribute specific to the proxied type.
    AttributeKey<Int> key = sc->getAttributeKey<Int>("library ladenness");
    envMap->beginUpdate();
    envMap->set(key, 42);
    envMap->endUpdate();

    // Write it out.
    BinaryWriter writer(context);
    writer.toFile("proxyEnvMap.rdlb");

    // Read it back in.
    SceneContext verifyContext;
    verifyContext.setProxyModeEnabled(true);
    BinaryReader reader(verifyContext);
    reader.fromFile("proxyEnvMap.rdlb");

    // Verify the attribute.
    EnvMap* verifyEnvMap = verifyContext.getSceneObject("/seq/shot/envmap")->asA<EnvMap>();
    CPPUNIT_ASSERT(verifyEnvMap->get(key) == 42);
}

void
TestProxies::testProxyGeometry()
{
    // Create the context, class, and object in proxy mode.
    SceneContext context;
    context.setProxyModeEnabled(true);
    const SceneClass* sc = context.createSceneClass("LibLadenGeometry");
    Geometry* geom = context.createSceneObject("LibLadenGeometry", "/seq/shot/envmap")->asA<Geometry>();

    // Set an attribute specific to the proxied type.
    AttributeKey<Int> key = sc->getAttributeKey<Int>("library ladenness");
    geom->beginUpdate();
    geom->set(key, 42);
    geom->endUpdate();

    // Write it out.
    BinaryWriter writer(context);
    writer.toFile("proxyGeometry.rdlb");

    // Read it back in.
    SceneContext verifyContext;
    verifyContext.setProxyModeEnabled(true);
    BinaryReader reader(verifyContext);
    reader.fromFile("proxyGeometry.rdlb");

    // Verify the attribute.
    Geometry* verifyGeom = verifyContext.getSceneObject("/seq/shot/envmap")->asA<Geometry>();
    CPPUNIT_ASSERT(verifyGeom->get(key) == 42);
}

void
TestProxies::testProxyLight()
{
    // Create the context, class, and object in proxy mode.
    SceneContext context;
    context.setProxyModeEnabled(true);
    const SceneClass* sc = context.createSceneClass("LibLadenLight");
    Light* light = context.createSceneObject("LibLadenLight", "/seq/shot/envmap")->asA<Light>();

    // Set an attribute specific to the proxied type.
    AttributeKey<Int> key = sc->getAttributeKey<Int>("library ladenness");
    light->beginUpdate();
    light->set(key, 42);
    light->endUpdate();

    // Write it out.
    BinaryWriter writer(context);
    writer.toFile("proxyLight.rdlb");

    // Read it back in.
    SceneContext verifyContext;
    verifyContext.setProxyModeEnabled(true);
    BinaryReader reader(verifyContext);
    reader.fromFile("proxyLight.rdlb");

    // Verify the attribute.
    Light* verifyLight = verifyContext.getSceneObject("/seq/shot/envmap")->asA<Light>();
    CPPUNIT_ASSERT(verifyLight->get(key) == 42);
}

void
TestProxies::testProxyLightFilter()
{
    // Create the context, class, and object in proxy mode.
    SceneContext context;
    context.setProxyModeEnabled(true);
    const SceneClass* sc = context.createSceneClass("LibLadenLightFilter");
    LightFilter* lightFilter =
        context.createSceneObject("LibLadenLightFilter", "/seq/shot/lightfilter")->asA<LightFilter>();

    // Set an attribute specific to the proxied type.
    AttributeKey<Int> key = sc->getAttributeKey<Int>("library ladenness");
    lightFilter->beginUpdate();
    lightFilter->set(key, 3561);
    lightFilter->endUpdate();

    // Write it out.
    BinaryWriter writer(context);
    writer.toFile("proxyLightFilter.rdlb");

    // Read it back in.
    SceneContext verifyContext;
    verifyContext.setProxyModeEnabled(true);
    BinaryReader reader(verifyContext);
    reader.fromFile("proxyLightFilter.rdlb");

    // Verify the attribute.
    LightFilter* verifyLightFilter =
        verifyContext.getSceneObject("/seq/shot/lightfilter")->asA<LightFilter>();
    CPPUNIT_ASSERT(verifyLightFilter->get(key) == 3561);
}

void
TestProxies::testProxyMap()
{
    // Create the context, class, and object in proxy mode.
    SceneContext context;
    context.setProxyModeEnabled(true);
    const SceneClass* sc = context.createSceneClass("LibLadenMap");
    Map* map = context.createSceneObject("LibLadenMap", "/seq/shot/envmap")->asA<Map>();

    // Set an attribute specific to the proxied type.
    AttributeKey<Int> key = sc->getAttributeKey<Int>("library ladenness");
    map->beginUpdate();
    map->set(key, 42);
    map->endUpdate();

    // Write it out.
    BinaryWriter writer(context);
    writer.toFile("proxyMap.rdlb");

    // Read it back in.
    SceneContext verifyContext;
    verifyContext.setProxyModeEnabled(true);
    BinaryReader reader(verifyContext);
    reader.fromFile("proxyMap.rdlb");

    // Verify the attribute.
    Map* verifyMap = verifyContext.getSceneObject("/seq/shot/envmap")->asA<Map>();
    CPPUNIT_ASSERT(verifyMap->get(key) == 42);
}

void
TestProxies::testProxyNormalMap()
{
    // Create the context, class, and object in proxy mode.
    SceneContext context;
    context.setProxyModeEnabled(true);
    const SceneClass* sc = context.createSceneClass("LibLadenNormalMap");
    NormalMap* normalMap = context.createSceneObject("LibLadenNormalMap", "/seq/shot/envmap")->asA<NormalMap>();

    // Set an attribute specific to the proxied type.
    AttributeKey<Int> key = sc->getAttributeKey<Int>("library ladenness");
    normalMap->beginUpdate();
    normalMap->set(key, 42);
    normalMap->endUpdate();

    // Write it out.
    BinaryWriter writer(context);
    writer.toFile("proxyNormalMap.rdlb");

    // Read it back in.
    SceneContext verifyContext;
    verifyContext.setProxyModeEnabled(true);
    BinaryReader reader(verifyContext);
    reader.fromFile("proxyNormalMap.rdlb");

    // Verify the attribute.
    NormalMap* verifyNormalMap = verifyContext.getSceneObject("/seq/shot/envmap")->asA<NormalMap>();
    CPPUNIT_ASSERT(verifyNormalMap->get(key) == 42);
}

void
TestProxies::testProxyMaterial()
{
    // Create the context, class, and object in proxy mode.
    SceneContext context;
    context.setProxyModeEnabled(true);
    const SceneClass* sc = context.createSceneClass("LibLadenMaterial");
    Material* mat = context.createSceneObject("LibLadenMaterial", "/seq/shot/envmap")->asA<Material>();

    // Set an attribute specific to the proxied type.
    AttributeKey<Int> key = sc->getAttributeKey<Int>("library ladenness");
    mat->beginUpdate();
    mat->set(key, 42);
    mat->endUpdate();

    // Write it out.
    BinaryWriter writer(context);
    writer.toFile("proxyMaterial.rdlb");

    // Read it back in.
    SceneContext verifyContext;
    verifyContext.setProxyModeEnabled(true);
    BinaryReader reader(verifyContext);
    reader.fromFile("proxyMaterial.rdlb");

    // Verify the attribute.
    Material* verifyMat = verifyContext.getSceneObject("/seq/shot/envmap")->asA<Material>();
    CPPUNIT_ASSERT(verifyMat->get(key) == 42);
}

} // namespace unittest
} // namespace rdl2
} // namespace scene_rdl2

