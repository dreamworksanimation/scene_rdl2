// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

///
///

#pragma once

#include "type_traits.h"

#include <scene_rdl2/common/platform/Platform.h>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>

namespace scene_rdl2 {
namespace util {

namespace detail {
// Like std::uninitialized_copy, but with allocator support.
template <class InputIt, class ForwardIt, class Allocator>
ForwardIt uninitialized_copy(InputIt first, InputIt last, ForwardIt d_first, Allocator& alloc)
{
    using traits = std::allocator_traits<Allocator>;

    ForwardIt current = d_first;
    try {
        for (; first != last; ++first, ++current) {
            traits::construct(alloc, std::addressof(*current), *first);
        }
        return current;
    } catch (...) {
        for (; d_first != current; ++d_first) {
            traits::destroy(alloc, std::addressof(*d_first));
        }
        throw;
    }
}

// Like std::uninitialized_fill, but with allocator support.
template <class ForwardIt, class T, class Allocator>
void uninitialized_fill(ForwardIt first, ForwardIt last, const T& value, Allocator& alloc)
{
    using traits = std::allocator_traits<Allocator>;

    ForwardIt current = first;
    try {
        for (; current != last; ++current) {
            traits::construct(alloc, std::addressof(*current), value);
        }
    } catch (...) {
        for (; first != current; ++first) {
            traits::destroy(alloc, std::addressof(*first));
        }
        throw;
    }
}

// Like std::uninitialized_fill, but with default construction and allocator
// support.
template <class ForwardIt, class Allocator>
void uninitialized_fill(ForwardIt first, ForwardIt last, Allocator& alloc)
{
    using traits = std::allocator_traits<Allocator>;

    ForwardIt current = first;
    try {
        for (; current != last; ++current) {
            traits::construct(alloc, std::addressof(*current));
        }
    } catch (...) {
        for (; first != current; ++first) {
            traits::destroy(alloc, std::addressof(*first));
        }
        throw;
    }
}
} // namespace detail

enum class Array2DOrder
{
    ROW_MAJOR, // This is what 2D C/C++ arrays are: use this for easy interoperability
    COL_MAJOR  // This is probably more intuitive from an image-processing standpoint
};

/// @class Array2DBase
/// @brief A dynamic 2D array that uses a single allocation.
template <typename T, Array2DOrder order, typename Allocator>
class Array2DBase
{
    // This odd construct is so that we can make use of the empty base class
    // optimization and not pay for storing an empty allocator.
    // This class does all of the heavy lifting for an Array2DBase.
    struct DataBlock : private Allocator
    {
        using traits                 = std::allocator_traits<Allocator>;
        using value_type             = T;
        using allocator_type         = typename traits::allocator_type;
        using size_type              = std::int32_t;
        using difference_type        = std::ptrdiff_t;
        using reference              = value_type&;
        using const_reference        = const value_type&;
        using pointer                = typename traits::pointer;
        using const_pointer          = typename traits::const_pointer;
        using iterator               = pointer;
        using const_iterator         = const_pointer;
        using reverse_iterator       = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        pointer   mP;
        size_type mURes;
        size_type mVRes;

        explicit DataBlock(const Allocator& alloc);

        DataBlock(size_type nu, size_type nv, const Allocator& alloc);

        DataBlock(size_type nu, size_type nv, const T& t, const Allocator& alloc);

        template <typename InputIter>
        DataBlock(size_type nu, size_type nv, InputIter first, InputIter last, const Allocator& alloc);

        DataBlock(const DataBlock& other);

        DataBlock(DataBlock&& other) noexcept;

        ~DataBlock();

        void destroy() noexcept;

        DataBlock& operator=(const DataBlock& other);

        DataBlock& operator=(DataBlock&& other) noexcept(traits::propagate_on_container_move_assignment::value ||
                                                         std::allocator_traits<allocator_type>::is_always_equal::value);

        void swap(DataBlock& other) noexcept;

        size_type getOffset(size_type u, size_type v) const noexcept;

