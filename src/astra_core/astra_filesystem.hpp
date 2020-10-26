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
#ifndef ASTRA_FILESYSTEM_H
#define ASTRA_FILESYSTEM_H

#include <string>
#include "astra_platform.hpp"

#include "unix/astra_filesystem_unix.hpp"

namespace astra { namespace filesystem {

    bool ends_with_path_separator(const std::string& path);
    std::string trim_path_separator(const std::string& path);
    std::string append_path_separator(const std::string& path);
    std::string directory_name(const std::string& filePath);
    std::string combine_paths(const std::string& left, const std::string& right);
}}

#endif /* ASTRA_FILESYSTEM_H */
