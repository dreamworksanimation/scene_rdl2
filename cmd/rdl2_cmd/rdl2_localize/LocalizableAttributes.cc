// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "LocalizableAttributes.h"

#include <scene_rdl2/scene/rdl2/rdl2.h>

#include <cstddef>
#include <ostream>
#include <map>
#include <utility>

using namespace scene_rdl2;

namespace rdl2_localize {

LocalizableAttributes::LocalizableAttributes(const rdl2::SceneContext& ctx) :
    mRecords()
{
    // Walk all the SceneClasses looking for filename attributes.
    for (auto classIter = ctx.beginSceneClass(); classIter != ctx.endSceneClass(); ++classIter) {
        const rdl2::SceneClass* sc = classIter->second;
        for (auto attrIter = sc->beginAttributes(); attrIter != sc->endAttributes(); ++attrIter) {
            const rdl2::Attribute* attr = *attrIter;
            if (attr->isFilename()) {
                mRecords.emplace(sc, attr);
            }
        }
    }
}

LocalizableAttributes::ConstRange
LocalizableAttributes::getLocalizableAttributes(const rdl2::SceneClass& sc) const
{
    return mRecords.equal_range(&sc);
}

} // namespace rdl2_localize

