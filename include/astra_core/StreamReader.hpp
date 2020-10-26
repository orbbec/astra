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
#ifndef ASTRA_STREAMREADER_HPP
#define ASTRA_STREAMREADER_HPP

#include "capi/astra_core.h"
#include <astra_core/FrameListener.hpp>
#include <astra_core/Frame.hpp>
#include <memory>
#include <vector>
#include <algorithm>
#include <functional>
#include <iterator>

#include <iostream>

namespace astra {

    class StreamReader
    {
    public:
        StreamReader()
            : readerRef_(nullptr)
        {}

        StreamReader(astra_reader_t reader)
            : readerRef_(std::make_shared<ReaderRef>(reader))
        {}

        StreamReader(const StreamReader& reader)
             : readerRef_(reader.readerRef_)
        {}

        StreamReader& operator=(const StreamReader& reader)
        {
            this->readerRef_ = reader.readerRef_;
            return *this;
        }

        template<typename T>
        T stream()
        {
            return stream<T>(DEFAULT_SUBTYPE);
        }

        template<typename T>
        T stream(astra_stream_subtype_t subtype)
        {
            std::cout << "stream" << std::endl;
            if (!is_valid())
                throw std::logic_error("StreamReader is not associated with a streamset.");

            astra_streamconnection_t connection;
            astra_reader_get_stream(readerRef_->get_reader(),
                                    T::id,
                                    subtype,
                                    &connection);

            return T(connection);
        }

        void add_listener(FrameListener& listener)
        {
            if (!is_valid())
                throw std::logic_error("StreamReader is not associated with a streamset.");

            readerRef_.get()->add_listener(listener);
        }

        void remove_listener(FrameListener& listener)
        {
            if (!is_valid())
                throw std::logic_error("StreamReader is not associated with a streamset.");

            readerRef_.get()->remove_listener(listener);
        }

        bool is_valid() { return readerRef_ != nullptr; }

        Frame get_latest_frame(int timeoutMillis = ASTRA_TIMEOUT_FOREVER)
        {
            if (!is_valid())
                throw std::logic_error("StreamReader is not associated with a streamset.");

            astra_reader_frame_t frame;
            astra_reader_open_frame(readerRef_->get_reader(), timeoutMillis, &frame);

            return astra::Frame(frame);
        }

    private:
        class ReaderRef;
        using ReaderRefPtr = std::shared_ptr<ReaderRef>;

        StreamReader(ReaderRefPtr readerRef)
            : readerRef_(readerRef)
        { }

        class ReaderRef :
            public std::enable_shared_from_this<ReaderRef>
        {
        public:
            ReaderRef(astra_reader_t reader)
                :  reader_(reader)
            { }

            ~ReaderRef()
            {
                listeners_.clear();
                ensure_callback_removed();
                astra_reader_destroy(&reader_);
            }

            static void frame_ready_thunk(void* clientTag,
                                          astra_reader_t reader,
                                          astra_reader_frame_t frame)
            {
                ReaderRef* self = static_cast<ReaderRef*>(clientTag);
                self->notify_listeners(frame);
            }

            void add_listener(FrameListener& listener)
            {
                ensure_callback_added();

                auto it = std::find(listeners_.begin(),
                                    listeners_.end(),
                                    listener);

                if (it != listeners_.end())
                    return;

                if (isNotifying_)
                {
                    addedListeners_.push_back(listener);
                }
                else
                {
                    listeners_.push_back(listener);
                }
            }

            void remove_listener(FrameListener& listener)
            {
                auto it = std::find(listeners_.begin(),
                                    listeners_.end(),
                                    listener);

                if (it == listeners_.end())
                    return;

                if (isNotifying_)
                {
                    removedListeners_.push_back(listener);
                }
                else
                {
                    listeners_.erase(it);
                }

                if (listeners_.size() == 0)
                {
                    ensure_callback_removed();
                }
            }

            void notify_listeners(astra_reader_frame_t readerFrame)
            {
                if (removedListeners_.size() > 0)
                {
                    for(FrameListener& listener : removedListeners_)
                    {
                        auto it = std::find(listeners_.begin(),
                                            listeners_.end(),
                                            listener);

                        listeners_.erase(it);
                    }
                    removedListeners_.clear();
                }

                std::move(addedListeners_.begin(),
                          addedListeners_.end(),
                          std::back_inserter(listeners_));

                if (listeners_.size() == 0)
                {
                    ensure_callback_removed();
                    return;
                }

                //we didn't open the frame, so don't auto close it.
                //the StreamReader internals will close it automatically
                const bool autoCloseFrame = false;
                astra::Frame frameWrapper(readerFrame, autoCloseFrame);

                isNotifying_ = true;
                StreamReader reader(shared_from_this());
                for(FrameListener& listener : listeners_)
                {
                    listener.on_frame_ready(reader, frameWrapper);
                }

                isNotifying_ = false;
            }

            astra_reader_t get_reader() { return reader_; }

        private:
            void ensure_callback_added()
            {
                if (!callbackRegistered_)
                {
                    astra_reader_register_frame_ready_callback(reader_,
                                                               &ReaderRef::frame_ready_thunk,
                                                               this,
                                                               &callbackId_);

                    callbackRegistered_ = true;
                }
            }

            void ensure_callback_removed()
            {
                if (callbackRegistered_)
                {
                    astra_reader_unregister_frame_ready_callback(&callbackId_);
                    callbackRegistered_ = false;
                }
            }

            astra_reader_t reader_;

            bool isNotifying_{false};
            bool callbackRegistered_{false};

            using ListenerList = std::vector<std::reference_wrapper<FrameListener>>;

            ListenerList listeners_;
            ListenerList addedListeners_;
            ListenerList removedListeners_;

            astra_reader_callback_id_t callbackId_;
        };

        ReaderRefPtr readerRef_;

        friend bool operator==(const StreamReader& lhs, const StreamReader& rhs);
    };

    inline bool operator==(const StreamReader& lhs, const StreamReader& rhs)
    {
        return lhs.readerRef_ == rhs.readerRef_;
    }

    inline bool operator!=(const StreamReader& lhs, const StreamReader& rhs)
    {
        return !(lhs == rhs);
    }
}

#endif // ASTRA_STREAMREADER_HPP
