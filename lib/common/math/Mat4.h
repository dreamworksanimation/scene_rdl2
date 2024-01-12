// Copyright 2023-2024 DreamWorks Animation LLC and Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <scene_rdl2/common/platform/HybridUniformData.h>
#include <scene_rdl2/common/platform/Platform.h>
#include "simd.h"
#include "BBox.h"
#include "Quaternion.h"
#include "Vec4.h"
#include "Vec3.h"
#include "Mat3.h"
#include "Xform.h"

// Forward declaration of the ISPC types
namespace ispc {
    struct Mat4f;
}

namespace scene_rdl2 {
namespace math {

  /** \brief 4x4 Column Major Matrix
   *
   *  Matrix formats can be confusing. When we say <b>column major</b> it's only meaningful when we
   *  are discussing it in certain context, such as transformations. <b>We are not referring to the underlying
   *  storage of the matrix data</b>, which could be in any form (arrays, vectors, etc.). Regardless of row major
   *  or column major, multiplications between matrices do not change (multiply a row by a column).
   *  However, when we define transformations of point, vectors, etc., we need to be clear about the meanings
   *  of the rows and columns of a matrix and their corresponding multiplication with vectors.
   *
   *  DWA specifies the transformation matrix in column major, so vector transformation is applied using
   *  post-multiply. Normal transformation is done using pre-multiply. These are defined as follows.
   *
   *  <b>Row vector</b> means vectors are defined as v = [x, y, z, w], also can be considered as a 1 by n matrix, where n = 4.
   *
   *  Column vector is the row vector transposed, which is a n by 1 matrix.
   *
   *  Matrix multiply for a row vector can be done by post-multiply, where the matrix is after the row vector:
   *
   *    u = v * M (equivalent to transpose(M) * transpose(v))
   *
   *  Or it's possible to do pre-multiply, which implicitly transposes the vector v into a column vector:
   *
   *    w = M * v (where v is assumed to be a column vector)
   *
   *  In general, u != w (note the missing transpose for M), because matrix multiplication is not commutative.
   *
   *  The set of <b>transform*()</b> interface should be preferable over multiplication since their intentions are unambiguous.
   */
  template<typename T> struct Mat4
  {
    typedef T Vector;
    typedef typename T::Scalar Scalar;
    typedef QuaternionT<Scalar> Quat;
    typedef Mat3<Vec3<Scalar> > Mat3T;
    typedef XformT<Mat3T> Xform;

    /*! default matrix constructor */
    __forceinline Mat4           ( ) {}
    __forceinline Mat4           ( const Mat4& other ) { vx = other.vx; vy = other.vy; vz = other.vz; vw = other.vw;}
    __forceinline Mat4& operator=( const Mat4& other ) { vx = other.vx; vy = other.vy; vz = other.vz; vw = other.vw; return *this; }

    template<typename L1> __forceinline explicit Mat4( const Mat4<L1>& s ) : vx(s.vx), vy(s.vy), vz(s.vz), vw(s.vw) {}

    /*! matrix construction from <b>row vectors</b> */
    __forceinline Mat4(const Vector& vxParam, const Vector& vyParam, const Vector& vzParam, const Vector& vwParam)
      : vx(vxParam), vy(vyParam), vz(vzParam), vw(vwParam) {}

    /*! construction from quaternion */
    __forceinline explicit Mat4( const Quat& q )
      : vx(1.0f-2.0f*(q.j*q.j + q.k*q.k),   2.0f*(q.i*q.j + q.r*q.k),     2.0f*(q.i*q.k - q.r*q.j), 0)
      , vy(     2.0f*(q.i*q.j - q.r*q.k), 1-2.0f*(q.i*q.i + q.k*q.k),     2.0f*(q.j*q.k + q.r*q.i), 0)
      , vz(     2.0f*(q.i*q.k + q.r*q.j),   2.0f*(q.j*q.k - q.r*q.i), 1.0-2.0f*(q.i*q.i + q.j*q.j), 0)
      , vw(0, 0, 0, 1) {}

    /*! construction from quaternion for rotation and a vector for translation */
    __forceinline Mat4( const Quat& q, const Vector& t )
      : vx(1.0f-2.0f*(q.j*q.j + q.k*q.k),   2.0f*(q.i*q.j + q.r*q.k),     2.0f*(q.i*q.k - q.r*q.j), 0)
      , vy(     2.0f*(q.i*q.j - q.r*q.k), 1-2.0f*(q.i*q.i + q.k*q.k),     2.0f*(q.j*q.k + q.r*q.i), 0)
      , vz(     2.0f*(q.i*q.k + q.r*q.j),   2.0f*(q.j*q.k - q.r*q.i), 1.0-2.0f*(q.i*q.i + q.j*q.j), 0)
      , vw(t.x, t.y, t.z, 1) {}

    /*! matrix construction from <b>row major</b> data */
    __forceinline Mat4(const Scalar& m00, const Scalar& m01, const Scalar& m02, const Scalar& m03,
                       const Scalar& m10, const Scalar& m11, const Scalar& m12, const Scalar& m13,
                       const Scalar& m20, const Scalar& m21, const Scalar& m22, const Scalar& m23,
                       const Scalar& m30, const Scalar& m31, const Scalar& m32, const Scalar& m33)
      : vx(m00,m01,m02,m03), vy(m10,m11,m12,m13), vz(m20,m21,m22,m23), vw(m30,m31,m32,m33) {}


    /*! convert from a Xform, by default the homogeneous part is 1. */
    __forceinline explicit Mat4(const Xform& xfm)
    : Mat4(xfm.l.vx.x, xfm.l.vx.y, xfm.l.vx.z, 0,
           xfm.l.vy.x, xfm.l.vy.y, xfm.l.vy.z, 0,
           xfm.l.vz.x, xfm.l.vz.y, xfm.l.vz.z, 0,
           xfm.p.x,    xfm.p.y,    xfm.p.z,    1) {}


    /*! returns row index of the matrix for m[i][j] type access, where i = row index, j = column index */
    __forceinline const Vector& operator []( const size_t index ) const { MNRY_ASSERT(index < 4); return (&vx)[index]; }
    __forceinline       Vector& operator []( const size_t index )       { MNRY_ASSERT(index < 4); return (&vx)[index]; }

    /*! compute the determinant of the matrix.
     *
     * We assume most of the cases the last column of the matrix will be [0, 0, 0, 1], in which case
     * we only need to compute one 3x3 determinant.
     */
    __forceinline const Scalar det() const {
      typedef Mat3<Vec3<Scalar> > Mat3f;
      const Scalar d1 = (vx.w == 0) ? 0 : -vx.w * Mat3f(vy.toVec3(),vz.toVec3(),vw.toVec3()).det();
      const Scalar d2 = (vy.w == 0) ? 0 :  vy.w * Mat3f(vx.toVec3(),vz.toVec3(),vw.toVec3()).det();
      const Scalar d3 = (vz.w == 0) ? 0 : -vz.w * Mat3f(vx.toVec3(),vy.toVec3(),vw.toVec3()).det();
      const Scalar d4 = (vw.w == 0) ? 0 :  vw.w * Mat3f(vx.toVec3(),vy.toVec3(),vz.toVec3()).det();
      return d1 + d2 + d3 + d4;
    }

    /*! compute adjoint matrix */
    __forceinline const Mat4 adjoint() const {
      typedef Mat3<Vec3<Scalar> > Mat3f;
      const Scalar m00 =  Mat3f(vy.toVec3(1,2,3),vz.toVec3(1,2,3),vw.toVec3(1,2,3)).det();
      const Scalar m01 = -Mat3f(vy.toVec3(0,2,3),vz.toVec3(0,2,3),vw.toVec3(0,2,3)).det();
      const Scalar m02 =  Mat3f(vy.toVec3(0,1,3),vz.toVec3(0,1,3),vw.toVec3(0,1,3)).det();
      const Scalar m03 = -Mat3f(vy.toVec3(0,1,2),vz.toVec3(0,1,2),vw.toVec3(0,1,2)).det();
      const Scalar m10 = -Mat3f(vx.toVec3(1,2,3),vz.toVec3(1,2,3),vw.toVec3(1,2,3)).det();
      const Scalar m11 =  Mat3f(vx.toVec3(0,2,3),vz.toVec3(0,2,3),vw.toVec3(0,2,3)).det();
      const Scalar m12 = -Mat3f(vx.toVec3(0,1,3),vz.toVec3(0,1,3),vw.toVec3(0,1,3)).det();
      const Scalar m13 =  Mat3f(vx.toVec3(0,1,2),vz.toVec3(0,1,2),vw.toVec3(0,1,2)).det();
      const Scalar m20 =  Mat3f(vx.toVec3(1,2,3),vy.toVec3(1,2,3),vw.toVec3(1,2,3)).det();
      const Scalar m21 = -Mat3f(vx.toVec3(0,2,3),vy.toVec3(0,2,3),vw.toVec3(0,2,3)).det();
      const Scalar m22 =  Mat3f(vx.toVec3(0,1,3),vy.toVec3(0,1,3),vw.toVec3(0,1,3)).det();
      const Scalar m23 = -Mat3f(vx.toVec3(0,1,2),vy.toVec3(0,1,2),vw.toVec3(0,1,2)).det();
      const Scalar m30 = -Mat3f(vx.toVec3(1,2,3),vy.toVec3(1,2,3),vz.toVec3(1,2,3)).det();
      const Scalar m31 =  Mat3f(vx.toVec3(0,2,3),vy.toVec3(0,2,3),vz.toVec3(0,2,3)).det();
      const Scalar m32 = -Mat3f(vx.toVec3(0,1,3),vy.toVec3(0,1,3),vz.toVec3(0,1,3)).det();
      const Scalar m33 =  Mat3f(vx.toVec3(0,1,2),vy.toVec3(0,1,2),vz.toVec3(0,1,2)).det();

      // transposed
      return Mat4(m00,m10,m20,m30,
                  m01,m11,m21,m31,
                  m02,m12,m22,m32,
                  m03,m13,m23,m33);
    }

    /*! compute inverse matrix */
    __forceinline const Mat4 inverse() const { return rcp(det())*adjoint(); }

    /*! compute transposed matrix */
    __forceinline const Mat4 transposed() const { return Mat4(vx.x,vy.x,vz.x,vw.x,
                                                              vx.y,vy.y,vz.y,vw.y,
                                                              vx.z,vy.z,vz.z,vw.z,
                                                              vx.w,vy.w,vz.w,vw.w); }

    /*! convert to a quaternion, assuming this is a rotation matrix */
    __forceinline const Quat quat() const {
      return Quat(vx.toVec3(), vy.toVec3(), vz.toVec3());
    }




    /*! returns first row of matrix */
    __forceinline const Vector& row0() const { return vx; }

    /*! returns second row of matrix */
    __forceinline const Vector& row1() const { return vy; }

    /*! returns third row of matrix */
    __forceinline const Vector& row2() const { return vz; }

    /*! returns fourth row of matrix */
    __forceinline const Vector& row3() const { return vw; }

    /*! returns first column of matrix */
    __forceinline const Vector col0() const { return Vector(vx.x, vy.x, vz.x, vw.x); }

    /*! returns second column of matrix */
    __forceinline const Vector col1() const { return Vector(vx.y, vy.y, vz.y, vw.y); }

    /*! returns third column of matrix */
    __forceinline const Vector col2() const { return Vector(vx.z, vy.z, vz.z, vw.z); }

    /*! returns fourth column of matrix */
    __forceinline const Vector col3() const { return Vector(vx.w, vy.w, vz.w, vw.w); }

    ////////////////////////////////////////////////////////////////////////////////
    /// Constants
    ////////////////////////////////////////////////////////////////////////////////

    __forceinline Mat4(ZeroTy) : vx(zero), vy(zero), vz(zero), vw(zero) {}
    __forceinline Mat4(OneTy) : vx(one, zero, zero, zero), vy(zero, one, zero, zero), vz(zero, zero, one, zero), vw(zero, zero, zero, one) {}

    /*! set current matrix to represent a translation. */
    __forceinline void setToTranslation(const Vector& v) { *this = translate(v); }

    /*! set current matrix to rotate around arbitrary axis
     *  @param u the axis of rotation
     *  @param r the angle of rotation in radian
     */
    __forceinline void setToRotation(const Vector& u, const Scalar& r) {
      *this = rotate(u, r);
    }

    __forceinline void setToRotation(const Quat& q) {
      *this = Mat4(q);
    }

    /*! set current matrix to represent scaling of s */
    __forceinline void setToScale(const Vector& s) {
      *this = scale(s);
    }

    /*! return matrix for scaling */
    static __forceinline Mat4 scale(const Vector& s) {
      return Mat4(s.x,   0,   0, 0,
                    0, s.y,   0, 0,
                    0,   0, s.z, 0,
                    0,   0,   0, 1);
    }

    /*! return matrix for translation */
    static __forceinline Mat4 translate(const Vector& p) {
      return Mat4(  1,   0,   0, 0,
                    0,   1,   0, 0,
                    0,   0,   1, 0,
                  p.x, p.y, p.z, 1);
    }

    /*! return matrix for rotation around arbitrary axis
     *  @param u the axis of rotation
     *  @param r the angle of rotation in radian
     */
    static __forceinline Mat4 rotate(const Vector& u, const Scalar& r) {
      Vector v = normalize(u);
      Scalar s = sin(r);
      Scalar c = cos(r);
      Scalar t = 1-c;
      return Mat4(v.x*v.x*t+c,     v.x*v.y*t+v.z*s, v.x*v.z*t-v.y*s, 0,
                  v.y*v.x*t-v.z*s, v.y*v.y*t+c,     v.y*v.z*t+v.x*s, 0,
                  v.z*v.x*t+v.y*s, v.z*v.y*t-v.x*s, v.z*v.z*t+c,     0,
                  0, 0, 0, 1);
    }

    /*! return an orthonormal version of the supplied matrix */
    static __forceinline Mat4 orthonormalize(const Mat4 &m) {
      Mat4 r;
      asVec3(r.vx) = normalize(cross(asVec3(m.vy), asVec3(m.vz)));
      asVec3(r.vy) = normalize(cross(asVec3(m.vz), asVec3(r.vx)));
      asVec3(r.vz) = cross(asVec3(r.vx), asVec3(r.vy));
      r.vx.w = 0.f;
      r.vy.w = 0.f;
      r.vz.w = 0.f;
      r.vw = m.vw;
      return r;
    }

    /*! return 3x3 matrix linear part of the matrix (rotation, scale, shear)*/
    __forceinline Mat3T extract3x3() {
      return Mat3T(Mat3T::Vector(vx),Mat3T::Vector(vy),Mat3T::Vector(vz));
    }

  public:

    /*! the row vectors of the matrix */
    Vector vx,vy,vz,vw;
  };

