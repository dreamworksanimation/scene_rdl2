// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <scene_rdl2/common/platform/Platform.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <mutex>
#include <type_traits>
#include <unordered_map>
#include <vector>

// Uncomment DO_INDEXABLE_ARRAY_INVARIANT_CHECKING to test invariants for all
// public APIs. Keep turned off unless trying to track down bugs. Very expensive,
// typically accounts for ~50% of render time in debug builds of moonray.
#ifdef DEBUG
//#define DO_INDEXABLE_ARRAY_INVARIANT_CHECKING
#endif

namespace scene_rdl2 {

///
/// @class IndexableArray
/// @brief A class that mostly acts like a std::vector, except it also allows
/// for constant-time lookup of the vector index by value. I.e. It allows for
/// bidirectional constant-time lookup either through array index or value.
///
/// Do not modify the order of the array (e.g. std::sort(a.begin(), a.end())).
/// This will break the class invariant. (In fact, the class does not provide
/// non-const iterators for this reason).
///
template <typename T, typename Hash = std::hash<T>, typename KeyEqual = std::equal_to<T>>
class IndexableArray
{
private:
    inline void invariant_check() const;

    template <typename U>
    inline void push_back_impl(U&& value);

public:
    typedef std::vector<T>                                                  container_type;
    typedef Hash                                                            hasher;
    typedef KeyEqual                                                        key_equal;

    // We store a map of the objects' hash values to the array index. This means
    // that we don't have to store the object twice (we already have it in a
    // vector), but we can still look the index up in constant time.
    //
    // We use two containers for bidirectional lookup.
    typedef std::unordered_multimap<typename hasher::result_type, int32_t>  map_type;
    typedef T                                                               value_type;
    typedef typename container_type::allocator_type                         allocator_type;
    typedef typename container_type::size_type                              size_type;
    typedef typename container_type::difference_type                        difference_type;
    typedef typename container_type::reference                              reference;
    typedef typename container_type::const_reference                        const_reference;
    typedef typename container_type::pointer                                pointer;
    typedef typename container_type::const_pointer                          const_pointer;
    typedef typename container_type::const_iterator                         const_iterator;
    typedef typename container_type::const_reverse_iterator                 const_reverse_iterator;
    typedef typename map_type::const_iterator                               const_map_iterator;

    // An iterator over the indices that match the passed in type. It's always
    // const, since it makes no sense to change the indices. The value_type of
    // the iterator is an index into the array (i.e. dereferencing the iterator
    // gives back an array index instead of an object).
    class MapIndexIterator
    {
        typedef const_map_iterator internal_iterator_type;
        typedef const IndexableArray<T, Hash, KeyEqual>* container_pointer;

    public:
        using value_type        = uint32_t;
        using difference_type   = std::ptrdiff_t;
        using pointer           = uint32_t*;
        using reference         = uint32_t&;
        using iterator_category = std::forward_iterator_tag;

        MapIndexIterator(container_pointer c,
                         const T& value,
                         internal_iterator_type first,
                         internal_iterator_type last) :
            mValue(value),
            mContainer(c),
            mCurrent(first),
            mLast(last)
        {
            find_next_valid_value();
        }

        friend bool operator==(const MapIndexIterator& a, const MapIndexIterator& b)
        {
            MNRY_ASSERT(a.mContainer == b.mContainer);
            return a.mCurrent == b.mCurrent;
        }

        friend bool operator!=(const MapIndexIterator& a, const MapIndexIterator& b)
        {
            return !(a == b);
        }

        value_type operator*() const
        {
            return mCurrent->second;
        }

        MapIndexIterator &operator++()
        {
            ++mCurrent;
            find_next_valid_value();
            return *this;
        }

        MapIndexIterator operator++(int)
        {
            const MapIndexIterator old(*this);
            ++(*this);
            return old;
        }

    private:
        void find_next_valid_value()
        {
            // Multiple values can hash to the same value, so we have to make
            // sure that our returned dereferenced type actually equals the
            // value for which we're searching.

            // Increment if not last and it's not equal
            while (mCurrent != mLast &&
                   !key_equal()((*mContainer)[mCurrent->second], mValue)) {
                ++mCurrent;
            }
        }

        T mValue;
        container_pointer mContainer;
        internal_iterator_type mCurrent;
        internal_iterator_type mLast;
    };

