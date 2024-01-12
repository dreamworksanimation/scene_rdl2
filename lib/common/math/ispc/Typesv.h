// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//

#pragma once

#include <scene_rdl2/common/platform/IspcUtil.h>
#include <scene_rdl2/common/math/ispc/Types_ispc_stubs.h>

namespace scene_rdl2 {
namespace math {

// basic types
typedef int8_t SCENE_RDL2_SIMD_ALIGN Int8v[VLEN];
typedef int32_t SCENE_RDL2_SIMD_ALIGN Intv[VLEN];
typedef float SCENE_RDL2_SIMD_ALIGN Floatv[VLEN];

typedef Intv Mask;

// TODO move to standalone header/ispc files if we start requiring
// more ispc export math functionality from C++ side
// (right now only partial Xform3f ispc library got exported to C++)
ISPC_UTIL_TYPEDEF_STRUCT(Col3f, Colorv);
ISPC_UTIL_TYPEDEF_STRUCT(Col3Dual3f, ColorDualv);
ISPC_UTIL_TYPEDEF_STRUCT(Mat3f, Mat3fv);
ISPC_UTIL_TYPEDEF_STRUCT(Vec2f, Vec2fv);
ISPC_UTIL_TYPEDEF_STRUCT(Vec3f, Vec3fv);
ISPC_UTIL_TYPEDEF_STRUCT(Vec3Dual3f, Vec3Dualv);
ISPC_UTIL_TYPEDEF_STRUCT(Xform3f, Xform3fv);


} // namespace math
} // namespace scene_rdl2