  /*! convert to a Xform, removing the perspective part. */
  template<typename XformType, typename T> __forceinline const XformType xform(const Mat4<T>& m) {
    return XformType(m.vx.x,m.vx.y,m.vx.z,m.vy.x,m.vy.y,m.vy.z,m.vz.x,m.vz.y,m.vz.z,m.vw.x,m.vw.y,m.vw.z);
  }

  template<typename VectorT> __forceinline const VectorT& row(const Mat4<VectorT>& a, const size_t idx) {
      switch (idx) {
          case 0: return a.row0();
          case 1: return a.row1();
          case 2: return a.row2();
          case 3: return a.row3();
          default: MNRY_ASSERT(!"Bad Arg"); return a.row0();
      }
  }

  template<typename VectorT> __forceinline VectorT col(const Mat4<VectorT>& a, const size_t idx) {
      switch (idx) {
          case 0: return a.col0();
          case 1: return a.col1();
          case 2: return a.col2();
          case 3: return a.col3();
          default: MNRY_ASSERT(!"Bad Arg"); return a.col0();
      }
  }

  ////////////////////////////////////////////////////////////////////////////////
  // Unary Operators
  ////////////////////////////////////////////////////////////////////////////////

  template<typename T> __forceinline Mat4<T> operator-(const Mat4<T>& a) { return Mat4<T>(-a.vx,-a.vy,-a.vz,-a.vw); }
  template<typename T> __forceinline Mat4<T> operator+(const Mat4<T>& a) { return Mat4<T>(+a.vx,+a.vy,+a.vz,+a.vw); }
  template<typename T> __forceinline Mat4<T> rcp      (const Mat4<T>& a) { return a.inverse(); }


