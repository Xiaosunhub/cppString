// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Ender_
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef CPPSTRING_MACRO_H_
#define CPPSTRING_MACRO_H_

// This module is header-only: every facility below is a macro, so there is no
// macro.cpp and nothing to link beyond the rest of cppString.
#include "cppString.h"

// ===========================================================================
// 1. Token primitives: two-stage stringizing and concatenation
// ===========================================================================

// CPPSTRING_STR keeps the spelling of its argument; CPPSTRING_XSTR first
// expands macros. e.g. #define N 7 -> STR(N) == "N", XSTR(N) == "7".
#define CPPSTRING_STR(x)                 #x
#define CPPSTRING_XSTR(x)                CPPSTRING_STR(x)

// CPPSTRING_CAT pastes raw tokens; CPPSTRING_XCAT expands both arguments first.
#define CPPSTRING_CAT(a, b)              a##b
#define CPPSTRING_XCAT(a, b)             CPPSTRING_CAT(a, b)

// Three-token variants. A direct a##b##c paste avoids the ## suppression you
// get from nesting two-argument pastes; the X form expands via one indirection.
#define CPPSTRING_CAT3(a, b, c)          a##b##c
#define CPPSTRING_XCAT3_I(a, b, c)       a##b##c
#define CPPSTRING_XCAT3(a, b, c)         CPPSTRING_XCAT3_I(a, b, c)

// Expands to nothing; useful for conditionally empty macro arguments.
#define CPPSTRING_EMPTY()

// ===========================================================================
// 2. Version information and compile-time gates
// ===========================================================================

// A single comparable integer: 6.0.0 -> 60000, 5.7.0 -> 50700.
#define CPPSTRING_VERSION_CODE                                                \
    (CPPSTRING_VERSION_MAJOR * 10000 + CPPSTRING_VERSION_MINOR * 100 +        \
     CPPSTRING_VERSION_PATCH)

// Integer constant expressions; 1 / 0, usable in #if and static_assert.
#define CPPSTRING_VERSION_AT_LEAST(major, minor, patch)                       \
    (CPPSTRING_VERSION_CODE >=                                                \
     ((major) * 10000 + (minor) * 100 + (patch)))
#define CPPSTRING_VERSION_LESS_THAN(major, minor, patch)                      \
    (CPPSTRING_VERSION_CODE <                                                 \
     ((major) * 10000 + (minor) * 100 + (patch)))
#define CPPSTRING_VERSION_EQ(major, minor, patch)                             \
    (CPPSTRING_VERSION_CODE ==                                                \
     ((major) * 10000 + (minor) * 100 + (patch)))

// ===========================================================================
// 3. Compiler, platform and build detection (1 / 0 integer constants)
// ===========================================================================

#if defined(__clang__)
#   define CPPSTRING_IS_CLANG 1
#else
#   define CPPSTRING_IS_CLANG 0
#endif
#if defined(_MSC_VER) && !defined(__clang__)
#   define CPPSTRING_IS_MSVC 1
#else
#   define CPPSTRING_IS_MSVC 0
#endif
#if defined(__GNUC__) && !defined(__clang__)
#   define CPPSTRING_IS_GCC 1
#else
#   define CPPSTRING_IS_GCC 0
#endif

#if defined(_WIN32) || defined(_WIN64)
#   define CPPSTRING_IS_WINDOWS 1
#else
#   define CPPSTRING_IS_WINDOWS 0
#endif
#if defined(__linux__)
#   define CPPSTRING_IS_LINUX 1
#else
#   define CPPSTRING_IS_LINUX 0
#endif
#if defined(__APPLE__)
#   define CPPSTRING_IS_MACOS 1
#else
#   define CPPSTRING_IS_MACOS 0
#endif

#if defined(NDEBUG)
#   define CPPSTRING_IS_RELEASE 1
#   define CPPSTRING_IS_DEBUG  0
#else
#   define CPPSTRING_IS_RELEASE 0
#   define CPPSTRING_IS_DEBUG  1
#endif

// The language standard the compiler claims, e.g. 202302L for C++23.
// MSVC keeps __cplusplus at 199711L unless /Zc:__cplusplus is passed; its
// _MSVC_LANG macro reports the real value, so prefer that when available.
#if defined(_MSVC_LANG)
#   define CPPSTRING_CPP_STANDARD _MSVC_LANG
#else
#   define CPPSTRING_CPP_STANDARD __cplusplus
#endif

