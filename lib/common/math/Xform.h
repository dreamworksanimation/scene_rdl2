// Copyright 2023-2024 DreamWorks Animation LLC and Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

// MoonRay: begin *****
#include <scene_rdl2/common/platform/HybridUniformData.h>
#include <scene_rdl2/common/platform/Platform.h>
#include "BBox.h"
#include "Mat3.h"
#include "Quaternion.h"

// Forward declaration of the ISPC types
namespace ispc {
    struct Xform3f;
}
// MoonRay: end *****
// Intel: begin *****
/*
#include "math/linearspace3.h"
#include "math/quaternion.h"
#include "math/bbox.h"
*/
// Intel: end *****

namespace scene_rdl2 {
namespace math {
// Intel: namespace embree {
  #define VectorT typename L::Vector
  #define ScalarT typename L::Vector::Scalar

  // MoonRay: added
  #define QuatT typename L::Quat

  // MoonRay: replaced Intel comment with a more detailed one
  /** \brief Xform, a representation of transformations, includes a linear part and an affine part.
   *
   *  Linear part is in a 3 by 3 matrix in column order representing the rotation, scale, and shear.
   *  The affine part is a vector representing the translation.
   *
   *  Concatenate transformations should be multiplied to the left in order, for example:
   *
   *  translate -> rotate -> scale
   *
   *  is the same as:
   *
   *  x = scale * (rotate * translate))
   *
   *  The set of <b>transform*()</b> interface should be preferable over multiplication since their intentions are unambiguous.
   */
    template<typename L>
    struct XformT
    // Intel: struct AffineSpaceT
    {
      L l;           /*< linear part of affine space */
      VectorT p;     /*< affine part of affine space */

      ////////////////////////////////////////////////////////////////////////////////
      // Constructors, Assignment, Cast, Copy Operations
      ////////////////////////////////////////////////////////////////////////////////

      __forceinline XformT           ( )                           { }
      __forceinline XformT           ( const XformT& other ) { l = other.l; p = other.p; }
      
      // MoonRay: begin *****
    
      __forceinline XformT           ( const L     & other ) { l = other  ; p = VectorT(zero); }
      __forceinline XformT& operator=( const XformT& other ) { l = other.l; p = other.p; return *this; }

      /*! matrix construction from <b>row major</b> data */
      __forceinline XformT(const ScalarT& m00, const ScalarT& m01, const ScalarT& m02,
                           const ScalarT& m10, const ScalarT& m11, const ScalarT& m12,
                           const ScalarT& m20, const ScalarT& m21, const ScalarT& m22,
                           const ScalarT& m30, const ScalarT& m31, const ScalarT& m32)
      : l(m00,m01,m02,m10,m11,m12,m20,m21,m22), p(m30,m31,m32) {}

      /*! <b>column order</b> vector */
      __forceinline XformT( const VectorT& vx, const VectorT& vy, const VectorT& vz, const VectorT& pParam ) : l(vx,vy,vz), p(pParam) {}
      __forceinline XformT( const L& lParam, const VectorT& pParam ) : l(lParam), p(pParam) {}

      template<typename L1> __forceinline explicit XformT( const XformT<L1>& s ) : l(s.l), p(s.p) {}

      /*! returns first row of linear part of affine space */
      __forceinline const VectorT& row0() const { return l.row0(); }

      /*! returns second row of linear part of affine space */
      __forceinline const VectorT& row1() const { return l.row1(); }

      /*! returns third row of linear part of affine space */
      __forceinline const VectorT& row2() const { return l.row2(); }

      /*! returns fourth row of matrix = affine part of affine space */
      __forceinline const VectorT& row3() const { return p; }

      // MoonRay: end *****
      // Intel: begin *****
      /*
      __forceinline AffineSpaceT           ( )                           { }
      __forceinline AffineSpaceT           ( const AffineSpaceT& other ) { l = other.l; p = other.p; }
      __forceinline AffineSpaceT           ( const L           & other ) { l = other  ; p = VectorT(zero); }
      __forceinline AffineSpaceT& operator=( const AffineSpaceT& other ) { l = other.l; p = other.p; return *this; }

      __forceinline AffineSpaceT( const VectorT& vx, const VectorT& vy, const VectorT& vz, const VectorT& p ) : l(vx,vy,vz), p(p) {}
      __forceinline AffineSpaceT( const L& l, const VectorT& p ) : l(l), p(p) {}

      template<typename L1> __forceinline explicit AffineSpaceT( const AffineSpaceT<L1>& s ) : l(s.l), p(s.p) {}
      */
      // Intel: end *****

      ////////////////////////////////////////////////////////////////////////////////
      // Constants
      ////////////////////////////////////////////////////////////////////////////////

      __forceinline XformT( ZeroTy ) : l(zero), p(zero) {}
      __forceinline XformT( OneTy )  : l(one),  p(zero) {}

// MoonRay: added begin *****

      /*! returns the inverse of this transform. */
      __forceinline XformT inverse() const { L il = l.inverse(); return XformT<L>(il,-(p*il)); }

      /*! set current matrix to represent a translation. */
      __forceinline void setToTranslation(const VectorT& pParam) { *this = translate(pParam); }

      /*! set current matrix to represent a rotation.
       *  @param u the axis of rotation
       *  @param r the angle of rotation in radian
       */
      __forceinline void setToRotation(const VectorT& u, const ScalarT& r) { *this = rotate(u, r); }

      /*! set current matrix to represent a scale. */
      __forceinline void setToScale(const VectorT& s) { *this = scale(s); }

  // MoonRay: added end *****

      /*! return matrix for scaling */
      static __forceinline XformT scale(const VectorT& s) { return XformT(L::scale(s),zero); }

      /*! return matrix for translation */
      static __forceinline XformT translate(const VectorT& pParam) { return XformT(one,pParam); }

      /*! return matrix for rotation around arbitrary axis
       *  @param u the axis of rotation
       *  @param r the angle of rotation in radian
       */
      static __forceinline XformT rotate(const VectorT& u, const ScalarT& r) { return XformT(L::rotate(u,r),zero); }

      /*! return matrix for rotation around arbitrary axis and point
       *  @param p origin of rotation
       *  @param u the axis of rotation
       *  @param r the angle of rotation in radian
       */
      static __forceinline XformT rotate(const VectorT& p, const VectorT& u, const ScalarT& r) { return translate(+p) * rotate(u,r) * translate(-p);  }

      /*! return matrix for looking at given point */
      static __forceinline XformT lookAtPoint(const VectorT& eye, const VectorT& point, const VectorT& up) {
        VectorT Z = normalize(eye-point);       // our camera looks down negative z
        // Intel: VectorT Z = normalize(point-eye);
        VectorT U = normalize(cross(up,Z));
        VectorT V = normalize(cross(Z,U));
        return XformT(L(U.x, U.y, U.z,
                        V.x, V.y, V.z,
                        Z.x, Z.y, Z.z),
                      eye);
        // Intel: return AffineSpaceT(L(U,V,Z),eye);
      }

    };

// MoonRay: added begin *****