        pointer getAddress(size_type u, size_type v) noexcept;

        const_pointer getAddress(size_type u, size_type v) const noexcept;

        size_type getAllocationSize() const noexcept;

        iterator begin() noexcept
        {
            return mP;
        }

        const_iterator begin() const noexcept
        {
            return mP;
        }

        const_iterator cbegin() const noexcept
        {
            return mP;
        }

        iterator end() noexcept
        {
            return mP + getAllocationSize();
        }

        const_iterator end() const noexcept
        {
            return mP + getAllocationSize();
        }

        const_iterator cend() const noexcept
        {
            return mP + getAllocationSize();
        }

        pointer data() noexcept
        {
            return mP;
        }

        const_pointer data() const noexcept
        {
            return mP;
        }

        Allocator& getAllocator() noexcept
        {
            return *this;
        }

        const Allocator& getAllocator() const noexcept
        {
            return *this;
        }
    };

protected:
    // Make the destructor protected and non-virtual: this is a base class that shouldn't be instantiated but shouldn't
    // be used polymorphically.
    ~Array2DBase() = default;

    // We declare these so that we can use them in the derived classes. We can't do it with the noexcept operator in
    // conjunction with a noexcept specifier because our destructor is protected, and even though the noexcept operator
    // is not supposed to evaluate the expression, GCC, ICC, and Clang all complain about the temporary not having a
    // public destructor (only MSVC gets this right, good job, Microsoft!).
    // E.g., noexcept(Array2DBase<std::decltype<Array2DBase&>()) fails in all of these compilers.
    static constexpr bool no_except_copy_constructor = false;
    static constexpr bool no_except_move_constructor = true;
    static constexpr bool no_except_copy_assignment  = false;
    static constexpr bool no_except_move_assignment  =
        noexcept(std::declval<DataBlock&>().operator=(std::declval<DataBlock&&>()));

public:
    using traits                 = typename DataBlock::traits;
    using value_type             = typename DataBlock::value_type;
    using allocator_type         = typename DataBlock::allocator_type;
    using size_type              = typename DataBlock::size_type;
    using difference_type        = typename DataBlock::difference_type;
    using reference              = typename DataBlock::reference;
    using const_reference        = typename DataBlock::const_reference;
    using pointer                = typename DataBlock::pointer;
    using const_pointer          = typename DataBlock::const_pointer;
    using iterator               = typename DataBlock::iterator;
    using const_iterator         = typename DataBlock::const_iterator;
    using reverse_iterator       = typename DataBlock::reverse_iterator;
    using const_reverse_iterator = typename DataBlock::const_reverse_iterator;

    explicit Array2DBase(const Allocator& alloc = Allocator())
    : mData(alloc)
    {
    }

    Array2DBase(const Array2DBase& other) noexcept(no_except_copy_constructor)
    : mData(other.mData)
    {
    }

    Array2DBase(Array2DBase&& other) noexcept(no_except_move_constructor)
    : mData(std::move(other.mData))
    {
    }

    Array2DBase(size_type nu, size_type nv, const T& t, const Allocator& alloc = Allocator())
    : mData(nu, nv, t, alloc)
    {
    }

    Array2DBase(size_type nu, size_type nv, const Allocator& alloc = Allocator())
    : mData(nu, nv, alloc)
    {
    }

    template <typename InputIter>
    Array2DBase(size_type nu, size_type nv, InputIter first, InputIter last, const Allocator& alloc = Allocator())
    : mData(nu, nv, first, last, alloc)
    {
    }

    // We don't do the usual strong exception guarantee method of copying and
    // swapping, because we have to handle the allocators specially, depending
    // on their traits. Just let mData take care of that responsibility.
    Array2DBase& operator=(const Array2DBase& other) noexcept(no_except_copy_assignment)
    {
        if (this != std::addressof(other)) {
            mData = other.mData;
        }
        return *this;
    }

