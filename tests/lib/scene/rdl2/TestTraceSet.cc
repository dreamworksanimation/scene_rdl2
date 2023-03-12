// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "TestTraceSet.h"

#include <scene_rdl2/scene/rdl2/BinaryReader.h>
#include <scene_rdl2/scene/rdl2/BinaryWriter.h>
#include <scene_rdl2/scene/rdl2/Geometry.h>
#include <scene_rdl2/scene/rdl2/TraceSet.h>
#include <scene_rdl2/scene/rdl2/SceneContext.h>
#include <scene_rdl2/scene/rdl2/SceneObject.h>

#include <cppunit/extensions/HelperMacros.h>

#include <string>
#include <sstream>

namespace scene_rdl2 {
namespace rdl2 {
namespace unittest {


void
TestTraceSet::setUp()
{
    mContext.reset(new SceneContext);
}

void
TestTraceSet::tearDown()
{
}

void
TestTraceSet::testSerialize()
{
    // Create some geometries.
    Geometry* teapot1 = mContext->createSceneObject("FakeTeapot", "/seq/shot/teapot1")->asA<Geometry>();
    Geometry* teapot2 = mContext->createSceneObject("FakeTeapot", "/seq/shot/teapot2")->asA<Geometry>();

    // Create a trace set
    TraceSet* traceSet = mContext->createSceneObject("TraceSet", "/seq/shot/traceset")->asA<TraceSet>();

    // Make some assignments in the trace set, verify they get new IDs.
    traceSet->beginUpdate();
    CPPUNIT_ASSERT(traceSet->assign(teapot1, "lid") == 0);
    CPPUNIT_ASSERT(traceSet->assign(teapot1, "spout") == 1);
    CPPUNIT_ASSERT(traceSet->assign(teapot2, "lid") == 2);
    CPPUNIT_ASSERT(traceSet->assign(teapot2, "body") == 3);
    traceSet->endUpdate();

    // Serialize
    BinaryWriter writer(*mContext);
    std::stringstream ss;
    writer.toStream(ss);

    // Create a copy of the context
    SceneContext context;
    BinaryReader reader(context);
    reader.fromStream(ss);
    context.commitAllChanges();

    // Perform an update of the trace set
    mContext->commitAllChanges();
    traceSet->beginUpdate();
    CPPUNIT_ASSERT(traceSet->assign(teapot2, "base") == 4);
    traceSet->endUpdate();

    // Serialize with delta encoding on
    BinaryWriter writer2(*mContext);
    writer2.setDeltaEncoding(true);
    std::stringstream ss2;
    writer2.toStream(ss2);

    // Deserialize to the copy
    SceneContext context2;
    BinaryReader reader2(context2);
    reader2.fromStream(ss2);

    // Get the trace set from the copy
    teapot1 = context2.getSceneObject("/seq/shot/teapot1")->asA<Geometry>();
    teapot2 = context2.getSceneObject("/seq/shot/teapot2")->asA<Geometry>();
    TraceSet * traceSetRead = context2.getSceneObject("/seq/shot/traceset")->asA<TraceSet>();

    // check attributes
    CPPUNIT_ASSERT(traceSetRead->getAssignmentCount() == 5);
    CPPUNIT_ASSERT(traceSetRead->getAssignmentId(teapot1, "lid") == 0);
    CPPUNIT_ASSERT(traceSetRead->getAssignmentId(teapot1, "spout") == 1);
    CPPUNIT_ASSERT(traceSetRead->getAssignmentId(teapot2, "lid") == 2);
    CPPUNIT_ASSERT(traceSetRead->getAssignmentId(teapot2, "body") == 3);
    CPPUNIT_ASSERT(traceSetRead->getAssignmentId(teapot2, "base") == 4);
    CPPUNIT_ASSERT(traceSetRead->contains(teapot1));
    CPPUNIT_ASSERT(traceSetRead->contains(teapot2));
    CPPUNIT_ASSERT(traceSetRead->lookupGeomAndPart(0).first == teapot1);
    CPPUNIT_ASSERT(traceSetRead->lookupGeomAndPart(0).second == "lid");
    CPPUNIT_ASSERT(traceSetRead->lookupGeomAndPart(1).first == teapot1);
    CPPUNIT_ASSERT(traceSetRead->lookupGeomAndPart(1).second == "spout");
    CPPUNIT_ASSERT(traceSetRead->lookupGeomAndPart(2).first == teapot2);
    CPPUNIT_ASSERT(traceSetRead->lookupGeomAndPart(2).second == "lid");
    CPPUNIT_ASSERT(traceSetRead->lookupGeomAndPart(3).first == teapot2);
    CPPUNIT_ASSERT(traceSetRead->lookupGeomAndPart(3).second == "body");
    CPPUNIT_ASSERT(traceSetRead->lookupGeomAndPart(4).first == teapot2);
    CPPUNIT_ASSERT(traceSetRead->lookupGeomAndPart(4).second == "base");
}


} // namespace unittest
} // namespace rdl2
} // namespace scene_rdl2