  /// Slerp between two 4x4 affine transformation matrices
  ///
  /// WARNING 1: In addition to interpolation, this function performs matrix decomposition
  /// and quaternion conversions on the input matrices.  You should NOT use this function if
  /// you need to perform many interpolations on the same pair of matrices.  In that that case you
  /// should decompose the matrices yourself into XformComponent classes and then slerp using
  /// the XformComponent classes.
  ///
  /// WARNING 2: This produces undefined results on non-affine matrices
  /// (i.e. perspective projection matrices)
  template<typename T, typename U> __forceinline Mat4<Vec4<T> > slerp(const Mat4<Vec4<T> >& a, const Mat4<Vec4<T> >& b, const U t) {
      typedef Mat3<Vec3<T> > Mat33T;
      typedef XformT<Mat33T> Xform33T;
      typedef XformComponent<Mat33T> XformComponent33T;

      XformComponent33T ca;
      XformComponent33T cb;
      decompose(xform<Xform33T>(a), ca);
      decompose(xform<Xform33T>(b), cb);
      if (dot(ca.r, cb.r) < 0) { cb.r *= U(-1.0); }
      return Mat4<Vec4<T> >(slerp(ca, cb, t).combined());
  }

  ////////////////////////////////////////////////////////////////////////////////
  // Binary Operators
  ////////////////////////////////////////////////////////////////////////////////

