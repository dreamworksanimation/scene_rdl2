// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstddef>
#include <iterator>
#include <type_traits>

namespace scene_rdl2 {
namespace rdl2 {

/// @class IndexIterator
/// @brief A simple iterator that simply counts, used to iterate through
/// container indices.

// We only provide const versions of this iterator, since it just accesses
// underlying array indices. We don't want the user to be able to change those.
class IndexIterator
{
public:
    using value_type        = std::int32_t;
    using difference_type   = std::ptrdiff_t;
    using pointer           = value_type*;
    using reference         = value_type&;
    using iterator_category = std::random_access_iterator_tag;

    explicit IndexIterator(value_type current) :
        mIdx(current)
    {
    }

    value_type operator*() const
    {
        return mIdx;
    }

    // Prefix decrement operator
    IndexIterator& operator--()
    {
        --mIdx;
        return *this;
    }

    // Postfix decrement operator
    IndexIterator operator--(int)
    {
        const IndexIterator old(*this);
        --(*this);
        return old;
    }

    // Prefix increment operator
    IndexIterator& operator++()
    {
        ++mIdx;
        return *this;
    }

    // Postfix increment operator
    IndexIterator operator++(int)
    {
        const IndexIterator old(*this);
        ++(*this);
        return old;
    }

    IndexIterator& operator+=(difference_type n)
    {
        mIdx += static_cast<value_type>(n);
        return *this;
    }

    IndexIterator& operator-=(difference_type n)
    {
        mIdx -= static_cast<value_type>(n);
        return *this;
    }

    value_type operator[](difference_type n)
    {
        return mIdx + static_cast<value_type>(n);
    }

private:
    value_type mIdx;
};

namespace detail {

// PointerToConst will take a pointer and make it point to a const value.
// E.g. "const T*" will become "const T*" (no-op)
//      "T*"       will become "const T*"
// Any type that's not a pointer will be a no-op.

template <typename T>
struct PointerToConst
{
    typedef T type;
};

template <typename T>
struct PointerToConst<T*>
{
    // It's not trivial to add const to the value of pointer. "add_const" will
    // only add a top-level const. E.g. "T*" will become "T* const", not
    // "const T*". We have to deconstruct the type, and built it back to a
    // pointer.
    typedef typename std::add_pointer<
        typename std::add_const<
            typename std::remove_pointer<T*>::type
        >::type>::type type;
};
} // namespace detail

/// @class FilterIndexIterator
/// @brief Like IndexIterator, it is used to iterate through indices for a
/// container. However, it will only return indices where the passed in match
/// equals (operator==) the value in the container. Unlike IndexIterator, it
/// uses a set of index iterators (e.g. IndexIterator) for its range
/// deliminators. This is to easily iterate over a sub-range (e.g. that given
/// by IndexableArray::equal_range).

// We only provide const versions of this iterator, since it just accesses
// underlying array indices. We don't want the user to be able to change those.
template <typename ListType, typename EnumerableIterator>
class FilterIndexIterator
{
public:
    // The PointerToConst allows us to have lists of non-const pointers, but to
    // use const pointers are our match type.
    using filter_type         = typename detail::PointerToConst<typename ListType::value_type>::type;
    using enumerable_iterator = EnumerableIterator;
    using value_type          = typename std::iterator_traits<enumerable_iterator>::value_type;
    using difference_type     = typename std::iterator_traits<enumerable_iterator>::difference_type;
    using pointer             = typename std::iterator_traits<enumerable_iterator>::pointer;
    using reference           = typename std::iterator_traits<enumerable_iterator>::reference;
    using iterator_category   = std::bidirectional_iterator_tag;

    // List is copied by value. If you don't want to pay for the copy, pass in
    // a std::cref.
    FilterIndexIterator(enumerable_iterator current,
                        enumerable_iterator first,
                        enumerable_iterator last,
                        ListType list,
                        filter_type match) :
        mIter(current),
        mFirst(first),
        mLast(last),
        mList(list),
        mMatch(match)
    {
        // Find our first matching value. operator++ will do this for us.
        if (mIter != mLast && mList[*mIter] != mMatch) {
            ++(*this);
        }
    }

    value_type operator*() const
    {
        return *mIter;
    }

    // Prefix decrement operator
    FilterIndexIterator& operator--()
    {
        MNRY_ASSERT(mIter != mFirst);
        do {
            --mIter;
        } while (mIter != mFirst && mList[*mIter] != mMatch);
        return *this;
    }

    // Postfix decrement operator
    FilterIndexIterator operator--(int)
    {
        const FilterIndexIterator old(*this);
        --(*this);
        return old;
    }

    // Prefix increment operator
    FilterIndexIterator& operator++()
    {
        MNRY_ASSERT(mIter != mLast);
        do {
            ++mIter;
        } while (mIter != mLast && mList[*mIter] != mMatch);
        return *this;
    }

    // Postfix increment operator
    FilterIndexIterator operator++(int)
    {
        const FilterIndexIterator old(*this);
        ++(*this);
        return old;
    }

    friend bool operator==(const FilterIndexIterator& a, const FilterIndexIterator& b)
    {
        MNRY_ASSERT(a.mMatch == b.mMatch, "This is probably due to the object"
            "being passed into to 'begin' and 'end' being different.");
        return a.mIter == b.mIter;
    }

    friend bool operator!=(const FilterIndexIterator& a, const FilterIndexIterator& b)
    {
        return !(a == b);
    }

private:
    enumerable_iterator mIter;
    enumerable_iterator mFirst;
    enumerable_iterator mLast;
    ListType mList;
    filter_type mMatch;
};

inline bool operator==(const IndexIterator& a, const IndexIterator& b)
{
    return *a == *b;
}

inline bool operator!=(const IndexIterator& a, const IndexIterator& b)
{
    return !(a == b);
}

inline bool operator<(const IndexIterator& a, const IndexIterator& b)
{
    return *a < *b;
}

inline bool operator>(const IndexIterator& a, const IndexIterator& b)
{
    return *b < *a;
}

inline bool operator<=(const IndexIterator& a, const IndexIterator& b)
{
    return !(b < a);
}

inline bool operator>=(const IndexIterator& a, const IndexIterator& b)
{
    return !(a < b);
}

// Pass by value on purpose. We're going to be making a copy anyway; allow for
// elision.
inline IndexIterator operator+(IndexIterator i, IndexIterator::difference_type n)
{
    i += n;
    return i;
}

// Pass by value on purpose. We're going to be making a copy anyway; allow for
// elision.
inline IndexIterator operator+(IndexIterator::difference_type n, IndexIterator i)
{
    i += n;
    return i;
}

// Pass by value on purpose. We're going to be making a copy anyway; allow for
// elision.
inline IndexIterator operator-(IndexIterator i, IndexIterator::difference_type n)
{
    i -= n;
    return i;
}

inline IndexIterator::difference_type operator-(const IndexIterator& a, const IndexIterator& b)
{
    const IndexIterator::difference_type r = (*a) - (*b);
    return r;
}

} // namespace rdl2
} // namespace scene_rdl2

