// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <scene_rdl2/render/util/Memory.h>
#include "ispc/PixelBuffer.hh"

#include <cstring>
#include <memory>
#include <type_traits>

namespace scene_rdl2 {
namespace fb_util {

// 
// A PixelBuffer represents a 2D buffer of elements of pixel type T. 
// It's a low level container which places no semantic constraints of any type
// on the layout of the data contained. The layout semantics are dictated by
// higher level code.
// 
template<typename T>
class PixelBuffer
{
    static_assert(std::is_trivially_destructible<T>::value, "We don't call destructors");
    struct AlignedDeleter
    {
        void operator()(T* t) const { util::alignedFreeArray(t); }
    };

public:
    using PixelType      = T;
    using value_type     = T;
    using pointer        = T*;
    using const_pointer  = const T*;
    using iterator       = pointer;
    using const_iterator = const_pointer;

    PixelBuffer() noexcept : mData(), mRawData(nullptr), mWidth(0), mHeight(0), mBytesAllocated(0) {}

    static uint32_t hudValidation(bool verbose)
    {
        PIXELBUFFER_VALIDATION;
    }

    bool init(unsigned width, unsigned height)
    {
        unsigned area = width * height;
        unsigned bytesToAllocate = area * static_cast<unsigned>(sizeof(T));
        MNRY_ASSERT(bytesToAllocate);

        if (mBytesAllocated < bytesToAllocate) {
            mBytesAllocated = bytesToAllocate;
            // mRawData is a member of the Hybrid Uniform Data struct so that ISPC can access it.
            // This (deliberately) doesn't call the constructor on objects!
            mRawData = (uint8_t *)util::alignedMallocArray<T>(area, CACHE_LINE_SIZE), AlignedDeleter();
            mData.reset((T*)mRawData);
        }

        MNRY_ASSERT(mBytesAllocated >= bytesToAllocate);

        mWidth = width;
        mHeight = height;

        return mData.get() != nullptr;
    }

    explicit operator bool() const noexcept { return static_cast<bool>(mData); }

    // Explicitly frees up any allocated memory.
    void cleanUp() noexcept
    {
        mWidth = 0;
        mHeight = 0;
        mBytesAllocated = 0;
        mData.reset();
        mRawData = nullptr;
    }

    // Clear the buffer to zeros.
    void clear()
    {
        static_assert(std::is_trivially_copyable<T>::value, "Calling memset");
        if (mData) {
            std::memset(mData.get(), 0, mBytesAllocated);
        }
    }

    // Clear the buffer with a specific value
    void clear(const T &val)
    {
        if (mData) {
            unsigned area = mWidth * mHeight;
            T *data = mData.get();

            for (unsigned i = 0; i < area; ++i) {
                data[i] = val;
            }
        }
    }

    unsigned getWidth() const noexcept   { return mWidth; }
    unsigned getHeight() const noexcept  { return mHeight; }
    unsigned getArea() const noexcept    { return mWidth * mHeight; }

    // x and y are straight indices here, the valid range is 0 to width - 1, 0 to height - 1 respectively
    T &getPixel(unsigned x, unsigned y)
    {
        MNRY_ASSERT(mData && x < mWidth && y < mHeight);
        return mData.get()[y * mWidth + x];
    }

    const T &getPixel(unsigned x, unsigned y) const
    {
        return const_cast<PixelBuffer<T> *>(this)->getPixel(x, y);
    }

    /**
     * Note: This returns the raw pointer.  Use with caution
     */
    T *getRow(unsigned row)
    {
        MNRY_ASSERT(mData && row < mHeight);
        return &mData.get()[row * mWidth];
    }

    /**
     * Note: This returns the raw pointer.  Use with caution
     */
    const T *getRow(unsigned row) const
    {
        return const_cast<PixelBuffer<T> *>(this)->getRow(row);
    }

    template <typename U>
    std::shared_ptr<U> getDataSharedAs()
    {
        // C++17 adds std::reinterpret_pointer_cast. So, have to provide it manually for now
        // http://en.cppreference.com/w/cpp/memory/shared_ptr/pointer_cast
        return std::shared_ptr<U>(mData, reinterpret_cast<typename std::shared_ptr<U>::element_type *>(mData.get()));
    }

    std::shared_ptr<T> getDataShared()
    {
        return mData;
    }

    /**
     * Note: This returns the raw pointer.  Use with caution
     */
    T *getData() noexcept
    {
        return mData.get();
    }

    /**
     * Note: This returns the raw pointer.  Use with caution
     */
    const T *getData() const noexcept
    {
        return mData.get();
    }

    void setPixel(unsigned x, unsigned y, const T &val)
    {
        MNRY_ASSERT(mData && x < mWidth && y < mHeight);
        mData.get()[y * mWidth + x] = val;
    }

    void addPixel(unsigned x, unsigned y, const T &val)
    {
        MNRY_ASSERT(mData && x < mWidth && y < mHeight);
        mData.get()[y * mWidth + x] += val;
    }

    void clone(const PixelBuffer<T> &src)
    {
        static_assert(std::is_trivially_copyable<T>::value, "Calling memcpy");
        init(src.getWidth(), src.getHeight());
        memcpy(mData.get(), src.mData.get(), getArea() * sizeof(T));
    }

private:
    // C++ uses a shared_ptr mData, so that is broken out of the Hybrid Uniform Data struct.
    std::shared_ptr<T>  mData; // Pixel[0] is the bottom left of the image.
    PIXELBUFFER_MEMBERS;
};

MNRY_STATIC_ASSERT(sizeof(PixelBuffer<uint8_t>) % CACHE_LINE_SIZE == 0);

} // namespace fb_util
} // namespace scene_rdl2