    Array2DBase&
    operator=(Array2DBase&& other) noexcept(no_except_move_assignment)
    {
        if (this != std::addressof(other)) {
            mData = std::move(other.mData);
        }
        return *this;
    }

    void swap(Array2DBase& other) noexcept
    {
        mData.swap(other.mData);
    }

    size_type uSize() const noexcept
    {
        return mData.mURes;
    }

    size_type vSize() const noexcept
    {
        return mData.mVRes;
    }

    reference operator()(size_type u, size_type v) noexcept
    {
        const auto p = mData.getAddress(u, v);
        return *p;
    }

    const_reference operator()(size_type u, size_type v) const noexcept
    {
        const auto p = mData.getAddress(u, v);
        return *p;
    }

    iterator begin() noexcept
    {
        return mData.begin();
    }

    const_iterator begin() const noexcept
    {
        return mData.begin();
    }

    const_iterator cbegin() const noexcept
    {
        return mData.cbegin();
    }

    iterator end() noexcept
    {
        return mData.end();
    }

    const_iterator end() const noexcept
    {
        return mData.end();
    }

    const_iterator cend() const noexcept
    {
        return mData.cend();
    }

    pointer data() noexcept
    {
        return mData.data();
    }

    const_pointer data() const noexcept
    {
        return mData.data();
    }

    allocator_type get_allocator() const
    {
        return mData.getAllocator();
    }

private:
    DataBlock mData;
};

// This array has the same memory layout as a 2D C array: row-major.
// Constructors using iterators expect linear access in row-major ordering.
// Iterators from this class are linear access in row-major ordering.
template <typename T, typename Allocator = std::allocator<T>>
class Array2DC : private Array2DBase<T, Array2DOrder::ROW_MAJOR, Allocator>
{
    using BaseArray = Array2DBase<T, Array2DOrder::ROW_MAJOR, Allocator>;

public:
    using traits                 = typename BaseArray::traits;
    using value_type             = typename BaseArray::value_type;
    using allocator_type         = typename BaseArray::allocator_type;
    using size_type              = typename BaseArray::size_type;
    using difference_type        = typename BaseArray::difference_type;
    using reference              = typename BaseArray::reference;
    using const_reference        = const value_type&;
    using pointer                = typename traits::pointer;
    using const_pointer          = typename traits::const_pointer;
    using iterator               = pointer;
    using const_iterator         = const_pointer;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    using BaseArray::uSize;
    using BaseArray::vSize;
    using BaseArray::operator();
    using BaseArray::begin;
    using BaseArray::cbegin;
    using BaseArray::cend;
    using BaseArray::data;
    using BaseArray::end;
    using BaseArray::get_allocator;

    explicit Array2DC(const Allocator& alloc = Allocator())
    : BaseArray(alloc)
    {
    }

    Array2DC(size_type nu, size_type nv, const T& t, const Allocator& alloc = Allocator())
    : BaseArray(nu, nv, t, alloc)
    {
    }

    Array2DC(size_type nu, size_type nv, const Allocator& alloc = Allocator())
    : BaseArray(nu, nv, alloc)
    {
    }

    template <typename InputIter>
    Array2DC(size_type nu, size_type nv, InputIter first, InputIter last, const Allocator& alloc = Allocator())
    : BaseArray(nu, nv, first, last, alloc)
    {
    }

    Array2DC(const Array2DC& other) noexcept(BaseArray::no_except_copy_constructor)
    : BaseArray(other)
    {
    }

    Array2DC(Array2DC&& other) noexcept(BaseArray::no_except_move_constructor)
    : BaseArray(other)
    {
    }

    Array2DC& operator=(const Array2DC& other) noexcept(BaseArray::no_except_copy_assignment)
    {
        BaseArray::operator=(other);
        return *this;
    }

    Array2DC& operator=(Array2DC&& other) noexcept(BaseArray::no_except_move_assignment)
    {
        BaseArray::operator=(std::move(other));
        return *this;
    }

