// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "Node.h"

namespace scene_rdl2 {
namespace rdl2 {

class LightFilter : public SceneObject
{
public:
    typedef SceneObject Parent;

    LightFilter(const SceneClass& sceneClass, const std::string& name);
    virtual ~LightFilter();
    static SceneObjectInterface declare(SceneClass& sceneClass);

    /// Is the LightFilter enabled?
    virtual bool isOn() const;

    virtual void getReferencedLightFilters(std::unordered_set<const rdl2::LightFilter *>& filters) const {}

    // Attributes common to all LightFilters.
    static AttributeKey<Bool> sOnKey;
};

template <>
inline const LightFilter*
SceneObject::asA() const
{
    return isA<LightFilter>() ? static_cast<const LightFilter*>(this) : nullptr;
}

template <>
inline LightFilter*
SceneObject::asA()
{
    return isA<LightFilter>() ? static_cast<LightFilter*>(this) : nullptr;
}

} // namespace rdl2
} // namespace scene_rdl2

