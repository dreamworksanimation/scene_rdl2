// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "Camera.h"

#include "AttributeKey.h"
#include "Geometry.h"
#include "Material.h"
#include "SceneClass.h"
#include "SceneContext.h"
#include "Types.h"

#include <string>

namespace scene_rdl2 {
namespace rdl2 {

AttributeKey<Float> Camera::sNearKey;
AttributeKey<Float> Camera::sFarKey;

AttributeKey<Float> Camera::sMbShutterOpenKey;
AttributeKey<Float> Camera::sMbShutterCloseKey;
AttributeKey<Float> Camera::sMbShutterBiasKey;

AttributeKey<String> Camera::sPixelSampleMap;

AttributeKey<SceneObject*> Camera::sMediumMaterial;    
AttributeKey<SceneObject*> Camera::sMediumGeometry;

Camera::Camera(const SceneClass& sceneClass, const std::string& name) :
    Parent(sceneClass, name)
{
    // Add the camera interface.
    mType |= INTERFACE_CAMERA;
}

Camera::~Camera()
{
}

SceneObjectInterface
Camera::declare(SceneClass& sceneClass)
{
    auto interface = Parent::declare(sceneClass);

    sNearKey = sceneClass.declareAttribute<Float>("near", 1.0f);
    sceneClass.setMetadata(sNearKey, SceneClass::sComment, "Near clipping plane");
    sFarKey = sceneClass.declareAttribute<Float>("far", 10000.0f);
    sceneClass.setMetadata(sFarKey, SceneClass::sComment, "Far clipping plane");

    sMbShutterOpenKey = sceneClass.declareAttribute<Float>("mb_shutter_open", -0.25f, { "mb shutter open" });
    sceneClass.setMetadata(sMbShutterOpenKey, "label", "mb shutter open");
    sceneClass.setMetadata(sMbShutterOpenKey, SceneClass::sComment, "Frame at which the shutter opens, i.e., the "
                                                                    "beginning of the motion blur interval.");
    sMbShutterCloseKey = sceneClass.declareAttribute<Float>("mb_shutter_close", 0.25f, { "mb shutter close" });
    sceneClass.setMetadata(sMbShutterCloseKey, "label", "mb shutter close");
    sceneClass.setMetadata(sMbShutterCloseKey, SceneClass::sComment, "Frame at which the shutter closes, i.e., the "
                                                                     "end of the motion blur interval.");
    sMbShutterBiasKey = sceneClass.declareAttribute<Float>("mb_shutter_bias", 0.0f, { "mb shutter bias" });
    sceneClass.setMetadata(sMbShutterBiasKey, "label", "mb shutter bias");
    sceneClass.setMetadata(sMbShutterBiasKey, SceneClass::sComment, "Biases the motion blur samples toward one end of "
                                                                    "the shutter interval.");

    sPixelSampleMap = sceneClass.declareAttribute<String>("pixel_sample_map", "", { "pixel sample map" });
    sceneClass.setMetadata(sPixelSampleMap, "label", "pixel sample map");
    sceneClass.setMetadata(sPixelSampleMap, SceneClass::sComment, "Map indicating the number of pixel samples that "
                                                                  "should be used per pixel (in uniform sampling mode). "
                                                                  "This is a multiplier on the global pixel sample "
                                                                  "count specified in SceneVariables. If the provided "
                                                                  "map has incompatible dimensions, it will be resized.");

    sMediumMaterial = sceneClass.declareAttribute<SceneObject*>("medium_material", nullptr, { "medium material" });
    sceneClass.setMetadata(sMediumMaterial, "label", "medium material");
    sceneClass.setMetadata(sMediumMaterial, SceneClass::sComment, 
            "The material the camera is 'inside'. If no medium_geometry is specified, ALL rays will have this initial "
            "index of refraction applied. ");
    sMediumGeometry = sceneClass.declareAttribute<SceneObject*>("medium_geometry", nullptr, { "medium geometry" });
    sceneClass.setMetadata(sMediumGeometry, "label", "medium geometry");
    sceneClass.setMetadata(sMediumGeometry, SceneClass::sComment, 
            "The geometry the camera is 'inside' to which you'd like the medium_material applied. (The use case for "
            "this is typically partially-submerged cameras)");

    // Grouping the attributes - the order of the attributes should be the same as how they are defined.
    sceneClass.setGroup("Frustum", sNearKey);
    sceneClass.setGroup("Frustum", sFarKey);
    
    sceneClass.setGroup("Motion Blur", sMbShutterOpenKey);
    sceneClass.setGroup("Motion Blur", sMbShutterCloseKey);
    sceneClass.setGroup("Motion Blur", sMbShutterBiasKey);

    sceneClass.setGroup("Render Masks", sPixelSampleMap);

    sceneClass.setGroup("Medium", sMediumMaterial);
    sceneClass.setGroup("Medium", sMediumGeometry);

    return interface | INTERFACE_CAMERA;
}

const Material* Camera::getMediumMaterial() const {
    SceneObject* so = get(sMediumMaterial);
    if (so) {
        Material* mediumMaterial = so->asA<Material>();
        return mediumMaterial;
    }
    return nullptr;
}

Geometry* Camera::getMediumGeometry() const {
    SceneObject* so = get(sMediumGeometry);
    if (so) {
        Geometry* mediumGeometry = so->asA<Geometry>();
        return mediumGeometry;
    }
    return nullptr;
}

} // namespace rdl2
} // namespace scene_rdl2

