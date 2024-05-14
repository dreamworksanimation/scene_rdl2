// Copyright 2023-2024 DreamWorks Animation LLC and Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

// MoonRay: begin *****
#include "simd.h"
#include "Quaternion.h"
#include "Vec3.h"
// MoonRay: end *****
// Intel: begin *****
/*
#include "vec3.h"
#include "quaternion.h"
*/
// Intel: end *****

namespace scene_rdl2 {
namespace math {
// Intel: namespace embree {

// MoonRay: added this large comment
  /** \brief 3x3 Column Major Matrix
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
   *  <b>Row vector</b> means vectors are defined as v = [x, y, z], also can be considered as a 1 by n matrix, where n = 3.
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
  template<typename T> struct Mat3
  // Intel: template<typename T> struct LinearSpace3
  {
    typedef T Vector;
    typedef typename T::Scalar Scalar;

    // MoonRay: added
    typedef QuaternionT<Scalar> Quat;

    /*! default matrix constructor */
    __forceinline Mat3           ( ) {}
    __forceinline Mat3           ( const Mat3& other ) { vx = other.vx; vy = other.vy; vz = other.vz; }
    __forceinline Mat3& operator=( const Mat3& other ) { vx = other.vx; vy = other.vy; vz = other.vz; return *this; }

    template<typename L1> __forceinline explicit Mat3( const Mat3<L1>& s ) : vx(s.vx), vy(s.vy), vz(s.vz) {}

    /*! matrix construction from <b>row vectors</b> */
    // MoonRay: vx->vxParam, vy->vyParam, vz->vzParam
    __forceinline Mat3(const Vector& vxParam, const Vector& vyParam, const Vector& vzParam)
      : vx(vxParam), vy(vyParam), vz(vzParam) {}

    /*! construction from quaternion */
    __forceinline Mat3( const Quat& q )
      : vx(1.0f-2.0f*(q.j*q.j + q.k*q.k),   2.0f*(q.i*q.j + q.r*q.k),     2.0f*(q.i*q.k - q.r*q.j))
      , vy(     2.0f*(q.i*q.j - q.r*q.k), 1-2.0f*(q.i*q.i + q.k*q.k),     2.0f*(q.j*q.k + q.r*q.i))
      , vz(     2.0f*(q.i*q.k + q.r*q.j),   2.0f*(q.j*q.k - q.r*q.i), 1.0-2.0f*(q.i*q.i + q.j*q.j)) {}
    // Intel: uses different formula:
    //     : vx((q.r*q.r + q.i*q.i - q.j*q.j - q.k*q.k), 2.0f*(q.i*q.j + q.r*q.k), 2.0f*(q.i*q.k - q.r*q.j))
    //  , vy(2.0f*(q.i*q.j - q.r*q.k), (q.r*q.r - q.i*q.i + q.j*q.j - q.k*q.k), 2.0f*(q.j*q.k + q.r*q.i))
    //  , vz(2.0f*(q.i*q.k + q.r*q.j), 2.0f*(q.j*q.k - q.r*q.i), (q.r*q.r - q.i*q.i - q.j*q.j + q.k*q.k)) {}

    /*! matrix construction from <b>row major</b> data */
    __forceinline Mat3(const Scalar& m00, const Scalar& m01, const Scalar& m02,
                       const Scalar& m10, const Scalar& m11, const Scalar& m12,
                       const Scalar& m20, const Scalar& m21, const Scalar& m22)
      : vx(m00,m01,m02), vy(m10,m11,m12), vz(m20,m21,m22) {}

    /*! returns row index of the matrix for m[i][j] type access, where i = row index, j = column index */
    __forceinline const Vector& operator []( const size_t index ) const { MNRY_ASSERT(index < 3); return (&vx)[index]; }
    __forceinline       Vector& operator []( const size_t index )       { MNRY_ASSERT(index < 3); return (&vx)[index]; }

    /*! compute the determinant of the matrix */
    __forceinline const Scalar det() const { return dot(vx,cross(vy,vz)); }

    /*! compute adjoint matrix */
    __forceinline const Mat3 adjoint() const { return Mat3(cross(vy,vz),cross(vz,vx),cross(vx,vy)).transposed(); }

    /*! compute inverse matrix */
    __forceinline const Mat3 inverse() const { return (1.0f/det())*adjoint(); }
    // Intel: return adjoint()/det();

    /*! compute transposed matrix */
    __forceinline const Mat3 transposed() const { return Mat3(vx.x,vy.x,vz.x,
                                                              vx.y,vy.y,vz.y,
                                                              vx.z,vy.z,vz.z); }