    template<typename L> __forceinline const VectorT& row(const XformT<L>& a, const size_t idx) {
      switch (idx) {
        case 0: return a.row0();
        case 1: return a.row1();
        case 2: return a.row2();
        case 3: return a.row3();
        default: MNRY_ASSERT(!"Bad Arg"); return a.row0();
      }
    }

  /// A Xform can be decompose into translate, rotate, and scale components
  template<typename L> // expecting L = Mat3 of some sort
  struct XformComponent {
    XformComponent()
    : t(zero),
      r(zero),
      s(one)
    {}

    XformT<L> combined() const
    {
      return XformT<L>(s * L(r), t);
    }

    VectorT t; // translate
    QuatT r;   // rotate
    L s;       // scale
  };

  template<typename L> static std::ostream& operator<<(std::ostream& cout, const XformComponent<L>& m) {
    return cout << "{ t = " << m.t << ", r = " << m.r << ", s = " << m.s << " }";
  }

  template<typename L> __forceinline XformComponent<L> slerp(const XformComponent<L>& a, const XformComponent<L>& b, const ScalarT t) {
    XformComponent<L> l2p;
    l2p.t =  lerp(a.t, b.t, t);
    l2p.r = normalize(slerp(a.r, b.r, t));
    l2p.s =  lerp(a.s, b.s, t);
    return l2p;
  }

// MoonRay: added end *****

