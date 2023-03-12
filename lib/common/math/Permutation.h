// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

// Include this before any other includes!
#include <scene_rdl2/common/platform/Platform.h>

#include <algorithm>
#include <numeric>
#include <utility>

namespace scene_rdl2 {
namespace math {
    class Permutation
    {
    public:
        template <typename RNG>
        Permutation(int size, RNG&& rng)
        : mCount(size)
        , mData(new int[size])
        {
            std::iota(mData, mData + mCount, 0);
            std::shuffle(mData, mData + mCount, std::forward<RNG>(rng));
        }

        ~Permutation()
        {
            delete[] mData;
            mData = nullptr;
        }

        int size() const noexcept
        {
            return mCount;
        }

        int operator[](int i) const noexcept
        {
            MNRY_ASSERT(i >= 0 && i < mCount);
            return mData[i];
        }

    private:
        int mCount;    //!< Size of the permutation.
        int* mData;   //!< Array storing the permutation.
    };
} // namespace math
} // namespace scene_rdl2


