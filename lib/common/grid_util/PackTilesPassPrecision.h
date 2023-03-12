// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#pragma once

#include <string>

namespace scene_rdl2 {
namespace grid_util {

enum class CoarsePassPrecision : char {
    F32,
    H16,
    UC8,
    RUNTIME_DECISION // precision is switched depend on the pixel value
};

std::string showCoarsePassPrecision(const CoarsePassPrecision &precision);

enum class FinePassPrecision : char {
    F32,
    H16
};

std::string showFinePassPrecision(const FinePassPrecision &precision);

} // namespace grid_util
} // namespace scene_rdl2

