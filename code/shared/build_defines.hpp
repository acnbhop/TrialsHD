//
// build_defines.hpp: Build configuration defines.
//

#pragma once

//
// Strip away all development code / development related features for release builds.
//
#if defined(__RELEASE_NO_DEV)
#define BUILD_RELEASE_NO_DEV 1
#else
#define BUILD_RELEASE_NO_DEV 0
#endif

//
// Standard debug build.
//
#if defined(__DEBUG)
#define BUILD_DEBUG 1
#else
#define BUILD_DEBUG 0
#endif

//
// Release build, contains development info.
//
#if defined(__RELEASE)
#define BUILD_RELEASE 1
#else
#define BUILD_RELEASE 0
#endif

//
// Profiling build.
//
#if defined(__PROFILE)
#define BUILD_PROFILE 1
#else
#define BUILD_PROFILE 0
#endif

//
// Bank build contains extra memory on consoles and also contains development tools whilest retaining
// the most amount of performance possible.
//
#if defined(__BANK)
#define BUILD_BANK 1
#else
#define BUILD_BANK 0
#endif

//
// Define BUILD_DEV.
//
#if !defined(__DEV)
#if BUILD_BANK || BUILD_RELEASE || BUILD_PROFILE || BUILD_DEBUG
#define BUILD_DEV 1
#else
#define BUILD_DEV 0
#endif
#else
#define BUILD_DEV 1
#endif

//
// Zero out all the build defines if they aren't defined by the build system.
//
#if !defined(__RELEASE_NO_DEV)
#define __RELEASE_NO_DEV 0
#endif

#if !defined(__DEBUG)
#define __DEBUG 0
#endif

#if !defined(__RELEASE)
#define __RELEASE 0
#endif

#if !defined(__PROFILE)
#define __PROFILE 0
#endif

#if !defined(__BANK)
#define __BANK 0
#endif

#if !defined(__DEV)
#if BUILD_DEV
#define __DEV 1
#else
#define __DEV 0
#endif
#endif

#ifdef __clang__
#define REDLYNX_CLANG 1
#ifdef __apple_build_version__
#define REDLYNX_APPLE_CLANG 1
#else
#define REDLYNX_APPLE_CLANG 0
#endif
#ifdef _MSC_VER
#define REDLYNX_CLANG_CL 1
#else
#define REDLYNX_CLANG_CL 0
#endif
#else
#define REDLYNX_CLANG 0
#define REDLYNX_APPLE_CLANG 0
#define REDLYNX_CLANG_CL 0
#endif

#if !defined(REDLYNX_CLANG) && defined(__GNUC__)
#define REDLYNX_GCC 1
#else
#define REDLYNX_GCC 0
#endif

#if !defined(REDLYNX_CLANG) && !defined(REDLYNX_GCC) && defined(_MSC_VER)
#define REDLYNX_MSVC 1
#else
#define REDLYNX_MSVC 0
#endif

#if defined(__x86_64__) || defined(__amd64__) || defined(_M_X64) || defined(_M_AMD64) || \
    defined(__i386__) || defined(__i686__) || defined(_M_IX86)
#define REDLYNX_X86 1
#if defined(__x86_64__) || defined(__amd64__) || defined(_M_X64) || defined(_M_AMD64)
#define REDLYNX_X86_64 1
#define REDLYNX_AMD64 1
#else
#define REDLYNX_X86_64 0
#define REDLYNX_AMD64 0
#endif
#if !REDLYNX_X86_64 && (defined(__i386__) || defined(__i686__) || defined(_M_IX86))
#define REDLYNX_X86_32 1
#else
#define REDLYNX_X86_32 0
#endif
#else
#define REDLYNX_X86 0
#define REDLYNX_X86_64 0
#define REDLYNX_AMD64 0
#define REDLYNX_X86_32 0
#endif

#if defined(__aarch64__) || defined(__arm64__) || defined(_M_ARM64) || \
    defined(__arm__) || defined(_M_ARM)
#define REDLYNX_ARM 1
#if defined(__aarch64__) || defined(__arm64__) || defined(_M_ARM64)
#define REDLYNX_ARM64 1
#define REDLYNX_AARCH64 1
#else
#define REDLYNX_ARM64 0
#define REDLYNX_AARCH64 0
#endif
#if !REDLYNX_ARM64 && (defined(__arm__) || defined(_M_ARM))
#define REDLYNX_ARM32 1
#else
#define REDLYNX_ARM32 0
#endif
#else
#define REDLYNX_ARM 0
#define REDLYNX_ARM64 0
#define REDLYNX_AARCH64 0
#define REDLYNX_ARM32 0
#endif

#if defined(_WIN32) || defined(_WIN64)
#define REDLYNX_WINDOWS 1
#if defined(_WIN64)
#define REDLYNX_WIN64 1
#else
#define REDLYNX_WIN64 0
#endif
#if !REDLYNX_WIN64 && defined(_WIN32)
#define REDLYNX_WIN32 1
#else
#define REDLYNX_WIN32 0
#endif
#else
#define REDLYNX_WINDOWS 0
#define REDLYNX_WIN64 0
#define REDLYNX_WIN32 0
#endif

