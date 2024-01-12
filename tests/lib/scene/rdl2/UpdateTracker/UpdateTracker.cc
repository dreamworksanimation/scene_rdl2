// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include <scene_rdl2/scene/rdl2/rdl2.h>

#include "attributes.cc"

using namespace scene_rdl2;

RDL2_DSO_CLASS_BEGIN(UpdateTracker, rdl2::Map)

public:
    UpdateTracker(const rdl2::SceneClass& sceneClass, const std::string& name);
    virtual void update();

private:
    static void sample(const rdl2::Map* self, moonray::shading::TLState *tls,
                       const moonray::shading::State& state, math::Color* result);

    int mTimesPizzaUpdated;
    int mTimesPizzaBindingUpdated;
    int mTimesCookieUpdated;

RDL2_DSO_CLASS_END(UpdateTracker)

UpdateTracker::UpdateTracker(const rdl2::SceneClass& sceneClass,
                             const std::string& name) :
    Parent(sceneClass, name),
    mTimesPizzaUpdated(0),
    mTimesPizzaBindingUpdated(0),
    mTimesCookieUpdated(0)
{
    mSampleFunc = UpdateTracker::sample;
}

void
UpdateTracker::update()
{
    if (hasChanged(attrPizza)) {
        ++mTimesPizzaUpdated;
    }
    if (hasBindingChanged(attrPizza)) {
        ++mTimesPizzaBindingUpdated;
    }
    if (hasChanged(attrCookie)) {
        ++mTimesCookieUpdated;
    }
}

void
UpdateTracker::sample(const rdl2::Map* self, moonray::shading::TLState * /*tls*/,
                      const moonray::shading::State& /*state*/, math::Color* result)
{
    const UpdateTracker* me = static_cast<const UpdateTracker*>(self);

    result->r = me->mTimesPizzaUpdated;
    result->g = me->mTimesPizzaBindingUpdated;
    result->b = me->mTimesCookieUpdated;
}

