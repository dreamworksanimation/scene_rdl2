// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <type_traits>

namespace fauxstd {
namespace detail {

// Use SFINAE to determine whether a type has a member "is_always_equal."
template <typename T>
struct has_is_always_equal
{
    typedef char Yes[1];
    typedef char No[2];

    template <typename U>
    static Yes& check(typename U::is_always_equal*);

    template <typename U>
    static No& check(...);

    static const bool value = sizeof(check<T>(nullptr)) == sizeof(Yes);
};

// If has_iae is true, the type has a member "is_always_equal."
template <typename T, bool has_iae>
struct is_always_equal_helper_dispatched;

// If the class has a member "is_always_equal," we defer to its value.
template <typename T>
struct is_always_equal_helper_dispatched<T, true>
{
    static const bool value = T::is_always_equal::value;
};

// If the class does not have a member "is_always_equal," we defer to is_empty
// (this is how is_always_equal is defined in the standard).
template <typename T>
struct is_always_equal_helper_dispatched<T, false>
{
    static const bool value = std::is_empty<T>::value;
};

template <typename T>
struct is_always_equal_helper
{
    // Determine if we have a member "is_always_equal," and dispatch our
    // decision based on that.
    static const bool value = is_always_equal_helper_dispatched<T, has_is_always_equal<T>::value>::value;
};

template <typename...>
struct And;

template <>
struct And<> : public std::true_type
{
};

template <typename B1>
struct And<B1> : public B1
{
};

template <typename B1, typename B2>
struct And<B1, B2> : public std::conditional<B1::value, B2, B1>::type
{
};

template <typename B1, typename B2, typename B3, typename... Bn>
struct And<B1, B2, B3, Bn...> : public std::conditional<B1::value, And<B2, B3, Bn...>, B1>::type
{
};

// Sanity checks
static_assert( fauxstd::detail::And<std::true_type,  std::true_type,  std::true_type,  std::true_type>::value,  "True");
static_assert(!fauxstd::detail::And<std::false_type, std::false_type, std::true_type,  std::true_type>::value,  "False");
static_assert(!fauxstd::detail::And<std::false_type, std::true_type,  std::true_type,  std::true_type>::value,  "False");
static_assert(!fauxstd::detail::And<std::true_type,  std::false_type, std::true_type,  std::true_type>::value,  "False");
static_assert(!fauxstd::detail::And<std::true_type,  std::true_type,  std::false_type, std::true_type>::value,  "False");
static_assert(!fauxstd::detail::And<std::true_type,  std::true_type,  std::true_type,  std::false_type>::value, "False");
} // namespace detail

// This is meant to be a stand-in for C++17's is_always_equal support for
// allocators, as defined in std::allocator_traits.
// TODO: C++17 remove and use std::alloctaor_traits<T>::is_always_equal
template <typename T>
struct is_always_equal :
    public std::integral_constant<bool, detail::is_always_equal_helper<T>::value>
{
};

} // namespace fauxstd

