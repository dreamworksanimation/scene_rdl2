// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once
#include "Arena.h"
#if defined(DEBUG)
#include "BlockAllocatorCheck.h"
#endif
#include <iostream>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

// Not until 13 does icc support constexpr.
#if defined(__INTEL_COMPILER) && (__INTEL_COMPILER < 1300)
#define constexpr
#endif

namespace scene_rdl2 {
namespace alloc {


const size_t kMemoryAlignment = 16;
const size_t L1CacheLineSize = 64;
static_assert(util::StaticIsPowerOfTwo<kMemoryAlignment>::value, "Alignment must be a power of 2.");
static_assert(util::StaticIsPowerOfTwo<L1CacheLineSize>::value, "Cache line size must be a power of 2.");


//---------------------------------------------------------------------------

namespace detail {
__forceinline constexpr size_t align(size_t val, size_t alignment)
{
    return val + ((alignment - val) & (alignment - 1u));
}
} // namespace detail

#ifdef __INTEL_COMPILER
#pragma warning(push)
#pragma warning(disable:1684) // conversion from pointer to same-sized integral type
#endif // end __INTEL_COMPILER
__forceinline void* align(void* val, size_t alignment)
{
    static_assert(sizeof(size_t) >= sizeof(void*), "");
    return reinterpret_cast<void*>(detail::align(reinterpret_cast<uintptr_t>(val), alignment));
}

inline bool isAligned(void* p, size_t alignment)
{
    return (reinterpret_cast<uintptr_t>(p) % alignment == 0);
}
#ifdef __INTEL_COMPILER
#pragma warning(pop)
#endif // end __INTEL_COMPILER

//---------------------------------------------------------------------------

template <typename T>
using ArenaPointer = T*;

#define USE_ARENA_ALLOCATION
#if defined(USE_ARENA_ALLOCATION)
/// A C++-like interface into using the MemoryArena (or something compatible).
/// Supply the type, it will be allocated with the correct number of bytes, and
/// any parameters passed in will be used in its constructor.
template <typename T, typename Allocator, typename... Params>
__forceinline ArenaPointer<T> arenaAlloc(Allocator& allocator, Params&&... params)
{
    void* const p = allocator.alloc(sizeof(T), kMemoryAlignment);
    return ArenaPointer<T>(new(p) T(std::forward<Params>(params)...));
}
#else
template <typename T, typename Allocator, typename... Params>
__forceinline ArenaPointer<T> arenaAlloc(Allocator& allocator, Params&&... params)
{
    return ArenaPointer<T>(new T(std::forward<Params>(params)...));
}
#endif


//---------------------------------------------------------------------------

///
/// @class MemoryArenaAllocator Alloc.h <scene_rdl2/render/util/Alloc.h>
/// @brief The MemoryArenaAllocator meets the requiresments of a C++ std
/// allocator (it can be used in std containers, etc). It does its memory
/// allocation through the supplied Arena.
///
template <typename T, std::size_t alignment = kMemoryAlignment>
class ArenaAllocator
{
    Arena* mArena;

public:
    typedef T value_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
    typedef std::true_type propagate_on_container_move_assignment;

    template <typename U>
    struct rebind { typedef ArenaAllocator<U, alignment> other; };

    explicit ArenaAllocator(Arena& ma) : mArena(&ma) {}
    ArenaAllocator(const ArenaAllocator&) = default;
    ArenaAllocator& operator=(const ArenaAllocator&) = default;

    template <typename U>
    ArenaAllocator(const ArenaAllocator<U, alignment>& other) :
        mArena(std::addressof(other.getArena()))
    {
    }

    pointer address(reference x) const { return std::addressof(x); }
    const_pointer address(const_reference x) const { return std::addressof(x); }

    pointer allocate(size_type n, void* const /*hint*/ = nullptr)
    {
        MNRY_ASSERT(mArena);
        MNRY_ASSERT(n * sizeof(T) <= max_size());
        return reinterpret_cast<pointer>(mArena->alloc(n * sizeof(T), alignment));
    }

    void deallocate(pointer /*p*/, size_type /*n*/)
    {
        // Do nothing. Somebody has to clear out the memory arena.
    }

    size_type max_size() const noexcept
    {
        MNRY_ASSERT(mArena);
        return mArena->getBlockSize();
    }

    template <typename U, typename... Args>
    void construct(U* p, Args&&... args)
    {
        assert(p);
        ::new(reinterpret_cast<void*>(p)) U(std::forward<Args>(args)...);
    }

    template <typename U>
    void destroy(U* p)
    {
        if (p) {
            p->~U();
        }
    }

    Arena& getArena() const
    {
        MNRY_ASSERT(mArena);
        return *mArena;
    }
};

template <typename T1, typename T2, std::size_t alignment>
bool operator==(const ArenaAllocator<T1, alignment>& lhs, const ArenaAllocator<T2, alignment>& rhs)
{
    return std::addressof(lhs.getArena()) == std::addressof(rhs.getArena());
}

template <typename T1, typename T2, std::size_t alignment>
bool operator!=(const ArenaAllocator<T1, alignment>& lhs, const ArenaAllocator<T2, alignment>& rhs)
{
    return !(lhs == rhs);
}

} // namespace alloc
} // namespace scene_rdl2

/// Overloaded placement new. Call as:
/// Allocator a;
/// X* x = new(a) X;
inline void* operator new(std::size_t sz, scene_rdl2::alloc::Arena& alloc)
{
    return alloc.alloc(static_cast<unsigned int>(sz));
}

/// Overloaded placement new. Call as:
/// Allocator a;
/// X* x = new(a) X[someSize];
inline void* operator new[](std::size_t sz, scene_rdl2::alloc::Arena& alloc)
{
    return alloc.alloc(static_cast<unsigned int>(sz));
}

/// Included for completeness. If a class constructor throws during allocation
/// with the overloaded placement new, the overloaded placement delete is
/// automatically called.
inline void operator delete(void* /*ptr*/, scene_rdl2::alloc::Arena& /*alloc*/)
{
    // No-op.
}

/// Included for completeness. If a class constructor throws during allocation
/// with the overloaded placement new, the overloaded placement delete is
/// automatically called.
inline void operator delete[](void* /*ptr*/, scene_rdl2::alloc::Arena& /*alloc*/)
{
    // No-op.
}

#undef constexpr