    // MoonRay: added
    /*! convert to a quaternion, assuming this is a rotation matrix */
    __forceinline const Quat quat() const {
      return Quat(vx, vy, vz);
    }

    /*! returns first row of matrix */
    __forceinline const Vector& row0() const { return vx; }
    // Intel: __forceinline const Vector row0() const { return Vector(vx.x,vy.x,vz.x); }

    /*! returns second row of matrix */
    __forceinline const Vector& row1() const { return vy; }
    // Intel: __forceinline const Vector row1() const { return Vector(vx.y,vy.y,vz.y); }

    /*! returns third row of matrix */
    __forceinline const Vector& row2() const { return vz; }
    // Intel: __forceinline const Vector row2() const { return Vector(vx.z,vy.z,vz.z); }

// MoonRay: begin *****
    /*! returns first column of matrix */
    __forceinline const Vector col0() const { return Vector(vx.x, vy.x, vz.x); }

    /*! returns second column of matrix */
    __forceinline const Vector col1() const { return Vector(vx.y, vy.y, vz.y); }

    /*! returns third column of matrix */
    __forceinline const Vector col2() const { return Vector(vx.z, vy.z, vz.z); }
// MoonRay:: end *****

    ////////////////////////////////////////////////////////////////////////////////
    /// Constants
    ////////////////////////////////////////////////////////////////////////////////

    __forceinline Mat3(ZeroTy) : vx(zero), vy(zero), vz(zero) {}
    __forceinline Mat3(OneTy) : vx(one, zero, zero), vy(zero, one, zero), vz(zero, zero, one) {}

// MoonRay: begin *****
    /*! set current matrix to rotate around arbitrary axis
     *  @param u the axis of rotation
     *  @param r the angle of rotation in radian
     */
    __forceinline void setToRotation(const Vector& u, const Scalar& r) {
      *this = rotate(u, r);
    }

    __forceinline void setToRotation(const Quat& q) {
      *this = Mat3(q);
    }

    /*! set current matrix to represent scaling of s */
    __forceinline void setToScale(const Vector& s) {
      *this = scale(s);
    }
// MoonRay: end *****

    /*! return matrix for scaling */
    static __forceinline Mat3 scale(const Vector& s) {
      return Mat3(s.x,   0,   0,
                    0, s.y,   0,
                    0,   0, s.z);
    }

    /*! return matrix for rotation around arbitrary axis
     *  @param u the axis of rotation
     *  @param r the angle of rotation in radian
     */
    static __forceinline Mat3 rotate(const Vector& u, const Scalar& r) {
      Vector v = normalize(u);
      Scalar s = sin(r);
      Scalar c = cos(r);
      Scalar t = 1-c;
      return Mat3(v.x*v.x*t+c,     v.x*v.y*t+v.z*s, v.x*v.z*t-v.y*s,
                  v.y*v.x*t-v.z*s, v.y*v.y*t+c,     v.y*v.z*t+v.x*s,
                  v.z*v.x*t+v.y*s, v.z*v.y*t-v.x*s, v.z*v.z*t+c);
    }
// Intel:
/*
    // return matrix for rotation around arbitrary axis
    static __forceinline LinearSpace3 rotate(const Vector& _u, const Scalar& r) {
      Vector u = normalize(_u);
      Scalar s = sin(r), c = cos(r);
      return LinearSpace3(u.x*u.x+(1-u.x*u.x)*c,  u.x*u.y*(1-c)-u.z*s,    u.x*u.z*(1-c)+u.y*s,
                          u.x*u.y*(1-c)+u.z*s,    u.y*u.y+(1-u.y*u.y)*c,  u.y*u.z*(1-c)-u.x*s,
                          u.x*u.z*(1-c)-u.y*s,    u.y*u.z*(1-c)+u.x*s,    u.z*u.z+(1-u.z*u.z)*c);
    }
*/

  public:

    /*! the row vectors of the matrix */
// Intel: /*! the column vectors of the matrix */
    Vector vx,vy,vz;
  };

#if !defined(__MIC__)
  /*! compute transposed matrix */
  template<> __forceinline const Mat3<Vec3fa> Mat3<Vec3fa>::transposed() const { 
    // MoonRay: added consts
    ssef rx,ry,rz; transpose((const ssef&)vx,(const ssef&)vy,(const ssef&)vz,ssef(zero),rx,ry,rz);
    return Mat3<Vec3fa>(Vec3fa(rx),Vec3fa(ry),Vec3fa(rz)); 
  }
#endif

// MoonRay: added begin *****
  template<typename VectorT> __forceinline const VectorT& row(const Mat3<VectorT>& a, const size_t idx) {
    switch (idx) {
      case 0: return a.row0();
      case 1: return a.row1();
      case 2: return a.row2();
      default: MNRY_ASSERT(!"Bad Arg"); return a.row0();
    }
  }

