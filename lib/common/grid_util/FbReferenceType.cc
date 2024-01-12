// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#include "FbReferenceType.h"

namespace scene_rdl2 {
namespace grid_util {

std::string
showFbReferenceType(const FbReferenceType referenceType)
{
    switch (referenceType) {
    case FbReferenceType::UNDEF      : return "UNDEF";
    case FbReferenceType::BEAUTY     : return "BEAUTY";
    case FbReferenceType::ALPHA      : return "ALPHA";
    case FbReferenceType::HEAT_MAP   : return "HEAT_MAP";
    case FbReferenceType::WEIGHT     : return "WEIGHT";
    case FbReferenceType::BEAUTY_AUX : return "BEAUTY_AUX";
    case FbReferenceType::ALPHA_AUX  : return "ALPHA_AUX";
    }
    return "UNDEF";
}

} // namespace grid_util
} // namespace scene_rdl2

