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
#include <astra_core/capi/astra_types.h>
#include "astra_stream.hpp"
#include "astra_stream_bin.hpp"

#include <cstdint>
#include <cassert>
#include <algorithm>
#include "astra_stream_listener.hpp"

#include <iostream>
namespace astra {

    stream::stream(astra_stream_desc_t description)
        : stream_backend(description) {}

    stream::~stream()
    {
        connections_.clear();
    }

    void stream::disconnect_connections(stream_bin* bin)
    {
        for (auto& connection : connections_)
        {
            if (connection->get_bin() == bin)
            {
                connection->set_bin(nullptr);
            }
        }
    }

    stream_connection* stream::create_connection()
    {
        connection_ptr conn(new stream_connection(this));

        stream_connection* rawPtr = conn.get();

        connections_.push_back(std::move(conn));

        if (is_available())
        {
            on_connection_created(rawPtr, get_handle());
        }

        return rawPtr;
    }

    void stream::destroy_connection(stream_connection* connection)
    {
        auto it = std::find_if(connections_.begin(),
                               connections_.end(),
                               [connection] (const std::unique_ptr<stream_connection>& element)
                               -> bool
                               {
                                   return element.get() == connection;
                               });

        if (it != connections_.end())
        {
            if (is_available())
            {
                on_connection_destroyed(it->get(), get_handle());
            }
            connections_.erase(it);
        }
    }

    void stream::start_connection(stream_connection* connection)
    {
        if (is_available())
        {
            //TODO check membership
            on_connection_started(connection, get_handle());
        }
    }

    void stream::stop_connection(stream_connection* connection)
    {
        if (is_available())
        {
            //TODO check membership
            on_connection_stopped(connection, get_handle());
        }
    }

    void stream::on_availability_changed()
    {
        if (is_available())
        {
            if (listener_)
                listener_->on_stream_registered(this);

            for(auto& connection : connections_)
            {
                on_connection_created(connection.get(), get_handle());

                if (connection->is_started())
                    start_connection(connection.get());
            }
        }
        else
        {
            if (listener_)
                listener_->on_stream_unregistering(this);

            for(auto& connection : connections_)
            {
                connection->set_bin(nullptr);
            }
        }
    }

    void stream::set_parameter(stream_connection* connection,
                               astra_parameter_id id,
                               size_t inByteLength,
                               astra_parameter_data_t inData)
    {
        if (is_available())
            on_set_parameter(connection, id, inByteLength, inData);
    }

    void stream::get_parameter(stream_connection* connection,
                               astra_parameter_id id,
                               astra_parameter_bin_t& parameterBin)
    {
      if (is_available())
      {
        std::cout << "avalible" << std::endl;
        on_get_parameter(connection, id, parameterBin);
      }
      else
      {
        std::cout << "avalible" << std::endl;

      }
    }

    void stream::invoke(stream_connection* connection,
                        astra_command_id commandId,
                        size_t inByteLength,
                        astra_parameter_data_t inData,
                        astra_parameter_bin_t& parameterBin)
    {
        if (is_available())
            on_invoke(connection, commandId, inByteLength, inData, parameterBin);
    }
}
