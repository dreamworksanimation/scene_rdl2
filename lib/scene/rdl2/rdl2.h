// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

// Include this before any other includes!
#include <scene_rdl2/common/platform/Platform.h>

/**
 * The goal of RDL is to track all the objects in the scene that the renderer
 * is interested in, store their attribute data efficiently, and provide fast
 * lookup of that attribute data in a thread safe way during rendering.
 *
 * The important classes are all very thoroughly documented, but here's a high
 * level overview of the major players.
 *  - SceneContext: The container for all scene data.
 *  - SceneClass: Defines a specific type of object in the scene, like a Camera,
 *      Map, Geometry, or Teapot. These may be provided as builtins by RDL or
 *      by DSOs at runtime.
 *  - SceneObject: The actual objects in the scene that the renderer is
 *      interested in. They are effectively bags of attributes.
 *  - SceneVariables: This is a singleton SceneObject that comes with every
 *      SceneContext by default. Its attributes are render globals that affect
 *      how the scene should be rendered.
 *  - Attribute: Storage for rich metadata about a particular attribute, such
 *      as its name, type, and user-defined metadata. Attributes DO NOT store
 *      the actual attribute values, they just describe the attribute itself.
 *  - AttributeKey: A lightweight structure for looking up attribute values in
 *      SceneObjects efficiently in a typesafe way.
 *  - BinaryWriter: Can serialize an RDL SceneContext to a binary byte stream
 *      or file.
 *  - BinaryReader: Can deserialize an RDL SceneContext from a binary byte
 *      stream or file.
 *  - Asset, Camera, Geometry, Light, Map, Material, etc.: Derived classes of
 *      SceneObject that declare specific attributes or provide useful methods
 *      that the renderer can call.
 *
 * Here are some basic examples of things you might want to do with RDL. For
 * more examples, check out the unit test suite. Tests with the BinaryReader,
 * BinaryWriter, SceneContext, and SceneObject are more likely to be helpful,
 * as they are mostly high level functional tests and not internal consistency
 * tests. You can also look at some of the example DSOs in the unit test
 * directory to get an idea for how DSOs are defined.
 *
 * Create a SceneContext and set some SceneVariables:
 *
 *      SceneContext context;
 *      SceneVariables& vars = context.sceneVariables();
 *      vars.setInteractive(true);
 *      vars.setRes(2.0f);
 *      vars.setCpuUtilization(100);
 *
 * Create a SceneContext, create an object, and set some attributes, and hand
 * a const context to the rendering libraries:
 *
 *      // Pre-render, loading the scene.
 *      SceneContext context;
 *      context.createSceneClass("Teapot");
 *      SceneObject* teapot = context.createSceneObject("Teapot", "/seq/shot/teapot");
 *
 *      AttributeKey<Int> awesomenessKey = teapot->sceneClass().attributeKey("awesomeness");
 *      teapot->set(awesomenessKey, 11);
 *
 *      AttributeKey<Mat4f> xformKey = teapot->sceneClass().attributeKey("node xform");
 *      teapot->set(xformKey, Mat4f(...));
 *
 *      // Render time, hand off a const context.
 *      const SceneContext& constContext = context;
 *      renderer->render(constContext);
 *
 * Load a binary RDL file, make some changes, and save it out again:
 *
 *      SceneContext context;
 *
 *      BinaryReader reader(context);
 *      reader.fromFile("scene.rdlb");
 *
 *      Map* pepperoniMap = context.map("/seq/shot/pepperoni");
 *      SceneObject* pizza = context.sceneObject("/seq/shot/pizza");
 *
 *      AttributeKey<Float> toppingsKey = pizza->sceneClass().attributeKey("toppings");
 *      pizza->setBinding(toppingsKey, pepperoniMap);
 *
 *      BinaryWriter writer(context);
 *      writer.toFile("tasty.rdlb", true); // true for persistence, see BinaryWriter
 */

#include "AsciiReader.h"
#include "AsciiWriter.h"
#include "Attribute.h"
#include "AttributeKey.h"
#include "BinaryReader.h"
#include "BinaryWriter.h"
#include "Camera.h"
#include "CommonAttributes.h"
#include "Displacement.h"
#include "DisplayFilter.h"
#include "Dso.h"
#include "DsoFinder.h"
#include "EnvMap.h"
#include "Geometry.h"
#include "GeometrySet.h"
#include "Joint.h"
#include "Layer.h"
#include "Light.h"
#include "LightFilter.h"
#include "LightFilterSet.h"
#include "LightSet.h"
#include "Macros.h"
#include "Map.h"
#include "NormalMap.h"
#include "Material.h"
#include "Metadata.h"
#include "Node.h"
#include "ObjectFactory.h"
#include "RootShader.h"
#include "SceneClass.h"
#include "SceneContext.h"
#include "SceneObject.h"
#include "SceneVariables.h"
#include "Shader.h"
#include "ShadowReceiverSet.h"
#include "ShadowSet.h"
#include "Slice.h"
#include "TraceSet.h"
#include "Types.h"
#include "UserData.h"
#include "Utils.h"
#include "VolumeShader.h"

