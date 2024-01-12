// Copyright 2023-2024 DreamWorks Animation LLC and Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

// Include this before any other includes!
#include <scene_rdl2/common/platform/Platform.h>

#include <atomic>
#include <memory>

namespace scene_rdl2 {
namespace util {

  // This class is written to allow for private inheritance, the preferred way
  // to use this class.
  //
  // Use CRTP (curiously recurring template pattern) to so that we don't
  // have to use virtual functions.
  //
  // Many concepts for this class taken from Peter Weinert's Dr. Dobb's
  // article, "A Base Class for Intrusively Reference-Counted Objects in C++."
  template <typename Derived, typename Deleter = std::default_delete<Derived>>
  class RefCount
  {
  public:
    friend void refInc(const Derived* d)
    {
        // C-style cast to allow private inheritance.
        // We can't static_cast to an inaccessible base class.
        // reinterpret_cast will fail in the face of multiple inheritance.
        ++((const RefCount*) d)->refCounter;
    }
    friend void refDec(const Derived* d)
    {
        // C-style cast to allow private inheritance.
        // We can't static_cast to an inaccessible base class.
        // reinterpret_cast will fail in the face of multiple inheritance.
        if (--((const RefCount*) d)->refCounter == 0u) {
            Deleter()(const_cast<Derived *>(d));
        }
    }

  protected:
    explicit RefCount(unsigned val = 0u) : refCounter(val) {}
    RefCount(const RefCount&) : refCounter(0) {}
    RefCount& operator=(const RefCount&) { return *this; }
    ~RefCount() = default;
    void swap(RefCount&) noexcept {};
    unsigned refCount() const { return refCounter; }

  private:
    // Mutable so that we can accept const versions of the objects.
    mutable std::atomic_uint refCounter;
  };
  
  ////////////////////////////////////////////////////////////////////////////////
  /// Reference to single object
  ////////////////////////////////////////////////////////////////////////////////

  template<typename Type>
  class Ref {
  public:
    Type* const ptr;

    ////////////////////////////////////////////////////////////////////////////////
    /// Constructors, Assignment & Cast Operators
    ////////////////////////////////////////////////////////////////////////////////

    __forceinline Ref( void ) : ptr(nullptr) {}
    __forceinline Ref(std::nullptr_t) : ptr(nullptr) {}
    __forceinline Ref( const Ref& input ) : ptr(input.ptr) { if ( ptr ) refInc(ptr); }

    __forceinline Ref( Type* const input ) : ptr(input) {
      if ( ptr )
        refInc(ptr);
    }

    __forceinline ~Ref( void ) {
      if (ptr) refDec(ptr);
    }

    __forceinline Ref& operator =( const Ref& input )
    {
      if ( input.ptr ) refInc(input.ptr);
      if (ptr) refDec(ptr);
      const_cast<Type*&>(ptr) = input.ptr;
      return *this;
    }

    __forceinline Ref& operator =( std::nullptr_t ) {
      if (ptr) refDec(ptr);
      const_cast<Type*&>(ptr) = nullptr;
      return *this;
    }

    __forceinline explicit operator bool( void ) const { return ptr != nullptr; }

    __forceinline const Type& operator  *( void ) const { return *ptr; }
    __forceinline       Type& operator  *( void )       { return *ptr; }
    __forceinline const Type* operator ->( void ) const { return  ptr; }
    __forceinline       Type* operator ->( void )       { return  ptr; }

    __forceinline const Type* get( void ) const     { return ptr; }
    __forceinline       Type* get( void )           { return ptr; }

    template<typename TypeOut>
    __forceinline       Ref<TypeOut> cast()       { return Ref<TypeOut>(static_cast<TypeOut*>(ptr)); }
    template<typename TypeOut>
    __forceinline const Ref<TypeOut> cast() const { return Ref<TypeOut>(static_cast<TypeOut*>(ptr)); }

    template<typename TypeOut>
    __forceinline       Ref<TypeOut> dynamicCast()       { return Ref<TypeOut>(dynamic_cast<TypeOut*>(ptr)); }
    template<typename TypeOut>
    __forceinline const Ref<TypeOut> dynamicCast() const { return Ref<TypeOut>(dynamic_cast<TypeOut*>(ptr)); }
  };

  template<typename Type> __forceinline  bool operator < ( const Ref<Type>& a, const Ref<Type>& b ) { return a.ptr <  b.ptr ; }

  template<typename Type> __forceinline  bool operator ==( const Ref<Type>& a, std::nullptr_t     ) { return a.ptr == nullptr  ; }
  template<typename Type> __forceinline  bool operator ==( std::nullptr_t    , const Ref<Type>& b ) { return nullptr  == b.ptr ; }
  template<typename Type> __forceinline  bool operator ==( const Ref<Type>& a, const Ref<Type>& b ) { return a.ptr == b.ptr ; }

  template<typename Type> __forceinline  bool operator !=( const Ref<Type>& a, std::nullptr_t     ) { return a.ptr != nullptr  ; }
  template<typename Type> __forceinline  bool operator !=( std::nullptr_t    , const Ref<Type>& b ) { return nullptr  != b.ptr ; }
  template<typename Type> __forceinline  bool operator !=( const Ref<Type>& a, const Ref<Type>& b ) { return a.ptr != b.ptr ; }
} // namespace util
} // namespace scene_rdl2