// ===========================================================================
// 4. Portable attributes and qualifiers
// ===========================================================================

#if defined(__has_cpp_attribute)
#   if __has_cpp_attribute(nodiscard) > 0
#       define CPPSTRING_NODISCARD [[nodiscard]]
#   endif
#endif
#ifndef CPPSTRING_NODISCARD
#   define CPPSTRING_NODISCARD
#endif

#if defined(__has_cpp_attribute)
#   if __has_cpp_attribute(maybe_unused) > 0
#       define CPPSTRING_MAYBE_UNUSED [[maybe_unused]]
#   endif
#endif
#ifndef CPPSTRING_MAYBE_UNUSED
#   define CPPSTRING_MAYBE_UNUSED
#endif

#if defined(__has_cpp_attribute)
#   if __has_cpp_attribute(deprecated) > 0
#       define CPPSTRING_DEPRECATED(msg) [[deprecated(msg)]]
#   endif
#endif
#if !defined(CPPSTRING_DEPRECATED) && defined(_MSC_VER)
#   define CPPSTRING_DEPRECATED(msg) __declspec(deprecated(msg))
#endif
#ifndef CPPSTRING_DEPRECATED
#   define CPPSTRING_DEPRECATED(msg)
#endif

#if defined(__has_cpp_attribute)
#   if __has_cpp_attribute(fallthrough) > 0
#       define CPPSTRING_FALLTHROUGH [[fallthrough]]
#   endif
#endif
#ifndef CPPSTRING_FALLTHROUGH
#   define CPPSTRING_FALLTHROUGH
#endif

// Place after an if/else label: `if (x) CPPSTRING_LIKELY { ... }`.
#if defined(__has_cpp_attribute)
#   if __has_cpp_attribute(likely) > 0
#       define CPPSTRING_LIKELY   [[likely]]
#       define CPPSTRING_UNLIKELY [[unlikely]]
#   endif
#endif
#ifndef CPPSTRING_LIKELY
#   define CPPSTRING_LIKELY
#   define CPPSTRING_UNLIKELY
#endif

#if defined(_MSC_VER) && !defined(__clang__)
#   define CPPSTRING_FORCEINLINE __forceinline
#   define CPPSTRING_NOINLINE    __declspec(noinline)
#elif defined(__GNUC__) || defined(__clang__)
#   define CPPSTRING_FORCEINLINE inline __attribute__((always_inline))
#   define CPPSTRING_NOINLINE    __attribute__((noinline))
#else
#   define CPPSTRING_FORCEINLINE inline
#   define CPPSTRING_NOINLINE
#endif

// Silence an unused variable / parameter in a portable way.
#define CPPSTRING_UNUSED(x) ((void)(x))

// ===========================================================================
// 5. Source location diagnostics
// ===========================================================================

// __func__ is standard C++11 and works on every supported toolchain. Note it
// is a local variable, not a literal, so HERE builds a std::string at runtime.
#define CPPSTRING_FUNC       __func__
#define CPPSTRING_LINE_STR   CPPSTRING_XSTR(__LINE__)
#define CPPSTRING_FILE_LINE  __FILE__ ":" CPPSTRING_LINE_STR
#define CPPSTRING_HERE       (std::string(CPPSTRING_FUNC) + " at " + CPPSTRING_FILE_LINE)

// "name=value" for quick tracing: CPPSTRING_VARSTR(n) -> "n=7" when n is 7.
#define CPPSTRING_VARSTR(x)  #x "=" CPPSTRING_XSTR(x)

// ===========================================================================
// 6. Expression helpers and unchecked assertions
// ===========================================================================

#define CPPSTRING_COUNT_OF(arr) (sizeof(arr) / sizeof((arr)[0]))

#define CPPSTRING_STATIC_ASSERT(cond, msg) static_assert((cond), msg)

// Unlike assert(), this is active in release builds. Throws the library's own
// Invalid_Operation, so it participates in the normal cppString error model.
#define CPPSTRING_ASSERT_ALWAYS(cond)                                         \
    do {                                                                      \
        if (!(cond))                                                          \
        {                                                                     \
            throw cppstr::Invalid_Operation(                                      \
                "assertion failed: " #cond " at " __FILE__ ":"                \
                CPPSTRING_XSTR(__LINE__));                                    \
        }                                                                     \
    } while (false)

// Construct a cppString from a string literal with one token.
#define CPPSTRING_LIT(text) cppstr::cppString{text}

#endif // CPPSTRING_MACRO_H_