    void swap(Array2DC& other) noexcept
    {
        BaseArray::swap(other);
    }
};

// This array has a column-major memory layout.
// Constructors using iterators expect linear access in column-major ordering.
// Iterators from this class are linear access in column-major ordering.
template <typename T, typename Allocator = std::allocator<T>>
class Array2D : private Array2DBase<T, Array2DOrder::COL_MAJOR, Allocator>
{
    using BaseArray = Array2DBase<T, Array2DOrder::COL_MAJOR, Allocator>;

public:
    using traits                 = typename BaseArray::traits;
    using value_type             = typename BaseArray::value_type;
    using allocator_type         = typename BaseArray::allocator_type;
    using size_type              = typename BaseArray::size_type;
    using difference_type        = typename BaseArray::difference_type;
    using reference              = typename BaseArray::reference;
    using const_reference        = const value_type&;
    using pointer                = typename traits::pointer;
    using const_pointer          = typename traits::const_pointer;
    using iterator               = pointer;
    using const_iterator         = const_pointer;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    using BaseArray::swap;
    using BaseArray::operator();
    using BaseArray::begin;
    using BaseArray::cbegin;
    using BaseArray::cend;
    using BaseArray::data;
    using BaseArray::end;
    using BaseArray::get_allocator;

    explicit Array2D(const Allocator& alloc = Allocator())
    : BaseArray(alloc)
    {
    }

    Array2D(size_type nu, size_type nv, const T& t, const Allocator& alloc = Allocator())
    : BaseArray(nu, nv, t, alloc)
    {
    }

    Array2D(size_type nu, size_type nv, const Allocator& alloc = Allocator())
    : BaseArray(nu, nv, alloc)
    {
    }

    template <typename InputIter>
    Array2D(size_type nu, size_type nv, InputIter first, InputIter last, const Allocator& alloc = Allocator())
    : BaseArray(nu, nv, first, last, alloc)
    {
    }

    Array2D(const Array2D& other) noexcept(BaseArray::no_except_copy_constructor)
    : BaseArray(other)
    {
    }

    Array2D(Array2D&& other) noexcept(BaseArray::no_except_move_constructor)
    : BaseArray(other)
    {
    }

    Array2D& operator=(const Array2D& other) noexcept(BaseArray::no_except_copy_assignment)
    {
        BaseArray::operator=(other);
        return *this;
    }

    Array2D& operator=(Array2D&& other) noexcept(BaseArray::no_except_move_assignment)
    {
        BaseArray::operator=(std::move(other));
        return *this;
    }

    auto getWidth() const noexcept
    {
        return BaseArray::uSize();
    }

    auto getHeight() const noexcept
    {
        return BaseArray::vSize();
    }

