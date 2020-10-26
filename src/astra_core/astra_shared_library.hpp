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
#ifndef ASTRA_SHARED_LIBRARY_H
#define ASTRA_SHARED_LIBRARY_H

#define ASTRA_FILE_MAX_PATH 256

#define ASTRA_STRINGIFY(n) ASTRA_STRINGIFY_HELPER(n)
#define ASTRA_STRINGIFY_HELPER(n) #n

namespace astra { namespace process {

    using lib_handle = void*;
    using far_proc = void(*)(void*);

    void load_library(const char* fileName, lib_handle& libHandle);
    void free_library(const lib_handle libHandle);
    void get_proc_address(const lib_handle libHandle, const char* procName, far_proc& procAddr);

}}

#endif /* ASTRA_SHARED_LIBRARY_H */
