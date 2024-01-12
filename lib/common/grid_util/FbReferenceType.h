// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#pragma once

#include <string>

namespace scene_rdl2 {
namespace grid_util {

enum class FbReferenceType : unsigned int
{
    UNDEF = 0,

    BEAUTY,
    ALPHA,
    HEAT_MAP,
    WEIGHT,
    BEAUTY_AUX,
    ALPHA_AUX
};

std::string showFbReferenceType(const FbReferenceType referenceType);

} // namespace grid_util
} // namespace scene_rdl2