  ////////////////////////////////////////////////////////////////////////////////
  // Unary Operators
  ////////////////////////////////////////////////////////////////////////////////

  template<typename L> __forceinline XformT<L> operator -( const XformT<L>& a ) { return XformT<L>(-a.l,-a.p); }
  template<typename L> __forceinline XformT<L> operator +( const XformT<L>& a ) { return XformT<L>(+a.l,+a.p); }

  ////////////////////////////////////////////////////////////////////////////////
  // Binary Operators
  ////////////////////////////////////////////////////////////////////////////////

  template<typename L> __forceinline const XformT<L> operator +( const XformT<L>& a, const XformT<L>& b ) { return XformT<L>(a.l+b.l,a.p+b.p); }
  template<typename L> __forceinline const XformT<L> operator -( const XformT<L>& a, const XformT<L>& b ) { return XformT<L>(a.l-b.l,a.p-b.p); }

  template<typename L> __forceinline const XformT<L> operator *( const ScalarT  & s, const XformT<L>& x ) { return XformT<L>(s*x.l,s*x.p); }
  template<typename L> __forceinline const XformT<L> operator *( const XformT<L>& a, const XformT<L>& b ) { return XformT<L>(a.l*b.l,a.p*b.l+b.p); }
  template<typename L> __forceinline const XformT<L> operator /( const XformT<L>& a, const XformT<L>& b ) { return a * b.inverse(); }
  template<typename L> __forceinline const XformT<L> operator /( const XformT<L>& x, const ScalarT  & s ) { return s * (1.0f/x); }

  template<typename L> __forceinline XformT<L>& operator *=( XformT<L>& a, const XformT<L>& b ) { return a = a * b; }
  template<typename L> __forceinline XformT<L>& operator *=( XformT<L>& x, const ScalarT  & b ) { return x = x * b; }
  template<typename L> __forceinline XformT<L>& operator /=( XformT<L>& a, const XformT<L>& b ) { return a = a / b; }
  template<typename L> __forceinline XformT<L>& operator /=( XformT<L>& a, const ScalarT  & b ) { return a = a / b; }

// MoonRay: begin *****
  /// transform a point with translation
  template<typename L> __forceinline VectorT transformPoint (const XformT<L>& m, const VectorT& p) { return transformPoint(m.l,p) + m.p; }
  /// transform a vector without translation
  template<typename L> __forceinline VectorT transformVector(const XformT<L>& m, const VectorT& v) { return transformVector(m.l,v); }
  /// transform normal expects the Xform m to be the inverse of the actual transformation, translation part of the transform is ignored.
  template<typename L> __forceinline VectorT transformNormal(const XformT<L>& m, const VectorT& n) { return transformNormal(m.l,n); }
// MoonRay: end *****
// Intel: begin *****
/*
  template<typename L> __forceinline const VectorT xfmPoint (const AffineSpaceT<L>& m, const VectorT& p) { return xfmPoint(m.l,p) + m.p; }
  template<typename L> __forceinline const VectorT xfmVector(const AffineSpaceT<L>& m, const VectorT& v) { return xfmVector(m.l,v); }
  template<typename L> __forceinline const VectorT xfmNormal(const AffineSpaceT<L>& m, const VectorT& n) { return xfmNormal(m.l,n); }
*/
// Intel: end *****

// MoonRay: begin *****
  __forceinline BBox<Vec3fa> transformBounds(const XformT<Mat3<Vec3fa> >& m, const BBox<Vec3fa>& b)
  { 
    BBox<Vec3fa> dst = util::empty;
    const Vec3fa p0(b.lower.x,b.lower.y,b.lower.z,0.f); dst.extend(transformPoint(m,p0));
    const Vec3fa p1(b.lower.x,b.lower.y,b.upper.z,0.f); dst.extend(transformPoint(m,p1));
    const Vec3fa p2(b.lower.x,b.upper.y,b.lower.z,0.f); dst.extend(transformPoint(m,p2));
    const Vec3fa p3(b.lower.x,b.upper.y,b.upper.z,0.f); dst.extend(transformPoint(m,p3));
    const Vec3fa p4(b.upper.x,b.lower.y,b.lower.z,0.f); dst.extend(transformPoint(m,p4));
    const Vec3fa p5(b.upper.x,b.lower.y,b.upper.z,0.f); dst.extend(transformPoint(m,p5));
    const Vec3fa p6(b.upper.x,b.upper.y,b.lower.z,0.f); dst.extend(transformPoint(m,p6));
    const Vec3fa p7(b.upper.x,b.upper.y,b.upper.z,0.f); dst.extend(transformPoint(m,p7));
    return dst;
  }

