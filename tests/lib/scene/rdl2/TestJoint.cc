// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "TestJoint.h"

#include <scene_rdl2/scene/rdl2/AsciiReader.h>
#include <scene_rdl2/scene/rdl2/AsciiWriter.h>
#include <scene_rdl2/scene/rdl2/BinaryReader.h>
#include <scene_rdl2/scene/rdl2/BinaryWriter.h>
#include <scene_rdl2/scene/rdl2/Joint.h>
#include <scene_rdl2/scene/rdl2/SceneContext.h>

namespace scene_rdl2 {
namespace rdl2 {
namespace unittest {

void
TestJoint::setUp()
{
    mContext.reset(new SceneContext);
    CPPUNIT_ASSERT(mContext);
    Joint* joint = mContext->createSceneObject("Joint","/joint")->asA<Joint>();
    CPPUNIT_ASSERT(joint);

    // sets
    joint->beginUpdate();
    Mat4d xform(2.0, 0.0, 0.0, 0.0,
                0.0, 3.0, 0.0, 0.0,
                0.0, 0.0, 4.0, 0.0,
                0.0, 0.0, 0.0, 1.0);
    joint->set(Node::sNodeXformKey, xform);
    joint->endUpdate();
}

void
TestJoint::tearDown()
{
}

void
TestJoint::testSetup()
{
    Joint* joint = mContext->getSceneObject("/joint")->asA<Joint>();
    CPPUNIT_ASSERT(joint);
    const auto xform = joint->get(Node::sNodeXformKey);
    CPPUNIT_ASSERT(xform[0][0] == 2.0);
    CPPUNIT_ASSERT(xform[0][1] == 0.0);
    CPPUNIT_ASSERT(xform[1][1] == 3.0);
    CPPUNIT_ASSERT(xform[2][2] == 4.0);
}

void
TestJoint::testAscii()
{
    AsciiWriter writer(*mContext);
    writer.toFile("Joint.rdla");
    SceneContext reContext;
    AsciiReader reader(reContext);
    reader.fromFile("Joint.rdla");
    compare(*mContext, reContext);
}

void
TestJoint::testBinary()
{
    BinaryWriter writer(*mContext);
    writer.toFile("Joint.rdlb");
    SceneContext reContext;
    BinaryReader reader(reContext);
    reader.fromFile("Joint.rdlb");
    compare(*mContext, reContext);
}

void
TestJoint::compare(SceneContext const &a, SceneContext const &b) const
{
    const auto xformA = a.getSceneObject("/joint")->asA<Joint>()->get(Node::sNodeXformKey);
    const auto xformB = b.getSceneObject("/joint")->asA<Joint>()->get(Node::sNodeXformKey);
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            CPPUNIT_ASSERT(xformA[i][j] == xformB[i][j]);
        }
    }
}

} // namespace unittest
} // namespace rdl2
} // namespace scene_rdl2


