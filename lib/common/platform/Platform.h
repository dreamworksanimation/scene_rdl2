// Copyright 2023-2024 DreamWorks Animation LLC and Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <stdexcept>

#include <iostream>
#include <sstream>
#include <string>

// Intel: begin *****
/*
#include <string.h>

////////////////////////////////////////////////////////////////////////////////
/// detect platform
////////////////////////////////////////////////////////////////////////////////

// detect 32 or 64 platform
#if defined(__x86_64__) || defined(__ia64__) || defined(_M_X64)
#define __X86_64__
#endif

// detect Linux platform
#if defined(linux) || defined(__linux__) || defined(__LINUX__)
#  if !defined(__LINUX__)
#     define __LINUX__
#  endif
#  if !defined(__UNIX__)
#     define __UNIX__
#  endif
#endif

// detect FreeBSD platform
#if defined(__FreeBSD__) || defined(__FREEBSD__)
#  if !defined(__FREEBSD__)
#     define __FREEBSD__
#  endif
#  if !defined(__UNIX__)
#     define __UNIX__
#  endif
#endif

// detect Windows 95/98/NT/2000/XP/Vista/7 platform
#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)) && !defined(__CYGWIN__)
#  if !defined(__WIN32__)
#     define __WIN32__
#  endif
#endif

// detect Cygwin platform
#if defined(__CYGWIN__)
#  if !defined(__UNIX__)
#     define __UNIX__
#  endif
#endif

// detect MAC OS X platform
#if defined(__APPLE__) || defined(MACOSX) || defined(__MACOSX__)
#  if !defined(__MACOSX__)
#     define __MACOSX__
#  endif
#  if !defined(__UNIX__)
#     define __UNIX__
#  endif
#endif

// try to detect other Unix systems
#if defined(__unix__) || defined (unix) || defined(__unix) || defined(_unix)
#  if !defined(__UNIX__)
#     define __UNIX__
#  endif
#endif

#if defined (_DEBUG)
#define DEBUG
#endif
*/
// Intel: end *****

////////////////////////////////////////////////////////////////////////////////
/// Configurations
////////////////////////////////////////////////////////////////////////////////

#if defined(__WIN32__) 
#if defined(CONFIG_AVX)
  #define __TARGET_AVX__
  #if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    #define __SSE3__
    #define __SSSE3__
    #define __SSE4_1__
    #define __SSE4_2__
    #if !defined(__AVX__)
      #define __AVX__
    #endif
  #endif
#endif
#if defined(CONFIG_AVX2)
  #define __TARGET_AVX__
  #define __TARGET_AVX2__
  #if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    #define __SSE3__
    #define __SSSE3__
    #define __SSE4_1__
    #define __SSE4_2__
    #if !defined(__AVX__)
      #define __AVX__
    #endif
    #if !defined(__AVX2__)
      #define __AVX2__
    #endif
#endif
#endif
//#define __USE_RAY_MASK__
//#define __USE_STAT_COUNTERS__
//#define __BACKFACE_CULLING__
#define __INTERSECTION_FILTER__
#define __BUFFER_STRIDE__
//#define __SPINLOCKS__
//#define __LOG_TASKS__
#endif

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define __SSE__
#define __SSE2__
#endif

////////////////////////////////////////////////////////////////////////////////
/// Makros
////////////////////////////////////////////////////////////////////////////////

#ifdef __WIN32__
#define __dllexport extern "C" __declspec(dllexport)
#define __dllimport extern "C" __declspec(dllimport)
#else
#define __dllexport extern "C" __attribute__ ((visibility ("default")))
#define __dllimport extern "C"
#endif

#if defined(_WIN32) || defined(__EXPORT_ALL_SYMBOLS__)
#  define __hidden
#else
#  define __hidden __attribute__ ((visibility ("hidden")))
#endif

