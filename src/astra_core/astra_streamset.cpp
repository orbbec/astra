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
#include "astra_streamset.hpp"
#include "astra_streamset_connection.hpp"
#include <cassert>
#include <algorithm>
#include "astra_cxx_compatibility.hpp"

namespace astra {

    streamset::streamset(std::string uri)
        : uri_(uri)
    { }

    stream_connection* streamset::create_stream_connection(const astra_stream_desc_t& desc)
    {
        LOG_INFO("astra.streamset", "connecting to (%u,%u) on %s", desc.type,desc.subtype, uri_.c_str());

        stream* stream = find_stream_by_type_subtype_impl(desc.type, desc.subtype);

        if (!stream)
        {
            LOG_TRACE("astra.streamset",
                      "registering orphan stream of type (%u,%u) on %s",
                      desc.type,
                      desc.subtype,
                      uri_.c_str());

            stream = register_stream(desc);
        }

        return stream->create_connection();
    }

    bool streamset::destroy_stream_connection(stream_connection* connection)
    {
        assert(connection != nullptr);

        if (!connection)
        {
            LOG_WARN("astra.streamset","destroy_stream_connection: attempt to destroy null connection on %s", uri_.c_str());
            return false;
        }

        stream* stream = connection->get_stream();
        LOG_TRACE("astra.streamset","destroying %p on %s", connection, uri_.c_str());
        stream->destroy_connection(connection);

        if (!stream->has_connections()
            && !stream->is_available())
        {
            LOG_TRACE("astra.streamset","removing unused/unavailable stream %p on %s",
                      stream,
                      uri_.c_str());

            streamCollection_.erase(stream);
            delete stream;
        }

        return true;
    }

    void streamset::claim_stream(stream* stream,
                                 stream_callbacks_t callbacks)
    {

        if (stream)
        {
            assert(!stream->is_available());

            if (stream->is_available())
            {
                LOG_WARN("astra.streamset","register_stream: (%u,%u) already exists, already inflated on %s",
                         stream->get_description().type,
                         stream->get_description().subtype,
                         uri_.c_str());

                return;
            }

            stream->set_callbacks(callbacks);
        }

        return;
    }

    stream* streamset::register_stream(astra_stream_desc_t desc)
    {
        stream* stream = find_stream_by_type_subtype_impl(desc.type, desc.subtype);

        LOG_DEBUG("astra.streamset","registering stream (%u, %u) on %s", desc.type, desc.subtype, uri_.c_str());

        if (!stream)
        {
            stream = new astra::stream(desc);
            stream->set_listener(this);
            streamCollection_.insert(stream);
        }

        return stream;
    }

    streamset_connection* streamset::add_new_connection()
    {
        LOG_TRACE("astra.streamset","new connection to %s", uri_.c_str());
        streamset_connectionPtr ptr = astra::make_unique<streamset_connection>(this);
        streamset_connection* conn = ptr.get();
        connections_.push_back(std::move(ptr));

        return conn;
    }

    void streamset::disconnect_streamset_connection(streamset_connection* connection)
    {
        assert(connection != nullptr);
        if (!connection)
        {
            LOG_WARN("astra.streamset","disconnect_streamset_connection: null connection on %s", uri_.c_str());
            return;
        }

        auto it = std::find_if(connections_.begin(), connections_.end(),
                               [&connection] (streamset_connectionPtr& ptr) -> bool
                               {
                                   return ptr.get() == connection;
                               });

        if (it != connections_.end())
        {
            LOG_DEBUG("astra.streamset","disconnecting %p connection from %s",
                      connection,
                      uri_.c_str());

            connections_.erase(it);
        }
        else
        {
            LOG_DEBUG("astra.streamset",
                      "disconnect_streamset_connection: %p connection not found on %s",
                      connection,
                      uri_.c_str());
        }
    }

    void streamset::destroy_stream(stream* stream)
    {
        assert(stream != nullptr);
        if (!stream)
        {
            LOG_WARN("astra.streamset",
                     "destroy_stream: null parameter on %s", uri_.c_str());
        }

        auto it = streamCollection_.find(stream);
        if (it != streamCollection_.end())
        {
            LOG_TRACE("astra.streamset",
                      "destroying stream %p on %s", stream, uri_.c_str());

            stream->clear_callbacks();
        }
        else
        {
            LOG_WARN("astra.streamset", "destroy_stream: stream %p not found on %s", stream, uri_.c_str());
        }
    }

    void streamset::claim()
    {
        assert(isClaimed_ != true);
        if (isClaimed_)
        {
            LOG_WARN("astra.streamset", "claim: %s already claimed", uri_.c_str());
            return;
        }

        isClaimed_ = true;
    }

    bool streamset::is_claimed() const
    {
        return isClaimed_;
    }

    astra_callback_id_t streamset::register_for_stream_registered_event(StreamRegisteredCallback callback)
    {
        return streamRegisteredSignal_ += callback;
    }

    astra_callback_id_t streamset::register_for_stream_unregistering_event(StreamUnregisteringCallback callback)
    {
        return streamUnregisteringSignal_ += callback;
    }

    void streamset::unregister_for_stream_registered_event(astra_callback_id_t callbackId)
    {
        streamRegisteredSignal_ -= callbackId;
    }

    void streamset::unregister_for_stream_unregistering_event(astra_callback_id_t callbackId)
    {
        streamUnregisteringSignal_ -= callbackId;
    }

    bool streamset::is_member(astra_stream_t streamHandle) const
    {
        assert(streamHandle != nullptr);

        stream* stream = reinterpret_cast<astra::stream*>(streamHandle);
        return streamCollection_.find(stream) != streamCollection_.end();
    }

    astra_stream_t streamset::find_stream_by_type_subtype(astra_stream_type_t type,
                                                          astra_stream_subtype_t subtype) const
    {
        stream* stream = find_stream_by_type_subtype_impl(type, subtype);

        astra_stream_t streamHandle = reinterpret_cast<astra_stream_t>(stream);
        return streamHandle;
    }

    void streamset::visit_streams(std::function<void(stream*)> visitorMethod)
    {
        for (auto* stream : streamCollection_)
        {
            visitorMethod(stream);
        }
    }

    stream* streamset::find_stream_by_type_subtype_impl(astra_stream_type_t type,
                                                        astra_stream_subtype_t subtype) const
    {
        for (auto* stream : streamCollection_)
        {
            const astra_stream_desc_t& desc = stream->get_description();

            if (desc.type == type && desc.subtype == subtype)
            {
                return stream;
            }
        }

        return nullptr;
    }

    void streamset::on_stream_registered(stream* stream)
    {
        streamRegisteredSignal_.raise(stream_registered_event_args(this, stream, stream->get_description()));
    }

    void streamset::on_stream_unregistering(stream* stream)
    {
        streamUnregisteringSignal_.raise(
            stream_unregistering_event_args(this, stream, stream->get_description()));
    }

}
