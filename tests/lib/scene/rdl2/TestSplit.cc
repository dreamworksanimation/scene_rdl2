// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


// Test split mode writing a context to both rdla and rdlb
#include "TestSplit.h"

#include <scene_rdl2/scene/rdl2/AttributeKey.h>
#include <scene_rdl2/scene/rdl2/Utils.h>
#include <scene_rdl2/scene/rdl2/SceneClass.h>
#include <scene_rdl2/scene/rdl2/SceneContext.h>
#include <scene_rdl2/scene/rdl2/SceneObject.h>

#include <scene_rdl2/common/except/exceptions.h>

#include <cppunit/extensions/HelperMacros.h>

#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>

namespace scene_rdl2 {
namespace rdl2 {
namespace unittest {

void
TestSplit::setUp()
{
    mShortVec.clear();
    for (unsigned i = 0; i < 4; i++) {
        mShortVec.push_back(Vec3f(i, i, i));
    }

    mLongVec.clear();
    for (unsigned i = 0; i < 100; i++) {
        mLongVec.push_back(Vec3f(i, 0, i));
    }
}

void
TestSplit::tearDown()
{
}

void
TestSplit::testRoundtrip()
{
    // Create the context, load a class, and create some objects.
    SceneContext context;
    const SceneClass* sc = context.createSceneClass("ExtensiveObject");
    SceneObject* apple = context.createSceneObject("ExtensiveObject", "/seq/shot/apple");
    SceneObject* banana = context.createSceneObject("ExtensiveObject", "/seq/shot/banana");
    Geometry* teapot = context.createSceneObject("FakeTeapot", "/seq/shot/teapot")->asA<Geometry>();
    Material* material = context.createSceneObject("FakeMaterial", "/seq/shot/material")->asA<Material>();
    LightSet* lightset = context.createSceneObject("LightSet", "/seq/shot/lightset")->asA<LightSet>();
    Layer* layer = context.createSceneObject("Layer", "/seq/shot/layer")->asA<Layer>();

    AttributeKey<String> stringKey = sc->getAttributeKey<String>("string");
    AttributeKey<Vec3fVector> vec3fVectorKey = sc->getAttributeKey<Vec3fVector>("vec3f_vector");

    std::string stringDefault = apple->get(stringKey);
    Vec3fVector vec3fVectorDefault = apple->get(vec3fVectorKey);

    apple->beginUpdate();
    apple->set(stringKey, std::string("apple"));
    apple->set(vec3fVectorKey, mShortVec);
    apple->endUpdate();
    banana->beginUpdate();
    banana->set(stringKey, std::string("banana"));
    banana->set(vec3fVectorKey, mLongVec);
    banana->endUpdate();

    layer->beginUpdate();
    for (unsigned i = 0; i < 50; i++) {
        layer->assign(teapot, "part" + std::to_string(i), material, lightset);
    }
    layer->endUpdate();

    writeSceneToFile(context, "roundtrip_split", false, true);

    // Create a fresh SceneContext and read in the Ascii file.
    SceneContext asciiContext;
    AsciiReader areader(asciiContext);
    areader.fromFile("roundtrip_split.rdla");

    SceneObject* asciiApple = asciiContext.getSceneObject("/seq/shot/apple");
    SceneObject* asciiBanana = asciiContext.getSceneObject("/seq/shot/banana");
    Geometry* asciiTeapot = asciiContext.getSceneObject("/seq/shot/teapot")->asA<Geometry>();
    Layer* asciiLayer = asciiContext.getSceneObject("/seq/shot/layer")->asA<Layer>();

    // everything except banana.vec3fVector should be in the ascii file
    CPPUNIT_ASSERT(apple->get(stringKey) == asciiApple->get(stringKey));
    CPPUNIT_ASSERT(apple->get(vec3fVectorKey) == asciiApple->get(vec3fVectorKey));
    CPPUNIT_ASSERT(banana->get(stringKey) == asciiBanana->get(stringKey));
    CPPUNIT_ASSERT(vec3fVectorDefault == asciiBanana->get(vec3fVectorKey));

    for (unsigned i = 0; i < 50; i++) {
        // will throw if entry doesn't exist
        asciiLayer->lookup(asciiTeapot, "part" + std::to_string(i));
    }

    // Create a fresh SceneContext and read in the Binary file.
    SceneContext binContext;
    BinaryReader breader(binContext);
    breader.fromFile("roundtrip_split.rdlb");

    SceneObject* binApple = binContext.getSceneObject("/seq/shot/apple");
    SceneObject* binBanana = binContext.getSceneObject("/seq/shot/banana");
    Layer* binLayer = binContext.getSceneObject("/seq/shot/layer")->asA<Layer>();

    // only banana.vec3fVector should be in the binary file
    CPPUNIT_ASSERT(stringDefault == binApple->get(stringKey));
    CPPUNIT_ASSERT(vec3fVectorDefault == binApple->get(vec3fVectorKey));
    CPPUNIT_ASSERT(stringDefault == binBanana->get(stringKey));
    CPPUNIT_ASSERT(banana->get(vec3fVectorKey) == binBanana->get(vec3fVectorKey));

    Layer::GeometrySet geoms;
    binLayer->getAllGeometries(geoms);
    CPPUNIT_ASSERT(geoms.empty());
}


} // namespace unittest
} // namespace rdl2
} // namespace scene_rdl2