  template<typename VectorT> __forceinline VectorT col(const Mat3<VectorT>& a, const size_t idx) {
    switch (idx) {
      case 0: return a.col0();
      case 1: return a.col1();
      case 2: return a.col2();
      default: MNRY_ASSERT(!"Bad Arg"); return a.col0();
    }
  }
  // MoonRay: added end *****

  ////////////////////////////////////////////////////////////////////////////////
  // Unary Operators
  ////////////////////////////////////////////////////////////////////////////////

  template<typename T> __forceinline Mat3<T> operator-(const Mat3<T>& a) { return Mat3<T>(-a.vx,-a.vy,-a.vz); }
  template<typename T> __forceinline Mat3<T> operator+(const Mat3<T>& a) { return Mat3<T>(+a.vx,+a.vy,+a.vz); }

  /*! constructs a coordinate frame form a normal */
  template<typename T> __forceinline Mat3<T> frame(const T& N) {
    T dx0 = cross(T(one,zero,zero),N);
    T dx1 = cross(T(zero,one,zero),N);
    T dx = normalize(select(dot(dx0,dx0) > dot(dx1,dx1),dx0,dx1));
    T dy = normalize(cross(N,dx));
    return Mat3<T>(dx,dy,N);
  }

// MoonRay: added begin *****

  /// Slerp between two rotation matrices
  /// WARNING: This only works if the matrix only contains rotation
  /// and produces undefined results otherwise. In the general case you should
  /// use slerp()
  template<typename VectorT, typename U> __forceinline Mat3<VectorT> slerpR(const Mat3<VectorT>& a, const Mat3<VectorT>& b, const U t) {
    typename Mat3<VectorT>::Quat qa = a.quat();
    typename Mat3<VectorT>::Quat qb = b.quat();
    if (dot(qa, qb) < 0) { qb *= U(-1); }
    return Mat3<VectorT>(slerp(qa, qb, t));
  }

  /// Slerp between two 3x3 rotation/scale matrices
  /// This produces undefined results on matrix contains reflection operation
  template<typename VectorT, typename U> __forceinline Mat3<VectorT> slerp(const Mat3<VectorT>& a, const Mat3<VectorT>& b, const U t) {
    typename Mat3<VectorT>::Quat qa;
    Mat3<VectorT> sa;
    decompose(a, sa, qa);
    typename Mat3<VectorT>::Quat qb;
    Mat3<VectorT> sb;
    decompose(b, sb, qb);

    return lerp(sa, sb, t) * (Mat3<VectorT>(slerp(qa, qb, t)));
  }

  enum DecomposeErrorCode
  {
      SUCCESS,
      SINGULAR,
      FLIPPED,
      MAX_ITER,
  };


  /// Perform a polar decomposition into a scale matrix and a rotation matrix,
  ///   M = S * R
  /// where R is returned in the form of a quaternion q.
  /// Note that in general S will not be a diagonal matrix, since the transformation it represents is not tied to
  /// any particular coordinate frame, i.e. the 3 mutually orthogonal scaling directions can be arbitrary.
  /// (In fact the directions will be the eigenvectors of S.)
  /// For further details see:
  /// [1] "Polar Matrix Decomposition" - Ken Shoemake. Graphics Gems IV
  /// [2] "Matrix Animation and Polar Decomposition" - Ken Shoemake, Tom Duff
  /// Note that the order of matrix multiplication differs from these papers, because we express points as row vectors.
  template<typename VectorT>
  __forceinline DecomposeErrorCode decompose(const Mat3<VectorT>& M, Mat3<VectorT>& S, typename Mat3<VectorT>::Quat& q)
  {
    const float detM = M.det();
  
    // Non-positive determinants not supported
    if (detM <= 0.0f) {
      q = one;
      S = M;
      return (detM == 0.0f) ? DecomposeErrorCode::SINGULAR : DecomposeErrorCode::FLIPPED;
    }
  
    typedef Mat3<VectorT> Mat3T;
    constexpr int maxCount = 100;
    Mat3T R = M;

    int count = 0;
    for (count = 0; count < maxCount; count++) {
      const Mat3T Rnext = 0.5f * (R + R.transposed().inverse());
      const Mat3T Rdiff = Rnext - R;

      typename VectorT::Scalar norm = 0.0f;
      for (size_t i = 0; i < 3; ++i) {
        norm = max(norm, abs(Rdiff[i].x)+abs(Rdiff[i].y)+abs(Rdiff[i].z));
      }

      // Note that if norm is below the threshold, we break here before updating R. This allows an input matrix that's
      // a pure rotation to pass straight through without modification.
      if (norm <= sEpsilon) break;

      R = Rnext;
    }
  
    q = R.quat();
    S = M * R.inverse();

    MNRY_ASSERT(count < maxCount, "Reached max iterations in matrix decompose function; result may be unexpected.");
    if (count >= maxCount) {
        return DecomposeErrorCode::MAX_ITER;
    }

    return DecomposeErrorCode::SUCCESS;
  }

// MoonRay: added end *****

