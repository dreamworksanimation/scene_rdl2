// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "TestSets.h"

#include <scene_rdl2/scene/rdl2/Geometry.h>
#include <scene_rdl2/scene/rdl2/GeometrySet.h>
#include <scene_rdl2/scene/rdl2/Light.h>
#include <scene_rdl2/scene/rdl2/LightSet.h>
#include <scene_rdl2/scene/rdl2/SceneContext.h>
#include <scene_rdl2/scene/rdl2/SceneObject.h>

#include <cppunit/extensions/HelperMacros.h>

#include <string>

namespace scene_rdl2 {
namespace rdl2 {
namespace unittest {

void
TestSets::setUp()
{
    mContext.reset(new SceneContext);
}

void
TestSets::tearDown()
{
}

void
TestSets::testAddGeometry()
{
    Geometry* teapot1 = mContext->createSceneObject("FakeTeapot", "/seq/shot/teapot1")->asA<Geometry>();
    Geometry* teapot2 = mContext->createSceneObject("FakeTeapot", "/seq/shot/teapot2")->asA<Geometry>();
    Geometry* teapot3 = mContext->createSceneObject("FakeTeapot", "/seq/shot/teapot3")->asA<Geometry>();
    GeometrySet* asset = mContext->createSceneObject("GeometrySet", "/seq/shot/asset")->asA<GeometrySet>();

    // No geometries should be in the set.
    CPPUNIT_ASSERT(asset->getGeometries().size() == 0);
    CPPUNIT_ASSERT(!asset->contains(teapot1));
    CPPUNIT_ASSERT(!asset->contains(teapot2));
    CPPUNIT_ASSERT(!asset->contains(teapot3));

    // Add some geometries.
    asset->beginUpdate();
    asset->add(teapot1);
    asset->add(teapot2);
    asset->endUpdate();
    CPPUNIT_ASSERT(asset->getGeometries().size() == 2);
    CPPUNIT_ASSERT(asset->contains(teapot1));
    CPPUNIT_ASSERT(asset->contains(teapot2));
    CPPUNIT_ASSERT(!asset->contains(teapot3));

    // Add an existing geometry.
    asset->beginUpdate();
    asset->add(teapot1);
    asset->endUpdate();
    CPPUNIT_ASSERT(asset->getGeometries().size() == 2);
    CPPUNIT_ASSERT(asset->contains(teapot1));
    CPPUNIT_ASSERT(asset->contains(teapot2));
    CPPUNIT_ASSERT(!asset->contains(teapot3));
}

void
TestSets::testRemoveGeometry()
{
    Geometry* teapot1 = mContext->createSceneObject("FakeTeapot", "/seq/shot/teapot1")->asA<Geometry>();
    Geometry* teapot2 = mContext->createSceneObject("FakeTeapot", "/seq/shot/teapot2")->asA<Geometry>();
    Geometry* teapot3 = mContext->createSceneObject("FakeTeapot", "/seq/shot/teapot3")->asA<Geometry>();
    Geometry* dummy = mContext->createSceneObject("FakeTeapot", "/seq/shot/dummy")->asA<Geometry>();
    GeometrySet* asset = mContext->createSceneObject("GeometrySet", "/seq/shot/asset")->asA<GeometrySet>();

    // Add some geometries.
    asset->beginUpdate();
    asset->add(teapot1);
    asset->add(teapot2);
    asset->add(teapot3);
    asset->endUpdate();

    // Verify initial state.
    CPPUNIT_ASSERT(asset->getGeometries().size() == 3);
    CPPUNIT_ASSERT(asset->contains(teapot1));
    CPPUNIT_ASSERT(asset->contains(teapot2));
    CPPUNIT_ASSERT(asset->contains(teapot3));
    CPPUNIT_ASSERT(!(asset->contains(dummy)));

    // Remove a geometry.
    asset->beginUpdate();
    asset->remove(teapot1);
    asset->endUpdate();
    CPPUNIT_ASSERT(asset->getGeometries().size() == 2);
    CPPUNIT_ASSERT(!asset->contains(teapot1));
    CPPUNIT_ASSERT(asset->contains(teapot2));
    CPPUNIT_ASSERT(asset->contains(teapot3));
    CPPUNIT_ASSERT(!asset->contains(dummy));

    // Remove a geometry that's not a member.
    asset->beginUpdate();
    asset->remove(dummy);
    asset->endUpdate();
    CPPUNIT_ASSERT(asset->getGeometries().size() == 2);
    CPPUNIT_ASSERT(!asset->contains(teapot1));
    CPPUNIT_ASSERT(asset->contains(teapot2));
    CPPUNIT_ASSERT(asset->contains(teapot3));
    CPPUNIT_ASSERT(!asset->contains(dummy));
}

void
TestSets::testClearGeometry()
{
    Geometry* teapot1 = mContext->createSceneObject("FakeTeapot", "/seq/shot/teapot1")->asA<Geometry>();
    Geometry* teapot2 = mContext->createSceneObject("FakeTeapot", "/seq/shot/teapot2")->asA<Geometry>();
    Geometry* teapot3 = mContext->createSceneObject("FakeTeapot", "/seq/shot/teapot3")->asA<Geometry>();
    GeometrySet* asset = mContext->createSceneObject("GeometrySet", "/seq/shot/asset")->asA<GeometrySet>();

    // Add some geometries.
    asset->beginUpdate();
    asset->add(teapot1);
    asset->add(teapot2);
    asset->add(teapot3);
    asset->endUpdate();

    // Verify initial state.
    CPPUNIT_ASSERT(asset->getGeometries().size() == 3);

    // Clearing the set should empty it.
    asset->beginUpdate();
    CPPUNIT_ASSERT_NO_THROW(asset->clear());
    asset->endUpdate();
    CPPUNIT_ASSERT(asset->getGeometries().size() == 0);

    // Attempting to clear outside an update should throw.
    CPPUNIT_ASSERT_THROW(asset->clear(), except::RuntimeError);
}

void
TestSets::testStaticGeometry()
{
    Geometry* teapot1 = mContext->createSceneObject("FakeTeapot", "/seq/shot/teapot1")->asA<Geometry>();
    Geometry* teapot2 = mContext->createSceneObject("FakeTeapot", "/seq/shot/teapot2")->asA<Geometry>();
    Geometry* teapot3 = mContext->createSceneObject("FakeTeapot", "/seq/shot/teapot3")->asA<Geometry>();

    GeometrySet* asset = mContext->createSceneObject("GeometrySet", "/seq/shot/asset")->asA<GeometrySet>();

    {
        SceneObject::UpdateGuard guard1(teapot1);
        teapot1->set(Geometry::sStaticKey, true);

        SceneObject::UpdateGuard guard2(teapot2);
        teapot2->set(Geometry::sStaticKey, true);

        SceneObject::UpdateGuard guard3(teapot3);
        teapot3->set(Geometry::sStaticKey, true);

        SceneObject::UpdateGuard guard4(asset);
        asset->add(teapot1);
        asset->add(teapot2);
        asset->add(teapot3);
    }

    CPPUNIT_ASSERT_EQUAL(true, asset->isStatic());

    {
        SceneObject::UpdateGuard guard5(teapot2);
        teapot2->set(Geometry::sStaticKey, false);
    }

    CPPUNIT_ASSERT_EQUAL(false, asset->isStatic());
}

void
TestSets::testAddLight()
{
    Light* key = mContext->createSceneObject("FakeLight", "/seq/shot/key")->asA<Light>();
    Light* fill = mContext->createSceneObject("FakeLight", "/seq/shot/fill")->asA<Light>();
    Light* rim = mContext->createSceneObject("FakeLight", "/seq/shot/rim")->asA<Light>();
    LightSet* rig = mContext->createSceneObject("LightSet", "/seq/shot/LT_RIG")->asA<LightSet>();

    // No lights should be in the set.
    CPPUNIT_ASSERT(rig->getLights().size() == 0);
    CPPUNIT_ASSERT(!rig->contains(key));
    CPPUNIT_ASSERT(!rig->contains(fill));
    CPPUNIT_ASSERT(!rig->contains(rim));

    // Add some lights.
    rig->beginUpdate();
    rig->add(key);
    rig->add(fill);
    rig->endUpdate();
    CPPUNIT_ASSERT(rig->getLights().size() == 2);
    CPPUNIT_ASSERT(rig->contains(key));
    CPPUNIT_ASSERT(rig->contains(fill));
    CPPUNIT_ASSERT(!rig->contains(rim));

    // Add an existing light.
    rig->beginUpdate();
    rig->add(key);
    rig->endUpdate();
    CPPUNIT_ASSERT(rig->getLights().size() == 2);
    CPPUNIT_ASSERT(rig->contains(key));
    CPPUNIT_ASSERT(rig->contains(fill));
    CPPUNIT_ASSERT(!rig->contains(rim));
}

void
TestSets::testRemoveLight()
{
    Light* key = mContext->createSceneObject("FakeLight", "/seq/shot/key")->asA<Light>();
    Light* fill = mContext->createSceneObject("FakeLight", "/seq/shot/fill")->asA<Light>();
    Light* rim = mContext->createSceneObject("FakeLight", "/seq/shot/rim")->asA<Light>();
    Light* dummy = mContext->createSceneObject("FakeLight", "/seq/shot/dummy")->asA<Light>();
    LightSet* rig = mContext->createSceneObject("LightSet", "/seq/shot/LT_RIG")->asA<LightSet>();

    // Add some lights.
    rig->beginUpdate();
    rig->add(key);
    rig->add(fill);
    rig->add(rim);
    rig->endUpdate();

    // Verify initial state.
    CPPUNIT_ASSERT(rig->getLights().size() == 3);
    CPPUNIT_ASSERT(rig->contains(key));
    CPPUNIT_ASSERT(rig->contains(fill));
    CPPUNIT_ASSERT(rig->contains(rim));
    CPPUNIT_ASSERT(!(rig->contains(dummy)));

    // Remove a light.
    rig->beginUpdate();
    rig->remove(key);
    rig->endUpdate();
    CPPUNIT_ASSERT(rig->getLights().size() == 2);
    CPPUNIT_ASSERT(!rig->contains(key));
    CPPUNIT_ASSERT(rig->contains(fill));
    CPPUNIT_ASSERT(rig->contains(rim));
    CPPUNIT_ASSERT(!rig->contains(dummy));

    // Remove a light that's not a member.
    rig->beginUpdate();
    rig->remove(dummy);
    rig->endUpdate();
    CPPUNIT_ASSERT(rig->getLights().size() == 2);
    CPPUNIT_ASSERT(!rig->contains(key));
    CPPUNIT_ASSERT(rig->contains(fill));
    CPPUNIT_ASSERT(rig->contains(rim));
    CPPUNIT_ASSERT(!rig->contains(dummy));
}

void
TestSets::testClearLight()
{
    Light* key = mContext->createSceneObject("FakeLight", "/seq/shot/key")->asA<Light>();
    Light* fill = mContext->createSceneObject("FakeLight", "/seq/shot/fill")->asA<Light>();
    Light* rim = mContext->createSceneObject("FakeLight", "/seq/shot/rim")->asA<Light>();
    LightSet* rig = mContext->createSceneObject("LightSet", "/seq/shot/LT_RIG")->asA<LightSet>();

    // Add some lights.
    rig->beginUpdate();
    rig->add(key);
    rig->add(fill);
    rig->add(rim);
    rig->endUpdate();

    // Verify initial state.
    CPPUNIT_ASSERT(rig->getLights().size() == 3);

    // Clearing the set should empty it.
    rig->beginUpdate();
    CPPUNIT_ASSERT_NO_THROW(rig->clear());
    rig->endUpdate();
    CPPUNIT_ASSERT(rig->getLights().size() == 0);

    // Attempting to clear outside an update should throw.
    CPPUNIT_ASSERT_THROW(rig->clear(), except::RuntimeError);
}

} // namespace unittest
} // namespace rdl2
} // namespace scene_rdl2

