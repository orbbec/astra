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
#include "astra_streamset_catalog.hpp"
#include <cassert>
#include <algorithm>
#include "astra_streamset.hpp"
#include "astra_streamset_connection.hpp"
#include "astra_signal.hpp"
#include "astra_logger.hpp"
#include "astra_cxx_compatibility.hpp"

namespace astra {

    streamset_catalog::~streamset_catalog() = default;

    streamset_connection& streamset_catalog::open_set_connection(std::string uri)
    {
        std::string finalUri = uri;
        if (uri == "device/default")
        {
            LOG_INFO("streamset_catalog", "default uri provided.");
            finalUri = "device/sensor0";
        }

        streamset& set = get_or_add(finalUri, false);
        return *set.add_new_connection();
    }

    void streamset_catalog::close_set_connection(streamset_connection* conn)
    {
        assert(conn != nullptr);

        streamset* set = conn->get_streamSet();
        set->disconnect_streamset_connection(conn);
    }

    streamset& streamset_catalog::get_or_add(std::string uri, bool claim)
    {
        auto it = streamSets_.find(uri);

        streamset* streamSet;
        if (it == streamSets_.end())
        {
            streamset_ptr ssPtr = astra::make_unique<streamset>(uri);
            streamSet = ssPtr.get();

            auto added = ssPtr->register_for_stream_registered_event(
                [this] (stream_registered_event_args args)
                {
                    on_stream_registered(args);
                });

            auto removed = ssPtr->register_for_stream_unregistering_event(
                [this] (stream_unregistering_event_args args)
                {
                    on_stream_unregistering(args);
                });

            streamSets_.insert(std::make_pair(uri, astra::make_unique<streamset_entry>(std::move(ssPtr), added, removed)));
        }
        else
        {
            streamSet = it->second->streamSet.get();
        }

        if (claim)
        {
            streamSet->claim();
        }

        return *streamSet;
    }

    void streamset_catalog::clear()
    {
        streamSets_.clear();
    }

    void streamset_catalog::on_stream_registered(stream_registered_event_args args)
    {
        streamRegisteredSignal_.raise(args);
    }

    void streamset_catalog::on_stream_unregistering(stream_unregistering_event_args args)
    {
        streamUnregisteringSignal_.raise(args);
    }

    streamset* streamset_catalog::find_streamset_for_stream(stream* stream)
    {
        assert(stream != nullptr);

        auto it = std::find_if(streamSets_.begin(), streamSets_.end(),
                               [&stream] (streamsetMap::value_type& el) -> bool
                               {
                                   return el.second->streamSet->is_member(stream->get_handle());
                               });

        return it != streamSets_.end() ? it->second->streamSet.get() : nullptr;
    }

    void streamset_catalog::visit_sets(std::function<void(streamset*)> visitorMethod)
    {
        for(auto& pair : streamSets_)
        {
            visitorMethod(pair.second->streamSet.get());
        }
    }

    void streamset_catalog::destroy_set(astra::streamset* set)
    {
        auto it = streamSets_.find(set->get_uri());

        if (it != streamSets_.end())
        {
            streamSets_.erase(it);
        }
    }
}
