// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#if defined(DEBUG)

#include <scene_rdl2/common/platform/Platform.h>

#include <map>
#include <typeinfo>
#include <typeindex>

namespace scene_rdl2 {
namespace alloc {

namespace detail {

typedef std::map<std::type_index, unsigned> TypeCountMap;

template <typename T>
unsigned& getValue(TypeCountMap& map)
{
    return map[std::type_index(typeid(T))];
}

// Termination functcion.
template <typename First>
void buildMap(TypeCountMap& map)
{
    ++getValue<First>(map);
}

template <typename First, typename Second, typename... Rest>
void buildMap(TypeCountMap& map)
{
    ++getValue<First>(map);
    buildMap<Second, Rest...>(map);
}

} // namespace detail

template <typename... Args>
class BlockAllocatorCheck
{
    detail::TypeCountMap mMap;

public:
    BlockAllocatorCheck() : mMap()
    {
        detail::buildMap<Args...>(mMap);
    }

    template <typename T>
    void used()
    {
        // If we hit this assert, we've used a type more times than it was
        // listed in the class template list. We potentially don't have enough
        // memory reserved.
        MNRY_ASSERT(detail::getValue<T>(mMap) > 0, "Type used in block allocator more often than declared.");
        --detail::getValue<T>(mMap);
    }
};

} // namespace alloc
} // namespace scene_rdl2

#endif // #if defined(DEBUG)