#ifdef __WIN32__
#undef __noinline
#define __noinline             __declspec(noinline)
//#define __forceinline          __forceinline
//#define __restrict             __restrict
#define __thread               __declspec(thread)
#define __aligned(...)           __declspec(align(__VA_ARGS__))
//#define __FUNCTION__           __FUNCTION__
#define debugbreak()           __debugbreak()

#else
#undef __noinline
#undef __forceinline
#define __noinline             __attribute__((noinline))

// MoonRay: begin added *****
#ifdef DEBUG
#define __forceinline          inline
#define finline                inline
#else
// The move to icc19 revealed quite a few bugs when forcing inline functions.
// For now we only truly force inlining when buidling for icc15.  We can expand
// this set of compilers if performance benefits and regression tests indicate
// it is wise to do so.
#if defined(__INTEL_COMPILER) && __INTEL_COMPILER < 1600
// MoonRay: end added *****
#define __forceinline          inline __attribute__((always_inline))
// MoonRay: begin *****
#define finline                inline __attribute__((always_inline))
#else
#define __forceinline          inline
#define finline                inline
#endif
#endif
//#define __restrict           __restrict
//#define __thread             __thread
#define __align(...)           __attribute__((aligned(__VA_ARGS__)))
#define ALIGN(x)               __attribute__((aligned(x)))
#define CACHE_LINE_SIZE        64u
#define CACHE_ALIGN            __align(CACHE_LINE_SIZE)
// MoonRay: end *****
// Intel: begin *****
/*
//#define __restrict             __restrict
//#define __thread               __thread
#define __aligned(...)           __attribute__((aligned(__VA_ARGS__)))
*/
// Intel: end *****
#define __FUNCTION__           __PRETTY_FUNCTION__
#define debugbreak()           asm ("int $3")
#endif

#ifdef __GNUC__
  #define MAYBE_UNUSED __attribute__((used))
#else
  #define MAYBE_UNUSED
#endif

#if defined(_MSC_VER)
  #define __restrict__
#endif

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define   likely(expr) (expr)
#define unlikely(expr) (expr)
#else
#define   likely(expr) __builtin_expect((expr),true )
#define unlikely(expr) __builtin_expect((expr),false)
#endif

/* compiler memory barriers */
#ifdef __GNUC__
#  define __memory_barrier() asm volatile("" ::: "memory")
#elif defined(__MIC__)
#define __memory_barrier()
#elif defined(__INTEL_COMPILER)
//#define __memory_barrier() __memory_barrier()
#elif  defined(_MSC_VER)
#  define __memory_barrier() _ReadWriteBarrier()
#endif

// MoonRay added begin: *****
namespace detail {
inline void debugPrintHelper(std::ostringstream& oss)
{
    // printf is required to be thread-safe by POSIX and the C standard. This
    // does not apply to C++ streams.
    std::printf("%s\n", oss.str().c_str());

    // Flush to make sure we get our debug output (useful if we crash
    // afterward).
    std::fflush(stdout);
}

template <typename Car, typename... Cdr>
void debugPrintHelper(std::ostringstream& oss, const char* name, const Car& car, const Cdr&... cdr);

template <typename Car, typename... Cdr>
void debugPrintHelper(std::ostringstream& oss, const char* name, const Car& car, const Cdr&... cdr)
{
    oss << ", " << name << " = " << car;
    debugPrintHelper(oss, cdr...);
}

template <typename Car, typename... Cdr>
void debugPrint(const char* name, const Car& car, const Cdr&... cdr);

template <typename Car, typename... Cdr>
void debugPrint(const char* name, const Car& car, const Cdr&... cdr)
{
    std::ostringstream oss;

    oss << name << " = " << car;
    debugPrintHelper(oss, cdr...);
}
} // namespace detail
// MoonRay added end: *****