  __forceinline BBox<Vec3f> transformBounds(const XformT<Mat3<Vec3f> >& m, const BBox<Vec3f>& b)
  { 
    BBox<Vec3f> dst = util::empty;
    const Vec3f p0(b.lower.x,b.lower.y,b.lower.z); dst.extend(transformPoint(m,p0));
    const Vec3f p1(b.lower.x,b.lower.y,b.upper.z); dst.extend(transformPoint(m,p1));
    const Vec3f p2(b.lower.x,b.upper.y,b.lower.z); dst.extend(transformPoint(m,p2));
    const Vec3f p3(b.lower.x,b.upper.y,b.upper.z); dst.extend(transformPoint(m,p3));
    const Vec3f p4(b.upper.x,b.lower.y,b.lower.z); dst.extend(transformPoint(m,p4));
    const Vec3f p5(b.upper.x,b.lower.y,b.upper.z); dst.extend(transformPoint(m,p5));
    const Vec3f p6(b.upper.x,b.upper.y,b.lower.z); dst.extend(transformPoint(m,p6));
    const Vec3f p7(b.upper.x,b.upper.y,b.upper.z); dst.extend(transformPoint(m,p7));
    return dst;
  }
  
  /// Transforms an AABB to an AABB. This is based on this:
  /// http://dev.theomader.com/transform-bounding-boxes/
  __forceinline BBox3f
  transformBBox(const XformT<Mat3<Vec3f> >& m, const BBox3f& bb)
  {
    const auto xa = m.row0() * bb.lower.x;
    const auto xb = m.row0() * bb.upper.x;

    const auto ya = m.row1() * bb.lower.y;
    const auto yb = m.row1() * bb.upper.y;

    const auto za = m.row2() * bb.lower.z;
    const auto zb = m.row2() * bb.upper.z;

    return BBox3f(
      min(xa, xb) + min(ya, yb) + min(za, zb) + m.row3(),
      max(xa, xb) + max(ya, yb) + max(za, zb) + m.row3()
    );
  }

  /// Decompose the transform into translate, rotate, and scale, where xfm = t*s*r
  template<typename L>
  __forceinline void decompose(const XformT<L>& xfm, VectorT& t, L& s, QuatT& r)
  {
    decompose(xfm.l, s, r);
    t = xfm.p;
  }

  template<typename L>
  __forceinline void decompose(const XformT<L>& xfm, XformComponent<L>& component)
  {
    decompose(xfm, component.t, component.s, component.r);
  }

  template<typename L> __forceinline XformT<L> lerp(const XformT<L>& a, const XformT<L>& b, const float t) { return XformT<L>(lerp(a.l,b.l,t), lerp(a.p,b.p,t)); }
// MoonRay: end *****
// Intel: begin *****
/*
  __forceinline const BBox<Vec3fa> xfmBounds(const AffineSpaceT<LinearSpace3<Vec3fa> >& m, const BBox<Vec3fa>& b) 
  { 
    BBox3fa dst = empty;
    const Vec3fa p0(b.lower.x,b.lower.y,b.lower.z); dst.extend(xfmPoint(m,p0));
    const Vec3fa p1(b.lower.x,b.lower.y,b.upper.z); dst.extend(xfmPoint(m,p1));
    const Vec3fa p2(b.lower.x,b.upper.y,b.lower.z); dst.extend(xfmPoint(m,p2));
    const Vec3fa p3(b.lower.x,b.upper.y,b.upper.z); dst.extend(xfmPoint(m,p3));
    const Vec3fa p4(b.upper.x,b.lower.y,b.lower.z); dst.extend(xfmPoint(m,p4));
    const Vec3fa p5(b.upper.x,b.lower.y,b.upper.z); dst.extend(xfmPoint(m,p5));
    const Vec3fa p6(b.upper.x,b.upper.y,b.lower.z); dst.extend(xfmPoint(m,p6));
    const Vec3fa p7(b.upper.x,b.upper.y,b.upper.z); dst.extend(xfmPoint(m,p7));
    return dst;
  }
*/
// Intel: end *****

