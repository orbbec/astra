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
#include "astra_plugin_manager.hpp"
#include "tinydir.h"
#include "astra_cxx_compatibility.hpp"

namespace astra {

    plugin_manager::plugin_manager(streamset_catalog& catalog)
        : pluginService_(astra::make_unique<plugin_service>(catalog)),
          pluginServiceProxy_(pluginService_->proxy())
    {}

    plugin_manager::~plugin_manager()
    {
        unload_all_plugins();
    }

    void plugin_manager::load_plugins(std::string searchPath)
    {
        LOG_TRACE("plugin_manager", "load plugins");
        std::vector<std::string> pluginFiles = find_libraries(searchPath);
        for(auto pluginPath : pluginFiles)
        {
            try_load_plugin(searchPath + pluginPath);
        }
    }

    void plugin_manager::load_plugin(std::string pluginPath)
    {
        try_load_plugin(pluginPath);
    }

    std::vector<std::string> plugin_manager::find_libraries(const std::string& pluginsPath)
    {

        std::vector<std::string> extensions = { "so", "dylib" };

        std::vector<std::string> result;
        tinydir_dir dir;
        if (tinydir_open(&dir, pluginsPath.c_str()) == -1)
        {
            goto bail;
        }

        while (dir.has_next)
        {
            tinydir_file file;
            if (tinydir_readfile(&dir, &file) == -1)
            {
                goto bail;
            }

            if (!file.is_dir)
            {
                for(auto candidate : extensions)
                {
                    std::string extension(file.extension);
                    if (extension.compare(candidate) == 0)
                    {
                        result.push_back(std::string(file.name));
                        break;
                    }
                }
            }

            tinydir_next(&dir);
        }

    bail:
        tinydir_close(&dir);
        return result;
    }

    void plugin_manager::try_load_plugin(const std::string& path)
    {
        process::lib_handle libHandle = nullptr;
        LOG_TRACE("plugin_manager", "try_load_plugin %s", path.c_str());

        process::load_library(path.c_str(), libHandle);

	if (!libHandle)
	  return;

        PluginFuncs pluginFuncs;
        process::get_proc_address(libHandle, ASTRA_STRINGIFY(astra_plugin_initialize), (process::far_proc&)pluginFuncs.initialize);
        process::get_proc_address(libHandle, ASTRA_STRINGIFY(astra_plugin_terminate), (process::far_proc&)pluginFuncs.terminate);
        process::get_proc_address(libHandle, ASTRA_STRINGIFY(astra_plugin_update), (process::far_proc&)pluginFuncs.update);
        pluginFuncs.libHandle = libHandle;

        if (pluginFuncs.is_valid())
        {
            LOG_TRACE("plugin_manager", "try_load_plugin valid plugin");
            pluginFuncs.initialize(pluginServiceProxy_);
            LOG_TRACE("plugin_manager", "try_load_plugin initialized plugin");
            pluginList_.push_back(pluginFuncs);
        }
        else
        {
            LOG_TRACE("plugin_manager", "try_load_plugin invalid lib: init: %p, term: %p, update: %p",
                   pluginFuncs.initialize,
                   pluginFuncs.terminate,
                   pluginFuncs.update);
            std::cout << "pluginFuncs.initialize" << pluginFuncs.initialize << std::endl;
            std::cout << "pluginFuncs.terminate" << pluginFuncs.terminate << std::endl;
            std::cout << "pluginFuncs.update" << pluginFuncs.update << std::endl;
            process::free_library(libHandle);
        }
    }

    void plugin_manager::update()
    {
        for(auto plinfo : pluginList_)
        {
            if (plinfo.update)
            {
                plinfo.update();
            }
        }
    }

    void plugin_manager::unload_all_plugins()
    {
        for(auto pluginFuncs : pluginList_)
        {
            pluginFuncs.terminate();
            process::free_library(pluginFuncs.libHandle);
        }

        pluginList_.clear();
    }

    void plugin_manager::notify_host_event(astra_event_id id, const void* data, size_t dataSize)
    {
        pluginService_->notify_host_event(id, data, dataSize);
    }
}
