// Copyright 2025 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <functional>

namespace scene_rdl2 {
namespace grid_util {
namespace unittest {

using DataSizeTestConstructionFunc = std::function<void(void* mem, size_t memSize)>;

bool
dataSizeTest(size_t memSize,
             bool expectedResult,
             const DataSizeTestConstructionFunc& constructObjFunc);

bool
dataSizeTest2(size_t memSize,
              bool expectedResultA,
              bool expectedResultB,
              bool expectedResultC,
              const DataSizeTestConstructionFunc& constructObjFunc);

} // namespace unittest
} // namespace grid_util
} // namespace scene_rdl2
