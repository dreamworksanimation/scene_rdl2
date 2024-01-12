// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "VolumeShader.h"

#include "SceneClass.h"
#include "Types.h"

#include <string>

namespace scene_rdl2 {
namespace rdl2 {

AttributeKey<String> VolumeShader::sLabel;
AttributeKey<Int> VolumeShader::sBakeResolutionMode;
AttributeKey<Int> VolumeShader::sBakeDivisions;
AttributeKey<Float> VolumeShader::sBakeVoxelSize;
AttributeKey<Float> VolumeShader::sSurfaceOpacityThreshold;

VolumeShader::VolumeShader(const SceneClass& sceneClass, const std::string& name) :
    Parent(sceneClass, name)
{
    // Add the VolumeShader interface.
    mType |= INTERFACE_VOLUMESHADER;
}

VolumeShader::~VolumeShader()
{
}

SceneObjectInterface
VolumeShader::declare(SceneClass& sceneClass)
{
    auto interface = Parent::declare(sceneClass);

    sLabel = sceneClass.declareAttribute<String>("label", "");
    sceneClass.setMetadata(sLabel, "comment", "label used in light aovs");

    sBakeResolutionMode = sceneClass.declareAttribute<Int>("bake_resolution_mode", 0, FLAGS_ENUMERABLE,
            INTERFACE_GENERIC);
    sceneClass.setMetadata(sBakeResolutionMode, "label", "bake resolution mode");
    sceneClass.setEnumValue(sBakeResolutionMode, 0, "default");
    sceneClass.setEnumValue(sBakeResolutionMode, 1, "divisions");
    sceneClass.setEnumValue(sBakeResolutionMode, 2, "voxel size");
    sceneClass.setMetadata(sBakeResolutionMode, "comment",
        "Method to specify grid resolution of baked density grid.  Choices are:\n"
        "\t\t\"default\": For shaders that are bound to vdb volumes, use vdb resolution.\n"
        "\t\t\t\t\t\tFor shaders that are bounds to mesh geometries use 100 divisions\n"
        "\t\t\"divisions\": Specify number of divisions.\n"
        "\t\t\"voxel size\": Specify voxel size.");
    sceneClass.setGroup("Volume Baking", sBakeResolutionMode);

    sBakeDivisions = sceneClass.declareAttribute<Int>("bake_divisions", 100);
    sceneClass.setMetadata(sBakeDivisions, "comment", "Divide widest axis by this many divisions");
    sceneClass.setGroup("Volume Baking", sBakeDivisions);

    sBakeVoxelSize = sceneClass.declareAttribute<Float>("bake_voxel_size", 10.f);
    sceneClass.setMetadata(sBakeVoxelSize, "comment", "Size of voxel in world space");
    sceneClass.setGroup("Volume Baking", sBakeVoxelSize);

    sSurfaceOpacityThreshold = sceneClass.declareAttribute<Float>("surface_opacity_threshold", 0.5f);
    sceneClass.setMetadata(sSurfaceOpacityThreshold, "comment",
        "Accumulated opacity that's considered the 'surface' for computing surface position and Z");
    sceneClass.setGroup("Volume", sSurfaceOpacityThreshold);

    return interface | INTERFACE_VOLUMESHADER;
}

bool
VolumeShader::isHomogenous() const
{
    unsigned int property = getProperties();
    bool result = true;
    if (property & rdl2::VolumeShader::IS_EXTINCTIVE) {
        result &= ((property & rdl2::VolumeShader::HOMOGENOUS_EXTINC) != 0);
    }
    if (property & rdl2::VolumeShader::IS_SCATTERING) {
        result &= ((property & rdl2::VolumeShader::HOMOGENOUS_ALBEDO) != 0);
    }
    if (property & rdl2::VolumeShader::IS_EMISSIVE) {
        result &= ((property & rdl2::VolumeShader::HOMOGENOUS_EMISS) != 0);
    }
    return result;
}

} // namespace rdl2
} // namespace scene_rdl2


