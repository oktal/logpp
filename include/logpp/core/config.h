#pragma once

#include "logpp/core/common.h"

#if defined(_MSC_VER)
    #define LOGPP_COMPILER_MSVC
#elif defined(__GNUC__)
    #define LOGPP_COMPILER_GCC
#elif defined(__clang__)
    #define LOGPP_COMPILER_CLANG
#else
    #error Unknown compiler !
#endif

#if defined(__linux__)
  #if defined(__ANDROID__)
    #define LOGPP_PLATFORM_ANDROID
  #else
    #define LOGPP_PLATFORM_LINUX
  #endif
#elif defined(__APPLE__)
    #define LOGPP_PLATFORM_DARWIN
#elif defined(_WIN32) || defined(_WIN64)
    #define LOGPP_PLATFORM_WINDOWS
#endif