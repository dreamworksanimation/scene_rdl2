// Copyright 2023 DreamWorks Animation LLC and Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Vec2.h"
#include "Vec3.h"
// Intel: #include "vec2.h"
// Intel: #include "vec3.h"

namespace scene_rdl2 {
namespace math {
// Intel: namespace embree {

  template<typename T>
  struct BBox
  {
    T lower, upper;

    ////////////////////////////////////////////////////////////////////////////////
    /// Construction
    ////////////////////////////////////////////////////////////////////////////////

    __forceinline BBox           ( )                   { }
    __forceinline BBox           ( const BBox& other ) { lower = other.lower; upper = other.upper; }
    __forceinline BBox& operator=( const BBox& other ) { lower = other.lower; upper = other.upper; return *this; }

    __forceinline BBox ( const T& v                     ) : lower(v), upper(v) {}
    // MoonRay: added underscores to fix compile warning
    __forceinline BBox ( const T& _lower, const T& _upper ) : lower(_lower), upper(_upper) {}

    ////////////////////////////////////////////////////////////////////////////////
    /// Extending Bounds
    ////////////////////////////////////////////////////////////////////////////////

    __forceinline void extend(const BBox& other) { lower = min(lower,other.lower); upper = max(upper,other.upper); }
    __forceinline void extend(const T   & other) { lower = min(lower,other      ); upper = max(upper,other      ); }

    __forceinline void extend_atomic(const BBox& other) { 
      atomic_min_f32(&lower.x,other.lower.x);
      atomic_min_f32(&lower.y,other.lower.y);
      atomic_min_f32(&lower.z,other.lower.z);
      atomic_max_f32(&upper.x,other.upper.x);
      atomic_max_f32(&upper.y,other.upper.y);
      atomic_max_f32(&upper.z,other.upper.z);
    }

    /*! computes the size of the box */
    __forceinline bool empty() const { for (int i=0; i<T::N; i++) if (lower[i] > upper[i]) return true; return false; }

    /*! computes the size of the box */
    __forceinline T size() const { return upper - lower; }

    ////////////////////////////////////////////////////////////////////////////////
    /// Constants
    ////////////////////////////////////////////////////////////////////////////////

    // MoonRay: added util:: namespace
    __forceinline BBox( util::EmptyTy ) : lower(pos_inf), upper(neg_inf) {}
    __forceinline BBox( util::FullTy  ) : lower(neg_inf), upper(pos_inf) {}
    __forceinline BBox( util::FalseTy ) : lower(pos_inf), upper(neg_inf) {}
    __forceinline BBox( util::TrueTy  ) : lower(neg_inf), upper(pos_inf) {}
    __forceinline BBox( NegInfTy ): lower(pos_inf), upper(neg_inf) {}
    __forceinline BBox( PosInfTy ): lower(neg_inf), upper(pos_inf) {}
  };

  // MoonRay added
  /*! tests if box is empty */
  template<typename T> __forceinline bool isEmpty(const BBox<T>& box) { for (int i=0; i<T::N; i++) if (box.lower[i] > box.upper[i]) return true; return false; }

  /*! computes the center of the box */
  template<typename T> __forceinline const T center (const BBox<T>& box) { return T(.5f)*(box.lower + box.upper); }
  template<typename T> __forceinline const T center2(const BBox<T>& box) { return box.lower + box.upper; }

  /*! computes the volume of a bounding box */
  template<typename T> __forceinline float volume( const BBox<T>& b ) { return reduce_mul(b.size()); }
  // Intel: __forceinline float volume( const BBox<Vec3fa>& b ) { return reduce_mul(b.size()); }
  __forceinline float safeVolume( const BBox<Vec3fa>& b ) { if (b.empty()) return 0.0f; else return volume(b); }

  /*! computes the volume of a bounding box */
  __forceinline float volume( const BBox<Vec3f>& b )  { return reduce_mul(b.size()); }

  // MoonRay added
  /*! computes the size of the box */
  template<typename T> __forceinline const T size(const BBox<T>& box) { return box.upper - box.lower; }

  /*! computes the surface area of a bounding box */
  template<typename T> __forceinline const T area( const BBox<Vec2<T> >& b ) { const Vec2<T> d = size(b); return d.x*d.y; }
  __forceinline float     area( const BBox<Vec3fa>& b ) { const Vec3fa d = b.size(); return 2.0f*(d.x*(d.y+d.z)+d.y*d.z); }
  __forceinline float safeArea( const BBox<Vec3fa>& b ) { if (b.empty()) return 0.0f; else return area(b); }
  __forceinline float halfArea( const BBox<Vec3fa>& b ) { const Vec3fa d = b.size(); return d.x*(d.y+d.z)+d.y*d.z; }

  /*! merges bounding boxes and points */
  template<typename T> __forceinline const BBox<T> merge( const BBox<T>& a, const       T& b ) { return BBox<T>(min(a.lower, b    ), max(a.upper, b    )); }
  template<typename T> __forceinline const BBox<T> merge( const       T& a, const BBox<T>& b ) { return BBox<T>(min(a    , b.lower), max(a    , b.upper)); }
  template<typename T> __forceinline const BBox<T> merge( const BBox<T>& a, const BBox<T>& b ) { return BBox<T>(min(a.lower, b.lower), max(a.upper, b.upper)); }

  /*! Merges three boxes. */
  template<typename T> __forceinline const BBox<T> merge( const BBox<T>& a, const BBox<T>& b, const BBox<T>& c ) { return merge(a,merge(b,c)); }
  // MoonRay added
  template<typename T> __forceinline const BBox<T>& operator+=( BBox<T>& a, const BBox<T>& b ) { return a = merge(a,b); }
  // MoonRay added
  template<typename T> __forceinline const BBox<T>& operator+=( BBox<T>& a, const       T& b ) { return a = merge(a,b); }

