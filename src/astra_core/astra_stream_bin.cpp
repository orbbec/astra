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
#include "astra_stream_bin.hpp"
#include <cassert>
#include <cstring>
#include <astra_core/capi/plugins/astra_plugin.h>

namespace astra {

    stream_bin::stream_bin(size_t bufferLengthInBytes)
        : bufferSize_(bufferLengthInBytes)
    {
        LOG_TRACE("stream_bin", "Created stream_bin %x", this);
        init_buffers(bufferLengthInBytes);
    }

    stream_bin::~stream_bin()
    {
        assert(!has_clients_connected());
        deinit_buffers();
    }

    void stream_bin::init_buffers(size_t bufferLengthInBytes)
    {
        for(int i = 0; i < BUFFER_COUNT; ++i)
        {
            init_buffer(buffers_[i], bufferLengthInBytes);
        }
    }

    void stream_bin::deinit_buffers()
    {
        for(int i = 0; i < BUFFER_COUNT; ++i)
        {
            deinit_buffer(buffers_[i]);
        }
    }

    void stream_bin::init_buffer(astra_frame_t& frame, size_t bufferLengthInBytes)
    {
        frame.byteLength = bufferLengthInBytes;
        frame.frameIndex = -1;
        frame.data = new uint8_t[bufferLengthInBytes];
        std::memset(frame.data, 0, bufferLengthInBytes);
    }

    void stream_bin::deinit_buffer(astra_frame_t& frame)
    {
        uint8_t* data = static_cast<uint8_t*>(frame.data);
        delete[] data;
        frame.data = nullptr;
        frame.frameIndex = -1;
        frame.byteLength = 0;
    }

    astra_callback_id_t stream_bin::register_front_buffer_ready_callback(FrontBufferReadyCallback callback)
    {
        return frontBufferReadySignal_ += callback;
    }

    void stream_bin::unregister_front_buffer_ready_callback(astra_callback_id_t& callbackId)
    {
        frontBufferReadySignal_ -= callbackId;
        callbackId = 0;
    }

    astra_frame_t* stream_bin::get_frontBuffer()
    {
        return &buffers_[frontBufferIndex_];
    }

    astra_frame_t* stream_bin::get_middleBuffer()
    {
        return &buffers_[middleBufferIndex_];
    }

    astra_frame_t* stream_bin::lock_front_buffer()
    {
        LOG_TRACE("stream_bin", "%x locking front buffer. lock count: %u -> %u", this, frontBufferLockCount_, frontBufferLockCount_+1);

        ++frontBufferLockCount_;
        return get_frontBuffer();
    }

    void stream_bin::unlock_front_buffer()
    {
        LOG_TRACE("stream_bin", "%x unlocking front buffer. lock count: %u -> %u", this, frontBufferLockCount_, frontBufferLockCount_-1);
        if (frontBufferLockCount_ == 0)
        {
            //TODO: error, logging
            LOG_WARN("stream_bin", "%x stream_bin unlocked too many times!", this);
            assert(frontBufferLockCount_ != 0);
            return;
        }
        --frontBufferLockCount_;
        if (frontBufferLockCount_ > 0)
        {
            //can't swap front buffers because there is still an outstanding lock
            return;
        }
        LOG_TRACE("stream_bin", "%x unlock pre indices: f: %d m: %d b: %d",
            this,
            get_frontBuffer()->frameIndex,
            get_middleBuffer()->frameIndex,
            get_backBuffer()->frameIndex);
        astra_frame_index_t frontFrameIndex = get_frontBuffer()->frameIndex;
        astra_frame_index_t middleFrameIndex = get_middleBuffer()->frameIndex;

        //if the middle buffer's frameIndex is newer, signal readiness
        if (middleFrameIndex != -1 && middleFrameIndex > frontFrameIndex)
        {
            //back buffer is locked.
            //swap front and middle buffers only
            size_t oldFrontBufferIndex = frontBufferIndex_;
            frontBufferIndex_ = middleBufferIndex_;
            middleBufferIndex_ = oldFrontBufferIndex;


            LOG_TRACE("stream_bin", "%x unlock front/mid indices: f: %d m: %d b: %d",
                this,
                get_frontBuffer()->frameIndex,
                get_middleBuffer()->frameIndex,
                get_backBuffer()->frameIndex);

            raiseFrameReadySignal();
        }
    }

    astra_frame_t* stream_bin::cycle_buffers()
    {
        LOG_TRACE("stream_bin", "%x cycling buffer. lock count: %u produced frame index: %d",
                            this, frontBufferLockCount_, get_backBuffer()->frameIndex);
        LOG_TRACE("stream_bin", "%x cycle pre indices: f: %d m: %d b: %d",
            this,
            get_frontBuffer()->frameIndex,
            get_middleBuffer()->frameIndex,
            get_backBuffer()->frameIndex);
        if (is_front_buffer_locked())
        {
            //Can't change front buffer.
            //swap back and middle buffers only
            size_t oldBackBufferIndex = backBufferIndex_;
            backBufferIndex_ = middleBufferIndex_;
            middleBufferIndex_ = oldBackBufferIndex;

            LOG_TRACE("stream_bin", "%x cycle back/mid indices: f: %d m: %d b: %d",
                this,
                get_frontBuffer()->frameIndex,
                get_middleBuffer()->frameIndex,
                get_backBuffer()->frameIndex);
        }
        else
        {
#ifdef DEBUG
            astra_frame_index_t oldFrameIndex = get_frontBuffer()->frameIndex;
#endif
            //The rare case where neither front or back buffers are locked.
            //Rotate back buffer directly to front buffer. (Ignore middle.)
            size_t oldFrontBufferIndex = frontBufferIndex_;
            frontBufferIndex_ = backBufferIndex_;
            backBufferIndex_ = oldFrontBufferIndex;

#ifdef DEBUG
            astra_frame_index_t newFrameIndex = get_frontBuffer()->frameIndex;
            if (frameIndex != -1 && newFrameIndex <= oldFrameIndex)
            {
                LOG_WARN("stream_bin", "%x buffers cycled with out-of-order frame indices: %d->%d",
                                this, oldFrameIndex, newFrameIndex);
                assert(frameIndex > oldFrameIndex);
            }
#endif

            LOG_TRACE("stream_bin", "%x cycle front/back indices: f: %d m: %d b: %d",
                this,
                get_frontBuffer()->frameIndex,
                get_middleBuffer()->frameIndex,
                get_backBuffer()->frameIndex);

            raiseFrameReadySignal();
        }

        return get_backBuffer();
    }

    void stream_bin::raiseFrameReadySignal()
    {
        astra_frame_index_t frameIndex = get_frontBuffer()->frameIndex;

        if (frameIndex != -1)
        {
            frontBufferReadySignal_.raise(this, frameIndex);
        }
    }
}
