// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "TestSceneContext.h"

#include <scene_rdl2/scene/rdl2/AttributeKey.h>
#include <scene_rdl2/scene/rdl2/SceneContext.h>
#include <scene_rdl2/scene/rdl2/SceneClass.h>
#include <scene_rdl2/scene/rdl2/SceneObject.h>
#include <scene_rdl2/scene/rdl2/SceneVariables.h>
#include <scene_rdl2/scene/rdl2/Types.h>

#include <scene_rdl2/common/except/exceptions.h>
#include <scene_rdl2/common/math/Color.h>

#include <string>

namespace scene_rdl2 {
namespace rdl2 {
namespace unittest {

void
TestSceneContext::setUp()
{
}

void
TestSceneContext::tearDown()
{
}

void
TestSceneContext::testDsoPath()
{
    SceneContext context;

    CPPUNIT_ASSERT_NO_THROW(context.setDsoPath("one:two:three"));
    CPPUNIT_ASSERT(context.getDsoPath() == "one:two:three");

    const SceneContext* constContext = &context;
    CPPUNIT_ASSERT(constContext->getDsoPath() == "one:two:three");
}

void
TestSceneContext::testCreateSceneClass()
{
    SceneContext context;
    CPPUNIT_ASSERT_NO_THROW(context.createSceneClass("ExampleObject"));

    // Creating the same SceneClass again shouldn't be a problem.
    CPPUNIT_ASSERT_NO_THROW(context.createSceneClass("ExampleObject"));
}

void
TestSceneContext::testGetSceneClass()
{
    SceneContext context;
    context.createSceneClass("ExampleObject");

    CPPUNIT_ASSERT_NO_THROW(
        const SceneClass* sc = context.getSceneClass("ExampleObject");
        CPPUNIT_ASSERT(sc->getName() == "ExampleObject");
    );
    CPPUNIT_ASSERT_THROW(
        context.getSceneClass("NotASceneClass");
    , except::KeyError);
}

void
TestSceneContext::testSceneClassExists()
{
    SceneContext context;
    context.createSceneClass("ExampleObject");

    CPPUNIT_ASSERT(context.sceneClassExists("ExampleObject"));
    CPPUNIT_ASSERT(!context.sceneClassExists("NotASceneClass"));
}

void
TestSceneContext::testIterateSceneClasses()
{
    SceneContext context;
    context.createSceneClass("ExampleObject");

    // The map is unordered, so this is a bit trickier.
    bool sawSceneVars = false;
    bool sawExampleObject = false;

    for (SceneContext::SceneClassConstIterator iter = context.beginSceneClass();
            iter != context.endSceneClass(); ++iter) {
        if (iter->first == "SceneVariables") {
            CPPUNIT_ASSERT(!sawSceneVars);
            sawSceneVars = true;
            CPPUNIT_ASSERT(iter->second->getName() == "SceneVariables");
        } else if (iter->first == "ExampleObject") {
            CPPUNIT_ASSERT(!sawExampleObject);
            sawExampleObject = true;
            CPPUNIT_ASSERT(iter->second->getName() == "ExampleObject");
        }
    }

    CPPUNIT_ASSERT(sawSceneVars);
    CPPUNIT_ASSERT(sawExampleObject);
}

void
TestSceneContext::testCreateSceneObject()
{
    SceneContext context;

    SceneObject* pizza = nullptr;
    CPPUNIT_ASSERT_NO_THROW(
        SceneObject* obj = context.createSceneObject("ExampleObject", "/seq/shot/pizza");
        CPPUNIT_ASSERT(obj->getName() == "/seq/shot/pizza");
        pizza = obj;
    );

    // Trying to create the object again should return the existing object.
    CPPUNIT_ASSERT_NO_THROW(
        SceneObject* obj = context.createSceneObject("ExampleObject", "/seq/shot/pizza");
        CPPUNIT_ASSERT(obj == pizza);
    );
}

void
TestSceneContext::testGetSceneObject()
{
    SceneContext context;
    SceneObject* pizza = context.createSceneObject("ExampleObject", "/seq/shot/pizza");

    CPPUNIT_ASSERT_NO_THROW(
        SceneObject* obj = context.getSceneObject("/seq/shot/pizza");
        CPPUNIT_ASSERT(obj == pizza);
    );

    CPPUNIT_ASSERT_NO_THROW(
        const SceneObject* constObj = context.getSceneObject("/seq/shot/pizza");
        CPPUNIT_ASSERT(constObj->getName() == "/seq/shot/pizza");
    );

    CPPUNIT_ASSERT_THROW(
        context.getSceneObject("/seq/shot/not_a_pizza");
    , except::KeyError);
}

void
TestSceneContext::testSceneObjectExists()
{
    SceneContext context;
    context.createSceneObject("ExampleObject", "/seq/shot/pizza");

    CPPUNIT_ASSERT(context.sceneObjectExists("/seq/shot/pizza"));
    CPPUNIT_ASSERT(!context.sceneObjectExists("/seq/shot/not_a_pizza"));
}

void
TestSceneContext::testIterateSceneObjects()
{
    SceneContext context;
    context.createSceneObject("ExampleObject", "/seq/shot/pizza");
    context.createSceneObject("ExampleObject", "/seq/shot/cookie");

    // The map is unordered, so this is a bit trickier.
    bool sawSceneVars = false;
    bool sawPizza = false;
    bool sawCookie = false;

    SceneContext::SceneObjectConstIterator iter = context.beginSceneObject();
    if (iter->first == "__SceneVariables__") {
        CPPUNIT_ASSERT(!sawSceneVars);
        sawSceneVars = true;
        CPPUNIT_ASSERT(iter->second->getName() == "__SceneVariables__");
    } else if (iter->first == "/seq/shot/pizza") {
        CPPUNIT_ASSERT(!sawPizza);
        sawPizza = true;
        CPPUNIT_ASSERT(iter->second->getName() == "/seq/shot/pizza");
    } else if (iter->first == "/seq/shot/cookie") {
        CPPUNIT_ASSERT(!sawCookie);
        sawCookie = true;
        CPPUNIT_ASSERT(iter->second->getName() == "/seq/shot/cookie");
    }

    ++iter;
    if (iter->first == "__SceneVariables__") {
        CPPUNIT_ASSERT(!sawSceneVars);
        sawSceneVars = true;
        CPPUNIT_ASSERT(iter->second->getName() == "__SceneVariables__");
    } else if (iter->first == "/seq/shot/pizza") {
        CPPUNIT_ASSERT(!sawPizza);
        sawPizza = true;
        CPPUNIT_ASSERT(iter->second->getName() == "/seq/shot/pizza");
    } else if (iter->first == "/seq/shot/cookie") {
        CPPUNIT_ASSERT(!sawCookie);
        sawCookie = true;
        CPPUNIT_ASSERT(iter->second->getName() == "/seq/shot/cookie");
    }

    ++iter;
    if (iter->first == "__SceneVariables__") {
        CPPUNIT_ASSERT(!sawSceneVars);
        sawSceneVars = true;
        CPPUNIT_ASSERT(iter->second->getName() == "__SceneVariables__");
    } else if (iter->first == "/seq/shot/pizza") {
        CPPUNIT_ASSERT(!sawPizza);
        sawPizza = true;
        CPPUNIT_ASSERT(iter->second->getName() == "/seq/shot/pizza");
    } else if (iter->first == "/seq/shot/cookie") {
        CPPUNIT_ASSERT(!sawCookie);
        sawCookie = true;
        CPPUNIT_ASSERT(iter->second->getName() == "/seq/shot/cookie");
    }

    ++iter;
    CPPUNIT_ASSERT(iter == context.endSceneObject());

    CPPUNIT_ASSERT(sawSceneVars);
    CPPUNIT_ASSERT(sawPizza);
    CPPUNIT_ASSERT(sawCookie);
}

void
TestSceneContext::testSetSceneObject()
{
    SceneContext context;
    context.createSceneObject("ExampleObject", "/seq/shot/pizza");
    const SceneClass* sc = context.getSceneClass("ExampleObject");
    AttributeKey<Int> awesomenessKey = sc->getAttributeKey<Int>("awesomeness");

    SceneObject* constObj = context.getSceneObject("/seq/shot/pizza");
    CPPUNIT_ASSERT(constObj->get(awesomenessKey) == Int(11));

    SceneObject* obj = context.getSceneObject("/seq/shot/pizza");
    obj->beginUpdate();
    obj->set(awesomenessKey, Int(42));
    obj->endUpdate();
    CPPUNIT_ASSERT(obj->get(awesomenessKey) == Int(42));
    CPPUNIT_ASSERT(constObj->get(awesomenessKey) == Int(42));
}

void
TestSceneContext::testLoadAllSceneClasses()
{
    SceneContext context;
    context.setProxyModeEnabled(true);
    CPPUNIT_ASSERT_NO_THROW(context.loadAllSceneClasses());

    bool sawDeclareAndCreateObject = false;
    bool sawDeclareAndDestroyObject = false;
    bool sawExampleObject = false;
    bool sawExtensiveObject = false;
    bool sawFakeLight = false;
    bool sawFakeMaterial = false;
    bool sawFakeTeapot = false;
    bool sawLibLadenCamera = false;
    bool sawLibLadenDisplayFilter = false;
    bool sawLibLadenEnvMap = false;
    bool sawLibLadenGeometry = false;
    bool sawLibLadenLight = false;
    bool sawLibLadenLightFilter = false;
    bool sawLibLadenMap = false;
    bool sawLibLadenNormalMap = false;
    bool sawLibLadenMaterial = false;
    bool sawThrowDuringConstruct = false;
    bool sawUpdateTracker = false;

    for (SceneContext::SceneClassConstIterator iter = context.beginSceneClass();
            iter != context.endSceneClass(); ++iter) {
        if (iter->first == "DeclareAndCreateObject") {
            sawDeclareAndCreateObject = true;
        } else if (iter->first == "DeclareAndDestroyObject") {
            sawDeclareAndDestroyObject = true;
        } else if (iter->first == "ExampleObject") {
            sawExampleObject = true;
        } else if (iter->first == "ExtensiveObject") {
            sawExtensiveObject = true;
        } else if (iter->first == "FakeLight") {
            sawFakeLight = true;
        } else if (iter->first == "FakeMaterial") {
            sawFakeMaterial = true;
        } else if (iter->first == "FakeTeapot") {
            sawFakeTeapot = true;
        } else if (iter->first == "LibLadenCamera") {
            sawLibLadenCamera = true;
        } else if (iter->first == "LibLadenDisplayFilter") {
            sawLibLadenDisplayFilter = true;
        } else if (iter->first == "LibLadenEnvMap") {
            sawLibLadenEnvMap = true;
        } else if (iter->first == "LibLadenGeometry") {
            sawLibLadenGeometry = true;
        } else if (iter->first == "LibLadenLight") {
            sawLibLadenLight = true;
        } else if (iter->first == "LibLadenLightFilter") {
            sawLibLadenLightFilter = true;
        } else if (iter->first == "LibLadenMap") {
            sawLibLadenMap = true;
        } else if (iter->first == "LibLadenNormalMap") {
            sawLibLadenNormalMap = true;
        } else if (iter->first == "LibLadenMaterial") {
            sawLibLadenMaterial = true;
        } else if (iter->first == "ThrowDuringConstruct") {
            sawThrowDuringConstruct = true;
        } else if (iter->first == "UpdateTracker") {
            sawUpdateTracker = true;
        }
    }

    CPPUNIT_ASSERT(sawDeclareAndCreateObject);
    CPPUNIT_ASSERT(sawDeclareAndDestroyObject);
    CPPUNIT_ASSERT(sawExampleObject);
    CPPUNIT_ASSERT(sawExtensiveObject);
    CPPUNIT_ASSERT(sawFakeLight);
    CPPUNIT_ASSERT(sawFakeMaterial);
    CPPUNIT_ASSERT(sawFakeTeapot);
    CPPUNIT_ASSERT(sawLibLadenCamera);
    CPPUNIT_ASSERT(sawLibLadenDisplayFilter);
    CPPUNIT_ASSERT(sawLibLadenEnvMap);
    CPPUNIT_ASSERT(sawLibLadenGeometry);
    CPPUNIT_ASSERT(sawLibLadenLight);
    CPPUNIT_ASSERT(sawLibLadenLightFilter);
    CPPUNIT_ASSERT(sawLibLadenMap);
    CPPUNIT_ASSERT(sawLibLadenNormalMap);
    CPPUNIT_ASSERT(sawLibLadenMaterial);
    CPPUNIT_ASSERT(sawThrowDuringConstruct);
    CPPUNIT_ASSERT(sawUpdateTracker);
}

void
TestSceneContext::testSceneVariables()
{
    SceneContext context;
    const SceneContext& constContext = context;

    // Check some default values.
    CPPUNIT_ASSERT(constContext.getSceneVariables().get(SceneVariables::sCheckpointActive) == false);
    CPPUNIT_ASSERT(constContext.getSceneVariables().get(SceneVariables::sCheckpointInterval) == 15.0f);
    CPPUNIT_ASSERT(constContext.getSceneVariables().get(SceneVariables::sCheckpointTimeCap) == 0.0f);
    CPPUNIT_ASSERT(constContext.getSceneVariables().get(SceneVariables::sResumableOutput) == false);
    CPPUNIT_ASSERT(constContext.getSceneVariables().get(SceneVariables::sResumeRender) == false);

    // Try changing them.
    context.getSceneVariables().beginUpdate();
    CPPUNIT_ASSERT_NO_THROW(
        context.getSceneVariables().set(SceneVariables::sCheckpointActive, true);
        context.getSceneVariables().set(SceneVariables::sCheckpointInterval, 5.0f);
        context.getSceneVariables().set(SceneVariables::sCheckpointTimeCap, 1.0f);
        context.getSceneVariables().set(SceneVariables::sResumableOutput, true);
        context.getSceneVariables().set(SceneVariables::sResumeRender, true);
    );
    context.getSceneVariables().endUpdate();

    // Check that they changed.
    CPPUNIT_ASSERT(constContext.getSceneVariables().get(SceneVariables::sCheckpointActive) == true);
    CPPUNIT_ASSERT(constContext.getSceneVariables().get(SceneVariables::sCheckpointInterval) == 5.0f);
    CPPUNIT_ASSERT(constContext.getSceneVariables().get(SceneVariables::sCheckpointTimeCap) == 1.0f);
    CPPUNIT_ASSERT(constContext.getSceneVariables().get(SceneVariables::sResumableOutput) == true);
    CPPUNIT_ASSERT(constContext.getSceneVariables().get(SceneVariables::sResumeRender) == true);

    // The SceneVariables object is a singleton, and we can't create another.
    SceneVariables* vars = static_cast<SceneVariables*>(
            context.createSceneObject("SceneVariables", "MoreVariables"));
    CPPUNIT_ASSERT(vars == &(context.getSceneVariables()));
}

void
TestSceneContext::testCreateClassFailure()
{
    SceneContext context;

    // Count the number of SceneClasses in the context before loading the
    // throwing DSO.
    int numBefore = 0;
    for (auto iter = context.beginSceneClass(); iter != context.endSceneClass(); ++iter) {
        ++numBefore;
    }

    // Load a DSO that throws in its declare() function.
    CPPUNIT_ASSERT_THROW(
        context.createSceneClass("ThrowDuringDeclare");
    , std::runtime_error);

    // Count the number of SceneClasses in the context after loading the
    // throwing DSO.
    int numAfter = 0;
    for (auto iter = context.beginSceneClass(); iter != context.endSceneClass(); ++iter) {
        ++numAfter;
    }

    CPPUNIT_ASSERT_EQUAL(numBefore, numAfter);
}

void
TestSceneContext::testCreateObjectFailure()
{
    SceneContext context;

    // Count the number of SceneObjects in the context before loading the
    // throwing DSO.
    int numBefore = 0;
    for (auto iter = context.beginSceneObject(); iter != context.endSceneObject(); ++iter) {
        ++numBefore;
    }

    // Load a DSO that throws in its constructor.
    CPPUNIT_ASSERT_THROW(
        context.createSceneObject("ThrowDuringConstruct", "/seq/shot/object");
    , std::runtime_error);

    // Count the number of SceneClasses in the context after loading the
    // throwing DSO.
    int numAfter = 0;
    for (auto iter = context.beginSceneObject(); iter != context.endSceneObject(); ++iter) {
        ++numAfter;
    }

    CPPUNIT_ASSERT_EQUAL(numBefore, numAfter);
}

} // namespace unittest
} // namespace rdl2
} // namespace scene_rdl2