  template<typename VectorT> __forceinline Mat4<VectorT> operator+(const Mat4<VectorT>& a, const Mat4<VectorT>& b) { return Mat4<VectorT>(a.vx+b.vx,a.vy+b.vy,a.vz+b.vz,a.vw+b.vw); }
  template<typename VectorT> __forceinline Mat4<VectorT> operator-(const Mat4<VectorT>& a, const Mat4<VectorT>& b) { return Mat4<VectorT>(a.vx-b.vx,a.vy-b.vy,a.vz-b.vz,a.vw-b.vw); }

  /// scalar multiply
  template<typename VectorT> __forceinline Mat4<VectorT> operator*(const typename VectorT::Scalar& s,  const Mat4<VectorT>& m) { return Mat4<VectorT>(s*m.vx, s*m.vy, s*m.vz, s*m.vw); }
  /// scalar multiply
  template<typename VectorT> __forceinline Mat4<VectorT> operator*(const Mat4<VectorT>& m, const typename VectorT::Scalar & s) { return s * m; }

  /// vector pre-multiply
  template<typename VectorT> __forceinline VectorT       operator*(const Mat4<VectorT>& m, const VectorT      & v) { return VectorT(dot(m.vx,v), dot(m.vy,v), dot(m.vz,v), dot(m.vw,v)); }
  /// vector post-multiply
  template<typename VectorT> __forceinline VectorT       operator*(const VectorT      & v, const Mat4<VectorT>& m) { return v.x*m.vx + v.y*m.vy + v.z*m.vz + v.w*m.vw; }
  template<typename VectorT> __forceinline Mat4<VectorT> operator*(const Mat4<VectorT>& a, const Mat4<VectorT>& b) { return Mat4<VectorT>(a.vx*b, a.vy*b, a.vz*b, a.vw*b); }
  /// matrix divide: a * b.inverse()
  template<typename VectorT> __forceinline Mat4<VectorT> operator/(const Mat4<VectorT>& a, const Mat4<VectorT>& b) { return a * rcp(b); }