  ////////////////////////////////////////////////////////////////////////////////
  /// Comparison Operators
  ////////////////////////////////////////////////////////////////////////////////

  template<typename L> __forceinline bool operator ==( const XformT<L>& a, const XformT<L>& b ) { return (a.l == b.l) && (a.p == b.p); }
  template<typename L> __forceinline bool operator !=( const XformT<L>& a, const XformT<L>& b ) { return (a.l != b.l) || (a.p != b.p); }

  ////////////////////////////////////////////////////////////////////////////////
  // Output Operators
  ////////////////////////////////////////////////////////////////////////////////

  template<typename L> static std::ostream& operator<<(std::ostream& cout, const XformT<L>& m) {
    return cout << "{ l = " << m.l << ", p = " << m.p << " }";
  }

  ////////////////////////////////////////////////////////////////////////////////
  // Template Instantiations
  ////////////////////////////////////////////////////////////////////////////////

// MoonRay: begin *****
  typedef XformT<Mat3f> Xform3f;
  typedef XformT<Mat3d> Xform3d;

  typedef XformComponent<Mat3f> XformComponent3f;
  typedef XformComponent<Mat3d> XformComponent3d;
// MoonRay: end *****
// Intel: begin *****
/*
  typedef AffineSpaceT<LinearSpace3f> AffineSpace3f;
  typedef AffineSpaceT<LinearSpace3fa> AffineSpace3fa;
  typedef AffineSpaceT<Quaternion3f > OrthonormalSpace3f;
*/
// Intel: end *****

  ////////////////////////////////////////////////////////////////////////////////
  // Data conversions for Xform3f
  ////////////////////////////////////////////////////////////////////////////////

  struct Array12f {
    float values[12];
    operator float*() { return(values); }
  };
  
  __forceinline Array12f copyToArray(const Xform3f& xfm)
  {
    Array12f values;
    values[ 0] = xfm.l.vx.x;  values[ 1] = xfm.l.vx.y;  values[ 2] = xfm.l.vx.z;       
    values[ 3] = xfm.l.vy.x;  values[ 4] = xfm.l.vy.y;  values[ 5] = xfm.l.vy.z;       
    values[ 6] = xfm.l.vz.x;  values[ 7] = xfm.l.vz.y;  values[ 8] = xfm.l.vz.z;       
    values[ 9] = xfm.p.x;     values[10] = xfm.p.y;     values[11] = xfm.p.z;       
    return values;
  }
  
  __forceinline Xform3f copyFromArray(const float* v)
  {
    return Xform3f(v[0],v[1],v[2],
                   v[3],v[4],v[5],
                   v[6],v[7],v[8],
                   v[9],v[10],v[11]);
  }
// Intel: begin *****
/*
  __forceinline AffineSpace3f copyFromArray(const float* v) 
  {
    return AffineSpace3f(LinearSpace3f(Vec3fa(v[0],v[1],v[2]),
                                       Vec3fa(v[3],v[4],v[5]),
                                       Vec3fa(v[6],v[7],v[8])),
                         Vec3fa(v[9],v[10],v[11]));
  }
*/
// Intel: end *****

// MoonRay: added begin *****
  ////////////////////////////////////////////////////////////////////////////////
  /// asIspc() and asCpp() C++ <--> ISPC Type-casting functions
  ////////////////////////////////////////////////////////////////////////////////
  HUD_AS_ISPC_FUNCTIONS(Xform3f);
  HUD_AS_CPP_FUNCTIONS(Xform3f);

  #undef QuatT
// MoonRay: added end *****

  #undef VectorT
  #undef ScalarT
}
}