/* debug printing macros */
#define STRING(x) #x
#define TOSTRING(x) STRING(x)
#define PING std::cout << __FILE__ << " (" << __LINE__ << "): " << __FUNCTION__ << std::endl
// MoonRay begin: *****
#define PRINT(x)        ::detail::debugPrint(STRING(x), x)
#define PRINT2(x,y)     ::detail::debugPrint(STRING(x), x, STRING(y), y)
#define PRINT3(x,y,z)   ::detail::debugPrint(STRING(x), x, STRING(y), y, STRING(z), z)
#define PRINT4(x,y,z,w) ::detail::debugPrint(STRING(x), x, STRING(y), y, STRING(z), z, STRING(w), w)

#define DBG_PRINT(x) ::detail::debugPrint(STRING(x), x)
// MoonRay end: *****
// Intel begin: *****
/*
#define PRINT(x) std::cout << STRING(x) << " = " << (x) << std::endl
#define PRINT2(x,y) std::cout << STRING(x) << " = " << (x) << ", " << STRING(y) << " = " << (y) << std::endl
#define PRINT3(x,y,z) std::cout << STRING(x) << " = " << (x) << ", " << STRING(y) << " = " << (y) << ", " << STRING(z) << " = " << (z) << std::endl
#define PRINT4(x,y,z,w) std::cout << STRING(x) << " = " << (x) << ", " << STRING(y) << " = " << (y) << ", " << STRING(z) << " = " << (z) << ", " << STRING(w) << " = " << (w) << std::endl

#define DBG_PRINT(x) std::cout << STRING(x) << " = " << (x) << std::endl
*/
// Intel end: *****
#define FATAL(x) { std::cout << "FATAL error in " << __FUNCTION__ << " : " << x << std::endl << std::flush; exit(0); }

/* forces linking unused compilation units from static library */
#define FORCE_LINK_THIS(unit) \
  bool unit##_force_link_me = true;

#define FORCE_LINK_THAT(unit) \
  extern bool unit##_force_link_me; bool unit##_force_link = unit##_force_link_me;

////////////////////////////////////////////////////////////////////////////////
/// Basic Types
////////////////////////////////////////////////////////////////////////////////

// MoonRay: begin *****
typedef std::int64_t  int64;
typedef std::uint64_t uint64;
typedef std::int32_t  int32;
typedef std::uint32_t uint32;
typedef std::int16_t  int16;
typedef std::uint16_t uint16;
typedef std::int8_t   int8;
typedef std::uint8_t  uint8;
// MoonRay: end *****
// Intel: begin *****
/*
typedef          long long  int64;
typedef unsigned long long uint64;
typedef                int  int32;
typedef unsigned       int uint32;
typedef              short  int16;
typedef unsigned     short uint16;
typedef               char   int8;
typedef unsigned      char  uint8;
*/
// Intel: end *****

#ifdef __WIN32__
#if defined(__X86_64__)
typedef int64 ssize_t;
#else
typedef int32 ssize_t;
#endif
#endif

////////////////////////////////////////////////////////////////////////////////
/// Disable some compiler warnings
////////////////////////////////////////////////////////////////////////////////

#if defined(__INTEL_COMPILER)
#pragma warning(disable:265 ) // floating-point operation result is out of range
#pragma warning(disable:383 ) // value copied to temporary, reference to temporary used
#pragma warning(disable:869 ) // parameter was never referenced
#pragma warning(disable:981 ) // operands are evaluated in unspecified order
#pragma warning(disable:1418) // external function definition with no prior declaration
#pragma warning(disable:1419) // external declaration in primary source file
#pragma warning(disable:1572) // floating-point equality and inequality comparisons are unreliable
#pragma warning(disable:94  ) // the size of an array must be greater than zero
#pragma warning(disable:1599) // declaration hides parameter
#pragma warning(disable:424 ) // extra ";" ignored
#endif

#if defined(_MSC_VER)
#pragma warning(disable:4200) // nonstandard extension used : zero-sized array in struct/union
#pragma warning(disable:4800) // forcing value to bool 'true' or 'false' (performance warning)
#pragma warning(disable:4267) // '=' : conversion from 'size_t' to 'unsigned long', possible loss of data
#pragma warning(disable:4244) // 'argument' : conversion from 'ssize_t' to 'unsigned int', possible loss of data
#pragma warning(disable:4355) // 'this' : used in base member initializer list
#pragma warning(disable:4996) // 'std::copy': Function call with parameters that may be unsafe 
#pragma warning(disable:391 ) // '<=' : signed / unsigned mismatch
#pragma warning(disable:4018) // '<' : signed / unsigned mismatch

