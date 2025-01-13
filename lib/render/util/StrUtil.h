// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cmath> // log10(), round()
#include <cstring>
#include <iomanip>
#include <sstream>

#include <cxxabi.h>

#ifdef __GNUC__
  #define ATTR_UNUSED __attribute__((unused))
#else
  #define ATTR_UNUSED
#endif

namespace scene_rdl2 {
namespace str_util {

namespace detail {

ATTR_UNUSED inline std::size_t size(const std::string &s) { return s.size(); }
ATTR_UNUSED inline std::size_t size(char) { return 1; }
ATTR_UNUSED inline std::size_t size(const char *s) { return std::strlen(s); }
inline std::size_t totalSize() { return 0; }

template <typename First, typename... Rest>
std::size_t
totalSize(const First &first, const Rest&... rest)
{
    return size(first) + totalSize(rest...);
}

inline void stringCatHelper(std::string &) {}

template <typename First, typename... Rest>
void
stringCatHelper(std::string &result, const First &first, const Rest&... rest)
{
    result += first;
    stringCatHelper(result, rest...);
}

} // namespace detail

//---------------------------------------------------------------------------------------------------------------

template <typename... T>
std::string
stringCat(const T&... vals)
{
    std::string result;
    result.reserve(detail::totalSize(vals...));
    detail::stringCatHelper(result, vals...);
    return result;
}

//---------------------------------------------------------------------------------------------------------------

inline
std::string
addIndent(const std::string &str, const int indentTotal = 1)
//
// str should not end by '\n'.
// It works without crash if you use string which terminated by '\n' but not recommended.
// Last '\n' (new line) control should be done by caller function level.
// General concensus of using addIndent() requires input string which is not terminated by '\n' (new line).
//
{
    std::string indentStr(indentTotal * 2, ' ');

    int totalNewLine = 0;
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '\n') ++totalNewLine;
    }

    std::string newStr;
    newStr.resize(str.size() + (totalNewLine + 1) * indentStr.size());
    newStr = indentStr;
    for (size_t i = 0; i < str.size(); ++i) {
        newStr.push_back(str[i]);
        if (str[i] == '\n') newStr += indentStr;
    }
    return newStr;
}

//---------------------------------------------------------------------------------------------------------------

inline
int
getNumberOfDigits(size_t n)
// returns number of digits in decimal representation of n
{
    return (!n) ? 1 : static_cast<int>(std::log10(static_cast<float>(n)) + 1.0f);
}

inline
int
getNumberOfDigits(unsigned n)
// returns number of digits in decimal representation of n
{
    return (!n) ? 1 : static_cast<int>(std::log10(static_cast<float>(n)) + 1.0f);
}

inline
std::string
byteStr(const size_t numByte)
{
    std::ostringstream ostr;
    if (numByte < (size_t)1024) {
        ostr << numByte << " Byte";
    } else if (numByte < (size_t)1024 * (size_t)1024) {
        float f = (float)numByte / 1024.0f;
        ostr << std::setw(3) << std::fixed << std::setprecision(2) << f << " KByte";
    } else if (numByte < (size_t)1024 * (size_t)1024 * (size_t)1024) {
        float f = (float)numByte / (1024.0f * 1024.0f);
        ostr << std::setw(3) << std::fixed << std::setprecision(2) << f << " MByte";
    } else {
        float f = (float)numByte / (1024.0f * 1024.0f * 1024.0f);
        ostr << std::setw(3) << std::fixed << std::setprecision(2) << f << " GByte";
    }
    return ostr.str();
}

inline
std::string
secStr(const float sec)
{
    float roundedSec;
    std::ostringstream ostr;

    // In order to displaying 1.000 sec instead of 1000.00 ms, we use rounded logic
    roundedSec = std::round(sec * 100000.0f) / 100000.0f;
    if (roundedSec < 1.0f) {
        float ms = roundedSec * 1000.0f;
        ostr << std::setw(6) << std::fixed << std::setprecision(2) << ms << " ms";
        return ostr.str();
    }

    // Without using a rounded sec value, we get 1 min 60.000 sec when the original sec is 119.9996f
    // With using a rounded sec value, the result would be 2 min 0.000 sec
    roundedSec = std::round(sec * 1000.0f) / 1000.0f;
    if (roundedSec < 60.0f) {
        ostr << std::setw(6) << std::fixed << std::setprecision(3) << roundedSec << " sec";
    } else {
        int m = static_cast<int>(roundedSec / 60.0f);
        float s = roundedSec - static_cast<float>(m) * 60.0f;
        ostr << m << " min " << std::setw(6) << std::fixed << std::setprecision(3) << s << " sec";
    }
    return ostr.str();
}

inline
std::string
boolStr(const bool b)
{
    return (b)? "true": "false";
}

inline
std::string
demangle(const std::string &str)
{
    int st;
    return std::string(std::move(abi::__cxa_demangle(str.c_str(), 0, 0, &st)));
}

inline
std::string
trimBlank(const std::string &str)
//
// Remove visually blank char of first and last part of the specified string
//
{
    static const std::string trimCharList = (const char *)" \t\v\r\n";

    std::string result;
    std::string::size_type left = str.find_first_not_of(trimCharList);
    if (left != std::string::npos) {
        std::string::size_type right = str.find_last_not_of(trimCharList);
        result = str.substr(left, right - left + 1);
    }
    return result;
}

inline
std::string
replaceNlToSingleSpace(const std::string &str)
//
// Replaces newlines (i.e. \n) with a single space
//
{
    std::string result;
    for (size_t i = 0; i < str.size(); ++i) {
        char c = str[i];
        if (c == '\n') c = ' ';
        result.push_back(c);
    }
    return result;
}

inline
std::string
replaceBlankToSingleSpace(const std::string &str)
//
// Replaces blank chars with a single space
//
{
    std::string result;

    if (str.empty()) {
        return result;
    }

    // first of all, multi spaces (i.e. blank) should be converted to single space
    for (size_t i = 0; i < str.size(); ++i) {
        char c = str[i];
        if (std::isblank(c)) {
            if (i > 0) {
                if (!std::isblank(str[i-1])) result.push_back(' ');
            }
        } else {
            result.push_back(c);
        }
    }

    if (!result.empty() && result.back() == '\n') result.pop_back(); // rm last newline
    if (!result.empty() && result.back() == ' ') result.pop_back(); // rm last space

    return result;
}

inline
std::string
upperStr(const std::string &str)
{
    std::string result;
    for (size_t i = 0; i < str.size(); ++i) {
        result.push_back(static_cast<char>(std::toupper(static_cast<int>(str[i]))));
    }
    return result;
}

inline
std::string
rmLastNL(const std::string& inStr)
{
    std::string outStr = inStr;
    while (!outStr.empty() && outStr.back() == '\n') {
        outStr.pop_back();
    }
    return outStr;
}

} // namespace str_util
} // namespace scene_rdl2
