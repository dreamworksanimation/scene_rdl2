// Copyright 2023-2024 DreamWorks Animation LLC
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

namespace scene_rdl2 { namespace shading { class State; } }

namespace scene_rdl2 {
namespace rdl2 {

class Map : public Shader
{
public:
    typedef Shader Parent;

    Map(const SceneClass& sceneClass, const std::string& name);
    virtual ~Map();
    static SceneObjectInterface declare(SceneClass& sceneClass);

    finline void sample(moonray::shading::TLState *tls,
                        const moonray::shading::State& state,
                        math::Color* result) const
    {
        MNRY_ASSERT(mSampleFunc != nullptr);
        mSampleFunc(this, tls, state, result);
    }

    finline void samplev(moonray::shading::TLState *tls,
                         const rdl2::Statev * statev,
                         rdl2::Colorv* resultv) const
    {
        if (mSampleFuncv != nullptr) {
            mSampleFuncv(this, tls, statev, resultv, util::sAllOnMask);
        }
    }

    // Unfortunately, this member has been made public to allow
    //  for computing its offset into the binary
    SampleFunc mSampleFunc;
    SampleFuncv mSampleFuncv;
    // Save away mSampleFunc when we fatal for future restore
    SampleFunc mOriginalSampleFunc;
    SampleFuncv mOriginalSampleFuncv;

    // Not thread safe!
    virtual void setFataled(bool fataled) {
        if (fataled) {
            mOriginalSampleFunc = mSampleFunc;
            mOriginalSampleFuncv = mSampleFuncv;
            mSampleFunc = mSceneClass.getSceneContext()->getFatalSampleFunc();
            mSampleFuncv = nullptr;
        } else {
            // If we're no longer fataled and we stored away
            // a sample func, restore it.
            if (mOriginalSampleFunc != nullptr) {
                mSampleFunc = mOriginalSampleFunc;
            }
            if (mOriginalSampleFuncv != nullptr) {
                mSampleFuncv = mOriginalSampleFuncv;
            }
        }
    }

    // Some maps can be used as an extra aov map.
    virtual bool getIsExtraAovMap(String &label, Bool &postScatter) const;

    // If this is a ListMap, this method will return true and
    // fill out the vector of map objects
    virtual bool getIsListMap(std::vector<const Map *> &mapList) const;
};

template <>
inline const Map*
SceneObject::asA() const
{
    return isA<Map>() ? static_cast<const Map*>(this) : nullptr;
}

template <>
inline Map*
SceneObject::asA()
{
    return isA<Map>() ? static_cast<Map*>(this) : nullptr;
}

} // namespace rdl2
} // namespace scene_rdl2