    typedef MapIndexIterator index_iterator;

    IndexableArray() :
        mIndexMap(),
        mValues()
    {
        invariant_check();
    }

    // O(n)
    template <typename Iter>
    IndexableArray(Iter first, Iter last) :
        mIndexMap(),
        mValues(first, last)
    {
        typedef typename map_type::value_type value_type;
        mIndexMap.reserve(mValues.size());
        int32_t size = static_cast<int32_t>(mValues.size());
        for (int32_t i = 0; i < size; ++i) {
            mIndexMap.insert(value_type(hasher()(mValues[i]), i));
        }
        invariant_check();
    }

    // O(n)
    IndexableArray(std::initializer_list<value_type> init) :
        IndexableArray(init.begin(), init.end())
    {
    }

    IndexableArray(const IndexableArray&) = default;
    IndexableArray(IndexableArray&&) = default;
    IndexableArray& operator=(const IndexableArray&) = default;
    IndexableArray& operator=(IndexableArray&&) = default;

    // Amortized O(1)
    void push_back(const T& t)
    {
        invariant_check();
        push_back_impl(t);
        invariant_check();
    }

    // Amortized O(1)
    void push_back(T&& t)
    {
        invariant_check();
        push_back_impl(std::move(t));
        invariant_check();
    }

    // Amortized O(1)
    template <typename... Args>
    void emplace_back(Args&&... args)
    {
        invariant_check();
        typedef typename map_type::value_type value_type;
        const auto s = mValues.size();
        mValues.emplace_back(std::forward<Args>(args)...);
        mIndexMap.insert(value_type(hasher()(mValues.back()), s));
        invariant_check();
    }

    // O(1)
    void clear()
    {
        invariant_check();
        mValues.clear();
        mIndexMap.clear();
        invariant_check();
    }

    // O(1)
    bool empty() const
    {
        invariant_check();
        return mValues.empty();
    }

    // O(1)
    size_type size() const
    {
        invariant_check();
        return mValues.size();
    }

    // We don't provide a non-const reference version of operator[]. We could
    // write one with a proxy value that does this logic, but there are hidden
    // costs. We make updating values explicit.
    // Average: O(1)
    // Worst case: O(n)
    void update_value(size_type i, const T& val)
    {
        invariant_check();
        update_value_impl(i, val);
        invariant_check();
    }

    // Average: O(1)
    // Worst case: O(n)
    void update_value(size_type i, T&& val)
    {
        invariant_check();
        update_value_impl(i, std::move(val));
        invariant_check();
    }

    // O(1)
    const_reference operator[](size_type i) const
    {
        invariant_check();
        return mValues[i];
    }

    // Average: O(n)
    // Worst case: (n^2)
    const_iterator erase(const_iterator pos)
    {
        invariant_check();
        const int32_t idx = pos - mValues.cbegin();

        auto pair = mIndexMap.equal_range(hasher()(*pos));
        for (auto it = pair.first; it != pair.second; ++it) {
            if (it->second == idx) { // If this is the index...
                mIndexMap.erase(it);
                break;
            }
        }

        // Everything beyond index is going to be shifted down one. We have to
        // adjust our stored indices.
        for (auto& v : mIndexMap) {
            if (v.second >= idx) {
                --v.second;
            }
        }

        //const_iterator ret = mValues.erase(pos);
        // TODO: A hack, since our version of the standard library doesn't
        // respect the const_iterator version of erase.
        const_iterator ret = mValues.erase(mValues.begin() + idx);
        invariant_check();
        return ret;
    }

    // Average: O(1)
    // Worst case: O(n)
    std::pair<index_iterator, index_iterator> equal_range(const T& val) const
    {
        invariant_check();
        const auto itpair = mIndexMap.equal_range(hasher()(val));
        return std::make_pair(index_iterator(this, val, itpair.first, itpair.second),
                              index_iterator(this, val, itpair.second, itpair.second));
    }

    // O(1)
    const_reference front() const
    {
        invariant_check();
        return mValues.front();
    }

    // O(1)
    const_reference back() const
    {
        invariant_check();
        return mValues.back();
    }

    // O(1)
    const_iterator begin() const
    {
        invariant_check();
        return mValues.begin();
    }

    // O(1)
    const_iterator cbegin() const
    {
        invariant_check();
        return mValues.cbegin();
    }

