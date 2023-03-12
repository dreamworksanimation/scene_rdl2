// Copyright 2023 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "integer_sequence.h"
#include <ostream>
#include <tuple>

namespace scene_rdl2 {
namespace util {

//
// How to use the SManip class:
//
// Define a function that returns std::ios_base& and takes std::ios_base& as
// the first argument. You can define any other arguments you like after the
// first. E.g.
//
// std::ios_base& myfunc(std::ios_base& outs, int& x, double y) { ... }
//
// This is the function that will do the stream manipulation.
//
// Define a function that will be used as your stream manipulator. In this
// function, declare an SManip with template parameters that match your
// function parameters exactly:
//
// SManip<int&, double> mymanip(int& x, double y)
// {
//     return SManip<int&, double>(&myfunc, x, y);
// }
//
// Call with your manip:
// int x = 42;
// std::cout << mymanip(x, 3.14) << '\n';
//

namespace detail {
// TODO: We can make these functions more general with C++14's decltype(auto)
// return type. We may be able to remove some code if C++17's apply() gets
// accepted. We may have to do some std::bind magic with the ios_base.
template<typename Func, typename Tup, std::size_t... index>
std::ios_base& /*decltype(auto)*/ invoke_helper(Func&& func, std::ios_base& base, Tup&& tup, fauxstd::index_sequence<index...>)
{
    return func(base, std::get<index>(std::forward<Tup>(tup))...);
}
 
template<typename Func, typename Tup>
std::ios_base& /*decltype(auto)*/ invoke(Func&& func, std::ios_base& base, Tup&& tup)
{
    constexpr auto Size = std::tuple_size<typename std::decay<Tup>::type>::value;
    return invoke_helper(std::forward<Func>(func),
                         base,
                         std::forward<Tup>(tup),
                         fauxstd::make_index_sequence<Size>{});
}
} // namespace detail

// Manipulator taking arguments
//
// There are no explicit references in this class, as we make the user
// fully-specify the types in the template arguments. If the user wants l-value
// or r-value reference semantics, they have to supply them in the type.
template <typename... Args>
struct SManip
{
    using Function = std::ios_base& (*)(std::ios_base&, Args...);

    Function mF;
    std::tuple<Args...> mArgs;

    SManip(Function ff, Args... ii) :
        mF(ff),
        mArgs(std::forward<Args>(ii)...)
    {
    }
};


template <typename... Args>
inline std::ostream& operator<<(std::ostream& os, SManip<Args...>&& m)
{
    detail::invoke(m.mF, os, std::forward<decltype(m.mArgs)>(m.mArgs));
    return os;
}

} // namespace util
} // namespace scene_rdl2


