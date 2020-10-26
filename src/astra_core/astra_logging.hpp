// This file is part of the Orbbec Astra SDK [https://orbbec3d.com]
// Copyright (c) 2015 Orbbec 3D
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Be excellent to each other.
#ifndef ASTRA_LOGGING_H
#define ASTRA_LOGGING_H

// TODO valgrind will go bananas with the default crash handler enabled
// #define ELPP_DISABLE_DEFAULT_CRASH_HANDLING
#define ELPP_NO_DEFAULT_LOG_FILE 1

// enable stacktraces for GCC/Clang on *nixes
#if ! defined(__ANDROID__) && ! defined(_MSC_VER)
#define ELPP_STACKTRACE_ON_CRASH
#endif

#if defined(__apple_build_version__)
#define ASTRA_APPLE_CLANG 1
#elif defined(__clang__)
#define ASTRA_APPLE_CLANG 0
#endif

#ifdef __clang__
#define ASTRA_CLANG 1
#ifdef __apple_build_version__
#define ASTRA_APPLE_CLANG 1
#define ASTRA_CLANG_VERSION __apple_build_version__
#else
#define ASTRA_APPLE_CLANG 0
#define ASTRA_CLANG_VERSION (__clang_major__ * 1000000) + (__clang_minor__ * 10000) + __clang_patchlevel__
#endif
#else
#define ASTRA_CLANG 0
#define ASTRA_APPLE_CLANG 0
#endif


#ifdef __clang__
#if (ASTRA_APPLE_CLANG && ASTRA_CLANG_VERSION >= 7030029)
#define ASTRA_PESSIMIZING_MOVE 1
#elif (!ASTRA_APPLE_CLANG && ASTRA_CLANG_VERSION >= 3070000)
#define ASTRA_PESSIMIZING_MOVE 1
#else
#define ASTRA_PESSIMIZING_MOVE 0
#endif

#if ASTRA_PESSIMIZING_MOVE
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpessimizing-move"
#endif //ASTRA_PESSIMIZING_MOVE
#endif //__clang__
#ifdef __clang__
#if ASTRA_PESSIMIZING_MOVE
#pragma clang diagnostic pop
#undef ASTRA_PESSIMIZING_MOVE
#endif //ASTRA_PESSIMIZING_MOVE
#endif //__clang__

#include <astra_core/capi/astra_types.h>


namespace astra {
    void initialize_logging(const char* logFilePath, astra_log_severity_t severity, bool consoleOutput, bool fileOutput);
}

#endif /* ASTRA_LOGGING_H */
