// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "Material.h"

#include "SceneClass.h"
#include "Types.h"

#include <string>

namespace scene_rdl2 {
namespace rdl2 {

AttributeKey<SceneObject *> Material::sExtraAovsKey;
AttributeKey<String> Material::sLabel;
AttributeKey<Int> Material::sPriority;
AttributeKey<Bool> Material::sRecordReflectedCryptomatte;
AttributeKey<Bool> Material::sRecordRefractedCryptomatte;

Material::Material(const SceneClass& sceneClass, const std::string& name) :
    Parent(sceneClass, name),
    mShadeFunc(nullptr),
    mShadeFuncv(nullptr),
    mOriginalShadeFunc(nullptr),
    mOriginalShadeFuncv(nullptr),
    mPresenceFunc(Material::defaultPresence),
    mOriginalPresenceFunc(nullptr),
    mIorFunc(Material::defaultIor),
    mOriginalIorFunc(nullptr),
    mPreventLightCullingFunc(Material::defaultPreventLightCulling),
    mOriginalPreventLightCullingFunc(nullptr)
{
    // Add the Material interface.
    mType |= INTERFACE_MATERIAL;
}

Material::~Material()
{
}

SceneObjectInterface
Material::declare(SceneClass& sceneClass)
{
    auto interface = Parent::declare(sceneClass);

    sExtraAovsKey = sceneClass.declareAttribute<SceneObject *>
        ("extra_aovs", FLAGS_NONE, INTERFACE_MAP);
    sceneClass.setMetadata(sExtraAovsKey, SceneClass::sComment, "Bind this attribute to a 'ListMap' that "
        "contains references to ExtraAovMaps that specify additional outputs that can be "
        "assigned to a RenderOutput \"light aov\" result");

    sLabel = sceneClass.declareAttribute<String>("label", "");
    sceneClass.setMetadata(sLabel, "comment", "label used in material and light aovs");

    sPriority = sceneClass.declareAttribute<Int>(
            "priority", 0, {"priority"});

    sceneClass.setMetadata(sPriority, "comment",
        "The material's place in an order of precedence for "
        "overlapping dielectrics. A value of 0 means the priority should be ignored. "
        "Materials with lower numbers (higher priority) \"override\" materials "
        "with higher numbers (lower priority).  To enable automatic removal of "
        "self-overlapping geometry, a non-zero priority must be set on the "
        "geometry's material.");

    sRecordReflectedCryptomatte = sceneClass.declareAttribute<Bool>(
            "record_reflected_cryptomatte",
            false); // default value
    sceneClass.setMetadata(sRecordReflectedCryptomatte, "label", "record reflected cryptomatte");
    sceneClass.setMetadata(sRecordReflectedCryptomatte, "comment", "Indicates whether the next reflected surface should "
                                                                    "appear in the reflected cryptomatte layers");

    sRecordRefractedCryptomatte = sceneClass.declareAttribute<Bool>(
            "record_refracted_cryptomatte",
            false, // default value
            {"invisible_refractive_cryptomatte", "invisible refractive cryptomatte"}); // aliases to support older scenes
    sceneClass.setMetadata(sRecordRefractedCryptomatte, "label", "record refracted cryptomatte");
    sceneClass.setMetadata(sRecordRefractedCryptomatte, "comment", "Indicates whether the next refracted surface should "
                                                                     "appear in the refracted cryptomatte layers");

    return interface | INTERFACE_MATERIAL;
}

float
Material::defaultPresence(const rdl2::Material* self,
                          moonray::shading::TLState *tls,
                          const moonray::shading::State& state)
{
    return 1.f;
}

float
Material::defaultIor(const rdl2::Material* self,
                     moonray::shading::TLState *tls,
                     const moonray::shading::State& state)
{
    return 1.f;
}

bool
Material::defaultPreventLightCulling(const rdl2::Material* self,
                                    const moonray::shading::State& state)
{
    return false;
}

} // namespace rdl2
} // namespace scene_rdl2

