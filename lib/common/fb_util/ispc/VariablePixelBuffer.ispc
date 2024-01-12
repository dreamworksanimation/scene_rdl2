// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include "VariablePixelBuffer.isph"

#include <scene_rdl2/common/platform/IspcUtil.isph>

export uniform uint32_t
PixelBuffer_hudValidation(uniform bool verbose)
{
    PIXELBUFFER_VALIDATION;
}

export uniform uint32_t
VariablePixelBuffer_hudValidation(uniform bool verbose)
{
    VARIABLE_PIXELBUFFER_VALIDATION;
}

ISPC_UTIL_EXPORT_ENUM_TO_HEADER(Format)
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(VariablePixelBuffer) 

