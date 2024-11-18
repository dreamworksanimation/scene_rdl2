// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "RootShader.h"
#include <string>

namespace scene_rdl2 {
namespace rdl2 {

// Enables a Material to switch in a substitute material during integration.
// Required to support the "Ray Switch" Material.
struct RaySwitchContext {
    enum RayType {
        CameraRay,
        IndirectMirrorRay,
        IndirectGlossyRay,
        IndirectDiffuseRay,
        OtherRay
    };
    int mRayType;
};


class Material : public RootShader
{
public:
    typedef RootShader Parent;

    static AttributeKey<SceneObject *> sExtraAovsKey;
    static AttributeKey<String> sLabel;
    static AttributeKey<Int> sPriority;
    static AttributeKey<Bool> sRecordReflectedCryptomatte;
    static AttributeKey<Bool> sRecordRefractedCryptomatte;

    Material(const SceneClass& sceneClass, const std::string& name);
    virtual ~Material();
    static SceneObjectInterface declare(SceneClass& sceneClass);

    finline void shade(moonray::shading::TLState *tls,
                       const moonray::shading::State &state,
                       moonray::shading::BsdfBuilder& bsdfBuilder) const
    {
        MNRY_ASSERT(mShadeFunc != nullptr);
        mShadeFunc(this, tls, state, bsdfBuilder);
    }

    finline void shadev(moonray::shading::TLState *tls,
                        unsigned numStatev,
                        const rdl2::Statev* const statev,
                        rdl2::BsdfBuilderv *bsdfBuilderv) const
    {
        if (mShadeFuncv != nullptr) {
            mShadeFuncv(this, tls, numStatev, statev, bsdfBuilderv, util::sAllOnMask);
        }
    }

    finline float presence(moonray::shading::TLState *tls,
                           const moonray::shading::State &state) const
    {
        MNRY_ASSERT(mPresenceFunc != nullptr);
        return mPresenceFunc(this, tls, state);
    }

    finline float ior(moonray::shading::TLState *tls,
                      const moonray::shading::State &state) const
    {
        MNRY_ASSERT(mIorFunc != nullptr);
        return mIorFunc(this, tls, state);
    }

    // This function is used to signal to the integrator to not cull lights,
    // which is necessary when the material is using an input normal that is
    // no longer in the same hemisphere as the geometric normal as part of 
    // certain non-photoreal techniques
    finline bool preventLightCulling(const moonray::shading::State &state) const
    {
        MNRY_ASSERT(mPreventLightCullingFunc != nullptr);
        return mPreventLightCullingFunc(this, state);
    }

    finline int priority() const
    {
        return get(sPriority);
    }

    finline bool getRecordReflectedCryptomatte() const
    {
        return get(sRecordReflectedCryptomatte);
    }

    finline bool getRecordRefractedCryptomatte() const
    {
        return get(sRecordRefractedCryptomatte);
    }

    // Unfortunately, this member has been made public to allow
    //  for computing its offset into the binary
    ShadeFunc mShadeFunc;
    ShadeFuncv mShadeFuncv;
    // Save away mShadeFunc when we fatal for future restore
    ShadeFunc mOriginalShadeFunc;
    ShadeFuncv mOriginalShadeFuncv;

    PresenceFunc mPresenceFunc;
    PresenceFunc mOriginalPresenceFunc;

    static float defaultPresence(const rdl2::Material* self,
                                 moonray::shading::TLState *tls,
                                 const moonray::shading::State& state);

    IorFunc mIorFunc;
    IorFunc mOriginalIorFunc;

    static float defaultIor(const rdl2::Material* self,
                            moonray::shading::TLState *tls,
                            const moonray::shading::State& state);

    PreventLightCullingFunc mPreventLightCullingFunc;
    PreventLightCullingFunc mOriginalPreventLightCullingFunc;

    static bool defaultPreventLightCulling(const rdl2::Material* self,
                                           const moonray::shading::State& state);

    virtual void setFataled(bool fataled) {
        if (fataled) {
            mOriginalShadeFunc = mShadeFunc;
            mOriginalShadeFuncv = mShadeFuncv;
            mShadeFunc = mSceneClass.getSceneContext()->getFatalShadeFunc();
            mShadeFuncv = nullptr;
            mOriginalPresenceFunc = mPresenceFunc;
            mPresenceFunc = mSceneClass.getSceneContext()->getFatalPresenceFunc();
            mOriginalIorFunc = mIorFunc;
            mIorFunc = mSceneClass.getSceneContext()->getFatalIorFunc();
            mOriginalPreventLightCullingFunc = mPreventLightCullingFunc;
            mPreventLightCullingFunc = mSceneClass.getSceneContext()->getFatalPreventLightCullingFunc();
        } else {
            // If we're no longer fataled and we stored away
            // a shade func, restore it.
            if (mOriginalShadeFunc != nullptr) {
                mShadeFunc = mOriginalShadeFunc;
            }
            if (mOriginalShadeFuncv != nullptr) {
                mShadeFuncv = mOriginalShadeFuncv;
            }
            if (mOriginalPresenceFunc != nullptr) {
                mPresenceFunc = mOriginalPresenceFunc;
            }
            if (mOriginalIorFunc != nullptr) {
                mIorFunc = mOriginalIorFunc;
            }
            if (mOriginalPreventLightCullingFunc != nullptr) {
                mPreventLightCullingFunc = mOriginalPreventLightCullingFunc;
            }
        }
    }

    virtual const Material *raySwitch(const RaySwitchContext& /*ctx*/) const { return this; }
};

template <>
inline const Material*
SceneObject::asA() const
{
    return isA<Material>() ? static_cast<const Material*>(this) : nullptr;
}

template <>
inline Material*
SceneObject::asA()
{
    return isA<Material>() ? static_cast<Material*>(this) : nullptr;
}

} // namespace rdl2
} // namespace scene_rdl2

