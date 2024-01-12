// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//

#include "Xform.isph"
#include <scene_rdl2/common/platform/IspcUtil.isph>

export void
xformToXformv(const uniform Xform3f* uniform input,
#pragma ignore warning(all)
    varying Xform3f* uniform output)
{
    *output = *input;
}

export void
#pragma ignore warning(all)
inverseXformv(const varying Xform3f* uniform input,
#pragma ignore warning(all)
    varying Xform3f* uniform output)
{
    *output = inverse(*input);
}

export void
#pragma ignore warning(all)
transformPointvv(const varying Xform3f* uniform xform,
#pragma ignore warning(all)
    const varying Vec3f* uniform p, varying Vec3f* uniform result)
{
    *result = transformPoint(*xform, *p);
}

export void
#pragma ignore warning(all)
transformPointvu(const varying Xform3f* uniform xform,
#pragma ignore warning(all)
    const uniform Vec3f* uniform p, varying Vec3f* uniform result)
{
    *result = transformPoint(*xform, *p);
}

export void
transformPointuv(const uniform Xform3f* uniform xform,
#pragma ignore warning(all)
    const varying Vec3f* uniform p, varying Vec3f* uniform result)
{
    *result = transformPoint(*xform, *p);
}

export void
#pragma ignore warning(all)
transformVectorvv(const varying Xform3f* uniform xform,
#pragma ignore warning(all)
    const varying Vec3f* uniform v, varying Vec3f* uniform result)
{
    *result = transformVector(*xform, *v);
}

export void
transformVectoruv(const uniform Xform3f* uniform xform,
#pragma ignore warning(all)
    const varying Vec3f* uniform v, varying Vec3f* uniform result)
{
    *result = transformVector(*xform, *v);
}

export void
#pragma ignore warning(all)
multXformvv(const varying Xform3f* uniform lhs,
#pragma ignore warning(all)
    const varying Xform3f* uniform rhs,
#pragma ignore warning(all)
    varying Xform3f* uniform result)
{
    *result = (*lhs) * (*rhs);
}

export void
#pragma ignore warning(all)
multXformvu(const varying Xform3f* uniform lhs,
    const uniform Xform3f* uniform rhs,
#pragma ignore warning(all)
    varying Xform3f* uniform result)
{
    *result = (*lhs) * (*rhs);
}

export void
selectXformv(const uniform int* uniform mask,
#pragma ignore warning(all)
    const varying Xform3f* uniform trueCase,
#pragma ignore warning(all)
    const varying Xform3f* uniform falseCase,
#pragma ignore warning(all)
    varying Xform3f* uniform result)
{
    if (mask[programIndex]) {
        *result = *trueCase;
    } else {
        *result = *falseCase;
    }
}

export void
slerpXformComponentuv(const uniform XformComponent3f* uniform lhs,
    const uniform XformComponent3f* uniform rhs,
    const uniform float* uniform t,
#pragma ignore warning(all)
    varying Xform3f* uniform result)
{
    const varying float time = t[programIndex];
    *result = slerp(*lhs, *rhs, time);
}