  template<typename VectorT> __forceinline Mat4<VectorT>& operator+=(Mat4<VectorT>& a, const Mat4<VectorT>& b) { return a = a + b; }
  template<typename VectorT> __forceinline Mat4<VectorT>& operator-=(Mat4<VectorT>& a, const Mat4<VectorT>& b) { return a = a - b; }
  template<typename VectorT> __forceinline Mat4<VectorT>& operator*=(Mat4<VectorT>& a, const Mat4<VectorT>& b) { return a = a * b; }
  /// matrix divide: a * b.inverse()
  template<typename VectorT> __forceinline Mat4<VectorT>& operator/=(Mat4<VectorT>& a, const Mat4<VectorT>& b) { return a = a / b; }

  /// Post-multiply transform
  template<typename VectorT> __forceinline VectorT       transform(const Mat4<VectorT>& m, const VectorT& v) { return v * m; }
  template<typename VectorT> __forceinline Vec3<typename VectorT::Scalar> transform3x3(const Mat4<VectorT>& m, const Vec3<typename VectorT::Scalar>& v) {
    return v.x * reinterpret_cast<Vec3<typename VectorT::Scalar> const &>(m.vx) +
           v.y * reinterpret_cast<Vec3<typename VectorT::Scalar> const &>(m.vy) +
           v.z * reinterpret_cast<Vec3<typename VectorT::Scalar> const &>(m.vz); }

