// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <cstddef>
#include <memory>
#include <type_traits>

#define DEBUG_MEMORY_POOL_LIMITS 0

namespace scene_rdl2 {
namespace alloc {
// A super-simple memory pool that only cares about one type for a fixed upper-bound.
// This memory pool neither constructs nor destroys the type: it is assumed that whatever mechanism is using the
// pool will construct/destroy as needed. The reason this is typed is so that we don't have to do alignment and
// size math nor worry about heterogeneous allocation sizes at runtime.
template <typename T>
class TypedStaticallySizedMemoryPool
{
public:
    explicit TypedStaticallySizedMemoryPool(std::size_t n)
    : mNumAllocated(0)
#if DEBUG_MEMORY_POOL_LIMITS
    , mCountDebug(n)
#endif
    , mStorage(new AlignedStorage[n])
    {
    }

    TypedStaticallySizedMemoryPool(const TypedStaticallySizedMemoryPool&) = delete;
    TypedStaticallySizedMemoryPool(TypedStaticallySizedMemoryPool&&) noexcept = default;
    TypedStaticallySizedMemoryPool& operator=(const TypedStaticallySizedMemoryPool&) = delete;
    TypedStaticallySizedMemoryPool& operator=(TypedStaticallySizedMemoryPool&&) noexcept = default;

    T* allocate(std::size_t n) noexcept
    {
#if DEBUG_MEMORY_POOL_LIMITS
        MNRY_ASSERT_REQUIRE(mNumAllocated + n <= mCountDebug);
#endif
        // Deliberately do not call constructor.
        T* const p = getPointer(mNumAllocated);
        mNumAllocated += n;
        return p;
    }

    void clear() noexcept
    {
        // Deliberately do not call destructors.
        mNumAllocated = 0;
    }

private:
    T* getPointer(std::size_t n) noexcept
    {
        return reinterpret_cast<T*>(std::addressof(mStorage[n]));
    }

    using AlignedStorage = typename std::aligned_storage<sizeof(T), std::alignment_of<T>::value>::type;
    std::size_t mNumAllocated;
#if DEBUG_MEMORY_POOL_LIMITS
    std::size_t mCountDebug;
#endif
    std::unique_ptr<AlignedStorage[]> mStorage;
};

template <typename T>
class TypedStaticallySizedPoolAllocator
{
    TypedStaticallySizedMemoryPool<T>* mPool;

public:
    typedef T value_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
    typedef std::true_type propagate_on_container_move_assignment;
    typedef std::false_type is_always_equal;

    template <typename U>
    struct rebind { typedef TypedStaticallySizedPoolAllocator<U> other; };

    explicit TypedStaticallySizedPoolAllocator(TypedStaticallySizedMemoryPool<T>& ma) : mPool(std::addressof(ma)) {}
    TypedStaticallySizedPoolAllocator(const TypedStaticallySizedPoolAllocator&) noexcept = default;
    TypedStaticallySizedPoolAllocator& operator=(const TypedStaticallySizedPoolAllocator&) noexcept = default;
    TypedStaticallySizedPoolAllocator(TypedStaticallySizedPoolAllocator&&) noexcept = default;
    TypedStaticallySizedPoolAllocator& operator=(TypedStaticallySizedPoolAllocator&&) noexcept = default;

    template <typename U>
    TypedStaticallySizedPoolAllocator(const TypedStaticallySizedPoolAllocator<U>& other)
    : mPool(std::addressof(other.getPool()))
    {
    }

    pointer address(reference x) const noexcept { return std::addressof(x); }
    const_pointer address(const_reference x) const noexcept { return std::addressof(x); }

    pointer allocate(size_type n, void* const /*hint*/ = nullptr) noexcept
    {
        MNRY_ASSERT(mPool);
        return mPool->allocate(n);
    }

    void deallocate(pointer /*p*/, size_type /*n*/) noexcept
    {
        // Do nothing. Somebody has to clear out the memory arena.
    }

    TypedStaticallySizedMemoryPool<T>& getPool() const noexcept
    {
        MNRY_ASSERT(mPool);
        return *mPool;
    }
};

template <typename T1, typename T2, std::size_t alignment>
bool operator==(const TypedStaticallySizedPoolAllocator<T1>& lhs,
                const TypedStaticallySizedPoolAllocator<T2>& rhs) noexcept
{
    // Pool allocators are not equal unless the underlying pool is the same. We don't want to allocate from one and
    // deallocate from another.
    return std::addressof(lhs.getPool()) == std::addressof(rhs.getPool());
}

template <typename T1, typename T2, std::size_t alignment>
bool operator!=(const TypedStaticallySizedPoolAllocator<T1>& lhs,
                const TypedStaticallySizedPoolAllocator<T2>& rhs) noexcept
{
    return !(lhs == rhs);
}

template <typename T>
class TypedStaticalySizedMemoryPoolRAII
{
    TypedStaticallySizedMemoryPool<T>& mPool;
public:
    explicit TypedStaticalySizedMemoryPoolRAII(TypedStaticallySizedMemoryPool<T>& pool) : mPool(pool) {}
    ~TypedStaticalySizedMemoryPoolRAII() { mPool.clear(); }
};

} // namespace alloc
} // namespace scene_rdl2

template <typename T>
void* operator new(std::size_t bytes, scene_rdl2::alloc::TypedStaticallySizedMemoryPool<T>& pool) noexcept
{
    // operator new passes in the number of bytes to allocate, but our pool works in relation to T.
    MNRY_ASSERT(sizeof(T) == bytes);
    return pool.allocate(1);
}

template <typename T>
void* operator new[](std::size_t bytes, scene_rdl2::alloc::TypedStaticallySizedMemoryPool<T>& pool) noexcept
{
    // operator new passes in the number of bytes to allocate, but our pool works in relation to T.
    MNRY_ASSERT(bytes % sizeof(T) == 0);
    return pool.allocate(bytes / sizeof(T));
}

template <typename T>
void operator delete(void*, scene_rdl2::alloc::TypedStaticallySizedMemoryPool<T>&) noexcept
{
    // No-op. We don't (can't) delete individual elements out of the memory pool. Instead we clear it all at once.
}

template <typename T>
void operator delete[](void*, scene_rdl2::alloc::TypedStaticallySizedMemoryPool<T>&) noexcept
{
    // No-op. We don't (can't) delete individual elements out of the memory pool. Instead we clear it all at once.
}

