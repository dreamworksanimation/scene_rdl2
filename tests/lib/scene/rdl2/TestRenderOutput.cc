// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file TestRenderOutput.cc

#include "TestRenderOutput.h"

#include <scene_rdl2/scene/rdl2/AsciiReader.h>
#include <scene_rdl2/scene/rdl2/AsciiWriter.h>
#include <scene_rdl2/scene/rdl2/BinaryReader.h>
#include <scene_rdl2/scene/rdl2/BinaryWriter.h>
#include <scene_rdl2/scene/rdl2/RenderOutput.h>
#include <scene_rdl2/scene/rdl2/SceneContext.h>

namespace scene_rdl2 {
namespace rdl2 {
namespace unittest {

void
TestRenderOutput::setUp()
{
    mContext.reset(new SceneContext);
    CPPUNIT_ASSERT(mContext);
    RenderOutput *ro =
        mContext->createSceneObject("RenderOutput",
                                    "/renderOutput")->asA<RenderOutput>();
    // check defaults
    CPPUNIT_ASSERT(ro->getActive());
    CPPUNIT_ASSERT(ro->getResult() == RenderOutput::RESULT_BEAUTY);
    CPPUNIT_ASSERT(ro->getStateVariable() == RenderOutput::STATE_VARIABLE_N);
    CPPUNIT_ASSERT(ro->getPrimitiveAttribute() == "");
    CPPUNIT_ASSERT(ro->getPrimitiveAttributeType() == RenderOutput::PRIMITIVE_ATTRIBUTE_TYPE_FLOAT);
    CPPUNIT_ASSERT(ro->getMaterialAov() == "");
    CPPUNIT_ASSERT(ro->getLpe() == "");
    CPPUNIT_ASSERT(ro->getFileName() == "scene.exr");
    CPPUNIT_ASSERT(ro->getFilePart() == "");
    CPPUNIT_ASSERT(ro->getCompression() == RenderOutput::COMPRESSION_ZIP);
    CPPUNIT_ASSERT(ro->getChannelName() == "");
    CPPUNIT_ASSERT(ro->getChannelFormat() == RenderOutput::CHANNEL_FORMAT_HALF);
    CPPUNIT_ASSERT(ro->getCheckpointFileName() == "checkpoint.exr");
    CPPUNIT_ASSERT(ro->getResumeFileName() == "");

    // sets
    ro->beginUpdate();
    ro->setActive(false);
    ro->setResult(RenderOutput::RESULT_DEPTH);
    ro->setStateVariable(RenderOutput::STATE_VARIABLE_P);
    ro->setPrimitiveAttribute("surface_st");
    ro->setPrimitiveAttributeType(RenderOutput::PRIMITIVE_ATTRIBUTE_TYPE_VEC2F);
    ro->setMaterialAov("diffuse");
    ro->setLpe("CD*L");
    ro->setFileName("foo.exr");
    ro->setFilePart("bar_part");
    ro->setCompression(RenderOutput::COMPRESSION_DWAA);
    ro->setChannelName("baz_channel");
    ro->setChannelFormat(RenderOutput::CHANNEL_FORMAT_FLOAT);
    ro->setCheckpointFileName("qux.exr");
    ro->setResumeFileName("quux.exr");
    ro->endUpdate();
}

void
TestRenderOutput::tearDown()
{
}

void
TestRenderOutput::testSetup()
{
    CPPUNIT_ASSERT(mContext->getAllRenderOutputs().size() == 1);
    auto ro = mContext->getAllRenderOutputs()[0];
    CPPUNIT_ASSERT(ro->getName() == "/renderOutput");
    CPPUNIT_ASSERT(ro->getActive() == false);
    CPPUNIT_ASSERT(ro->getResult() == RenderOutput::RESULT_DEPTH);
    CPPUNIT_ASSERT(ro->getStateVariable() == RenderOutput::STATE_VARIABLE_P);
    CPPUNIT_ASSERT(ro->getPrimitiveAttribute() == "surface_st");
    CPPUNIT_ASSERT(ro->getPrimitiveAttributeType() == RenderOutput::PRIMITIVE_ATTRIBUTE_TYPE_VEC2F);
    CPPUNIT_ASSERT(ro->getMaterialAov() == "diffuse");
    CPPUNIT_ASSERT(ro->getLpe() == "CD*L");
    CPPUNIT_ASSERT(ro->getFileName() == "foo.exr");
    CPPUNIT_ASSERT(ro->getFilePart() == "bar_part");
    CPPUNIT_ASSERT(ro->getCompression() == RenderOutput::COMPRESSION_DWAA);
    CPPUNIT_ASSERT(ro->getChannelName() == "baz_channel");
    CPPUNIT_ASSERT(ro->getChannelFormat() == RenderOutput::CHANNEL_FORMAT_FLOAT);
    CPPUNIT_ASSERT(ro->getCheckpointFileName() == "qux.exr");
    CPPUNIT_ASSERT(ro->getResumeFileName() == "quux.exr");
}

void
TestRenderOutput::testAscii()
{
    AsciiWriter writer(*mContext);
    writer.toFile("RenderOutput.rdla");
    SceneContext reContext;
    AsciiReader reader(reContext);
    reader.fromFile("RenderOutput.rdla");
    compare(*mContext, reContext);
}

void
TestRenderOutput::testBinary()
{
    BinaryWriter writer(*mContext);
    writer.toFile("RenderOutput.rdlb");
    SceneContext reContext;
    BinaryReader reader(reContext);
    reader.fromFile("RenderOutput.rdlb");
    compare(*mContext, reContext);
}

void
TestRenderOutput::compare(SceneContext const &a, SceneContext const &b) const
{
    CPPUNIT_ASSERT(a.getAllRenderOutputs().size() ==
                   b.getAllRenderOutputs().size());
    CPPUNIT_ASSERT(a.getAllRenderOutputs()[0]->getName() ==
                   b.getAllRenderOutputs()[0]->getName());
    CPPUNIT_ASSERT(a.getAllRenderOutputs()[0]->getFileName() ==
                   b.getAllRenderOutputs()[0]->getFileName());
    CPPUNIT_ASSERT(a.getAllRenderOutputs()[0]->getFilePart() ==
                   b.getAllRenderOutputs()[0]->getFilePart());
    CPPUNIT_ASSERT(a.getAllRenderOutputs()[0]->getChannelName() ==
                   b.getAllRenderOutputs()[0]->getChannelName());
    CPPUNIT_ASSERT(a.getAllRenderOutputs()[0]->getCheckpointFileName() ==
                   b.getAllRenderOutputs()[0]->getCheckpointFileName());
}

} // namespace unittest
} // namespace rdl2
} // namespace scene_rdl2


