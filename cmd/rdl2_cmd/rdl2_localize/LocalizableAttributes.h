// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <scene_rdl2/scene/rdl2/rdl2.h>

#include <cstddef>
#include <ostream>
#include <map>
#include <utility>

namespace rdl2_localize {

/**
 * Takes an RDL2 SceneContext and produces a list of localizable attributes
 * (e.g. file name attributes) for each SceneClass.
 */
class LocalizableAttributes
{
private:
    typedef scene_rdl2::rdl2::SceneClass ClassType;
    typedef scene_rdl2::rdl2::Attribute AttrType;
    typedef std::multimap<const ClassType*, const AttrType*> MapType;

public:
    typedef MapType::const_iterator ConstIterator;
    typedef std::pair<ConstIterator, ConstIterator> ConstRange;

    LocalizableAttributes(const scene_rdl2::rdl2::SceneContext& context);

    /**
     * Retrieves a range (std::pair of iterators) which can be iterated over to
     * retrieve the list of Attributes for a given SceneClass that are
     * localizable.
     */
    ConstRange getLocalizableAttributes(const scene_rdl2::rdl2::SceneClass& sceneClass) const;

private:
    MapType mRecords;
};

} // namespace rdl2_localize

