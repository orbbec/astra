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
#ifndef ASTRA_CONFIGURATION_H
#define ASTRA_CONFIGURATION_H

#include <astra_core/capi/astra_types.h>
#include <string>

namespace astra {

    class configuration
    {
    public:
        configuration();

        astra_log_severity_t severityLevel() { return severityLevel_; }
        void set_severityLevel(astra_log_severity_t level) { severityLevel_ = level; }

        const std::string& pluginsPath() const { return pluginsPath_; }
        void set_pluginsPath(std::string pluginsPath) { pluginsPath_ = pluginsPath; }

        const std::string& fileLogPath() const { return fileLogPath_; }
        void set_fileLogPath(std::string fileLogPath) { fileLogPath_ = fileLogPath; }

        static configuration* load_from_file(const char* tomlFilePath);

        bool consoleOutput(){ return consoleOutput_; }
        void set_consoleOutput(bool consoleOutput){ consoleOutput_ = consoleOutput; }

        bool fileOutput(){ return fileOutput_; }
        void set_fileOutput(bool fileOutput){ fileOutput_ = fileOutput; }

    private:
        astra_log_severity_t severityLevel_{ASTRA_SEVERITY_FATAL};
        std::string pluginsPath_;
        std::string fileLogPath_;
        bool consoleOutput_;
        bool fileOutput_;
    };
}

#endif /* ASTRA_CONFIGURATION_H */
