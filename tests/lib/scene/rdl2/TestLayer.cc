// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "TestLayer.h"

#include <scene_rdl2/scene/rdl2/BinaryReader.h>
#include <scene_rdl2/scene/rdl2/BinaryWriter.h>
#include <scene_rdl2/scene/rdl2/Displacement.h>
#include <scene_rdl2/scene/rdl2/Geometry.h>
#include <scene_rdl2/scene/rdl2/Layer.h>
#include <scene_rdl2/scene/rdl2/Light.h>
#include <scene_rdl2/scene/rdl2/LightSet.h>
#include <scene_rdl2/scene/rdl2/Material.h>
#include <scene_rdl2/scene/rdl2/SceneContext.h>
#include <scene_rdl2/scene/rdl2/SceneObject.h>
#include <scene_rdl2/scene/rdl2/VolumeShader.h>

#include <cppunit/extensions/HelperMacros.h>

#include <string>
#include <sstream>

namespace scene_rdl2 {
namespace rdl2 {
namespace unittest {

void
TestLayer::setUp()
{
    mContext.reset(new SceneContext);
}

void
TestLayer::tearDown()
{
}

void
TestLayer::testSerialize()
{
    // Create some geometries and materials.
    Geometry* teapot1 = mContext->createSceneObject("FakeTeapot", "/seq/shot/teapot1")->asA<Geometry>();
    Material* material1 = mContext->createSceneObject("FakeMaterial", "/seq/shot/material1")->asA<Material>();
    Material* material2 = mContext->createSceneObject("FakeMaterial", "/seq/shot/material2")->asA<Material>();
    Displacement* displacement1 = mContext->createSceneObject("FakeDisplacement", "/seq/shot/displacement1")->asA<Displacement>();
    VolumeShader* volumeShader1 = mContext->createSceneObject("FakeVolumeShader", "/seq/shot/volumeShader1")->asA<VolumeShader>();

    // Create some lights.
    Light* key = mContext->createSceneObject("FakeLight", "/seq/shot/key")->asA<Light>();
    Light* fill = mContext->createSceneObject("FakeLight", "/seq/shot/fill")->asA<Light>();

    // Create a light set.
    LightSet* lights1 = mContext->createSceneObject("LightSet", "/seq/shot/keyfill")->asA<LightSet>();
    lights1->beginUpdate();
    lights1->add(key);
    lights1->add(fill);
    lights1->endUpdate();

    // Create a layer.
    Layer* layer = mContext->createSceneObject("Layer", "/seq/shot/layer")->asA<Layer>();

    // Make some assignments in the layer, verify they get new IDs.
    layer->beginUpdate();
    CPPUNIT_ASSERT(layer->assign(teapot1, "lid", material1, lights1, displacement1, volumeShader1) == 0);
    layer->endUpdate();

    // Serialize
    BinaryWriter writer(*mContext);
    std::stringstream ss;
    writer.toStream(ss);

    // Create a copy of the context
    SceneContext context;
    BinaryReader reader(context);
    reader.fromStream(ss);
    context.commitAllChanges();

    // Perform an update of the layer
    mContext->commitAllChanges();
    layer->beginUpdate();
    CPPUNIT_ASSERT(layer->assign(teapot1, "lid", material2, lights1, displacement1, volumeShader1) == 0);
    layer->endUpdate();

    // Serialize with delta encoding on
    BinaryWriter writer2(*mContext);
    writer2.setDeltaEncoding(true);
    std::stringstream ss2;
    writer2.toStream(ss2);

    // Deserialize to the copy
    BinaryReader reader2(context);
    reader2.fromStream(ss2);

    // Get the layer from the copy
    layer = context.getSceneObject("/seq/shot/layer")->asA<Layer>();
    teapot1 = context.getSceneObject("/seq/shot/teapot1")->asA<Geometry>();
    material2 = context.getSceneObject("/seq/shot/material2")->asA<Material>();

    // Read the data and check if the change was applied
    Layer::GeometryToRootShadersMap gts;
    layer->getAllGeometryToRootShaders(gts);
    CPPUNIT_ASSERT(gts.size() == 1);
    CPPUNIT_ASSERT(gts.count(teapot1) == 1);
    CPPUNIT_ASSERT(gts[teapot1].count(material2) == 1);
}

void
TestLayer::testAssignAndLookup()
{
    // Create some geometries and materials.
    Geometry* teapot1 = mContext->createSceneObject("FakeTeapot", "/seq/shot/teapot1")->asA<Geometry>();
    Geometry* teapot2 = mContext->createSceneObject("FakeTeapot", "/seq/shot/teapot2")->asA<Geometry>();
    Material* material1 = mContext->createSceneObject("FakeMaterial", "/seq/shot/material1")->asA<Material>();
    Material* material2 = mContext->createSceneObject("FakeMaterial", "/seq/shot/material2")->asA<Material>();
    Displacement* displacement1 = mContext->createSceneObject("FakeDisplacement", "/seq/shot/displacement1")->asA<Displacement>();
    Displacement* displacement2 = mContext->createSceneObject("FakeDisplacement", "/seq/shot/displacement2")->asA<Displacement>();

    // Create some lights.
    Light* key = mContext->createSceneObject("FakeLight", "/seq/shot/key")->asA<Light>();
    Light* fill = mContext->createSceneObject("FakeLight", "/seq/shot/fill")->asA<Light>();
    Light* rim = mContext->createSceneObject("FakeLight", "/seq/shot/rim")->asA<Light>();

    // Create some light sets.
    LightSet* lights1 = mContext->createSceneObject("LightSet", "/seq/shot/keyfill")->asA<LightSet>();
    lights1->beginUpdate();
    lights1->add(key);
    lights1->add(fill);
    lights1->endUpdate();
    LightSet* lights2 = mContext->createSceneObject("LightSet", "/seq/shot/fillrim")->asA<LightSet>();
    lights2->beginUpdate();
    lights2->add(fill);
    lights2->add(rim);
    lights2->endUpdate();

    // Create a layer.
    Layer* layer = mContext->createSceneObject("Layer", "/seq/shot/layer")->asA<Layer>();

    // Make some assignments in the layer, verify they get new IDs.
    layer->beginUpdate();
    CPPUNIT_ASSERT(layer->assign(teapot1, "lid", material1, lights1, displacement1, nullptr) == 0);
    CPPUNIT_ASSERT(layer->assign(teapot1, "body", material1, lights1, nullptr, nullptr) == 1);
    layer->endUpdate();

    // Can we look up those assignments?
    Layer::MaterialLightSetPair idPair0 = layer->lookup(0);
    CPPUNIT_ASSERT(idPair0.first == material1);
    CPPUNIT_ASSERT(idPair0.second == lights1);
    Layer::MaterialLightSetPair tuplePair0 = layer->lookup(teapot1, "lid");
    CPPUNIT_ASSERT(tuplePair0.first == material1);
    CPPUNIT_ASSERT(tuplePair0.second == lights1);
    Layer::MaterialLightSetPair idPair1 = layer->lookup(1);
    CPPUNIT_ASSERT(idPair1.first == material1);
    CPPUNIT_ASSERT(idPair1.second == lights1);
    Layer::MaterialLightSetPair tuplePair1 = layer->lookup(teapot1, "body");
    CPPUNIT_ASSERT(tuplePair1.first == material1);
    CPPUNIT_ASSERT(tuplePair1.second == lights1);

    // Bad lookups should throw.
    CPPUNIT_ASSERT_THROW(layer->lookup(2), except::IndexError);
    CPPUNIT_ASSERT_THROW(layer->lookup(teapot2, "lid"), except::IndexError);
    CPPUNIT_ASSERT_THROW(layer->lookup(teapot1, "spout"), except::IndexError);

    // Reassignments should reuse the same assignment ID.
    layer->beginUpdate();
    CPPUNIT_ASSERT(layer->assign(teapot1, "lid", material2, lights2, displacement2, nullptr) == 0);
    layer->endUpdate();
    Layer::MaterialLightSetPair newIdPair = layer->lookup(0);
    CPPUNIT_ASSERT(newIdPair.first == material2);
    CPPUNIT_ASSERT(newIdPair.second == lights2);
    Layer::MaterialLightSetPair newTuplePair = layer->lookup(teapot1, "lid");
    CPPUNIT_ASSERT(newTuplePair.first == material2);
    CPPUNIT_ASSERT(newTuplePair.second == lights2);

    // Same part names on different geometries should be fine.
    layer->beginUpdate();
    layer->assign(teapot1, "lid", material1, lights1, displacement1, nullptr);
    layer->assign(teapot1, "body", material1, lights1, nullptr, nullptr);
    layer->assign(teapot2, "lid", material2, lights2, displacement2, nullptr);
    layer->assign(teapot2, "body", material2, lights2, nullptr, nullptr);
    layer->endUpdate();
    Layer::MaterialLightSetPair teapot1Lid = layer->lookup(teapot1, "lid");
    CPPUNIT_ASSERT(teapot1Lid.first == material1);
    CPPUNIT_ASSERT(teapot1Lid.second == lights1);
    Layer::MaterialLightSetPair teapot1Body = layer->lookup(teapot1, "body");
    CPPUNIT_ASSERT(teapot1Body.first == material1);
    CPPUNIT_ASSERT(teapot1Body.second == lights1);
    Layer::MaterialLightSetPair teapot2Lid = layer->lookup(teapot2, "lid");
    CPPUNIT_ASSERT(teapot2Lid.first == material2);
    CPPUNIT_ASSERT(teapot2Lid.second == lights2);
    Layer::MaterialLightSetPair teapot2Body = layer->lookup(teapot2, "body");
    CPPUNIT_ASSERT(teapot2Body.first == material2);
    CPPUNIT_ASSERT(teapot2Body.second == lights2);
}

void
TestLayer::testDefaultAssignments()
{
    Geometry* teapot = mContext->createSceneObject("FakeTeapot", "/seq/shot/teapot")->asA<Geometry>();
    Material* material1 = mContext->createSceneObject("FakeMaterial", "/seq/shot/material1")->asA<Material>();
    Material* material2 = mContext->createSceneObject("FakeMaterial", "/seq/shot/material2")->asA<Material>();
    Displacement* displacement = mContext->createSceneObject("FakeDisplacement", "/seq/shot/displacement")->asA<Displacement>();
    Light* key = mContext->createSceneObject("FakeLight", "/seq/shot/key")->asA<Light>();

    LightSet* rig = mContext->createSceneObject("LightSet", "/seq/shot/rig")->asA<LightSet>();
    rig->beginUpdate();
    rig->add(key);
    rig->endUpdate();

    // Create a layer.
    Layer* layer = mContext->createSceneObject("Layer", "/seq/shot/layer")->asA<Layer>();

    // Make some assignments, but not a default one.
    layer->beginUpdate();
    layer->assign(teapot, "lid", material1, rig, displacement, nullptr);
    layer->assign(teapot, "body", material1, rig, nullptr, nullptr);
    layer->assign(teapot, "spout", material1, rig, displacement, nullptr);
    layer->endUpdate();

    // Look up a valid and invalid part. Ensure we get "no assignment".
    CPPUNIT_ASSERT_NO_THROW(
        Layer::MaterialLightSetPair assignment = layer->lookup(teapot, "body");
        CPPUNIT_ASSERT(assignment.first == static_cast<RootShader*>(material1));
        CPPUNIT_ASSERT(assignment.second == rig);
    );
    CPPUNIT_ASSERT_THROW(layer->lookup(teapot, "handle"), except::IndexError);

    // Add a default part assignment.
    layer->beginUpdate();
    layer->assign(teapot, "", material2, rig, displacement, nullptr);
    layer->endUpdate();

    // Verify that both lookups succeed.
    CPPUNIT_ASSERT_NO_THROW(
        Layer::MaterialLightSetPair assignment = layer->lookup(teapot, "body");
        CPPUNIT_ASSERT(assignment.first == static_cast<RootShader*>(material1));
        CPPUNIT_ASSERT(assignment.second == rig);
    );
    CPPUNIT_ASSERT_NO_THROW(
        Layer::MaterialLightSetPair assignment = layer->lookup(teapot, "handle");
        CPPUNIT_ASSERT(assignment.first == static_cast<RootShader*>(material2));
        CPPUNIT_ASSERT(assignment.second == rig);
    );
}

void
TestLayer::testClearLayer()
{
    Geometry* teapot = mContext->createSceneObject("FakeTeapot", "/seq/shot/teapot")->asA<Geometry>();
    Material* material = mContext->createSceneObject("FakeMaterial", "/seq/shot/material")->asA<Material>();
    Light* key = mContext->createSceneObject("FakeLight", "/seq/shot/key")->asA<Light>();
    Displacement* displacement = mContext->createSceneObject("FakeDisplacement", "/seq/shot/displacement")->asA<Displacement>();

    LightSet* rig = mContext->createSceneObject("LightSet", "/seq/shot/rig")->asA<LightSet>();
    rig->beginUpdate();
    rig->add(key);
    rig->endUpdate();

    // Create a layer.
    Layer* layer = mContext->createSceneObject("Layer", "/seq/shot/layer")->asA<Layer>();

    // Make some assignments, verify the assignment IDs.
    layer->beginUpdate();
    CPPUNIT_ASSERT(layer->assign(teapot, "lid", material, rig, displacement, nullptr) == 0);
    CPPUNIT_ASSERT(layer->assign(teapot, "body", material, rig, nullptr, nullptr) == 1);
    CPPUNIT_ASSERT(layer->assign(teapot, "spout", material, rig, displacement, nullptr) == 2);
    layer->endUpdate();

    // Clear the layer, verify that assignment IDs reset to 0.
    layer->beginUpdate();
    CPPUNIT_ASSERT_NO_THROW(layer->clear());
    CPPUNIT_ASSERT(layer->assign(teapot, "handle", material, rig, displacement, nullptr) == 0);
    layer->endUpdate();

    // Clearing the layer outside an update should throw.
    CPPUNIT_ASSERT_THROW(layer->clear(), except::RuntimeError);
}

void
TestLayer::testIterators()
{
    std::vector<std::string> sv = {"alpha",
                                   "beta",
                                   "alpha",
                                   "alpha",
                                   "gamma",
                                   "delta" };

    // This is just a raw by-value test, since all of the layer iterators work
    // on pointers.
    typedef FilterIndexIterator<decltype(sv), IndexIterator> SVIter;
    SVIter svFirst(IndexIterator(0),
                   IndexIterator(0), IndexIterator(sv.size()),
                   sv,
                   "alpha");
    SVIter svLast(IndexIterator(sv.size()),
                  IndexIterator(0), IndexIterator(sv.size()),
                  sv,
                  "alpha");

    // We should have three values.
    CPPUNIT_ASSERT(std::distance(svFirst, svLast) == 3);

    for ( ; svFirst != svLast; ++svFirst) {
        CPPUNIT_ASSERT(sv[*svFirst] == "alpha");
    }

    // Create some geometries and materials.
    Geometry* teapot1 = mContext->createSceneObject("FakeTeapot", "/seq/shot/teapot1")->asA<Geometry>();
    Geometry* teapot2 = mContext->createSceneObject("FakeTeapot", "/seq/shot/teapot2")->asA<Geometry>();
    Material* material1 = mContext->createSceneObject("FakeMaterial", "/seq/shot/material1")->asA<Material>();
    Material* material2 = mContext->createSceneObject("FakeMaterial", "/seq/shot/material2")->asA<Material>();
    Displacement* displacement1 = mContext->createSceneObject("FakeDisplacement", "/seq/shot/displacement1")->asA<Displacement>();
    Displacement* displacement2 = mContext->createSceneObject("FakeDisplacement", "/seq/shot/displacement2")->asA<Displacement>();
    VolumeShader* volumeShader1 = mContext->createSceneObject("FakeVolumeShader", "/seq/shot/volumeShader1")->asA<VolumeShader>();
    VolumeShader* volumeShader2 = mContext->createSceneObject("FakeVolumeShader", "/seq/shot/volumeShader2")->asA<VolumeShader>();

    // Create some lights.
    Light* key = mContext->createSceneObject("FakeLight", "/seq/shot/key")->asA<Light>();
    Light* fill = mContext->createSceneObject("FakeLight", "/seq/shot/fill")->asA<Light>();
    Light* rim = mContext->createSceneObject("FakeLight", "/seq/shot/rim")->asA<Light>();

    // Create some light sets.
    LightSet* lights1 = mContext->createSceneObject("LightSet", "/seq/shot/keyfill")->asA<LightSet>();
    lights1->beginUpdate();
    lights1->add(key);
    lights1->add(fill);
    lights1->endUpdate();
    LightSet* lights2 = mContext->createSceneObject("LightSet", "/seq/shot/fillrim")->asA<LightSet>();
    lights2->beginUpdate();
    lights2->add(fill);
    lights2->add(rim);
    lights2->endUpdate();

    // Create a layer.
    Layer* layer = mContext->createSceneObject("Layer", "/seq/shot/layer")->asA<Layer>();

    // Make some assignments in the layer.
    layer->beginUpdate();
    layer->assign(teapot1, "lid", material1, lights1, displacement1, volumeShader1);
    layer->assign(teapot1, "body", material1, lights1, nullptr, nullptr);
    layer->assign(teapot2, "lid", material2, lights2, displacement2, volumeShader2);
    layer->assign(teapot2, "body", material2, lights2, nullptr, nullptr);
    layer->endUpdate();

    // We have two assignments with teapot1
    CPPUNIT_ASSERT(std::distance(layer->begin(teapot1),
                                 layer->end(teapot1)) == 2);
    for (auto it = layer->begin(teapot1); it != layer->end(teapot1); ++it) {
        const auto p = layer->lookupGeomAndPart(*it);
        CPPUNIT_ASSERT(p.first == teapot1);
    }

    // We have two assignments with teapot2
    CPPUNIT_ASSERT(std::distance(layer->begin(teapot2),
                                 layer->end(teapot2)) == 2);
    for (auto it = layer->begin(teapot2); it != layer->end(teapot2); ++it) {
        const auto p = layer->lookupGeomAndPart(*it);
        CPPUNIT_ASSERT(p.first == teapot2);
    }

    // We have two assignments with material1
    CPPUNIT_ASSERT(std::distance(layer->begin(material1),
                                 layer->end(material1)) == 2);
    for (auto it = layer->begin(material1); it != layer->end(material1); ++it) {
        const auto p = layer->lookup(*it);
        CPPUNIT_ASSERT(p.first == material1);
    }

    // We have two assignments with material2
    CPPUNIT_ASSERT(std::distance(layer->begin(material2),
                                 layer->end(material2)) == 2);
    for (auto it = layer->begin(material2); it != layer->end(material2); ++it) {
        const auto p = layer->lookup(*it);
        CPPUNIT_ASSERT(p.first == material2);
    }

    // We have two assignments with lights1
    CPPUNIT_ASSERT(std::distance(layer->begin(lights1),
                                 layer->end(lights1)) == 2);
    for (auto it = layer->begin(lights1); it != layer->end(lights1); ++it) {
        const auto p = layer->lookup(*it);
        CPPUNIT_ASSERT(p.second == lights1);
    }

    // We have two assignments with lights2
    CPPUNIT_ASSERT(std::distance(layer->begin(lights2),
                                 layer->end(lights2)) == 2);
    for (auto it = layer->begin(lights2); it != layer->end(lights2); ++it) {
        const auto p = layer->lookup(*it);
        CPPUNIT_ASSERT(p.second == lights2);
    }

    // We have one assignments with displacement1
    CPPUNIT_ASSERT(std::distance(layer->begin(displacement1),
                                 layer->end(displacement1)) == 1);
    for (auto it = layer->begin(displacement1); it != layer->end(displacement1); ++it) {
        const auto p = layer->lookupDisplacement(*it);
        CPPUNIT_ASSERT(p == displacement1);
    }

    // We have one assignment with displacement2
    CPPUNIT_ASSERT(std::distance(layer->begin(displacement2),
                                 layer->end(displacement2)) == 1);
    for (auto it = layer->begin(displacement2); it != layer->end(displacement2); ++it) {
        const auto p = layer->lookupDisplacement(*it);
        CPPUNIT_ASSERT(p == displacement2);
    }

    // We have one assignments with volumeShader1
    CPPUNIT_ASSERT(std::distance(layer->begin(volumeShader1),
                                 layer->end(volumeShader1)) == 1);
    for (auto it = layer->begin(volumeShader1); it != layer->end(volumeShader1); ++it) {
        const auto p = layer->lookupVolumeShader(*it);
        CPPUNIT_ASSERT(p == volumeShader1);
    }

    // We have one assignment with volumeShader2
    CPPUNIT_ASSERT(std::distance(layer->begin(volumeShader2),
                                 layer->end(volumeShader2)) == 1);
    for (auto it = layer->begin(volumeShader2); it != layer->end(volumeShader2); ++it) {
        const auto p = layer->lookupVolumeShader(*it);
        CPPUNIT_ASSERT(p == volumeShader2);
    }
}

void
TestLayer::testContextLookup()
{
    // Create some geometries and materials.
    Geometry* teapot1 = mContext->createSceneObject("FakeTeapot", "/seq/shot/teapot1")->asA<Geometry>();
    Geometry* teapot2 = mContext->createSceneObject("FakeTeapot", "/seq/shot/teapot2")->asA<Geometry>();
    Geometry* teapot3 = mContext->createSceneObject("FakeTeapot", "/seq/shot/teapot3")->asA<Geometry>();
    Material* material1 = mContext->createSceneObject("FakeMaterial", "/seq/shot/material1")->asA<Material>();
    Material* material2 = mContext->createSceneObject("FakeMaterial", "/seq/shot/material2")->asA<Material>();
    Displacement* displacement1 = mContext->createSceneObject("FakeDisplacement", "/seq/shot/displacement1")->asA<Displacement>();
    Displacement* displacement2 = mContext->createSceneObject("FakeDisplacement", "/seq/shot/displacement2")->asA<Displacement>();
    VolumeShader* volumeShader1 = mContext->createSceneObject("FakeVolumeShader", "/seq/shot/volumeShader1")->asA<VolumeShader>();
    VolumeShader* volumeShader2 = mContext->createSceneObject("FakeVolumeShader", "/seq/shot/volumeShader2")->asA<VolumeShader>();

    // Create some lights.
    Light* key = mContext->createSceneObject("FakeLight", "/seq/shot/key")->asA<Light>();
    Light* fill = mContext->createSceneObject("FakeLight", "/seq/shot/fill")->asA<Light>();
    Light* rim = mContext->createSceneObject("FakeLight", "/seq/shot/rim")->asA<Light>();

    // Create some geometry sets.
    GeometrySet* geomset0 = mContext->createSceneObject("GeometrySet", "/seq/shot/asset0")->asA<GeometrySet>();
    GeometrySet* geomset1 = mContext->createSceneObject("GeometrySet", "/seq/shot/asset1")->asA<GeometrySet>();

    // Create some layers.
    Layer* layer0 = mContext->createSceneObject("Layer", "/seq/shot/layer0")->asA<Layer>();
    Layer* layer1 = mContext->createSceneObject("Layer", "/seq/shot/layer1")->asA<Layer>();

    // Create some light sets.
    LightSet* lights1 = mContext->createSceneObject("LightSet", "/seq/shot/keyfill")->asA<LightSet>();
    lights1->beginUpdate();
    lights1->add(key);
    lights1->add(fill);
    lights1->endUpdate();
    LightSet* lights2 = mContext->createSceneObject("LightSet", "/seq/shot/fillrim")->asA<LightSet>();
    lights2->beginUpdate();
    lights2->add(fill);
    lights2->add(rim);
    lights2->endUpdate();

    layer0->beginUpdate();
    layer0->assign(teapot1, "lid", material1, lights1, displacement1, volumeShader1);
    layer0->assign(teapot1, "body", material1, lights1, nullptr, nullptr);
    layer0->endUpdate();

    layer1->beginUpdate();
    layer1->assign(teapot1, "lid", material1, lights1, displacement1, volumeShader1);
    layer1->assign(teapot1, "body", material1, lights1, nullptr, nullptr);
    layer1->assign(teapot2, "lid", material2, lights2, displacement2, volumeShader2);
    layer1->assign(teapot2, "body", material2, lights2, nullptr, nullptr);
    layer1->endUpdate();

    // No geometries should be in the set.
    CPPUNIT_ASSERT(geomset0->getGeometries().size() == 0);
    CPPUNIT_ASSERT(geomset1->getGeometries().size() == 0);

    // Add some geometries.
    geomset0->beginUpdate();
    geomset0->add(teapot1); // Used in layer0 and layer1
    geomset0->add(teapot2); // Used in layer1
    geomset0->endUpdate();

    geomset1->beginUpdate();
    geomset1->add(teapot2); // Used in layer1
    geomset1->add(teapot3); // Not used in a layer.
    geomset1->endUpdate();

    CPPUNIT_ASSERT(geomset0->getGeometries().size() == 2);
    CPPUNIT_ASSERT(geomset1->getGeometries().size() == 2);

    // Get geometry sets with geometry in layer0.
    const auto layer0geom = mContext->getGeometrySetsForLayer(layer0);

    // Get geometry sets with geometry in layer1.
    const auto layer1geom = mContext->getGeometrySetsForLayer(layer1);

    PRINT(layer0geom.size());
    PRINT(layer1geom.size());
    CPPUNIT_ASSERT(layer0geom.size() == 1);
    CPPUNIT_ASSERT(layer1geom.size() == 2);

    CPPUNIT_ASSERT(std::find(layer0geom.cbegin(), layer0geom.cend(), geomset0) != layer0geom.cend());
    CPPUNIT_ASSERT(std::find(layer0geom.cbegin(), layer0geom.cend(), geomset1) == layer0geom.cend());
    CPPUNIT_ASSERT(std::find(layer1geom.cbegin(), layer1geom.cend(), geomset0) != layer1geom.cend());
    CPPUNIT_ASSERT(std::find(layer1geom.cbegin(), layer1geom.cend(), geomset1) != layer1geom.cend());
}



} // namespace unittest
} // namespace rdl2
} // namespace scene_rdl2

