// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

// Include this before any other includes!
#include <scene_rdl2/common/platform/Platform.h>

#include "SceneClass.h"
#include "SceneContext.h"
#include "SceneObject.h"
#include "Shader.h"
#include "Types.h"

#include <scene_rdl2/common/except/exceptions.h>
#include <scene_rdl2/common/math/Color.h>
#include <scene_rdl2/render/logging/logging.h>

#include <string>

namespace moonray { namespace shading { class State; } }

namespace scene_rdl2 {
namespace rdl2 {

class NormalMap : public Shader
{
public:
    typedef Shader Parent;

    NormalMap(const SceneClass& sceneClass, const std::string& name);
    virtual ~NormalMap();
    static SceneObjectInterface declare(SceneClass& sceneClass);

    finline void sampleNormal(moonray::shading::TLState *tls,
                              const moonray::shading::State& state,
                              math::Vec3f* result) const
    {
        MNRY_ASSERT(mSampleNormalFunc != nullptr);
        mSampleNormalFunc(this, tls, state, result);
    }

    finline void sampleNormalv(moonray::shading::TLState *tls,
                         const rdl2::Statev * statev,
                         rdl2::Vec3fv* resultv) const
    {
        if (mSampleNormalFuncv != nullptr) {
            mSampleNormalFuncv(this, tls, statev, resultv, util::sAllOnMask);
        }
    }

    // Unfortuantely, this member has been made public to allow
    //  for computing its offset into the binary
    SampleNormalFunc mSampleNormalFunc;
    SampleNormalFuncv mSampleNormalFuncv;
    // Save away mSampleFunc when we fatal for future restore
    SampleNormalFunc mOriginalSampleNormalFunc;
    SampleNormalFuncv mOriginalSampleNormalFuncv;

    // Not thread safe!
    virtual void setFataled(bool fataled) {
        if (fataled) {
            mOriginalSampleNormalFunc = mSampleNormalFunc;
            mOriginalSampleNormalFuncv = mSampleNormalFuncv;
            mSampleNormalFunc = mSceneClass.getSceneContext()->getFatalSampleNormalFunc();
            mSampleNormalFuncv = nullptr;
        } else {
            // If we're no longer fataled and we stored away
            // a sample func, restore it.
            if (mOriginalSampleNormalFunc != nullptr) {
                mSampleNormalFunc = mOriginalSampleNormalFunc;
            }
            if (mOriginalSampleNormalFuncv != nullptr) {
                mSampleNormalFuncv = mOriginalSampleNormalFuncv;
            }
        }
    }
};

template <>
inline const NormalMap*
SceneObject::asA() const
{
    return isA<NormalMap>() ? static_cast<const NormalMap*>(this) : nullptr;
}

template <>
inline NormalMap*
SceneObject::asA()
{
    return isA<NormalMap>() ? static_cast<NormalMap*>(this) : nullptr;
}

} // namespace rdl2
} // namespace scene_rdl2