  /// Pre-multiply transform
  template<typename VectorT> __forceinline VectorT    pretransform(const Mat4<VectorT>& m, const VectorT& v) { return m * v; }
  template<typename VectorT> __forceinline Vec3<typename VectorT::Scalar> pretransform3x3(const Mat4<VectorT>& m, const Vec3<typename VectorT::Scalar>& v) {
    return Vec3<typename VectorT::Scalar>(dot(reinterpret_cast<Vec3<typename VectorT::Scalar> const &>(m.vx),v),
                                          dot(reinterpret_cast<Vec3<typename VectorT::Scalar> const &>(m.vy),v),
                                          dot(reinterpret_cast<Vec3<typename VectorT::Scalar> const &>(m.vz),v)); }

  /// Transforms a Vec3 point.
  template<typename VectorT>
  __forceinline Vec3<typename VectorT::Scalar>
  transformPoint(const Mat4<VectorT>& m, const Vec3<typename VectorT::Scalar>& p)
  {
      Vec4<typename VectorT::Scalar> r = transform(m, Vec4<typename VectorT::Scalar>(p.x,p.y,p.z,1));
      return Vec3<typename VectorT::Scalar>(r.x,r.y,r.z);
  }

  /// Transforms a Vec3fa point.
  __forceinline Vec3fa
  transformPoint(const Mat4<Vec4f>& m, const Vec3fa& p)
  {
    Vec4f r = transform(m, Vec4f(p.x,p.y,p.z,1));
    return Vec3fa(r.x, r.y, r.z, 0.f);
  }

  /// Transforms a Vec3 vector. Translation is ignored.
  template<typename VectorT>
  __forceinline Vec3<typename VectorT::Scalar>
  transformVector(const Mat4<VectorT>& m, const Vec3<typename VectorT::Scalar>& v)
  {
    return transform3x3(m, v);
  }

  /// Transforms a Vec3fa vector. Translation is ignored.
  __forceinline Vec3fa
  transformVector(const Mat4<Vec4f>& m, const Vec3fa& v)
  {
      Vec3f r = transform3x3(m, v.asVec3f());
      return Vec3fa(r.x, r.y, r.z, 0.f);
  }

  /// Assuming m is an inverse matrix, does a pre-multiply without the need to
  /// transpose on a Vec3 normal. Translation is ignored.
  template<typename VectorT> __forceinline Vec3<typename VectorT::Scalar>
  transformNormal(const Mat4<VectorT>& m, const Vec3<typename VectorT::Scalar>& n)
  {
      return pretransform3x3(m, n);
  }

  /// Assuming m is an inverse matrix, does a pre-multiply without the need to
  /// transpose on a Vec3fa normal. Translation is ignored.
  __forceinline Vec3fa
  transformNormal(const Mat4<Vec4f>& m, const Vec3fa& n)
  {
      Vec3f r = pretransform3x3(m, n.asVec3f());
      return Vec3fa(r.x, r.y, r.z, 0.f);
  }