    void swap(Array2D& other) noexcept
    {
        BaseArray::swap(other);
    }
};

template <typename T, Array2DOrder order, typename Allocator>
Array2DBase<T, order, Allocator>::DataBlock::DataBlock(const Allocator& alloc)
: Allocator(alloc)
, mP(nullptr)
, mURes(0)
, mVRes(0)
{
    // Strong exception guarantee. The only thing that can throw is the
    // allocator copy constructor. In that case, our destructor is never called,
    // and there's nothing to release.
}

template <typename T, Array2DOrder order, typename Allocator>
Array2DBase<T, order, Allocator>::DataBlock::DataBlock(size_type nu, size_type nv, const Allocator& alloc)
: Allocator(alloc)
, mP(nullptr)
, mURes(nu)
, mVRes(nv)
{
    // Strong exception guarantee. If the allocator copy throws, we never
    // allocate anything, our destructor is never called, and there's nothing to
    // release.

    // If the allocation throws, the memory is released. Nothing to clean up.
    mP = traits::allocate(this->getAllocator(), getAllocationSize());
    try {
        // uninitialized_fill destroys anything constructed automatically. We
        // just have to clean up the allocated memory.
        detail::uninitialized_fill(begin(), end(), this->getAllocator());
    } catch (...) {
        this->getAllocator().deallocate(mP, getAllocationSize());
        throw;
    }
}

template <typename T, Array2DOrder order, typename Allocator>
Array2DBase<T, order, Allocator>::DataBlock::DataBlock(size_type nu, size_type nv, const T& t, const Allocator& alloc)
: Allocator(alloc)
, mP(nullptr)
, mURes(nu)
, mVRes(nv)
{
    // Strong exception guarantee. If the allocator copy throws, we never
    // allocate anything, our destructor is never called, and there's nothing to
    // release.

    // If the allocation throws, the memory is released. Nothing to clean up.
    mP = traits::allocate(this->getAllocator(), getAllocationSize());
    try {
        // uninitialized_fill destroys anything constructed automatically. We
        // just have to clean up the allocated memory.
        detail::uninitialized_fill(begin(), end(), t, this->getAllocator());
    } catch (...) {
        this->getAllocator().deallocate(mP, getAllocationSize());
        throw;
    }
}

template <typename T, Array2DOrder order, typename Allocator>
template <typename InputIter>
Array2DBase<T, order, Allocator>::DataBlock::DataBlock(size_type        nu,
                                                       size_type        nv,
                                                       InputIter        first,
                                                       InputIter        last,
                                                       const Allocator& alloc)
: Allocator(alloc)
, mP(nullptr)
, mURes(nu)
, mVRes(nv)
{
    // Strong exception guarantee. If the allocator copy throws, we never
    // allocate anything, our destructor is never called, and there's nothing to
    // release.

    // If the allocation throws, the memory is released. Nothing to clean up.
    mP = traits::allocate(this->getAllocator(), getAllocationSize());
    try {
        // uninitialized_copy destroys anything constructed automatically. We
        // just have to clean up the allocated memory.
        detail::uninitialized_copy(first, last, begin(), this->getAllocator());
    } catch (...) {
        this->getAllocator().deallocate(mP, getAllocationSize());
        throw;
    }
}

template <typename T, Array2DOrder order, typename Allocator>
Array2DBase<T, order, Allocator>::DataBlock::DataBlock(const DataBlock& other)
: Allocator(traits::select_on_container_copy_construction(other))
, mP(nullptr)
, mURes(other.mURes)
, mVRes(other.mVRes)
{
    // Strong exception guarantee. If the allocator copy throws, we never
    // allocate anything, our destructor is never called, and there's nothing to
    // release.

    // If the allocation throws, the memory is released. Nothing to clean up.
    mP = traits::allocate(this->getAllocator(), getAllocationSize());
    try {
        // uninitialized_copy destroys anything constructed automatically. We
        // just have to clean up the allocated memory.
        detail::uninitialized_copy(other.cbegin(), other.cend(), begin(), this->getAllocator());
    } catch (...) {
        traits::deallocate(this->getAllocator(), mP, getAllocationSize());
        throw;
    }
}

template <typename T, Array2DOrder order, typename Allocator>
Array2DBase<T, order, Allocator>::DataBlock::DataBlock(DataBlock&& other) noexcept
: Allocator(std::move(other))
, mP(other.mP)
, mURes(other.mURes)
, mVRes(other.mVRes)
{
    other.mP    = nullptr;
    other.mURes = 0;
    other.mVRes = 0;
}

template <typename T, Array2DOrder order, typename Allocator>
Array2DBase<T, order, Allocator>::DataBlock::~DataBlock()
{
    destroy();
}

template <typename T, Array2DOrder order, typename Allocator>
void Array2DBase<T, order, Allocator>::DataBlock::destroy() noexcept
{
    if (mP) {
        std::for_each(begin(), end(), [this](reference p) {
            traits::destroy(this->getAllocator(), std::addressof(p));
        });

        traits::deallocate(this->getAllocator(), mP, getAllocationSize());
    }
}

template <typename T, Array2DOrder order, typename Allocator>
typename Array2DBase<T, order, Allocator>::DataBlock&
Array2DBase<T, order, Allocator>::DataBlock::operator=(const DataBlock& other)
{
    // Offers the strong exception guarantee.

    const bool equalAllocators =
        std::allocator_traits<allocator_type>::is_always_equal::value || this->getAllocator() == other.getAllocator();

    if (traits::propagate_on_container_copy_assignment::value && !equalAllocators) {
        // Strong exception guarantee. Any potentially throwing work is done
        // before we modify this.
        allocator_type     alloc(other);
        std::unique_ptr<T> p(traits::allocate(this->getAllocator(), getAllocationSize()));
        detail::uninitialized_copy(other.mP, other.mP + getAllocationSize(), p.get(), alloc);
        destroy();
        allocator_type::operator=(std::move(alloc));
        mP = p.release();
    } else {
        // Strong exception guarantee. Any potentially throwing work is done
        // before we modify this.
        std::unique_ptr<T> p(traits::allocate(this->getAllocator(), other.getAllocationSize()));
        detail::uninitialized_copy(other.mP, other.mP + other.getAllocationSize(), p.get(), this->getAllocator());
        destroy();
        mP = p.release();
    }
    mURes = other.mURes;
    mVRes = other.mVRes;
    return *this;
}

template <typename T, Array2DOrder order, typename Allocator>
typename Array2DBase<T, order, Allocator>::DataBlock& Array2DBase<T, order, Allocator>::DataBlock::operator=(
    DataBlock&& other) noexcept(traits::propagate_on_container_move_assignment::value ||
                                std::allocator_traits<allocator_type>::is_always_equal::value)
{
    // Offers the strong exception guarantee (at worst), and the no exception
    // guarantee (at best).

    using std::swap; // Allow ADL

    if (traits::propagate_on_container_move_assignment::value && this->getAllocator() != other.getAllocator()) {
        // We get to move the allocator. This is good.
        allocator_type::operator=(std::move(other));
        swap(mP, other.mP);
        swap(mURes, other.mURes);
        swap(mVRes, other.mVRes);
    } else if (std::allocator_traits<allocator_type>::is_always_equal::value ||
               this->getAllocator() == other.getAllocator()) {
        // We don't have to move the allocator. This is good.
        swap(mP, other.mP);
        swap(mURes, other.mURes);
        swap(mVRes, other.mVRes);
    } else {
        // Our only option is to copy all of the elements, because we
        // can't do anything smart with the allocators. This is bad.

        // Strong exception guarantee. Any potentially throwing work is done
        // before we modify this.
        std::unique_ptr<T> p(traits::allocate(this->getAllocator(), other.getAllocationSize()));

        // Const cast, because "other" is non-const. If the type has a non-const
        // template single-argument constructor, we will try to call that
        // instead of the copy constructor.
        detail::uninitialized_copy(const_cast<const T*>(other.mP),
                                   const_cast<const T*>(other.mP + other.getAllocationSize()),
                                   p.get(),
                                   this->getAllocator());
        destroy();
        mP = p.release();
        mURes = other.mURes;
        mVRes = other.mVRes;
    }
    return *this;
}

template <typename T, Array2DOrder order, typename Allocator>
void Array2DBase<T, order, Allocator>::DataBlock::swap(DataBlock& other) noexcept
{
    using std::swap; // Allow ADL
    if (traits::propagate_on_container_swap::value) {
        // We get to swap the allocators. This is good.
        swap(this->getAllocator(), other.getAllocator());
    } else {
        // If we're not swapping the allocators, they had better be equal!
        MNRY_ASSERT(std::allocator_traits<allocator_type>::is_always_equal::value ||
                    this->getAllocator() == other.getAllocator());
    }

    swap(mP, other.mP);
    swap(mURes, other.mURes);
    swap(mVRes, other.mVRes);
}

template <typename T, Array2DOrder order, typename Allocator>
typename Array2DBase<T, order, Allocator>::DataBlock::size_type
Array2DBase<T, order, Allocator>::DataBlock::getOffset(size_type u, size_type v) const noexcept
{
    MNRY_ASSERT(u >= 0);
    MNRY_ASSERT(v >= 0);
    MNRY_ASSERT(u < mURes);
    MNRY_ASSERT(v < mVRes);
    if (order == Array2DOrder::ROW_MAJOR) {
        const size_type ofst = u * mVRes + v;
        MNRY_ASSERT(ofst < mURes * mVRes);
        MNRY_ASSERT(ofst >= 0);
        return ofst;
    } else {
        const size_type ofst = v * mURes + u;
        MNRY_ASSERT(ofst < mURes * mVRes);
        MNRY_ASSERT(ofst >= 0);
        return ofst;
    }
}

template <typename T, Array2DOrder order, typename Allocator>
typename Array2DBase<T, order, Allocator>::DataBlock::pointer
Array2DBase<T, order, Allocator>::DataBlock::getAddress(size_type u, size_type v) noexcept
{
    return mP + getOffset(u, v);
}

template <typename T, Array2DOrder order, typename Allocator>
typename Array2DBase<T, order, Allocator>::DataBlock::const_pointer
Array2DBase<T, order, Allocator>::DataBlock::getAddress(size_type u, size_type v) const noexcept
{
    return mP + getOffset(u, v);
}

template <typename T, Array2DOrder order, typename Allocator>
typename Array2DBase<T, order, Allocator>::DataBlock::size_type
Array2DBase<T, order, Allocator>::DataBlock::getAllocationSize() const noexcept
{
    return mURes * mVRes;
}

template <typename T, typename Allocator>
inline bool operator==(const Array2DC<T, Allocator>& a, const Array2DC<T, Allocator>& b)
{
    return a.uSize() == b.uSize() && a.vSize() == b.vSize() && std::equal(a.cbegin(), a.cend(), b.cbegin());
}

template <typename T, typename Allocator>
inline bool operator!=(const Array2DC<T, Allocator>& a, const Array2DC<T, Allocator>& b)
{
    return !(a == b);
}

template <typename T, typename Allocator>
inline bool operator<(const Array2DC<T, Allocator>& a, const Array2DC<T, Allocator>& b)
{
    return std::lexicographical_compare(a.cbegin(), a.cend(), b.cbegin(), b.cend());
}

template <typename T, typename Allocator>
inline bool operator>(const Array2DC<T, Allocator>& a, const Array2DC<T, Allocator>& b)
{
    return b < a;
}

template <typename T, typename Allocator>
inline bool operator>=(const Array2DC<T, Allocator>& a, const Array2DC<T, Allocator>& b)
{
    return !(a < b);
}

template <typename T, typename Allocator>
inline bool operator<=(const Array2DC<T, Allocator>& a, const Array2DC<T, Allocator>& b)
{
    return !(b < a);
}

template <typename T, typename Allocator>
inline bool operator==(const Array2D<T, Allocator>& a, const Array2D<T, Allocator>& b)
{
    return a.getWidth() == b.getWidth() && a.getHeight() == b.getHeight() && std::equal(a.cbegin(), a.cend(), b.cbegin());
}

template <typename T, typename Allocator>
inline bool operator!=(const Array2D<T, Allocator>& a, const Array2D<T, Allocator>& b)
{
    return !(a == b);
}

template <typename T, typename Allocator>
inline bool operator<(const Array2D<T, Allocator>& a, const Array2D<T, Allocator>& b)
{
    return std::lexicographical_compare(a.cbegin(), a.cend(), b.cbegin(), b.cend());
}

template <typename T, typename Allocator>
inline bool operator>(const Array2D<T, Allocator>& a, const Array2D<T, Allocator>& b)
{
    return b < a;
}

template <typename T, typename Allocator>
inline bool operator>=(const Array2D<T, Allocator>& a, const Array2D<T, Allocator>& b)
{
    return !(a < b);
}

template <typename T, typename Allocator>
inline bool operator<=(const Array2D<T, Allocator>& a, const Array2D<T, Allocator>& b)
{
    return !(b < a);
}

} // namespace util
} // namespace scene_rdl2

