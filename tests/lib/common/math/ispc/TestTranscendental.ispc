// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#include <scene_rdl2/common/math/ispc/Transcendental.isph>


export uniform float
exported_rcp(uniform float x)
{
    return rcp(x);
}


export uniform float
exported_acos_f32(uniform float x)
{
    return acos(x);
}


export uniform float
exported_acos_f64(uniform float x)
{
    return (uniform float)acos((uniform double)x);
}


export uniform float
exported_dw_acos(uniform float x)
{
    return dw_acos(x);
}

