// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <atomic>
#include <type_traits>
#include <memory>


// In C++17, std::atomic<float>, std::atomic<double>, and std::atomic<long
// double> behave more like the integral atomic types. We're going to add this
// behavior. A C++17 library that supports this behavior will define this
// preprocessor directive.
#if !defined(__cpp_lib_atomic_float)

#if defined(__GNUC__) && !defined(__INTEL_COMPILER)
// GCC complains about some of the pointers being passed into the intrinsics
// aren't being used, which is ironic, as those are GCC intrinsics.
#pragma GCC diagnostic ignored "-Wunused-but-set-parameter"
#endif

namespace af_detail {

// We do a lot of casting to integer values for the intrinsics interface. Let's make sure we are casting the values we
// think we are.
static_assert(__ATOMIC_RELAXED == static_cast<int>(std::memory_order_relaxed));
static_assert(__ATOMIC_CONSUME == static_cast<int>(std::memory_order_consume));
static_assert(__ATOMIC_ACQUIRE == static_cast<int>(std::memory_order_acquire));
static_assert(__ATOMIC_RELEASE == static_cast<int>(std::memory_order_release));
static_assert(__ATOMIC_ACQ_REL == static_cast<int>(std::memory_order_acq_rel));
static_assert(__ATOMIC_SEQ_CST == static_cast<int>(std::memory_order_seq_cst));

// In compare_exchange overloads where only one memory order is given, we have
// to decide on the other. These are what is laid out by the standard.
constexpr std::memory_order compare_exchange_duo(std::memory_order in) noexcept
{
    constexpr int mapping[6] = {
            /* std::memory_order_relaxed -> */ static_cast<int>(std::memory_order_relaxed),
            /* std::memory_order_consume -> */ static_cast<int>(std::memory_order_consume),
            /* std::memory_order_acquire -> */ static_cast<int>(std::memory_order_acquire),
            /* std::memory_order_release -> */ static_cast<int>(std::memory_order_relaxed),
            /* std::memory_order_acq_rel -> */ static_cast<int>(std::memory_order_acquire),
            /* std::memory_order_seq_cst -> */ static_cast<int>(std::memory_order_seq_cst)

    };
    return static_cast<std::memory_order>(mapping[static_cast<int>(in)]);
}

template <typename T>
class atomic_fp
{
    static_assert(std::is_floating_point<T>::value, "Only for floating point types");
    static constexpr std::size_t k_alignment = alignof(T);

    template <typename U>
    using NV = std::remove_volatile_t<U>;

public:
    using value_type = T;
    using difference_type = value_type;

    static constexpr bool is_always_lock_free = __atomic_always_lock_free(sizeof(T), 0);

    atomic_fp() noexcept = default;
    constexpr atomic_fp(T t) : m_fp(t) { }

    atomic_fp(const atomic_fp&) = delete;
    atomic_fp& operator=(const atomic_fp&) = delete;
    atomic_fp& operator=(const atomic_fp&) volatile = delete;

    T operator=(T t) noexcept
    {
        this->store(t);
        return t;
    }

    T operator=(T t) volatile noexcept
    {
        this->store(t);
        return t;
    }

    bool is_lock_free() const noexcept
    {
        return __atomic_is_lock_free(sizeof(T), 0);
    }

    bool is_lock_free() const volatile noexcept
    {
        return __atomic_is_lock_free(sizeof(T), 0);
    }

    void store(T t, std::memory_order m = std::memory_order_seq_cst) noexcept
    {
        do_store(std::addressof(m_fp), t, m);
    }

    void store(T t, std::memory_order m = std::memory_order_seq_cst) volatile noexcept
    {
        do_store(std::addressof(m_fp), t, m);
    }

    T load(std::memory_order m = std::memory_order_seq_cst) const noexcept
    {
        return do_load(std::addressof(m_fp), m);
    }

    T load(std::memory_order m = std::memory_order_seq_cst) volatile const noexcept
    {
        return do_load(std::addressof(m_fp), m);
    }

    operator T() const noexcept
    {
        return this->load();
    }

    operator T() volatile const noexcept
    {
        return this->load();
    }

    T exchange(T desired, std::memory_order m = std::memory_order_seq_cst) noexcept
    {
        return do_exchange(std::addressof(m_fp), desired, m);
    }

