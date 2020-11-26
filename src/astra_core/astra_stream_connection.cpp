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
#include "astra_stream_connection.hpp"
#include "astra_stream_bin.hpp"
#include "astra_stream.hpp"
#include <cassert>

namespace astra {

    stream_connection::stream_connection(stream* stream)
        : stream_(stream)
    {
        assert (stream != nullptr);

        connection_.handle =
            reinterpret_cast<astra_streamconnection_handle_t>(this);

        connection_.desc = stream->get_description();

        binFrontBufferReadyCallback_ =
            [this](stream_bin* b, astra_frame_index_t frameIndex)
            {
                this->on_bin_front_buffer_ready(b, frameIndex);
            };
    }

    stream_connection::~stream_connection()
    {
        clear_pending_parameter_result();
        set_bin(nullptr);
    }

    astra_bin_t stream_connection::get_bin_handle()
    {
        if (bin_ != nullptr)
        {
            return bin_->get_handle();
        }
        else
        {
            return nullptr;
        }
    }

    const astra_stream_desc_t& stream_connection::get_description() const
    {
        return stream_->get_description();
    }

    astra_frame_t* stream_connection::lock()
    {
        LOG_TRACE("astra.stream_connection", "%x lock", this);

        if (locked_)
        {
            return currentFrame_;
        }

        if (is_started() && bin_ != nullptr)
        {
            currentFrame_ = bin_->lock_front_buffer();
        }
        else
        {
            currentFrame_ = nullptr;
        }

        locked_ = true;

        return currentFrame_;
    }

    void stream_connection::unlock()
    {
        LOG_TRACE("astra.stream_connection", "%x unlock", this);

        if (!locked_)
        {
            LOG_WARN("astra.stream_connection", "%x stream_connection::unlock() not locked", this);
            assert(locked_);
        }

        if (is_started() && bin_ != nullptr)
        {
            bin_->unlock_front_buffer();
        }

        currentFrame_ = nullptr;
        locked_ = false;
    }

    void stream_connection::start()
    {
        if (started_)
            return;

        LOG_DEBUG("astra.stream_connection",
                  "%p starting (%d, %d)",
                  this,
                  get_description().type,
                  get_description().subtype);

        if (stream_->is_available())
        {
            stream_->start_connection(this);
        }

        started_ = true;
    }

    void stream_connection::stop()
    {
        if (!started_)
            return;

        LOG_DEBUG("astra.stream_connection",
                  "%p stopping, stream = (%u, %u)",
                  this,
                  get_description().type,
                  get_description().subtype);

        if (stream_->is_available())
        {
            stream_->stop_connection(this);
        }

        started_ = false;
    }

    void stream_connection::set_bin(stream_bin* bin)
    {
        if (bin_ != nullptr)
        {
            bin_->unregister_front_buffer_ready_callback(binFrontBufferReadyCallbackId_);
            bin_->dec_connected();
        }

        bin_ = bin;

        if (bin_ != nullptr)
        {
            bin_->inc_connected();

            binFrontBufferReadyCallbackId_ =
                bin_->register_front_buffer_ready_callback(binFrontBufferReadyCallback_);
        }
    }

    void stream_connection::on_bin_front_buffer_ready(stream_bin* bin, astra_frame_index_t frameIndex)
    {
        assert(bin_ != nullptr);
        assert(bin_ == bin);

        frameReadySignal_.raise(this, frameIndex);
    }

    astra_callback_id_t stream_connection::register_frame_ready_callback(FrameReadyCallback callback)
    {
        return frameReadySignal_ += callback;
    }

    void stream_connection::unregister_frame_ready_callback(astra_callback_id_t& callbackId)
    {
        frameReadySignal_ -= callbackId;
        callbackId = 0;
    }

    void stream_connection::set_parameter(astra_parameter_id id,
                                          size_t inByteLength,
                                          astra_parameter_data_t inData)
    {
        stream_->set_parameter(this, id, inByteLength, inData);
    }

    void stream_connection::get_parameter(astra_parameter_id id,
                                          size_t& resultByteLength,
                                          astra_result_token_t& token)
    {
        astra_parameter_bin_t parameterBinHandle = nullptr;
        stream_->get_parameter(this, id, parameterBinHandle);

        if (parameterBinHandle == nullptr)
        {
            throw std::runtime_error("stream_connection::get_parameter: parameter handler is null");
        }
        cache_parameter_bin_token(parameterBinHandle, resultByteLength, token);
    }

    astra_status_t stream_connection::get_result(astra_result_token_t token,
                                                 size_t dataByteLength,
                                                 astra_parameter_data_t dataDestination)
    {
        parameter_bin* parameterBin = parameter_bin::get_ptr(token);
        if (pendingParameterResult_ != nullptr && parameterBin == pendingParameterResult_)
        {
            std::memcpy(dataDestination, parameterBin->data(), dataByteLength);
            clear_pending_parameter_result();
            return ASTRA_STATUS_SUCCESS;
        }
        else
        {
            //Results are read-once-only, then they self destruct.
            //Client tried to get result in wrong order, or with duplicate or stale token
            LOG_WARN("astra.stream_connection", "%p no pending parameter result.", this);
            clear_pending_parameter_result();
            return ASTRA_STATUS_INVALID_PARAMETER_TOKEN;
        }
    }

    void stream_connection::invoke(astra_command_id commandId,
                                   size_t inByteLength,
                                   astra_parameter_data_t inData,
                                   size_t& resultByteLength,
                                   astra_result_token_t& token)
    {
        astra_parameter_bin_t parameterBinHandle = nullptr;
        stream_->invoke(this, commandId, inByteLength, inData, parameterBinHandle);

        cache_parameter_bin_token(parameterBinHandle, resultByteLength, token);
    }

    void stream_connection::cache_parameter_bin_token(astra_parameter_bin_t parameterBinHandle,
                                                      size_t& resultByteLength,
                                                      astra_result_token_t& token)
    {
        //delete old parameter result -- only one outstanding allowed at a time
        clear_pending_parameter_result();
        if (parameterBinHandle != nullptr)
        {
            pendingParameterResult_ = parameter_bin::get_ptr(parameterBinHandle);
            resultByteLength = pendingParameterResult_->byteLength();
            token = parameterBinHandle;
        }
        else
        {
            pendingParameterResult_ = nullptr;
            resultByteLength = 0;
            token = nullptr;
        }
    }

    void stream_connection::clear_pending_parameter_result()
    {
        if (pendingParameterResult_ != nullptr)
        {
            delete pendingParameterResult_;
            pendingParameterResult_ = nullptr;
        }
    }
}
