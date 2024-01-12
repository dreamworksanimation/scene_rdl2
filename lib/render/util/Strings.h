// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <string>
#include <sstream>
#include <utility>

namespace scene_rdl2 {
namespace util {

namespace detail {

inline void
combineString(std::ostream&)
{
    // Base case for recursion. Do nothing.
}

template <typename First, typename... Rest>
inline void
combineString(std::ostream& o, const First& value, const Rest&... rest)
{
    o << value;
    combineString(o, rest...);
}

} // namespace detail

/**
 * Builds a std::string from the passed arguments by inserting them into a
 * std::ostringstream.
 *
 * Arguments can be any type that can be written to an ostream with operator<<,
 * as well as any of the standard iostream manipulators.
 *
 * Example:
 *      const char* name = "Bob";
 *      int year = 2014;
 *      std::string myStr = buildString("Hello ", name, " the year is ", year);
 *
 * @param   value...    A list of values to be concatenated together into a
 *                      single string.
 * @return  A string with all the values concatenated.
 */
template <typename... T>
std::string
buildString(const T&... value)
{
    std::ostringstream o;
    detail::combineString(o, value...);
    return o.str();
}

} // namespace util
} // namespace scene_rdl2