    T exchange(T desired, std::memory_order m = std::memory_order_seq_cst) volatile noexcept
    {
        return do_exchange(std::addressof(m_fp), desired, m);
    }

    bool compare_exchange_weak(T& expected, T desired, std::memory_order success, std::memory_order failure) noexcept
    {
        return do_compare_exchange_weak(std::addressof(m_fp), expected, desired, success, failure);
    }

    bool compare_exchange_weak(T& expected, T desired, std::memory_order success, std::memory_order failure) volatile noexcept
    {
        return do_compare_exchange_weak(std::addressof(m_fp), expected, desired, success, failure);
    }

    bool compare_exchange_strong(T& expected, T desired, std::memory_order success, std::memory_order failure) noexcept
    {
        return do_compare_exchange_strong(std::addressof(m_fp), expected, desired, success, failure);
    }

    bool compare_exchange_strong(T& expected, T desired, std::memory_order success, std::memory_order failure) volatile noexcept
    {
        return do_compare_exchange_strong(std::addressof(m_fp), expected, desired, success, failure);
    }

    bool compare_exchange_weak(T& expected, T desired, std::memory_order order = std::memory_order_seq_cst) noexcept
    {
        return do_compare_exchange_weak(std::addressof(m_fp), expected, desired, order, compare_exchange_duo(order));
    }

    bool compare_exchange_weak(T& expected, T desired, std::memory_order order = std::memory_order_seq_cst) volatile noexcept
    {
        return do_compare_exchange_weak(std::addressof(m_fp), expected, desired, order, compare_exchange_duo(order));
    }

    bool compare_exchange_strong(T& expected, T desired, std::memory_order order = std::memory_order_seq_cst) noexcept
    {
        return do_compare_exchange_strong(std::addressof(m_fp), expected, desired, order, compare_exchange_duo(order));
    }

    bool compare_exchange_strong(T& expected, T desired, std::memory_order order = std::memory_order_seq_cst) volatile noexcept
    {
        return do_compare_exchange_strong(std::addressof(m_fp), expected, desired, order, compare_exchange_duo(order));
    }

    T fetch_add(T i, std::memory_order m = std::memory_order_seq_cst) noexcept
    {
        return do_fetch_add(std::addressof(m_fp), i, m);
    }

    T fetch_add(T i, std::memory_order m = std::memory_order_seq_cst) volatile noexcept
    {
        return do_fetch_add(std::addressof(m_fp), i, m);
    }

    T fetch_sub(T i, std::memory_order m = std::memory_order_seq_cst) noexcept
    {
        return do_fetch_sub(std::addressof(m_fp), i, m);
    }

    T fetch_sub(T i, std::memory_order m = std::memory_order_seq_cst) volatile noexcept
    {
        return do_fetch_sub(std::addressof(m_fp), i, m);
    }

    T operator+=(T i) noexcept
    {
        return do_add_fetch(std::addressof(m_fp), i);
    }

    T operator+=(T i) volatile noexcept
    {
        return do_add_fetch(std::addressof(m_fp), i);
    }

    T operator-=(T i) noexcept
    {
        return do_sub_fetch(std::addressof(m_fp), i);
    }

    T operator-=(T i) volatile noexcept
    {
        return do_sub_fetch(std::addressof(m_fp), i);
    }

protected:
    // This is meant to be used as a base class in a non-virtual way. Disable
    // instantiating objects of this type.
    ~atomic_fp() noexcept = default;

private:
    template <typename U>
    static void do_store(U* ptr, NV<U> t, std::memory_order m) noexcept
    {
        __atomic_store(ptr, std::addressof(t), static_cast<int>(m));
    }

    template <typename U>
    static U do_load(const U* ptr, std::memory_order m) noexcept
    {
        alignas(U) unsigned char buf[sizeof(U)];
        auto* const dest = reinterpret_cast<NV<U>*>(buf);
        __atomic_load(ptr, dest, static_cast<int>(m));
        return *dest;
    }

    template <typename U>
    static NV<U> do_exchange(U* ptr, NV<U> desired, std::memory_order m) noexcept
    {
        alignas(U) unsigned char buf[sizeof(U)];
        auto* const dest = reinterpret_cast<NV<U>*>(buf);
        __atomic_exchage(ptr, std::addressof(desired), dest, static_cast<int>(m));
        return *dest;
    }

