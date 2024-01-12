// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#include "Color.isph"
#include "Mat4.isph"
#include "ReferenceFrame.isph"
#include "Vec2.isph"
#include "Vec3.isph"
#include "Xform.isph"

#include <scene_rdl2/common/platform/IspcUtil.isph>


//----------------------------------------------------------------------------

// TODO move to standalone header/ispc files if we start requiring
// more ispc export math functionality from C++ side
// (right now only partial Xform3f ispc library got exported to C++)
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(Color);
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(Mat4f);
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(ReferenceFrame);
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(Vec2f);
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(Vec3f);
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(Xform3f);
ISPC_UTIL_EXPORT_UNIFORM_STRUCT_TO_HEADER(XformComponent3f);

#pragma ignore warning(all)
ISPC_UTIL_EXPORT_STRUCT_TO_HEADER(Col3f);
#pragma ignore warning(all)
ISPC_UTIL_EXPORT_STRUCT_TO_HEADER(Col3Dual3f);
#pragma ignore warning(all)
ISPC_UTIL_EXPORT_STRUCT_TO_HEADER(Mat3f);
#pragma ignore warning(all)
ISPC_UTIL_EXPORT_STRUCT_TO_HEADER(Vec2f);
#pragma ignore warning(all)
ISPC_UTIL_EXPORT_STRUCT_TO_HEADER(Vec3f);
#pragma ignore warning(all)
ISPC_UTIL_EXPORT_STRUCT_TO_HEADER(Vec3Dual3f);
#pragma ignore warning(all)
ISPC_UTIL_EXPORT_STRUCT_TO_HEADER(Xform3f);

//----------------------------------------------------------------------------