  /// Transform a Vec3 by post-multiplication, doing homogeneous divison.
  template<typename VectorT> __forceinline Vec3<typename VectorT::Scalar> transformH(const Mat4<VectorT>& m, const Vec3<typename VectorT::Scalar>& p) {
      Vec4<typename VectorT::Scalar> v = transform(m, VectorT(p.x,p.y,p.z,1));
      if (v.w != typename VectorT::Scalar(0)) return Vec3<typename VectorT::Scalar>(v.x / v.w, v.y / v.w, v.z / v.w);
      return Vec3<typename VectorT::Scalar>(0, 0, 0);
  }

  /// Transforms an AABB to an AABB. This is based on this:
  /// http://dev.theomader.com/transform-bounding-boxes/
  __forceinline BBox3f
  transformBBox(const Mat4<Vec4f>& m, const BBox3f& bb)
  {
    const auto xa = asVec3(m.row0() * bb.lower.x);
    const auto xb = asVec3(m.row0() * bb.upper.x);
                    
    const auto ya = asVec3(m.row1() * bb.lower.y);
    const auto yb = asVec3(m.row1() * bb.upper.y);

    const auto za = asVec3(m.row2() * bb.lower.z);
    const auto zb = asVec3(m.row2() * bb.upper.z);

    return BBox3f(
      min(xa, xb) + min(ya, yb) + min(za, zb) + asVec3(m.row3()),
      max(xa, xb) + max(ya, yb) + max(za, zb) + asVec3(m.row3())
    );
  }

  ////////////////////////////////////////////////////////////////////////////////
  /// Comparison Operators
  ////////////////////////////////////////////////////////////////////////////////

  template<typename VectorT> __forceinline bool operator==( const Mat4<VectorT>& a, const Mat4<VectorT>& b) { return a.vx == b.vx && a.vy == b.vy && a.vz == b.vz && a.vw == b.vw; }
  template<typename VectorT> __forceinline bool operator!=( const Mat4<VectorT>& a, const Mat4<VectorT>& b) { return a.vx != b.vx || a.vy != b.vy || a.vz != b.vz || a.vw != b.vw; }

  template<typename VectorT> __forceinline bool isEqual(const Mat4<VectorT>& a, const Mat4<VectorT>& b, typename VectorT::Scalar eps = typename VectorT::Scalar(epsilon))  { return isEqual(a.vx, b.vx, eps) && isEqual(a.vy, b.vy, eps) && isEqual(a.vz, b.vz, eps) && isEqual(a.vw, b.vw, eps); }

  template<typename VectorT> __forceinline bool isEqualFixedEps(const Mat4<VectorT>& a, const Mat4<VectorT>& b, typename VectorT::Scalar eps = typename VectorT::Scalar(epsilon))  { return isEqualFixedEps(a.vx, b.vx, eps) && isEqualFixedEps(a.vy, b.vy, eps) && isEqualFixedEps(a.vz, b.vz, eps) && isEqualFixedEps(a.vw, b.vw, eps); }

  ////////////////////////////////////////////////////////////////////////////////
  /// Output Operators
  ////////////////////////////////////////////////////////////////////////////////

  template<typename VectorT> static std::ostream& operator<<(std::ostream& cout, const Mat4<VectorT>& m) {
    return cout << "{ vx = " << m.vx << ", vy = " << m.vy << ", vz = " << m.vz << ", vw = " << m.vw << " }";
  }

  /*! Shortcuts for common linear spaces. */
  /// Scalar vector type
  typedef Mat4<Vec4f> Mat4f;
  typedef Mat4<Vec4d> Mat4d;

  ////////////////////////////////////////////////////////////////////////////////
  /// asIspc() and asCpp() C++ <--> ISPC Type-casting functions
  ////////////////////////////////////////////////////////////////////////////////
  HUD_AS_ISPC_FUNCTIONS(Mat4f);

  HUD_AS_CPP_FUNCTIONS(Mat4f);

  /// down-cast a Mat4d to a Mat4f
  __forceinline Mat4f toFloat(const Mat4d &m) {
      return Mat4f(toFloat(m.vx), toFloat(m.vy), toFloat(m.vz), toFloat(m.vw));
  }

  /// up-cast a Mat4f to a Mat4d
  __forceinline Mat4d toDouble(const Mat4f &m) {
      return Mat4d(toDouble(m.vx), toDouble(m.vy), toDouble(m.vz), toDouble(m.vw));
  }

}
}