    template <typename U>
    static bool do_compare_exchange_weak(U* ptr,
                                         NV<U>& expected,
                                         NV<U>& desired,
                                         std::memory_order success,
                                         std::memory_order failure) noexcept
    {
        return __atomic_compare_exchange(ptr,
                                         std::addressof(expected),
                                         std::addressof(desired),
                                         true,
                                         static_cast<int>(success),
                                         static_cast<int>(failure));
    }

    template <typename U>
    static bool do_compare_exchange_strong(U* ptr,
                                           NV<U>& expected,
                                           NV<U>& desired,
                                           std::memory_order success,
                                           std::memory_order failure) noexcept
    {
        return __atomic_compare_exchange(ptr,
                                         std::addressof(expected),
                                         std::addressof(desired),
                                         false,
                                         static_cast<int>(success),
                                         static_cast<int>(failure));
    }

    template <typename U>
    static U do_fetch_add(U* ptr, NV<U> i, std::memory_order m = std::memory_order_seq_cst) noexcept
    {
        NV<U> old_val = do_load(ptr, std::memory_order_relaxed);
        NV<U> new_val = old_val + i;
        while (!do_compare_exchange_weak(ptr, old_val, new_val, m, std::memory_order_relaxed)) {
            new_val = old_val + i;
        }
        return old_val;
    }

    template <typename U>
    static U do_add_fetch(U* ptr, NV<U> i) noexcept
    {
        NV<U> old_val = do_load(ptr, std::memory_order_relaxed);
        NV<U> new_val = old_val + i;
        while (!do_compare_exchange_weak(ptr, old_val, new_val, std::memory_order_seq_cst, std::memory_order_relaxed)) {
            new_val = old_val + i;
        }
        return new_val;
    }

    template <typename U>
    static U do_fetch_sub(U* ptr, NV<U> i, std::memory_order m = std::memory_order_seq_cst) noexcept
    {
        NV<U> old_val = do_load(ptr, std::memory_order_relaxed);
        NV<U> new_val = old_val - i;
        while (!do_compare_exchange_weak(ptr, old_val, new_val, m, std::memory_order_relaxed)) {
            new_val = old_val - i;
        }
        return old_val;
    }

    template <typename U>
    static U do_sub_fetch(U* ptr, NV<U> i) noexcept
    {
        NV<U> old_val = do_load(ptr, std::memory_order_relaxed);
        NV<U> new_val = old_val - i;
        while (!do_compare_exchange_weak(ptr, old_val, new_val, std::memory_order_seq_cst, std::memory_order_relaxed)) {
            new_val = old_val - i;
        }
        return new_val;
    }

    alignas(k_alignment) T m_fp;
};

} // namespace af_detail

namespace std {

template <>
class atomic<float> : public af_detail::atomic_fp<float>
{
public:
    atomic() noexcept = default;
    atomic(float desired) noexcept : af_detail::atomic_fp<float>(desired) { }

    atomic& operator=(const atomic&) = delete;
    atomic& operator=(const atomic&) volatile = delete;

    using af_detail::atomic_fp<float>::operator=;
};

template <>
class atomic<double> : public af_detail::atomic_fp<double>
{
public:
    atomic() noexcept = default;
    atomic(double desired) noexcept : af_detail::atomic_fp<double>(desired) { }

    atomic& operator=(const atomic&) = delete;
    atomic& operator=(const atomic&) volatile = delete;

    using af_detail::atomic_fp<double>::operator=;
};

template <>
class atomic<long double> : public af_detail::atomic_fp<long double>
{
public:
    atomic() noexcept = default;
    atomic(long double desired) noexcept : af_detail::atomic_fp<long double>(desired) { }

    atomic& operator=(const atomic&) = delete;
    atomic& operator=(const atomic&) volatile = delete;

    using af_detail::atomic_fp<long double>::operator=;
};

// C++ standard requirements.
static_assert(std::is_standard_layout<atomic<float>>::value);
static_assert(std::is_standard_layout<atomic<double>>::value);
static_assert(std::is_standard_layout<atomic<long double>>::value);
static_assert(std::is_trivially_destructible<atomic<float>>::value);
static_assert(std::is_trivially_destructible<atomic<double>>::value);
static_assert(std::is_trivially_destructible<atomic<long double>>::value);

} // namespace std
#endif // #if !defined(__cpp_lib_atomic_float)


