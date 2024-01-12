// Copyright 2023-2024 DreamWorks Animation LLC and Intel Corporation
// SPDX-License-Identifier: Apache-2.0

// Low level include file which is shared between C++ and ISPC.
//
// Note: .hh files are files that are included from both C++ and ISPC code.
// You have to be very careful about what goes in a .hh file. A .hh file can
// only include another .hh file, and can NEVER include a .h or .isph file.
// The main usage of .hh files is to define members for matching C++ classes /
// ISPC uniform struct. This header contains utility macros / defines useful
// for this purpose.

#pragma once

////////////////////////////////////////////////////////////////////////////////
/// detect platform
////////////////////////////////////////////////////////////////////////////////

/* detect 32 or 64 platform */
#if defined(__x86_64__) || defined(__ia64__) || defined(_M_X64)
#define __X86_64__
#endif

/* detect Linux platform */
#if defined(linux) || defined(__linux__) || defined(__LINUX__)
#  if !defined(__LINUX__)
#     define __LINUX__
#  endif
#  if !defined(__UNIX__)
#     define __UNIX__
#  endif
#endif

/* detect FreeBSD platform */
#if defined(__FreeBSD__) || defined(__FREEBSD__)
#  if !defined(__FREEBSD__)
#     define __FREEBSD__
#  endif
#  if !defined(__UNIX__)
#     define __UNIX__
#  endif
#endif

/* detect Windows 95/98/NT/2000/XP/Vista/7 platform */
#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)) && !defined(__CYGWIN__)
#  if !defined(__WIN32__)
#     define __WIN32__
#  endif
#endif

/* detect Cygwin platform */
#if defined(__CYGWIN__)
#  if !defined(__UNIX__)
#     define __UNIX__
#  endif
#endif

/* detect MAC OS X platform */
#if defined(__APPLE__) || defined(MACOSX) || defined(__MACOSX__)
#  if !defined(__MACOSX__)
#     define __MACOSX__
#  endif
#  if !defined(__UNIX__)
#     define __UNIX__
#  endif
#endif

/* try to detect other Unix systems */
#if defined(__unix__) || defined (unix) || defined(__unix) || defined(_unix)
#  if !defined(__UNIX__)
#     define __UNIX__
#  endif
#endif

#if defined (_DEBUG)
#define DEBUG
#endif

// MoonRay: adds the VLEN to each of these cases
#if defined(__AVX512F__)
    #define isa knl
    #define VLEN 16u
#elif (defined(__AVX512F__) || defined(__AVX512__))
    #define isa knl
    #define VLEN 16u
#elif defined (__AVX2__)
    #define isa avx2
    #define VLEN 8u
#elif defined(__AVXI__)
    #define isa avxi
    #define VLEN 8u
#elif defined(__AVX__)
    #define isa avx
    #define VLEN 8u
#elif defined (__SSE4_2__)
    #define isa sse42
    #define VLEN 4u
#elif defined (__SSE4_1__)
    #define isa sse41
    #define VLEN 4u
#elif defined(__SSSE3__)
    #define isa ssse3
    #define VLEN 4u
#elif defined(__SSE3__)
    #define isa sse3
    #define VLEN 4u
#elif defined(__SSE2__)
    #define isa sse2
    #define VLEN 4u
#elif defined(__SSE__)
    #define isa sse
    #define VLEN 4u
#else
    #error Unknown ISA
#endif

// MoonRay: adds the code below

#define AVX512_SIMD_MEMORY_ALIGNMENT    64u
#define AVX512_SIMD_REGISTER_SIZE       64u
#define AVX512_VLEN                     16u
#define AVX512_VLEN_MASK                15u
#define AVX512_VLEN_SHIFT               4u

#define AVX_SIMD_MEMORY_ALIGNMENT       32u
#define AVX_SIMD_REGISTER_SIZE          32u
#define AVX_VLEN                        8u
#define AVX_VLEN_MASK                   7u
#define AVX_VLEN_SHIFT                  3u

#define SSE_SIMD_MEMORY_ALIGNMENT       16u
#define SSE_SIMD_REGISTER_SIZE          16u
#define SSE_VLEN                        4u
#define SSE_VLEN_MASK                   3u
#define SSE_VLEN_SHIFT                  2u

#if (VLEN == 16u)
    #define SIMD_MEMORY_ALIGNMENT       AVX512_SIMD_MEMORY_ALIGNMENT
    #define SIMD_REGISTER_SIZE          AVX512_SIMD_REGISTER_SIZE
    #define VLEN_MASK                   AVX512_VLEN_MASK
    #define VLEN_SHIFT                  AVX512_VLEN_SHIFT
#elif (VLEN == 8u)
    #define SIMD_MEMORY_ALIGNMENT       AVX_SIMD_MEMORY_ALIGNMENT
    #define SIMD_REGISTER_SIZE          AVX_SIMD_REGISTER_SIZE
    #define VLEN_MASK                   AVX_VLEN_MASK
    #define VLEN_SHIFT                  AVX_VLEN_SHIFT
#elif (VLEN == 4u)
    #define SIMD_MEMORY_ALIGNMENT       SSE_SIMD_MEMORY_ALIGNMENT
    #define SIMD_REGISTER_SIZE          SSE_SIMD_REGISTER_SIZE
    #define VLEN_MASK                   SSE_VLEN_MASK
    #define VLEN_SHIFT                  SSE_VLEN_SHIFT
#else
    #error Unknown vector width
#endif

#define CACHE_LINE_SIZE 64u

