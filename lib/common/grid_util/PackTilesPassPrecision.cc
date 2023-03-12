// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#include "PackTilesPassPrecision.h"

namespace scene_rdl2 {
namespace grid_util {

// static function
std::string
showCoarsePassPrecision(const CoarsePassPrecision &precision)
{
    switch (precision) {
    case CoarsePassPrecision::F32 : return "F32";
    case CoarsePassPrecision::H16 : return "H16";
    case CoarsePassPrecision::UC8 : return "UC8";
    case CoarsePassPrecision::RUNTIME_DECISION : return "RUNTIME_DECISION";
    default : return "?";
    }
}

// static function
std::string
showFinePassPrecision(const FinePassPrecision &precision)
{
    switch (precision) {
    case FinePassPrecision::F32 : return "F32";
    case FinePassPrecision::H16 : return "H16";
    default : return "?";
    }
}

} // namespace arras
} // namespace scene_rdl2