    // O(1)
    const_iterator end() const
    {
        invariant_check();
        return mValues.end();
    }

    // O(1)
    const_iterator cend() const
    {
        invariant_check();
        return mValues.cend();
    }

    // O(1)
    const_iterator rbegin() const
    {
        invariant_check();
        return mValues.rbegin();
    }

    // O(1)
    const_iterator crbegin() const
    {
        invariant_check();
        return mValues.crbegin();
    }

    // O(1)
    const_iterator rend() const
    {
        invariant_check();
        return mValues.crend();
    }

    // O(1)
    const_iterator crend() const
    {
        invariant_check();
        return mValues.crend();
    }

private:
    template <typename U>
    void update_value_impl(size_type i, U&& value)
    {
        MNRY_ASSERT(i < mValues.size());
        typedef typename map_type::value_type value_type;

        // We have to remove the old value from the hash.
        const auto oldhash = hasher()(mValues[i]);
        auto pair = mIndexMap.equal_range(oldhash);
        for (auto it = pair.first; it != pair.second; ++it) {
            if (it->second == i) { // If this is the index...
                mIndexMap.erase(it);
                break;
            }
        }

        mValues[i] = std::forward<U>(value);
        mIndexMap.insert(value_type(hasher()(mValues[i]), i));
    }

    typename map_type::const_iterator get_key(const T& val) const
    {
        const auto itpair = mIndexMap.equal_range(hasher()(val));
        for (auto it = itpair.first; it != itpair.second; ++it) {
            const T& lookup = (*this)[it->second];
            if (key_equal()(lookup, val)) {
                return it;
            }
        }
        return mIndexMap.end();
    }

    int32_t get_index(const T& val) const
    {
        auto it = get_key(val);
        return (it == mIndexMap.end()) ? -1 : it->second;
    }

    map_type       mIndexMap;
    container_type mValues;
};

template <typename T, typename Hash, typename KeyEqual>
void IndexableArray<T, Hash, KeyEqual>::invariant_check() const
{
#if defined(DO_INDEXABLE_ARRAY_INVARIANT_CHECKING)
    MNRY_ASSERT(mValues.size() == mIndexMap.size());

    std::deque<bool> indexList(mValues.size(), false);
    for (const auto p : mIndexMap) {
        const std::size_t idx = p.second;

        MNRY_ASSERT(idx < mValues.size());
        // We haven't already seen this index.
        MNRY_ASSERT(indexList[idx] != true);
        MNRY_ASSERT(hasher()(mValues[idx]) == p.first);
        indexList[idx] = true;
    }

    // We have a map to all items in the list.
    MNRY_ASSERT(std::all_of(indexList.cbegin(), indexList.cend(),
                [](bool b) { return b; }));
#endif
}

template <typename T, typename Hash, typename KeyEqual>
template <typename U>
void IndexableArray<T, Hash, KeyEqual>::push_back_impl(U&& value)
{
    typedef typename map_type::value_type value_type;
    mIndexMap.insert(value_type(hasher()(value), mValues.size()));
    mValues.push_back(std::forward<U>(value));
}

template <typename T, typename Hash, typename KeyEqual>
bool operator==(const IndexableArray<T, Hash, KeyEqual>& lhs,
                const IndexableArray<T, Hash, KeyEqual>& rhs)
{
    return lhs.size() == rhs.size() &&
           std::equal(lhs.begin(), lhs.end(), rhs.begin());
};

template <typename T, typename Hash, typename KeyEqual>
bool operator!=(const IndexableArray<T, Hash, KeyEqual>& lhs,
                const IndexableArray<T, Hash, KeyEqual>& rhs)
{
    return !(lhs == rhs);
};

// Average case: O(n), where n is the number of elements in the container.
// Worst case: O(n^3), when every element matches the value, and you have very
// bad luck. Don't be that guy.
template <typename T, typename Hash, typename KeyEqual, typename U>
void erase_all(IndexableArray<T, Hash, KeyEqual>& a, const U& value)
{
    auto p = a.equal_range(value);
    auto first = p.first;
    auto last  = p.second;
    while (first != last) {
        // Tricky business. The iterator returned by equal_range will be
        // invalided when we call erase on that element, but the other iterators
        // aren't. Move to the next iterator before we call erase.
        auto idx = *first;
        ++first;
        a.erase(a.begin() + idx);
    }
};

} // namespace scene_rdl2


