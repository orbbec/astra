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
#ifndef ASTRA_CXX_MAKE_UNIQUE_H
#define ASTRA_CXX_MAKE_UNIQUE_H

// Implementation of C++14's make_unique for C++11 compilers.
//
// This has been tested with:
// - MSVC 11.0 (Visual Studio 2012)
// - gcc 4.6.3
// - Xcode 4.4 (with clang "4.0")
//
// It is based off an implementation proposed by Stephan T. Lavavej for
// inclusion in the C++14 standard:
//    http://isocpp.org/files/papers/N3656.txt
// Where appropriate, it borrows the use of MSVC's _VARIADIC_EXPAND_0X macro
// machinery to compensate for lack of variadic templates.
//
// This file injects make_unique into the std namespace, which I acknowledge is
// technically forbidden ([C++11: 17.6.4.2.2.1/1]), but is necessary in order
// to have syntax compatibility with C++14.
//
// I perform compiler version checking for MSVC, gcc, and clang to ensure that
// we don't add make_unique if it is already there (instead, we include
// <memory> to get the compiler-provided one). You can override the compiler
// version checking by defining the symbol COMPILER_SUPPORTS_MAKE_UNIQUE.
//
//
// ===============================================================================
// This file is released into the public domain. See LICENCE for more information.
// ===============================================================================

// If the compiler supports std::make_unique, then pull in <memory> to get it.
#include <memory>
#include <utility>

namespace astra {

    template<typename T, typename... Args>
    auto make_unique(Args&&... args) -> std::unique_ptr<T>
    {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }
}

#endif /* ASTRA_CXX_MAKE_UNIQUE_H */
