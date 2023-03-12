// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

/// @file IspcUtil.h

#pragma once

#include "Platform.h"


/// Macro to compute the struct name ISPC uses when exporting varying structs
#if (VLEN == 16u)
#define ISPC_UTIL_EXPORTED_STRUCT_NAME(Type)  ispc::v16_varying_##Type
#elif (VLEN == 8u)
#define ISPC_UTIL_EXPORTED_STRUCT_NAME(Type)  ispc::v8_varying_##Type
#elif (VLEN == 4u)
#define ISPC_UTIL_EXPORTED_STRUCT_NAME(Type)  ispc::v4_varying_##Type
#endif

/// Macro used to typedef an exported ISPC varying struct.  ISPC varying structs
/// are defined as 'ispc::vVLEN_varying_TYPE'
#define ISPC_UTIL_TYPEDEF_STRUCT(IspcType, Typedef)             \
    typedef ISPC_UTIL_EXPORTED_STRUCT_NAME(IspcType) Typedef;


