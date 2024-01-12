// Copyright 2023-2024 DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

//
//
#pragma once

#include <ratio>

namespace scene_rdl2 {
namespace grid_util {

//
// User defined literals for size of memory
//   _b   : Bit      : 8_b   = 1 byte
//   _B   : Byte     : 1_B   = 1 byte
//   _KiB : Kibibyte : 1_KiB = 1024_B
//   _MiB : Mebibyte : 1_MiB = 1024_KiB
//   _GiB : Gibitye  : 1_GiB = 1024_MiB
//

/* CPPCHECK returns error from following code unfortunately.
using Bit      = std::ratio<1, 8>;
using Byte     = std::ratio<1, 1>;
using Kibibyte = std::ratio<1024, 1>;
using Mebibyte = std::ratio<1024*1024, 1>;
using Gibibyte = std::ratio<1024*1024*1024, 1>;
*/
typedef std::ratio<1, 8>              Bit;
typedef std::ratio<1, 1>              Byte;
typedef std::ratio<1024, 1>           Kibibyte;
typedef std::ratio<1024*1024, 1>      Mebibyte;
typedef std::ratio<1024*1024*1024, 1> Gibibyte;

constexpr std::intmax_t operator "" _b(unsigned long long s)
{
    return (s * Bit::num + (Bit::den - 1)) / Bit::den;
}

constexpr long double operator "" _b(long double s)
{
    return (s * Bit::num) / Bit::den;
}

constexpr std::intmax_t operator "" _B(unsigned long long s)
{
    return s;
}

constexpr long double operator "" _B(long double s)    
{
    return (s * Byte::num) / Byte::den;
}

constexpr std::intmax_t operator "" _KiB(unsigned long long s)    
{
    return (s * Kibibyte::num) / Kibibyte::den;
}

constexpr long double operator "" _KiB(long double s)
{
    return (s * Kibibyte::num) / Kibibyte::den;
}

constexpr std::intmax_t operator "" _MiB(unsigned long long s)
{
    return (s * Mebibyte::num) / Mebibyte::den;
}

constexpr long double operator "" _MiB(long double s)
{
    return (s * Mebibyte::num) / Mebibyte::den;
}

constexpr std::intmax_t operator "" _GiB(unsigned long long s)
{
    return (s * Gibibyte::num) / Gibibyte::den;
}

constexpr long double operator "" _GiB(long double s)
{
    return (s * Gibibyte::num) / Gibibyte::den;
}

} // namespace grid_util
} // namespace scene_rdl2

