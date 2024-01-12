// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "BBox.h"

#include <iterator>
#include <type_traits>


namespace scene_rdl2 {
namespace math {

// We only do const iterators because there are no values to manipulate.
class BBox2iIterator
{
public:
    using iterator_category = std::random_access_iterator_tag;
    using size_type         = int;
    using difference_type   = int;
    using value_type        = Vec2i;
    using pointer           = value_type*;
    using reference         = value_type&;
    static_assert(std::is_signed<difference_type>::value, "difference_type needs to be signed");

    BBox2iIterator() noexcept : mIdx(0), mBox(nullptr) {}
    BBox2iIterator(const BBox2i* box, size_type i) noexcept : mIdx(i), mBox(box) {}

    friend bool operator==(const BBox2iIterator& a, const BBox2iIterator& b) noexcept
    {
        return a.mIdx == b.mIdx;
    }

    friend bool operator<(const BBox2iIterator& a, const BBox2iIterator& b) noexcept
    {
        return a.mIdx < b.mIdx;
    }

    value_type operator*() const noexcept
    {
        const auto& mn = mBox->lower;
        const auto& mx = mBox->upper;
        const auto w = mx[0] - mn[0];
        const auto x = mIdx % w + mn[0];
        const auto y = mIdx / w + mn[1];
        return value_type(x, y);
    }

    value_type operator[](size_type n)
    {
        BBox2iIterator bi(mBox, mIdx + n);
        return *bi;
    }

    BBox2iIterator& operator++() noexcept
    {
        ++mIdx;
        return *this;
    }

    const BBox2iIterator operator++(int) noexcept
    {
        BBox2iIterator cp(*this);
        ++mIdx;
        return cp;
    }

    BBox2iIterator& operator--() noexcept
    {
        --mIdx;
        return *this;
    }

    const BBox2iIterator operator--(int) noexcept
    {
        BBox2iIterator cp(*this);
        --mIdx;
        return cp;
    }

    BBox2iIterator& operator+=(difference_type n) noexcept
    {
        mIdx += n;
        return *this;
    }

    BBox2iIterator& operator-=(difference_type n) noexcept
    {
        mIdx -= n;
        return *this;
    }

    friend BBox2iIterator::difference_type operator-(BBox2iIterator a, BBox2iIterator b)
    {
        return a.mIdx - b.mIdx;
    }

private:
    size_type mIdx;
    const BBox2i* mBox;
};

inline BBox2iIterator operator+(BBox2iIterator a, BBox2iIterator::difference_type n)
{
    a += n;
    return a;
}

inline BBox2iIterator operator+(BBox2iIterator::difference_type n, BBox2iIterator a)
{
    a += n;
    return a;
}

inline BBox2iIterator operator-(BBox2iIterator a, BBox2iIterator::difference_type n)
{
    a -= n;
    return a;
}

inline bool operator!=(const BBox2iIterator& a, const BBox2iIterator& b)
{
    return !(b == a);
}

inline bool operator>(const BBox2iIterator& a, const BBox2iIterator& b)
{
    return b < a;
}

inline bool operator<=(const BBox2iIterator& a, const BBox2iIterator& b)
{
    return !(b < a);
}

inline bool operator>=(const BBox2iIterator& a, const BBox2iIterator& b)
{
    return !(a < b);
}

inline BBox2iIterator begin(const BBox2i& bbox)
{
    static_assert(std::is_same<typename std::iterator_traits<BBox2iIterator>::iterator_category,
                  std::random_access_iterator_tag>::value, "");
    return { std::addressof(bbox), 0 };
}

inline BBox2iIterator end(const BBox2i& bbox)
{
    static_assert(std::is_same<typename std::iterator_traits<BBox2iIterator>::iterator_category,
                  std::random_access_iterator_tag>::value, "");
    return { std::addressof(bbox), extents(bbox, 0) * extents(bbox, 1) };
}

} // namespace math
} // namespace scene_rdl2