#endif

////////////////////////////////////////////////////////////////////////////////
// Shared config
// Intel: Default Includes and Functions
////////////////////////////////////////////////////////////////////////////////

// Intel: #include "sys/constants.h"

// MoonRay: begin added *****

#include "Platform.hh"

#define SCENE_RDL2_SIMD_ALIGN          __align(SIMD_MEMORY_ALIGNMENT)

////////////////////////////////////////////////////////////////////////////////
// Default Includes and Functions
////////////////////////////////////////////////////////////////////////////////

#include "DebugLog.h"

#include <scene_rdl2/render/logging/logging.h>

/// Use MNRY_ASSERT for assertions you want compiled out of opt builds.
#ifdef DEBUG
#define MNRY_ASSERT(a, ...)                                           \
    if (!(a)) {                                                       \
        scene_rdl2::logging::Logger::fatal(__FILE__,                  \
                                           ":",                       \
                                           __LINE__,                  \
                                           ":",                       \
                                           __func__,                  \
                                           "() Assertion `",          \
                                           #a,                        \
                                           "' failed.  ",             \
                                           std::string{__VA_ARGS__}); \
        std::abort();                                                 \
    }

/// Code within this macro will only get executed when asserts are active
#define MNRY_DURING_ASSERTS(exp) exp
#else
#define MNRY_ASSERT(a, ...) static_cast<void>(0)
#define MNRY_DURING_ASSERTS(exp)
#endif

/// Use MNRY_ASSERT_REQUIRE for assertions you want active in both debug and opt builds
#define MNRY_ASSERT_REQUIRE(a, ...)                                   \
    if (!(a)) {                                                       \
        scene_rdl2::logging::Logger::fatal(__FILE__,                  \
                                           ":",                       \
                                           __LINE__,                  \
                                           ":",                       \
                                           __func__,                  \
                                           "() Assertion `",          \
                                           #a,                        \
                                           "' failed.  ",             \
                                           std::string{__VA_ARGS__}); \
        std::abort();                                                 \
    }

// Additional assertion helpers. Move these into dwa/Assert.h at some point
// in the future if they become popular.

// Wrapper around C++0x static_assert.
#define MNRY_STATIC_ASSERT(exp)  static_assert(exp, #exp)

// VERIFY macro. The expression is always evaluated in both opt and debug but
// if the expression evaluates to false then we assert when we are in debug.
#ifdef DEBUG
    namespace dwa
    {
        template<typename T>
        inline T Verify(T exp, const char *expStr, const char *file, int line, const char *func)
        {
            if (!exp) {
                // log the error
                scene_rdl2::logging::Logger::error(file,
                                                   ":",
                                                   line,
                                                   ":",
                                                   func,
                                                   "() Assertion `",
                                                   expStr,
                                                   "' failed");
                debugbreak();
            }
            return exp;
        }
    }
    #define MNRY_VERIFY(exp)             dwa::Verify(exp, #exp, __FILE__, __LINE__, __func__)
#else
    #define MNRY_VERIFY(exp)             (exp)
#endif

// MoonRay: end added *****

