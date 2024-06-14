// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "AttributeKey.h"
#include "Node.h"
#include "SceneClass.h"
#include "SceneObject.h"
#include "Types.h"

#include <scene_rdl2/common/except/exceptions.h>

#include <string>

namespace scene_rdl2 {
namespace rdl2 {

enum class TextureFilterType
{
    // Keep this in sync with moonray/lib/rendering/pbr/core/Distribution.h and .hh
    TEXTURE_FILTER_NEAREST = 0,
    TEXTURE_FILTER_BILINEAR,
    TEXTURE_FILTER_NEAREST_MIP_NEAREST,
    TEXTURE_FILTER_BILINEAR_MIP_NEAREST,

    TEXTURE_FILTER_NUM_TYPES
};
        
class Light : public Node
{
public:
    typedef Node Parent;

    Light(const SceneClass& sceneClass, const std::string& name);
    virtual ~Light();
    static SceneObjectInterface declare(SceneClass& sceneClass);

    /// Returns the visibility mask.
    int getVisibilityMask() const;

    // Attributes common to all Lights.
    static AttributeKey<Bool>  sOnKey;
    static AttributeKey<Bool>  sMbKey;
    static AttributeKey<Int> sVisibleInCameraKey;
    static AttributeKey<Rgb> sColorKey;
    static AttributeKey<Float> sIntensityKey;
    static AttributeKey<Float> sExposureKey;
    static AttributeKey<Float> sMaxShadowDistanceKey;
    static AttributeKey<Float> sMinShadowDistanceKey;
    /// enum PresenceShadows {
    ///    PRESENCE_SHADOWS_OFF,         // Presence shadows off for this light.
    ///    PRESENCE_SHADOWS_ON,          // Presence shadows on for this light.
    ///    PRESENCE_SHADOWS_USE_GLOBAL,  // Use "enable presence shadows" from scene vars.
    /// };
    static AttributeKey<Int> sPresenceShadowsKey;
    static AttributeKey<Bool> sRayTerminationKey;
    /// see enum class TextureFilterType above
    static AttributeKey<Int> sTextureFilterKey;

    static AttributeKey<String> sTextureKey;
    static AttributeKey<Rgb> sSaturationKey;
    static AttributeKey<Rgb> sContrastKey;
    static AttributeKey<Rgb> sGammaKey;
    static AttributeKey<Rgb> sGainKey;
    static AttributeKey<Rgb> sOffsetKey;
    static AttributeKey<Vec3f> sTemperatureKey;
    static AttributeKey<Float> sTextureRotationKey;
    static AttributeKey<Vec2f> sTextureTranslationKey;
    static AttributeKey<Vec2f> sTextureCoverageKey;
    static AttributeKey<Float> sTextureRepsUKey;
    static AttributeKey<Float> sTextureRepsVKey;
    static AttributeKey<Bool> sTextureMirrorUKey;
    static AttributeKey<Bool> sTextureMirrorVKey;
    static AttributeKey<Rgb> sTextureBorderColorKey;
    static AttributeKey<SceneObjectVector> sLightFiltersKey;
    static AttributeKey<String> sLabel;

    // visibility flags
    static AttributeKey<Bool> sVisibleDiffuseReflection;
    static AttributeKey<Bool> sVisibleDiffuseTransmission;
    static AttributeKey<Bool> sVisibleGlossyReflection;
    static AttributeKey<Bool> sVisibleGlossyTransmission;
    static AttributeKey<Bool> sVisibleMirrorReflection;
    static AttributeKey<Bool> sVisibleMirrorTransmission;
};

template <>
inline const Light*
SceneObject::asA() const
{
    return isA<Light>() ? static_cast<const Light*>(this) : nullptr;
}

template <>
inline Light*
SceneObject::asA()
{
    return isA<Light>() ? static_cast<Light*>(this) : nullptr;
}

} // namespace rdl2
} // namespace scene_rdl2

