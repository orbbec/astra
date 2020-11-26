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
#include "astra_configuration.hpp"
#include "cpptoml.h"
#include <iostream>

namespace astra {

    astra_log_severity_t convert_string_to_severity(std::string s)
    {
        if (s.find("trace") != std::string::npos)
        {
            return ASTRA_SEVERITY_TRACE;
        }
        else if (s.find("warn") != std::string::npos)
        {
            return ASTRA_SEVERITY_WARN;
        }
        else if (s.find("debug") != std::string::npos)
        {
            return ASTRA_SEVERITY_DEBUG;
        }
        else if (s.find("fatal") != std::string::npos)
        {
            return ASTRA_SEVERITY_FATAL;
        }
        else if (s.find("info") != std::string::npos)
        {
            return ASTRA_SEVERITY_INFO;
        }
        else if (s.find("error") != std::string::npos)
        {
            return ASTRA_SEVERITY_ERROR;
        }

        return ASTRA_SEVERITY_UNKNOWN;
    }

    configuration::configuration()
        : pluginsPath_("Plugins")
    {
        set_severityLevel(ASTRA_SEVERITY_INFO);
        set_consoleOutput(false);
        set_fileOutput(false);
        set_fileLogPath("/sm3/vision_dumps/devices_logs/orbbec/orbbec.log");
    }

    configuration* configuration::load_from_file(const char* tomlFilePath)
    {
        configuration* config = new configuration();
        cpptoml::table t;

        try
        {
            t = cpptoml::parse_file(tomlFilePath);
        }
        catch (const cpptoml::parse_exception& e)
        {
            return config;
        }

        const char* loggingLevelKey = "logging.level";
        if (t.contains_qualified(loggingLevelKey))
        {
            auto logLevel = t.get_qualified(loggingLevelKey)->as<std::string>()->get();
            auto severity = convert_string_to_severity(logLevel);

            if (severity != ASTRA_SEVERITY_UNKNOWN)
            {
                config->set_severityLevel(severity);
            }
        }

        const char* outputLogsToConsole = "logging.console_output";
        if (t.contains_qualified(outputLogsToConsole))
        {
            bool consoleOutput = t.get_qualified(outputLogsToConsole)->as<bool>()->get();
            config->set_consoleOutput(consoleOutput);
        }

        const char* outputLogsToFile = "logging.file_output";
        if (t.contains_qualified(outputLogsToFile))
        {
            bool fileOutput = t.get_qualified(outputLogsToFile)->as<bool>()->get();
            config->set_fileOutput(fileOutput);
        }

        const char* pluginsPathKey = "plugins.path";
        if (t.contains_qualified(pluginsPathKey))
        {
            auto pluginsPath = t.get_qualified(pluginsPathKey)->as<std::string>()->get();

            if (pluginsPath.length() > 0)
            {
                config->set_pluginsPath(pluginsPath);
            }
        }

        const char* fileLogsPathKey = "logging.filepath";
        if (t.contains_qualified(fileLogsPathKey))
        {
          auto fileLogsPath = t.get_qualified(fileLogsPathKey)->as<std::string>()->get();

          if (fileLogsPath.length() > 0)
          {
            config->set_fileLogPath(fileLogsPath);
          }
        }

        return config;
    }
}
