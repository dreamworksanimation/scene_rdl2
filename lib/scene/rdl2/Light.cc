// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "Light.h"

#include "AttributeKey.h"
#include "SceneClass.h"
#include "Types.h"
#include "VisibilityFlags.h"

#include <string>

namespace scene_rdl2 {
namespace rdl2 {

AttributeKey<Bool>   Light::sOnKey;
AttributeKey<Bool>   Light::sMbKey;
AttributeKey<Int>    Light::sVisibleInCameraKey;
AttributeKey<Rgb>    Light::sColorKey;
AttributeKey<Float>  Light::sIntensityKey;
AttributeKey<Float>  Light::sExposureKey;
AttributeKey<Float>  Light::sMaxShadowDistanceKey;
AttributeKey<Float>  Light::sMinShadowDistanceKey;
AttributeKey<Int>    Light::sPresenceShadowsKey;
AttributeKey<Bool>   Light::sRayTerminationKey;
AttributeKey<Int>    Light::sTextureFilterKey;

AttributeKey<String> Light::sTextureKey;
AttributeKey<Rgb>    Light::sSaturationKey;
AttributeKey<Rgb>    Light::sContrastKey;
AttributeKey<Rgb>    Light::sGammaKey;
AttributeKey<Rgb>    Light::sGainKey;
AttributeKey<Rgb>    Light::sOffsetKey;
AttributeKey<Vec3f>  Light::sTemperatureKey;

AttributeKey<Float>  Light::sTextureRotationKey;
AttributeKey<Vec2f>  Light::sTextureTranslationKey;
AttributeKey<Vec2f>  Light::sTextureCoverageKey;
AttributeKey<Float>  Light::sTextureRepsUKey;
AttributeKey<Float>  Light::sTextureRepsVKey;
AttributeKey<Bool>   Light::sTextureMirrorUKey;
AttributeKey<Bool>   Light::sTextureMirrorVKey;
AttributeKey<Rgb>    Light::sTextureBorderColorKey;
AttributeKey<SceneObjectVector> Light::sLightFiltersKey;

AttributeKey<String> Light::sLabel;

AttributeKey<Bool> Light::sVisibleDiffuseReflection;
AttributeKey<Bool> Light::sVisibleDiffuseTransmission;
AttributeKey<Bool> Light::sVisibleGlossyReflection;
AttributeKey<Bool> Light::sVisibleGlossyTransmission;
AttributeKey<Bool> Light::sVisibleMirrorReflection;
AttributeKey<Bool> Light::sVisibleMirrorTransmission;

Light::Light(const SceneClass& sceneClass, const std::string& name) :
    Parent(sceneClass, name)
{
    // Add the Light interface.
    mType |= INTERFACE_LIGHT;
}

Light::~Light()
{
}

SceneObjectInterface
Light::declare(SceneClass& sceneClass)
{
    auto interface = Parent::declare(sceneClass);

    sOnKey = sceneClass.declareAttribute<Bool>("on", true);
    sceneClass.setMetadata(sOnKey, SceneClass::sComment,
            "Whether the light is switched on.");

    sMbKey = sceneClass.declareAttribute<Bool>("mb", false);
    sceneClass.setMetadata(sMbKey, SceneClass::sComment,
            "Whether motion-blur is active for this light. "
            "When set to true, the scene's illumination will correctly account for any blur() "
            "applied to the light's transformation matrix.");

    sVisibleInCameraKey = sceneClass.declareAttribute<Int>("visible_in_camera", 2,
        rdl2::FLAGS_ENUMERABLE, INTERFACE_GENERIC, { "visible in camera" });
    sceneClass.setMetadata(sVisibleInCameraKey, "label", "visible in camera");
    sceneClass.setEnumValue(sVisibleInCameraKey, 0, "force off");
    sceneClass.setEnumValue(sVisibleInCameraKey, 1, "force on");
    sceneClass.setEnumValue(sVisibleInCameraKey, 2, "use default");
    sceneClass.setMetadata(sVisibleInCameraKey, SceneClass::sComment,
            "Whether the light is directly visible in the scene's active camera. When set to \"use default\" "
            "it reads from the value of SceneVariable lights_visible_in_camera.");

    sColorKey = sceneClass.declareAttribute<Rgb>("color", Rgb(1.0f));
    sceneClass.setMetadata(sColorKey, SceneClass::sComment,
            "The light's RGB values.\n"
            "These are combined multiplicatively with the intensity and other attributes "
            "in determining the light's 3-channel radiance.");

    sIntensityKey = sceneClass.declareAttribute<Float>("intensity", 1.0f);
    sceneClass.setMetadata(sIntensityKey, SceneClass::sComment,
            "The light's intensity.\n"
            "This is combined multiplicatively with the color and other attributes "
            "in determining the light's 3-channel radiance.");

    sExposureKey = sceneClass.declareAttribute<Float>("exposure", 0.0f);
    sceneClass.setMetadata(sExposureKey, SceneClass::sComment,
            "The light's exposure value.\n"
            "This value provides an alternative to the intensity value as a mechanism for controlling the light's "
            "overall brightness, and is inspired by the corresponding photographic term but is generalised to "
            "apply independently to each light. To calculate its effect, pow(2, exposure) is combined "
            "multiplicatively with the color and other attributes in determining the light's 3-channel radiance.");

    sMaxShadowDistanceKey = sceneClass.declareAttribute<Float>("max_shadow_distance", 0.f);
    sceneClass.setMetadata(sMaxShadowDistanceKey, SceneClass::sComment,
            "The distance from the light beyond which a light-receiving surface will no longer "
            "receive shadows cast from that light.\n"
            "Note that the distance is thresholded for each occlusion ray cast for this light, it is possible "
            "for a receiving point to lie at an intermediate distance such that some parts of the light are "
            "closer than the threshold distance and other parts beyond it, in which case the point will appear"
            "to be in partial shadow.");

    sMinShadowDistanceKey = sceneClass.declareAttribute<Float>("min_shadow_distance", 0.f);
    sceneClass.setMetadata(sMinShadowDistanceKey, SceneClass::sComment,
            "The distance from the light before which a light-receiving surface will no longer "
            "receive shadows cast from that light.\n"
            "Note that the distance is thresholded for each occlusion ray cast for this light, it is possible "
            "for a receiving point to lie at an intermediate distance such that some parts of the light are "
            "closer than the threshold distance and other parts beyond it, in which case the point will appear"
            "to be in partial shadow.");

    sPresenceShadowsKey = sceneClass.declareAttribute<Int>("presence_shadows", 2,
        rdl2::FLAGS_ENUMERABLE, INTERFACE_GENERIC, { "presence shadows" });
    sceneClass.setMetadata(sPresenceShadowsKey, "label", "presence shadows");
    sceneClass.setEnumValue(sPresenceShadowsKey, 0, "force off");
    sceneClass.setEnumValue(sPresenceShadowsKey, 1, "force on");
    sceneClass.setEnumValue(sPresenceShadowsKey, 2, "use default");
    sceneClass.setMetadata(sPresenceShadowsKey, SceneClass::sComment,
            "Switch this attribute on for shadows cast from this light to correctly respect presence values. "
            "When off, surfaces with a material with presence less than 1.0 will cast opaque shadows from this "
            "light. This is an optimization - when the attribute is off, occlusion rays (fast) "
            "are used for testing for shadows. When it is on, regular rays (slower) are used, "
            "and the material's presence is evaluated to determine how much shadowing should occur. "
            "When set to \"use default\" it reads from the value of SceneVariable enable_presence_shadows.");

    sRayTerminationKey = sceneClass.declareAttribute<Bool>("ray_termination", false);
    sceneClass.setMetadata(sRayTerminationKey, SceneClass::sComment,
            "Whether the light is used for ray termination color. "
            "Ray termination color is used for filling in falsely dark areas where ray paths have "
            "been terminated too early by the depth controls. Such a ray path immediately exits "
            "to any ray termination light(s) present in the light set being applied to the lobe, "
            "ignoring occlusion by scene geometry. "
            "Any light can either be a regular light or a ray termination light (but not both). "
            "Thus they can be freely assigned to light sets, which provides a mechanism for applying "
            "specific ray termination lights to specific materials, parts or objects. "
            "Ray termination color is only applied to non-hair transmission lobes.");

    sTextureFilterKey = sceneClass.declareAttribute<Int>("texture_filter",
        Int(TextureFilterType::TEXTURE_FILTER_NEAREST),
        rdl2::FLAGS_ENUMERABLE, INTERFACE_GENERIC, { "texture filter" });
    sceneClass.setMetadata (sTextureFilterKey, "label", "texture filter");
    sceneClass.setEnumValue(sTextureFilterKey, Int(TextureFilterType::TEXTURE_FILTER_NEAREST),
                            "nearest neighbor");
    sceneClass.setEnumValue(sTextureFilterKey, Int(TextureFilterType::TEXTURE_FILTER_BILINEAR),
                            "bilinear");
    sceneClass.setEnumValue(sTextureFilterKey, Int(TextureFilterType::TEXTURE_FILTER_NEAREST_MIP_NEAREST),
                            "nearest neighbor with nearest mip");
    sceneClass.setEnumValue(sTextureFilterKey, Int(TextureFilterType::TEXTURE_FILTER_BILINEAR_MIP_NEAREST),
                            "bilinear with nearest mip");
    sceneClass.setMetadata(sTextureFilterKey, SceneClass::sComment,
            "The filtering mode to apply to the texture. Nearest neighbor is the cheapest filtering mode "
            "but produces a blocky result. Switch linear filtering on for a smoother result. Additionally, "
            "mip-mapping can be switched on with either nearest neighbor or linear filtering.");

    sTextureKey = sceneClass.declareAttribute<String>("texture", "", rdl2::FLAGS_FILENAME);
    sceneClass.setMetadata(sTextureKey, SceneClass::sComment,
        "File name of the texture applied to the light. If set to the empty string, no texture is applied. "
        "Any file format supported by OpenImageIO can be used. "
        "The texture is used in 2 ways - for looking up the texture value at the intersection point when a ray "
        "hits the light, and for building a lookup-table-based auxilliary data structure used for "
        "distributing light samples over the texture.");

    sSaturationKey = sceneClass.declareAttribute<Rgb>("saturation", Rgb(1.0f));
    sceneClass.setMetadata(sSaturationKey, SceneClass::sComment,
        "Per-channel saturation used in color-correcting the light's texture, if one is present. "
        "This is achieved by applying the following formula for each channel:\n"
        "  output = lerp(luminance(inputRGB), input, saturation).");

    sContrastKey = sceneClass.declareAttribute<Rgb>("contrast", Rgb(1.0f));
    sceneClass.setMetadata(sContrastKey, SceneClass::sComment,
        "Per-channel contrast used in color-correcting the light's texture, if one is present. "
        "The operation mimics Nuke's ColorCorrect node's contrast function:\n"
        "  For input >  0, output = 0.18 * pow(inputCompnent/0.18, contrast).\n"
        "  For input <= 0, output = 0.18 * input * pow(1/0.18, contrast).");

    sGammaKey = sceneClass.declareAttribute<Rgb>("gamma", Rgb(1.0f));
    sceneClass.setMetadata(sGammaKey, SceneClass::sComment,
        "Per-channel gamma used in color-correcting the light's texture, if one is present. "
        "This is achieved by applying the following formula for each channel:\n"
        "  For input >  0, output = pow(input, gamma)\n"
        "  For input <= 0, output = input");

    sGainKey = sceneClass.declareAttribute<Rgb>("gain", Rgb(1.0f));
    sceneClass.setMetadata(sGainKey, SceneClass::sComment,
        "Per-channel gain used in tandem with a per-channel offset for color-correcting the "
        "light's texture, if one is present. "
        "This is achieved by applying the following formula for each channel:\n"
        "  output = input * gain + offset");

    sOffsetKey = sceneClass.declareAttribute<Rgb>("offset", Rgb(0.0f));
    sceneClass.setMetadata(sOffsetKey, SceneClass::sComment,
        "Per-channel offset used in tandem with a per-channel gain for color-correcting the "
        "light's texture, if one is present. "
        "This is achieved by applying the following formula for each channel:\n"
        "  output = input * gain + offset");

    sTemperatureKey = sceneClass.declareAttribute<Vec3f>("temperature", Vec3f(0.f));
    sceneClass.setMetadata(sTemperatureKey, SceneClass::sComment,
        "Color temperature using Nuke-style T/M/I settings (T = temperature, M = magenta/green, I = intensity). "
        "This is achieved as follows:\n"
        "The 3-channel temperature is interpreted as the vector (T,M,I). "
        "The followiong scale values are then applied to the RGB components:\n"
        "  outputR = inputR * (pow(2,I) + M/3 - T/2)\n"
        "  outputG = inputG * (pow(2,I) - 2*M/3\n"
        "  outputB = inputB * (pow(2,I) + M/3 + T/2)");

    sTextureRotationKey = sceneClass.declareAttribute<Float>("texture_rotation", 0.0f);
    sceneClass.setMetadata(sTextureRotationKey, SceneClass::sComment,
        "Clockwise rotation angle in degrees.");

    sTextureTranslationKey = sceneClass.declareAttribute<Vec2f>("texture_translation", Vec2f(0.0f));
    sceneClass.setMetadata(sTextureTranslationKey, SceneClass::sComment,
        "Translation of the texture in (u,v)-space, in units of the texture size. For example, "
        "a translation of (0.25, 0.5) will translate the texture one-quarter of its width in the u-direction "
        "and one-half of its height in the v-direction.");

    sTextureCoverageKey = sceneClass.declareAttribute<Vec2f>("texture_coverage", Vec2f(1.0f));
    sceneClass.setMetadata(sTextureCoverageKey, SceneClass::sComment,
        "Texture scales in the u and v-directions.");

    sTextureRepsUKey = sceneClass.declareAttribute<Float>("texture_reps_u", 1.0f);
    sceneClass.setMetadata(sTextureRepsUKey, SceneClass::sComment,
        "Number of times texture repeats in u over the scaled texture space.");

    sTextureRepsVKey = sceneClass.declareAttribute<Float>("texture_reps_v", 1.0f);
    sceneClass.setMetadata(sTextureRepsVKey, SceneClass::sComment,
        "Number of times texture repeats in v over the scaled texture space.");

    sTextureMirrorUKey = sceneClass.declareAttribute<Bool>("texture_mirror_u", false);
    sceneClass.setMetadata(sTextureMirrorUKey, SceneClass::sComment,
        "Whether to mirror the texture in the u-direction. "
        "If set to false, the texture is repeated in the u-direction.");

    sTextureMirrorVKey = sceneClass.declareAttribute<Bool>("texture_mirror_v", false);
    sceneClass.setMetadata(sTextureMirrorVKey, SceneClass::sComment,
        "Whether to mirror the texture in the v-direction. "
        "If set to false, the texture is repeated in the v-direction.");

    sTextureBorderColorKey = sceneClass.declareAttribute<Rgb>("texture_border_color", math::sWhite);
    sceneClass.setMetadata(sTextureBorderColorKey, SceneClass::sComment,
        "RGB value used when a texture lookup occurs outside the texture.");

    sLightFiltersKey = sceneClass.declareAttribute<SceneObjectVector>("light_filters", { "light filters" });
    sceneClass.setMetadata(sLightFiltersKey, "label", "light filters");
    sceneClass.setMetadata(sLightFiltersKey, SceneClass::sComment,
        "Vector of LightFilters associated with the light.");

    sLabel = sceneClass.declareAttribute<String>("label", "");
    sceneClass.setMetadata(sLabel, SceneClass::sComment,
        "Label used in light aov expressions.");

    sVisibleDiffuseReflection = sceneClass.declareAttribute<Bool>("visible_diffuse_reflection", true, { "visible diffuse reflection" });
    sceneClass.setMetadata(sVisibleDiffuseReflection, "label", "visible diffuse reflection");
    sceneClass.setMetadata(sVisibleDiffuseReflection, SceneClass::sComment,
        "Whether the light is visible in diffuse reflection.");

    sVisibleDiffuseTransmission = sceneClass.declareAttribute<Bool>("visible_diffuse_transmission", true, { "visible diffuse transmission" });
    sceneClass.setMetadata(sVisibleDiffuseTransmission, "label", "visible diffuse transmission");
    sceneClass.setMetadata(sVisibleDiffuseTransmission, SceneClass::sComment,
        "Whether the light is visible in diffuse transmission.");

    sVisibleGlossyReflection = sceneClass.declareAttribute<Bool>("visible_glossy_reflection", true, { "visible glossy reflection" });
    sceneClass.setMetadata(sVisibleGlossyReflection, "label", "visible glossy reflection");
    sceneClass.setMetadata(sVisibleGlossyReflection, SceneClass::sComment,
        "Whether the light is visible in glossy reflection.");

    sVisibleGlossyTransmission = sceneClass.declareAttribute<Bool>("visible_glossy_transmission", true, { "visible glossy transmission" });
    sceneClass.setMetadata(sVisibleGlossyTransmission, "label", "visible glossy transmission");
    sceneClass.setMetadata(sVisibleGlossyTransmission, SceneClass::sComment,
        "Whether the light is visible in glossy transmission (refraction).");

    sVisibleMirrorReflection = sceneClass.declareAttribute<Bool>("visible_mirror_reflection", true, { "visible mirror reflection" });
    sceneClass.setMetadata(sVisibleMirrorReflection, "label", "visible mirror reflection");
    sceneClass.setMetadata(sVisibleMirrorReflection, SceneClass::sComment,
        "Whether the light is visible in miror reflection.");

    sVisibleMirrorTransmission = sceneClass.declareAttribute<Bool>("visible_mirror_transmission", true, { "visible mirror transmission" });
    sceneClass.setMetadata(sVisibleMirrorTransmission, "label", "visible mirror transmission");
    sceneClass.setMetadata(sVisibleMirrorTransmission, SceneClass::sComment,
        "Whether the light is visible in miror transmission (refraction).");

    // Grouping the attributes - the order of the attributes should be the same as how they are defined.
    sceneClass.setGroup("Properties", sMbKey);
    sceneClass.setGroup("Properties", sOnKey);
    sceneClass.setGroup("Properties", sVisibleInCameraKey);
    sceneClass.setGroup("Properties", sColorKey);
    sceneClass.setGroup("Properties", sIntensityKey);
    sceneClass.setGroup("Properties", sExposureKey);
    sceneClass.setGroup("Properties", sMaxShadowDistanceKey);
    sceneClass.setGroup("Properties", sMinShadowDistanceKey);
    sceneClass.setGroup("Properties", sPresenceShadowsKey);
    sceneClass.setGroup("Properties", sRayTerminationKey);
    sceneClass.setGroup("Properties", sTextureFilterKey);
    sceneClass.setGroup("Properties", sLabel);

    sceneClass.setGroup("Map", sTextureKey);
    sceneClass.setGroup("Map", sSaturationKey);
    sceneClass.setGroup("Map", sContrastKey);
    sceneClass.setGroup("Map", sGammaKey);
    sceneClass.setGroup("Map", sGainKey);
    sceneClass.setGroup("Map", sOffsetKey);
    sceneClass.setGroup("Map", sTemperatureKey);
    sceneClass.setGroup("Map", sTextureRotationKey);
    sceneClass.setGroup("Map", sTextureTranslationKey);
    sceneClass.setGroup("Map", sTextureCoverageKey);
    sceneClass.setGroup("Map", sTextureRepsUKey);
    sceneClass.setGroup("Map", sTextureRepsVKey);
    sceneClass.setGroup("Map", sTextureMirrorUKey);
    sceneClass.setGroup("Map", sTextureMirrorVKey);
    sceneClass.setGroup("Map", sTextureBorderColorKey);

    sceneClass.setGroup("Visibility Flags", sVisibleDiffuseReflection);
    sceneClass.setGroup("Visibility Flags", sVisibleDiffuseTransmission);
    sceneClass.setGroup("Visibility Flags", sVisibleGlossyReflection);
    sceneClass.setGroup("Visibility Flags", sVisibleGlossyTransmission);
    sceneClass.setGroup("Visibility Flags", sVisibleMirrorReflection);
    sceneClass.setGroup("Visibility Flags", sVisibleMirrorTransmission);

    return interface | INTERFACE_LIGHT;
}

int
Light::getVisibilityMask() const
{
    int visibilityMask = NONE_VISIBLE;
    visibilityMask |= get(sVisibleDiffuseReflection) ? DIFFUSE_REFLECTION : 0;
    visibilityMask |= get(sVisibleDiffuseTransmission) ? DIFFUSE_TRANSMISSION : 0;
    visibilityMask |= get(sVisibleGlossyReflection) ? GLOSSY_REFLECTION : 0;
    visibilityMask |= get(sVisibleGlossyTransmission) ? GLOSSY_TRANSMISSION : 0;
    visibilityMask |= get(sVisibleMirrorReflection) ? MIRROR_REFLECTION : 0;
    visibilityMask |= get(sVisibleMirrorTransmission) ? MIRROR_TRANSMISSION : 0;
    return visibilityMask;
}

} // namespace rdl2
} // namespace scene_rdl2

