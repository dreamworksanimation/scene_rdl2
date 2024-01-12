// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//

#include "Color.h"
#include "Mat4.h"
#include "ReferenceFrame.h"
#include "Vec2.h"
#include "Vec3.h"
#include "Xform.h"

#include <scene_rdl2/common/math/ispc/Types_ispc_stubs.h>
#include <scene_rdl2/common/platform/HybridUniformData.h>


namespace scene_rdl2 {
namespace math {


//---------------------------------------------------------------------------

// Memory layout validation with corresponding ISPC types
#pragma warning push
#pragma warning disable 1875

// Color
HUD_VALIDATE_STATIC(Color, r);
HUD_VALIDATE_STATIC(Color, g);
HUD_VALIDATE_STATIC(Color, b);

// Mat4
HUD_VALIDATE_STATIC(Mat4f, vx);
HUD_VALIDATE_STATIC(Mat4f, vy);
HUD_VALIDATE_STATIC(Mat4f, vz);
HUD_VALIDATE_STATIC(Mat4f, vw);

// ReferenceFrame
void
ReferenceFrame::hudValidation()
{
    HUD_VALIDATE_STATIC(ReferenceFrame, mX);
    HUD_VALIDATE_STATIC(ReferenceFrame, mY);
    HUD_VALIDATE_STATIC(ReferenceFrame, mZ);
}

// Vec2
HUD_VALIDATE_STATIC(Vec2f, x);
HUD_VALIDATE_STATIC(Vec2f, y);

// Vec3
HUD_VALIDATE_STATIC(Vec3f, x);
HUD_VALIDATE_STATIC(Vec3f, y);
HUD_VALIDATE_STATIC(Vec3f, z);

// Xform
HUD_VALIDATE_STATIC(Xform3f, l);
HUD_VALIDATE_STATIC(Xform3f, p);

// XformComponent
HUD_VALIDATE_STATIC(XformComponent3f, t);
HUD_VALIDATE_STATIC(XformComponent3f, r);
HUD_VALIDATE_STATIC(XformComponent3f, s);


#pragma warning pop


//---------------------------------------------------------------------------

} // namespace math
} // namespace scene_rdl2

