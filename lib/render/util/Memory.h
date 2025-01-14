// Copyright 2023-2024 DreamWorks Animation LLC
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
getVectorMemory(const std::vector<T, Alloc>& vec)
{
    return sizeof(std::vector<T>) + vec.capacity() * sizeof(T);
}

template<class T, class Alloc = std::allocator<T>>
size_t
getVectorElementsMemory(const std::vector<T, Alloc>& vec)
{
    // doesn't include the size of the vector container itself
    return vec.capacity() * sizeof(T);
}

//
// Ctor / dtor helpers for raw memory:
//

template <typename T, typename... Args>
inline T*
construct(T* elem, Args&&... args)
{
    new(elem) T(std::forward<Args>(args)...);
    return elem;
}

template <typename T, typename... Args>
inline T*
constructArray(T* elems, size_t numElems, Args&&... args)
{
    for (size_t i = 0; i < numElems; ++i) {
        new(elems + i) T(args...);
    }
    return elems;
}

template <typename T>
inline T*
destruct(T* elem)
{
    elem->~T();
    return elem;
}

template <typename T>
inline T*
destructArray(T* elems, size_t numElems)
{
    for (size_t i = 0; i < numElems; ++i) {
        elems[i].~T();
    }
    return elems;
}

//------------------------------------------------------------------------------------------
//
// Aligned memory low-level helpers
//

template <typename T, typename F>
inline T*
alignedMallocBasis(size_t alignment, const F& allocCallBack)
{
    return static_cast<T*>(allocCallBack(sizeof(T), alignment));
}

template <typename T, typename F>
inline T*
alignedMallocArrayBasis(size_t numElems, size_t alignment, const F& allocCallBack)
{
    return static_cast<T*>(allocCallBack(sizeof(T) * numElems, alignment));
}

template <typename T, typename F>
inline T*
alignedMallocCtorBasis(size_t alignment, const F& allocCallBack)
{
    return construct(alignedMallocBasis<T, F>(alignment, allocCallBack));
}

template <typename T, typename F>
inline T*
alignedMallocArrayCtorBasis(size_t numElems, size_t alignment, const F& allocCallBack)
{
    return constructArray(alignedMallocArrayBasis<T, F>(numElems, alignment, allocCallBack), numElems);    
}

template <typename T, typename F>
inline void
alignedFreeArrayBasis(T* ptr, const F& freeCallBack)
{
    if (ptr) {
        freeCallBack((void*)ptr);
    }
}

template <typename T, typename F>
inline void
alignedFreeDtorBasis(T* ptr, const F& freeCallBack)
{
    if (ptr) {
        freeCallBack(static_cast<void*>(destruct(ptr)));
    }
}

template <typename T, typename F> 
inline void
alignedFreeArrayDtorBasis(T* ptr, size_t numElems, const F& freeCallBack)
{
    if (ptr) {
        freeCallBack(static_cast<void*>(destructArray(ptr, numElems)));
    }
}

//------------------------------------------------------------------------------------------    
//
// Aligned memory helpers:
//

template <typename T>
inline T*
alignedMalloc(size_t alignment = DEFAULT_MEMORY_ALIGNMENT)
{
    return static_cast<T*>(alignedMallocBasis<T, void*(size_t, size_t)>
                           (alignment, alignedMalloc));
}

template <typename T>
inline T*
alignedMallocArray(size_t numElems, size_t alignment = DEFAULT_MEMORY_ALIGNMENT)
{
    return static_cast<T*>(alignedMallocArrayBasis<T, void*(size_t, size_t)>
                           (numElems, alignment, alignedMalloc));
}

template <typename T>
inline T*
alignedMallocCtor(size_t alignment = DEFAULT_MEMORY_ALIGNMENT)
{
    return alignedMallocCtorBasis<T, void*(size_t, size_t)>(alignment, alignedMalloc);
}

template <typename T>
inline T*
alignedMallocArrayCtor(size_t numElems, size_t alignment = DEFAULT_MEMORY_ALIGNMENT)
{
    return alignedMallocArrayCtorBasis<T, void*(size_t, size_t)>(numElems, alignment, alignedMalloc);
}

template <typename T, typename... Args>
inline T*
alignedMallocCtorArgs(size_t alignment, Args&&... args)
{
    return construct(alignedMalloc<T>(alignment), std::forward<Args>(args)...);
}

template <typename T, typename... Args>
inline T*
alignedMallocArrayCtorArgs(size_t numElems, size_t alignment, Args&&... args)
{
    return constructArray(alignedMallocArray<T>(numElems, alignment), numElems, std::forward<Args>(args)...);
}

template <typename T>
inline void
alignedFreeArray(T* ptr)
{
    alignedFreeArrayBasis<T, void(const void*)>(ptr, alignedFree);
}

template <typename T>
inline void
alignedFreeDtor(T* ptr)
{
    alignedFreeDtorBasis<T, void(const void*)>(ptr, alignedFree);
}

template <typename T>
inline void
alignedFreeArrayDtor(T* ptr, size_t numElems)
{
    alignedFreeArrayDtorBasis<T, void(const void*)>(ptr, numElems, alignedFree);
}

//
// Aligned deleter for ref-counted objects.
//
template <typename T>
struct AlignedDeleter
{
    void operator()(T* ptr)
    {
        alignedFreeDtor<T>(ptr);
    }
};

} // namespace util
} // namespace scene_rdl2
