// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

// Include this before any other includes!
#include <scene_rdl2/common/platform/Platform.h>

#include "Alloc.h"
#include "BitUtils.h"

#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace scene_rdl2 {
namespace alloc {

///
/// @class AlignedAllocator Alloc.h <scene_rdl2/render/util/AlignedAllocator.h>
/// @brief The AlignedAllocator meets the requirements of a C++ std
/// allocator (it can be used in std containers, etc). It guarantees
/// memory allocation aligned on the supplied template argument boundary.
///
template <typename T, std::size_t alignment = kMemoryAlignment>
class AlignedAllocator;

template <typename T, std::size_t alignment>
class AlignedAllocator
{
public:
    static_assert(util::StaticIsPowerOfTwo<alignment>::value,
            "Alignment must be a power of 2.");

    typedef T                   value_type;
    typedef T*                  pointer;
    typedef const T*            const_pointer;
    typedef T&                  reference;
    typedef const T&            const_reference;
    typedef std::size_t         size_type;
    typedef std::ptrdiff_t      difference_type;
    typedef std::true_type      propagate_on_container_copy_assignment;
    typedef std::true_type      propagate_on_container_move_assignment;
    typedef std::true_type      propagate_on_container_swap;
    typedef std::true_type      is_always_equal;

    template <typename U>
    struct rebind { typedef AlignedAllocator<U, alignment> other; };

    AlignedAllocator() noexcept = default;
    AlignedAllocator(const AlignedAllocator&) noexcept = default;
    AlignedAllocator& operator=(const AlignedAllocator&) noexcept = default;

    template <typename U>
    AlignedAllocator(const AlignedAllocator<U, alignment>&)
    {
    }

    pointer address(reference x) const noexcept { return std::addressof(x); }
    const_pointer address(const_reference x) const noexcept { return std::addressof(x); }

    pointer allocate(size_type n, void* const /*hint*/ = nullptr)
    {
        void* const mem = util::alignedMalloc(n * sizeof(T), alignment);
        if (unlikely(!mem)) {
            throw std::bad_alloc();
        } else {
            return static_cast<pointer>(mem);
        }
    }

    void deallocate(pointer p, size_type /*n*/)
    {
        util::alignedFree(p);
    }

    size_type max_size() const noexcept
    {
        return std::numeric_limits<size_type>::max();
    }

    template <typename U, typename... Args>
    void construct(U* p, Args&&... args)
    {
        MNRY_ASSERT(p);
        ::new(reinterpret_cast<void*>(p)) U(std::forward<Args>(args)...);
    }

    template <typename U>
    void destroy(U* p)
    {
        if (p) {
            p->~U();
        }
    }
};

template <std::size_t alignment>
class AlignedAllocator<void, alignment>
{
public:
    static_assert(util::StaticIsPowerOfTwo<alignment>::value,
            "Alignment must be a power of 2.");

    typedef void                value_type;
    typedef void*               pointer;
    typedef const void*         const_pointer;
    typedef std::true_type  propagate_on_container_move_assignment;
    typedef std::true_type  is_always_equal;

    template <typename U>
    struct rebind { typedef AlignedAllocator<U, alignment> other; };
};

template <typename T1, typename T2, std::size_t alignment>
bool operator==(const AlignedAllocator<T1, alignment>& lhs, const AlignedAllocator<T2, alignment>& rhs)
{
    return true;
}

template <typename T1, typename T2, std::size_t alignment>
bool operator!=(const AlignedAllocator<T1, alignment>& lhs, const AlignedAllocator<T2, alignment>& rhs)
{
    return !(lhs == rhs);
}

} // namespace alloc
} // namespace scene_rdl2