  ////////////////////////////////////////////////////////////////////////////////
  // Binary Operators
  ////////////////////////////////////////////////////////////////////////////////

// MoonRay: begin *****
  template<typename VectorT> __forceinline Mat3<VectorT> operator+(const Mat3<VectorT>& a, const Mat3<VectorT>& b) { return Mat3<VectorT>(a.vx+b.vx,a.vy+b.vy,a.vz+b.vz); }
  template<typename VectorT> __forceinline Mat3<VectorT> operator-(const Mat3<VectorT>& a, const Mat3<VectorT>& b) { return Mat3<VectorT>(a.vx-b.vx,a.vy-b.vy,a.vz-b.vz); }

  /// scalar multiply
  template<typename VectorT> __forceinline Mat3<VectorT> operator*(const typename VectorT::Scalar& s,  const Mat3<VectorT>& m) { return Mat3<VectorT>(s*m.vx, s*m.vy, s*m.vz); }
  /// scalar multiply
  template<typename VectorT> __forceinline Mat3<VectorT> operator*(const Mat3<VectorT>& m, const typename VectorT::Scalar & s) { return s * m; }

  /// vector pre-multiply
  template<typename VectorT> __forceinline VectorT       operator*(const Mat3<VectorT>& m, const VectorT      & v) { return VectorT(dot(m.vx,v), dot(m.vy,v), dot(m.vz,v)); }
  /// vector post-multiply
  template<typename VectorT> __forceinline VectorT       operator*(const VectorT      & v, const Mat3<VectorT>& m) { return v.x*m.vx + v.y*m.vy + v.z*m.vz; }
  template<typename VectorT> __forceinline Mat3<VectorT> operator*(const Mat3<VectorT>& a, const Mat3<VectorT>& b) { return Mat3<VectorT>(a.vx*b, a.vy*b, a.vz*b); }
  /// matrix divide: a * b.inverse()
  template<typename VectorT> __forceinline Mat3<VectorT> operator/(const Mat3<VectorT>& a, const Mat3<VectorT>& b) { return a * b.inverse(); }

  template<typename VectorT> __forceinline Mat3<VectorT>& operator+=(Mat3<VectorT>& a, const Mat3<VectorT>& b) { return a = a + b; }
  template<typename VectorT> __forceinline Mat3<VectorT>& operator-=(Mat3<VectorT>& a, const Mat3<VectorT>& b) { return a = a - b; }
  template<typename VectorT> __forceinline Mat3<VectorT>& operator*=(Mat3<VectorT>& a, const Mat3<VectorT>& b) { return a = a * b; }
  /// matrix divide: a * b.inverse()
  template<typename VectorT> __forceinline Mat3<VectorT>& operator/=(Mat3<VectorT>& a, const Mat3<VectorT>& b) { return a = a / b; }

