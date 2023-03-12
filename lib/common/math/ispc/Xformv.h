// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//

#pragma once

#include <scene_rdl2/common/math/Xform.h>
#include <scene_rdl2/common/math/ispc/Typesv.h>
#include <scene_rdl2/common/math/ispc/Xform_ispc_stubs.h>
#include <scene_rdl2/common/platform/IspcUtil.h>

namespace scene_rdl2 {
namespace math {

finline Xform3f
getXform(const Xform3fv& vec, size_t lane) {
    return Xform3f(
        vec.l.vx.x[lane], vec.l.vx.y[lane], vec.l.vx.z[lane],
        vec.l.vy.x[lane], vec.l.vy.y[lane], vec.l.vy.z[lane],
        vec.l.vz.x[lane], vec.l.vz.y[lane], vec.l.vz.z[lane],
        vec.p.x[lane], vec.p.y[lane], vec.p.z[lane]);
}

finline Xform3fv
broadcast(const Xform3f& xform)
{
    SCENE_RDL2_SIMD_ALIGN Xform3fv output;
    ispc::xformToXformv(reinterpret_cast<const ispc::Xform3f*>(&xform),
        &output);
    return output;
}

finline Xform3fv
inverse(const Xform3fv& xform)
{
    SCENE_RDL2_SIMD_ALIGN Xform3fv output;
    ispc::inverseXformv(&xform, &output);
    return output;
}

finline Vec3fv
transformPointv(const Xform3fv& xform, const Vec3fv& p)
{
    SCENE_RDL2_SIMD_ALIGN Vec3fv result;
    ispc::transformPointvv(&xform, &p, &result);
    return result;
}

finline Vec3fv
transformPointv(const Xform3f& xform, const Vec3fv& p)
{
    SCENE_RDL2_SIMD_ALIGN Vec3fv result;
    ispc::transformPointuv(reinterpret_cast<const ispc::Xform3f*>(&xform), &p,
        &result);
    return result;
}

finline Vec3fv
transformVectorv(const Xform3fv& xform, const Vec3fv& v)
{
    SCENE_RDL2_SIMD_ALIGN Vec3fv result;
    ispc::transformVectorvv(&xform, &v, &result);
    return result;
}

finline Vec3fv
transformVectorv(const Xform3f& xform, const Vec3fv& v)
{
    SCENE_RDL2_SIMD_ALIGN Vec3fv result;
    ispc::transformVectoruv(reinterpret_cast<const ispc::Xform3f*>(&xform), &v,
        &result);
    return result;

}

finline Xform3fv
multiply(const Xform3fv& lhs, const Xform3fv& rhs)
{
    SCENE_RDL2_SIMD_ALIGN Xform3fv result;
    ispc::multXformvv(&lhs, &rhs, &result);
    return result;
}

finline Xform3fv
multiply(const Xform3fv& lhs, const Xform3f& rhs)
{
    SCENE_RDL2_SIMD_ALIGN Xform3fv result;
    ispc::multXformvu(&lhs, reinterpret_cast<const ispc::Xform3f*>(&rhs),
        &result);
    return result;
}

finline Xform3fv
select(const Mask mask, const Xform3fv& trueCase, const Xform3fv& falseCase)
{
    SCENE_RDL2_SIMD_ALIGN Xform3fv result;
    ispc::selectXformv(mask, &trueCase, &falseCase, &result);
    return result;
}

finline Xform3fv
slerp(const XformComponent3f& lhs, const XformComponent3f& rhs, const Floatv& t)
{
    SCENE_RDL2_SIMD_ALIGN Xform3fv result;
    ispc::slerpXformComponentuv(
        reinterpret_cast<const ispc::XformComponent3f*>(&lhs),
        reinterpret_cast<const ispc::XformComponent3f*>(&rhs), t, &result);
    return result;
}

} // namespace math
} // namespace scene_rdl2

