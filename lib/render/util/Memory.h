// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

// Include this before any other includes!
#include <scene_rdl2/common/platform/Platform.h>

#include <vector>

#define DEFAULT_MEMORY_ALIGNMENT    SIMD_MEMORY_ALIGNMENT

namespace scene_rdl2 {
namespace util {

template<class T, class Alloc = std::allocator<T>>
size_t
getVectorMemory(const std::vector<T, Alloc> &vec)
{
    return sizeof(std::vector<T>) + vec.capacity() * sizeof(T);
}

template<class T, class Alloc = std::allocator<T>>
size_t
getVectorElementsMemory(const std::vector<T, Alloc> &vec)
{
    // doesn't include the size of the vector container itself
    return vec.capacity() * sizeof(T);
}

//
// Ctor / dtor helpers for raw memory:
//

template <typename T, typename... Args>
inline T *
construct(T *elem, Args&&... args)
{
    new(elem) T(std::forward<Args>(args)...);
    return elem;
}

template <typename T, typename... Args>
inline T *
constructArray(T *elems, size_t numElems, Args&&... args)
{
    for (size_t i = 0; i < numElems; ++i) {
        new(elems + i) T(args...);
    }
    return elems;
}

template <typename T>
inline T *
destruct(T *elem)
{
    elem->~T();
    return elem;
}

template <typename T>
inline T *
destructArray(T *elems, size_t numElems)
{
    for (size_t i = 0; i < numElems; ++i) {
        elems[i].~T();
    }
    return elems;
}

//
// Aligned memory helpers:
//

template <typename T>
inline T *
alignedMalloc(size_t alignment = DEFAULT_MEMORY_ALIGNMENT)
{
    return (T *)alignedMalloc(sizeof(T), alignment);
}

template <typename T>
inline T *
alignedMallocArray(size_t numElems, size_t alignment = DEFAULT_MEMORY_ALIGNMENT)
{
    return (T *)alignedMalloc(sizeof(T) * numElems, alignment);
}

template <typename T>
inline T *
alignedMallocCtor(size_t alignment = DEFAULT_MEMORY_ALIGNMENT)
{
    return construct(alignedMalloc<T>(alignment));
}

template <typename T>
inline T *
alignedMallocArrayCtor(size_t numElems, size_t alignment = DEFAULT_MEMORY_ALIGNMENT)
{
    return constructArray(alignedMallocArray<T>(numElems, alignment), numElems);
}

template <typename T, typename... Args>
inline T *
alignedMallocCtorArgs(size_t alignment, Args&&... args)
{
    return construct(alignedMalloc<T>(alignment), std::forward<Args>(args)...);
}

template <typename T, typename... Args>
inline T *
alignedMallocArrayCtorArgs(size_t numElems, size_t alignment, Args&&... args)
{
    return constructArray(alignedMallocArray<T>(numElems, alignment), numElems, std::forward<Args>(args)...);
}

template <typename T>
inline void
alignedFreeArray(T *ptr)
{
    if (ptr) {
        alignedFree((void *)ptr);
    }
}

template <typename T>
inline void
alignedFreeDtor(T *ptr)
{
    if (ptr) {
        alignedFree((void *)destruct(ptr));
    }
}

template <typename T>
inline void
alignedFreeArrayDtor(T *ptr, size_t numElems)
{
    if (ptr) {
        alignedFree((void *)destructArray(ptr, numElems));
    }
}

//
// Aligned deleter for ref-counted objects.
//
template <typename T>
struct AlignedDeleter
{
    void operator()(T *ptr)
    {
        alignedFreeDtor<T>(ptr);
    }
};

} // namespace util
} // namespace scene_rdl2