  /// post-multiply transform
  template<typename VectorT> __forceinline VectorT       transform(const Mat3<VectorT>& m, const VectorT& v) { return v * m; }
  /// pre-multiply transform
  template<typename VectorT> __forceinline VectorT    pretransform(const Mat3<VectorT>& m, const VectorT& v) { return m * v; }
  /// transforms a point
  template<typename VectorT> __forceinline VectorT  transformPoint(const Mat3<VectorT>& m, const VectorT& p) { return transform(m, p); }
  /// transforms a vector
  template<typename VectorT> __forceinline VectorT transformVector(const Mat3<VectorT>& m, const VectorT& v) { return transform(m, v); }
  /// assuming s is an inverse matrix, does a pre-multiply without the need to transpose
  template<typename VectorT> __forceinline VectorT transformNormal(const Mat3<VectorT>& m, const VectorT& n) { return pretransform(m, n); }
// MoonRay: end *****
// Intel: begin *****
/*
  template<typename T> __forceinline LinearSpace3<T> operator +( const LinearSpace3<T>& a, const LinearSpace3<T>& b ) { return LinearSpace3<T>(a.vx+b.vx,a.vy+b.vy,a.vz+b.vz); }
  template<typename T> __forceinline LinearSpace3<T> operator -( const LinearSpace3<T>& a, const LinearSpace3<T>& b ) { return LinearSpace3<T>(a.vx-b.vx,a.vy-b.vy,a.vz-b.vz); }

  template<typename T> __forceinline LinearSpace3<T> operator*(const typename T::Scalar & a, const LinearSpace3<T>& b) { return LinearSpace3<T>(a*b.vx, a*b.vy, a*b.vz); }
  template<typename T> __forceinline T               operator*(const LinearSpace3<T>& a, const T              & b) { return b.x*a.vx + b.y*a.vy + b.z*a.vz; }
  template<typename T> __forceinline LinearSpace3<T> operator*(const LinearSpace3<T>& a, const LinearSpace3<T>& b) { return LinearSpace3<T>(a*b.vx, a*b.vy, a*b.vz); }

  template<typename T> __forceinline LinearSpace3<T> operator/(const LinearSpace3<T>& a, const typename T::Scalar & b) { return LinearSpace3<T>(a.vx/b, a.vy/b, a.vz/b); }
  template<typename T> __forceinline LinearSpace3<T> operator/(const LinearSpace3<T>& a, const LinearSpace3<T>& b) { return a * rcp(b); }

  template<typename T> __forceinline LinearSpace3<T>& operator *=( LinearSpace3<T>& a, const LinearSpace3<T>& b ) { return a = a * b; }
  template<typename T> __forceinline LinearSpace3<T>& operator /=( LinearSpace3<T>& a, const LinearSpace3<T>& b ) { return a = a / b; }

  template<typename T> __forceinline T       xfmPoint (const LinearSpace3<T>& s, const T      & a) { return madd(T(a.x),s.vx,madd(T(a.y),s.vy,T(a.z)*s.vz)); }
  template<typename T> __forceinline T       xfmVector(const LinearSpace3<T>& s, const T      & a) { return madd(T(a.x),s.vx,madd(T(a.y),s.vy,T(a.z)*s.vz)); }
  template<typename T> __forceinline T       xfmNormal(const LinearSpace3<T>& s, const T      & a) { return xfmVector(s.inverse().transposed(),a); }
*/
// Intel: end *****

  ////////////////////////////////////////////////////////////////////////////////
  /// Comparison Operators
  ////////////////////////////////////////////////////////////////////////////////

  template<typename VectorT> __forceinline bool operator==( const Mat3<VectorT>& a, const Mat3<VectorT>& b) { return a.vx == b.vx && a.vy == b.vy && a.vz == b.vz; }
  template<typename VectorT> __forceinline bool operator!=( const Mat3<VectorT>& a, const Mat3<VectorT>& b) { return a.vx != b.vx || a.vy != b.vy || a.vz != b.vz; }

  // MoonRay: added these two
  template<typename VectorT> __forceinline bool isEqual(const Mat3<VectorT>& a, const Mat3<VectorT>& b, typename VectorT::Scalar eps = typename VectorT::Scalar(epsilon))  { return isEqual(a.vx, b.vx, eps) && isEqual(a.vy, b.vy, eps) && isEqual(a.vz, b.vz, eps); }
  template<typename VectorT> __forceinline bool isEqualFixedEps(const Mat3<VectorT>& a, const Mat3<VectorT>& b, typename VectorT::Scalar eps = typename VectorT::Scalar(epsilon))  { return isEqualFixedEps(a.vx, b.vx, eps) && isEqualFixedEps(a.vy, b.vy, eps) && isEqualFixedEps(a.vz, b.vz, eps); }

  ////////////////////////////////////////////////////////////////////////////////
  /// Output Operators
  ////////////////////////////////////////////////////////////////////////////////

  template<typename VectorT> static std::ostream& operator<<(std::ostream& cout, const Mat3<VectorT>& m) {
    return cout << "{ vx = " << m.vx << ", vy = " << m.vy << ", vz = " << m.vz << " }";
  }

  /*! Shortcuts for common linear spaces. */
  // MoonRay: begin *****
  /// Scalar vector type
  typedef Mat3<Vec3f> Mat3f;
  /// Scalar double precision
  typedef Mat3<Vec3d> Mat3d;
  // MoonRay: end *****
  
  // Intel: typedef LinearSpace3<Vec3fa> LinearSpace3f;
  // Intel: typedef LinearSpace3<Vec3fa> LinearSpace3fa;
}
}