#ifdef __linux__
#define REDLYNX_LINUX 1
#else
#define REDLYNX_LINUX 0
#endif

#ifdef __APPLE__
#define REDLYNX_APPLE 1
#ifdef __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__
#define REDLYNX_MACOS 1
#define REDLYNX_OSX 1
#else
#define REDLYNX_MACOS 0
#define REDLYNX_OSX 0
#endif
#ifdef __ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__
#define REDLYNX_IOS 1
#else
#define REDLYNX_IOS 0
#endif
#else
#define REDLYNX_APPLE 0
#define REDLYNX_MACOS 0
#define REDLYNX_OSX 0
#define REDLYNX_IOS 0
#endif

#if REDLYNX_LINUX || REDLYNX_APPLE
#define REDLYNX_POSIX 1
#else
#define REDLYNX_POSIX 0
#endif

#define REDLYNX_NAMESPACE_BEGIN     namespace redlynx {
#define REDLYNX_NAMESPACE_END       }

#define REDLYNX_NAMESPACE_BEGIN_TOOLS  namespace redlynx { namespace tools {
#define REDLYNX_NAMESPACE_END_TOOLS    } }

#define REDLYNX_NAMESPACE_BEGIN_ENGINE namespace redlynx { namespace engine {
#define REDLYNX_NAMESPACE_END_ENGINE   } }

#define REDLYNX_NAMESPACE_BEGIN_ENGINE_ASSET namespace redlynx { namespace engine { namespace asset {
#define REDLYNX_NAMESPACE_END_ENGINE_ASSET   } } }

#define REDLYNX_NAMESPACE_BEGIN_GAME   namespace redlynx { namespace game {
#define REDLYNX_NAMESPACE_END_GAME     } }

REDLYNX_NAMESPACE_BEGIN

typedef float   f32;
typedef double  f64;

#if defined(__INT8_TYPE__)
typedef __INT8_TYPE__ int8;
#else
typedef signed char int8;
#endif

#if defined(__UINT8_TYPE__)
typedef __UINT8_TYPE__ uint8;
#else
typedef unsigned char uint8;
#endif

#if defined(__INT16_TYPE__)
typedef __INT16_TYPE__ int16;
#else
typedef signed short int16;
#endif

#if defined(__UINT16_TYPE__)
typedef __UINT16_TYPE__ uint16;
#else
typedef unsigned short uint16;
#endif

#if defined(__INT32_TYPE__)
typedef __INT32_TYPE__ int32;
#else
typedef signed int int32;
#endif

#if defined(__UINT32_TYPE__)
typedef __UINT32_TYPE__ uint32;
#else
typedef unsigned int uint32;
#endif

#if defined(__INT64_TYPE__)
typedef __INT64_TYPE__ int64;
#else
#if defined(_MSC_VER)
typedef signed __int64 int64;
#else
#if defined(_LP64) || defined(__LP64__)
typedef signed long int64;
#else
typedef signed long long int64;
#endif
#endif
#endif

#if defined(__UINT64_TYPE__)
typedef __UINT64_TYPE__ uint64;
#else
#if defined(_MSC_VER)
typedef unsigned __int64 uint64;
#else
#if defined(_LP64) || defined(__LP64__)
typedef unsigned long uint64;
#else
typedef unsigned long long uint64;
#endif
#endif
#endif

#if defined(__INTMAX_TYPE__)
typedef __INTMAX_TYPE__ intmax;
#else
#if defined(_MSC_VER)
typedef signed __int64 intmax;
#else
#if defined(_LP64) || defined(__LP64__)
typedef signed long intmax;
#else
typedef signed long long intmax;
#endif
#endif
#endif

#if defined(__UINTMAX_TYPE__)
typedef __UINTMAX_TYPE__ uintmax;
#else
#if defined(_MSC_VER)
typedef unsigned __int64 uintmax;
#else
#if defined(_LP64) || defined(__LP64__)
typedef unsigned long uintmax;
#else
typedef unsigned long long uintmax;
#endif
#endif
#endif

#if defined(__SIZE_TYPE__)
typedef __SIZE_TYPE__ size;
#else
#if defined(_MSC_VER)
typedef unsigned __int64 size;
#else
#if defined(_LP64) || defined(__LP64__)
typedef unsigned long size;
#else
typedef unsigned long long size;
#endif
#endif
#endif

#if defined(__PTRDIFF_TYPE__)
typedef __PTRDIFF_TYPE__ ptrdiff;
#else
#if defined(_MSC_VER)
typedef signed __int64 ptrdiff;
#else
#if defined(_LP64) || defined(__LP64__)
typedef signed long ptrdiff;
#else
typedef signed long long ptrdiff;
#endif
#endif
#endif

REDLYNX_NAMESPACE_END
