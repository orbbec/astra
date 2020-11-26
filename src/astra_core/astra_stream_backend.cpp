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
#include "astra_stream_backend.hpp"
#include "astra_stream_bin.hpp"
#include <algorithm>
#include "astra_cxx_compatibility.hpp"

#include <iostream>
namespace astra {

    void stream_backend::set_callbacks(const stream_callbacks_t& callbacks)
    {
        callbacks_ = astra::make_unique<stream_callbacks_t>(callbacks);
        on_availability_changed();
    }

    void stream_backend::clear_callbacks()
    {
        callbacks_.reset();
        on_availability_changed();
    }

    stream_bin* stream_backend::create_bin(size_t bufferLengthInBytes)
    {
        bin_ptr bin(astra::make_unique<stream_bin>(bufferLengthInBytes));
        stream_bin* rawPtr = bin.get();

        bins_.push_back(std::move(bin));

        return rawPtr;
    }

    void stream_backend::destroy_bin(stream_bin* bin)
    {
        auto it = std::find_if(bins_.begin(),
                               bins_.end(),
                               [&bin] (const bin_ptr& element)
                               {
                                   return bin == element.get();
                               });



        if (it != bins_.end())
        {
            on_destroying_bin(it->get());
            bins_.erase(it);
        }
    }

    void stream_backend::on_connection_created(stream_connection* connection, astra_stream_t stream)
    {
        if (callbacks_ &&
            callbacks_->connection_added_callback)
            callbacks_->connection_added_callback(callbacks_->context,
                                                   stream,
                                                   connection->get_handle());
    }

    void stream_backend::on_connection_started(stream_connection* connection, astra_stream_t stream)
    {
        if (callbacks_ &&
            callbacks_->connection_started_callback)
            callbacks_->connection_started_callback(callbacks_->context,
                                                     stream,
                                                     connection->get_handle());

    }

    void stream_backend::on_connection_stopped(stream_connection* connection, astra_stream_t stream)
    {
        if (callbacks_ &&
            callbacks_->connection_stopped_callback)
            callbacks_->connection_stopped_callback(callbacks_->context,
                                                     stream,
                                                     connection->get_handle());

    }

    void stream_backend::on_connection_destroyed(stream_connection* connection, astra_stream_t stream)
    {
        if (callbacks_ &&
            callbacks_->connection_removed_callback)
        {
            astra_bin_t binHandle = connection->get_bin_handle();

            callbacks_->connection_removed_callback(callbacks_->context,
                                                     stream,
                                                     binHandle,
                                                     connection->get_handle());
        }
    }

    void stream_backend::on_set_parameter(stream_connection* connection,
                                         astra_parameter_id id,
                                         size_t inByteLength,
                                         astra_parameter_data_t inData)
    {

        if (callbacks_ &&
            callbacks_->set_parameter_callback != nullptr)
        {
            callbacks_->set_parameter_callback(callbacks_->context,
                                                connection->get_handle(),
                                                id,
                                                inByteLength,
                                                inData);
        }
    }

    void stream_backend::on_get_parameter(stream_connection* connection,
                                         astra_parameter_id id,
                                         astra_parameter_bin_t& parameterBin)
    {

        if (callbacks_ &&
            callbacks_->get_parameter_callback != nullptr)
        {
          std::cout << "on_get_" << std::endl;
            callbacks_->get_parameter_callback(callbacks_->context,
                                                connection->get_handle(),
                                                id,
                                                &parameterBin);
        }
        else
        {
          std::cout << "false on_get" << std::endl;
        }
    }

    void stream_backend::on_invoke(stream_connection* connection,
                                  astra_command_id commandId,
                                  size_t inByteLength,
                                  astra_parameter_data_t inData,
                                  astra_parameter_bin_t& parameterBin)
    {
        if (callbacks_ &&
            callbacks_->invoke_callback != nullptr)
        {
            callbacks_->invoke_callback(callbacks_->context,
                                         connection->get_handle(),
                                         commandId,
                                         inByteLength,
                                         inData,
                                         &parameterBin);
        }
    }
}
