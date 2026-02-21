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

#if defined(REDLYNX_LINUX) || defined(REDLYNX_APPLE)
#define REDLYNX_POSIX 1
#else
#define REDLYNX_POSIX 0
#endif

#define REDLYNX_NAMESPACE_BEGIN     namespace redlynx {
#define REDLYNX_NAMESPACE_END       }

#define REDLYNX_NAMESPACE_BEGIN_TOOLS  namespace redlynx { namespace tools {
#define REDLYNX_NAMESPACE_END_TOOLS    } }

#define REDLYNX_NAMESPACE_BEGIN_GAME   namespace redlynx { namespace game {
#define REDLYNX_NAMESPACE_END_GAME     } }