namespace scene_rdl2 {
namespace util {
// Intel: namespace embree {

// MoonRay: added scene_rdl2::util namespace  
#define ALIGNED_STRUCT          \
  void* operator new(size_t size) { return scene_rdl2::util::alignedMalloc(size); }  \
  void operator delete(void* ptr) { scene_rdl2::util::alignedFree(ptr); }              \
  void* operator new[](size_t size) { return scene_rdl2::util::alignedMalloc(size); }  \
  void operator delete[](void* ptr) { scene_rdl2::util::alignedFree(ptr); }

// MoonRay: added scene_rdl2::util namespace
#define ALIGNED_STRUCT_(align)                                           \
  void* operator new(size_t size) { return scene_rdl2::util::alignedMalloc(size,align); } \
  void operator delete(void* ptr) { scene_rdl2::util::alignedFree(ptr); }                 \
  void* operator new[](size_t size) { return scene_rdl2::util::alignedMalloc(size,align); } \
  void operator delete[](void* ptr) { scene_rdl2::util::alignedFree(ptr); }

#define ALIGNED_CLASS                                                \
  public:                                                            \
    ALIGNED_STRUCT                                                  \
  private:

#define ALIGNED_CLASS_(align)                                           \
 public:                                                               \
    ALIGNED_STRUCT_(align)                                              \
 private:
  
// MoonRay: begin *****

// Add this to a class to forbid copying it or assigning it.
#define DISALLOW_COPY_OR_ASSIGNMENT(type)   \
    type(const type &) = delete;            \
    type& operator = (const type &) = delete

  /*! aligned allocation */
  inline void* alignedMalloc(size_t size, size_t align = CACHE_LINE_SIZE)
  {
      // Alignment must be a multiple of sizeof(void*).
      // Since alignment must also be a power of two (checked below), everything
      // greater to or equal to sizeof(void*) is automatically a multiple of
      // sizeof(void*). E.g. if sizeof(void*) == (2*2*2), every following power
      // of two is a multiple of that number: 2*(2*2*2), 2*2*(2*2*2), ...
      MNRY_ASSERT(align >= sizeof(void*));

      void* memptr = nullptr;
#if defined(DEBUG)
      const int error =
#endif
      posix_memalign(&memptr, align, size);
#if defined(DEBUG)
      MNRY_ASSERT(error != EINVAL, "Alignment is not a power of two");
      MNRY_ASSERT(error != ENOMEM, "Insufficient memory");
      MNRY_ASSERT(error == 0, "Some unexpected error");
#endif
      return memptr;
  }

  inline void alignedFree(const void* ptr)      { free(const_cast<void *>(ptr)); }

// MoonRay: end *****

// Intel:   /*! aligned allocation */
// Intel:     void* alignedMalloc(size_t size, size_t align = 64);
// Intel:     void alignedFree(const void* ptr);

  /*! allocates pages directly from OS */
  void* os_malloc (size_t bytes);
  void* os_reserve(size_t bytes);
  void  os_commit (void* ptr, size_t bytes);
  void  os_shrink (void* ptr, size_t bytesNew, size_t bytesOld);
  void  os_free   (void* ptr, size_t bytes);
  void* os_realloc(void* ptr, size_t bytesNew, size_t bytesOld);

  /*! returns performance counter in seconds */
  double getSeconds();

// MoonRay: begin added *****

  /*! Constants */
  static struct NullTy {} null MAYBE_UNUSED;

  static struct TrueTy {
    __forceinline operator bool( ) const { return true; }
  } True MAYBE_UNUSED;

  static struct FalseTy {
    __forceinline operator bool( ) const { return false; }
  } False MAYBE_UNUSED;

  static struct EmptyTy {} empty MAYBE_UNUSED;

  static struct FullTy {} full MAYBE_UNUSED;

// MoonRay: end added *****
// Intel: begin *****
#if defined(__MIC__)
#define isa knc
#elif defined (__AVX2__)
#define isa avx2
#elif defined(__AVXI__)
#define isa avxi
#elif defined(__AVX__)
#define isa avx
#elif defined (__SSE4_2__)
#define isa sse42
#elif defined (__SSE4_1__)
#define isa sse41
#elif defined(__SSSE3__)
#define isa ssse3
#elif defined(__SSE3__)
#define isa sse3
#elif defined(__SSE2__)
#define isa sse2
#elif defined(__SSE__)
#define isa sse
#else 
#error Unknown ISA
#endif
// Intel: end *****

}
}