  /*! Merges four boxes. */
  template<typename T> __forceinline BBox<T> merge(const BBox<T>& a, const BBox<T>& b, const BBox<T>& c, const BBox<T>& d) {
    return merge(merge(a,b),merge(c,d));
  }

  // MoonRay added
  /*! Merges eight boxes. */
  template<typename T> __forceinline BBox<T> merge(const BBox<T>& a, const BBox<T>& b, const BBox<T>& c, const BBox<T>& d,
                                                   const BBox<T>& e, const BBox<T>& f, const BBox<T>& g, const BBox<T>& h) {
    return merge(merge(a,b,c,d),merge(e,f,g,h));
  }

  /*! Comparison Operators */
  template<typename T> __forceinline bool operator==( const BBox<T>& a, const BBox<T>& b ) { return a.lower == b.lower && a.upper == b.upper; }
  template<typename T> __forceinline bool operator!=( const BBox<T>& a, const BBox<T>& b ) { return a.lower != b.lower || a.upper != b.upper; }

  /*! scaling */
  template<typename T> __forceinline BBox<T> operator *( const float& a, const BBox<T>& b ) { return BBox<T>(a*b.lower,a*b.upper); }

  /*! extension */
  template<typename T> __forceinline BBox<T> enlarge(const BBox<T>& a, const T& b) { return BBox<T>(a.lower - b, a.upper +b); }

  /*! intersect bounding boxes */
  template<typename T> __forceinline const BBox<T> intersect( const BBox<T>& a, const BBox<T>& b ) { return BBox<T>(max(a.lower, b.lower), min(a.upper, b.upper)); }
  template<typename T> __forceinline const BBox<T> intersect( const BBox<T>& a, const BBox<T>& b, const BBox<T>& c ) { return intersect(a,intersect(b,c)); }

  /*! tests if bounding boxes (and points) are disjoint (empty intersection) */
  template<typename T> __inline bool disjoint( const BBox<T>& a, const BBox<T>& b )
  { const T d = min(a.upper, b.upper) - max(a.lower, b.lower); for ( size_t i = 0 ; i < T::N ; i++ ) if ( d[i] < typename T::Scalar(zero) ) return true; return false; }
  template<typename T> __inline bool disjoint( const BBox<T>& a, const  T& b )
  { const T d = min(a.upper, b)       - max(a.lower, b);       for ( size_t i = 0 ; i < T::N ; i++ ) if ( d[i] < typename T::Scalar(zero) ) return true; return false; }
  template<typename T> __inline bool disjoint( const  T& a, const BBox<T>& b )
  { const T d = min(a, b.upper)       - max(a, b.lower);       for ( size_t i = 0 ; i < T::N ; i++ ) if ( d[i] < typename T::Scalar(zero) ) return true; return false; }

  /*! tests if bounding boxes (and points) are conjoint (non-empty intersection) */
  template<typename T> __inline bool conjoint( const BBox<T>& a, const BBox<T>& b )
  { const T d = min(a.upper, b.upper) - max(a.lower, b.lower); for ( size_t i = 0 ; i < T::N ; i++ ) if ( d[i] < typename T::Scalar(zero) ) return false; return true; }
  template<typename T> __inline bool conjoint( const BBox<T>& a, const  T& b )
  { const T d = min(a.upper, b)       - max(a.lower, b);       for ( size_t i = 0 ; i < T::N ; i++ ) if ( d[i] < typename T::Scalar(zero) ) return false; return true; }
  template<typename T> __inline bool conjoint( const  T& a, const BBox<T>& b )
  { const T d = min(a, b.upper)       - max(a, b.lower);       for ( size_t i = 0 ; i < T::N ; i++ ) if ( d[i] < typename T::Scalar(zero) ) return false; return true; }

  // MoonRay added
  template <typename T> __inline bool conjointExclusive(const BBox<T>& a, const T& b)
  {
    for (std::size_t i = 0; i < T::N; ++i) {
        if (b[i] < a.lower[i] || b[i] >= a.upper[i]) {
            return false;
        }
    }

    return true;
  }

  // MoonRay added
  template <typename T> __inline bool conjointExclusive(const T& a, const BBox<T>& b)
  {
    return conjointExclusive(b, a);
  }

  /*! subset relation */
  template<typename T> __inline bool subset( const BBox<T>& a, const BBox<T>& b )
  { 
    for ( size_t i = 0 ; i < T::N ; i++ ) if ( a.lower[i] < b.lower[i] ) return false;
    for ( size_t i = 0 ; i < T::N ; i++ ) if ( a.upper[i] > b.upper[i] ) return false;
    return true; 
  }

  // MoonRay added
  template <typename T>
  inline typename T::Scalar extents(const BBox<T>& box, int dimension)
  {
    return box.upper[dimension] - box.lower[dimension];
  }

  /*! output operator */
  template<typename T> __forceinline std::ostream& operator<<(std::ostream& cout, const BBox<T>& box) {
    return cout << "[" << box.lower << "; " << box.upper << "]";
  }

  /*! default template instantiations */
  typedef BBox<Vec2f> BBox2f;
  // MoonRay added: begin
  typedef BBox<Vec3f> BBox3f;
  typedef BBox<Vec2i> BBox2i;
  typedef BBox<Vec3i> BBox3i;
  typedef BBox<Vec3fa> BBox3fa;
  // MoonRay added: end
  // Intel: typedef BBox<Vec3fa> BBox3f;
}
}

