// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

// Include this before any other includes!
#include <scene_rdl2/common/platform/Platform.h>

#include <tbb/atomic.h>

namespace scene_rdl2 {
namespace util {

template<typename T>
struct CACHE_ALIGN CacheLineAtomic : public tbb::atomic<T>
{
};

MNRY_STATIC_ASSERT(sizeof(CacheLineAtomic<uint32_t>) == CACHE_LINE_SIZE);

} // namespace util
} // namespace scene_rdl2


