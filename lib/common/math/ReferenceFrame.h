// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

///
/// @file ReferenceFrame.h
/// $Id$
///

#pragma once

#include "Mat4.h"
#include "Vec3.h"

#include <scene_rdl2/common/platform/HybridUniformData.h>

// Forward declaration of the ISPC types
namespace ispc {
    struct ReferenceFrame;
}


namespace scene_rdl2 {
namespace math {


//----------------------------------------------------------------------------

///
/// @class ReferenceFrame ReferenceFrame.h <scene_rdl2/common/math/ReferenceFrame.h>
/// @brief This class handles the transformation of directions from any global
/// space (typically render space) to and from the local frame of
/// reference used for sampling / manipulating spherical directions.
/// This reference frame is similar as the one defined in PBRT
/// (see: figure 8.3), except we use a right-handed orthonormal basis.
/// 
class ReferenceFrame
{
public:
    /// Create an identity reference frame, where local space and global space
    /// are the same
    finline ReferenceFrame() :
        mX(1.0f, 0.0f, 0.0f),
        mY(0.0f, 1.0f, 0.0f),
        mZ(0.0f, 0.0f, 1.0f)    { }

    /// Use this for constructing an isotropic surface bsdf lobe reference frame.
    /// This reference is defined as:
    ///     X = an arbitrary direction orthogonal to N
    ///     Y = The third leg to form a right-handed orthogonal basis
    ///     Z = N
    /// This method assumes input vector is normalized
    finline ReferenceFrame(const Vec3f &N)
    {
        MNRY_ASSERT(isNormalized(N));
        
        // Building an Orthonormal Basis, Revisited
        // Duff, Tom et. al
        // Pixar
        // https://graphics.pixar.com/library/OrthonormalB/paper.pdf
        const float sign = std::copysign(1.0f, N.z);
        const float a = -1.0f / (sign + N.z);
        const float b = N.x * N.y * a;
        mX = Vec3f(1.0f + sign * N.x * N.x * a, sign * b, -sign * N.x);
        mY = Vec3f(b, sign + N.y * N.y * a, -N.y);
        mZ = N;
    }

    /// Use this for constructing an anisotropic surface bsdf lobe reference frame.
    /// You can call it with arguments (N, normalize(dPdu)) or
    /// (N, normalize(anisotropicDirection)).
    /// This reference is defined as:
    ///     X = T made orthogonal to N and stay in the plane (N, T)
    ///     Y = The third leg to form a right-handed orthogonal basis
    ///     Z = N
    /// This method assumes both input vectors are normalized
    finline ReferenceFrame(const Vec3f &N, const Vec3f &T)
    {
        MNRY_ASSERT(isNormalized(N));
        MNRY_ASSERT(isNormalized(T));

        mZ = N;
        mY = cross(N, T);
        mY = normalize(mY);
        mX = cross(mY, mZ);
    }

    /// Use this for constructing a hair bsdf lobe reference frame. You can call
    /// it with arguments (I, normalize(dPdu)) or (I, normalize(hairDirection)).
    /// The argument isHair is unused and is only here to de-ambiguate with the
    /// constructor above.
    /// This reference is defined as:
    ///     X = T
    ///     Y = vector orthogonal to plane (wo, T)
    ///     Z = The third leg to form a right-handed orthogonal basis. Z ends up
    ///         in the (wo, T) plane.
    /// This method assumes both input vectors are normalized
    finline ReferenceFrame(const Vec3f &wo, const Vec3f &T, bool /*isHair*/):
        mX(T),
        mY(normalize(cross(wo, T))),
        mZ(cross(mX, mY))
    {
        MNRY_ASSERT(isNormalized(wo));
        MNRY_ASSERT(isNormalized(T));
    }

    /// Utility function to create a ReferenceFrame from the rows of a matrix.
    /// The matrix is assumed to be orthonormal.
    finline ReferenceFrame(const Mat4f &m):
        mX(asVec3(m.row0())),
        mY(asVec3(m.row1())),
        mZ(asVec3(m.row2()))
    {
    }

    ~ReferenceFrame()  { }


    /// Accessors for global space vectors that form the orthonormal basis (X, Y, Z).
    finline const Vec3f &getX() const   {  return mX;  }
    finline const Vec3f &getY() const   {  return mY;  }
    finline const Vec3f &getZ() const   {  return mZ;  }

    /// Convention: in all bases, N is mapped onto Z.
    finline const Vec3f &getN() const   {  return mZ;  }

    /// Convention: T is mapped onto X.
    finline const Vec3f &getT() const   {  return mX;  }


    /// Transform normalized directions to/from the caller's global coordinate
    /// system (i.e. render space, world space, etc.) to/from the frame of
    /// reference local coordinate system. If the input direction is normalized,
    /// the ouput direction is also normalized.
    finline Vec3f globalToLocal(const Vec3f &dir) const {
        return Vec3f(dot(mX, dir), dot(mY, dir), dot(mZ, dir));
    }
    finline  Vec3f localToGlobal(const Vec3f &dir) const {
        return mX * dir[0] + mY * dir[1] + mZ * dir[2];
    }


    /// TODO: Flip the frame of reference. Rotate 180 about mX or negate all vectors ?

    /// TODO: add method to make a global vector out of theta and phi ?
    /// see: Util.h: computeLocalSphericalDirection()


    /// Static HUD validation
    static void hudValidation();

    /// Right-handed orthonormal basis vectors, expressed in the caller's
    /// coordinate system
    Vec3f mX;
    Vec3f mY;
    Vec3f mZ;
};


////////////////////////////////////////////////////////////////////////////////
/// asIspc() and asCpp() C++ <--> ISPC Type-casting functions
////////////////////////////////////////////////////////////////////////////////
HUD_AS_ISPC_FUNCTIONS(ReferenceFrame);
HUD_AS_CPP_FUNCTIONS(ReferenceFrame);


//----------------------------------------------------------------------------

} // namespace pbr
} // namespace scene_rdl2


