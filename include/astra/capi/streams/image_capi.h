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
#ifndef IMAGE_CAPI_H
#define IMAGE_CAPI_H

#include <astra_core/capi/astra_defines.h>
#include <astra_core/capi/astra_types.h>
#include <astra/capi/streams/image_types.h>

ASTRA_BEGIN_DECLS

ASTRA_API_EX astra_status_t astra_imagestream_get_mirroring(astra_imagestream_t imageStream,
                                                            bool* mirroring);

ASTRA_API_EX astra_status_t astra_imagestream_set_mirroring(astra_imagestream_t imageStream,
                                                            bool mirroring);

ASTRA_API_EX astra_status_t astra_imagestream_get_hfov(astra_imagestream_t imageStream,
                                                       float* hFov);

ASTRA_API_EX astra_status_t astra_imagestream_get_vfov(astra_imagestream_t imageStream,
                                                       float* vFov);

ASTRA_API_EX astra_status_t astra_imagestream_request_modes(astra_imagestream_t imageStream,
                                                            astra_result_token_t* token,
                                                            size_t* count);

ASTRA_API_EX astra_status_t astra_imagestream_get_modes_result(astra_imagestream_t imageStream,
                                                               astra_result_token_t token,
                                                               astra_imagestream_mode_t* modes,
                                                               size_t count);

ASTRA_API_EX astra_status_t astra_imagestream_set_mode(astra_imagestream_t imageStream,
                                                       const astra_imagestream_mode_t* mode);

ASTRA_API_EX astra_status_t astra_reader_get_imageframe(astra_reader_frame_t readerFrame,
                                                        astra_stream_type_t type,
                                                        astra_stream_subtype_t subtype,
                                                        astra_imageframe_t* imageFrame);

ASTRA_API_EX astra_status_t astra_imageframe_get_stream(astra_imageframe_t imageFrame,
                                                        astra_streamconnection_t* stream);

ASTRA_API_EX astra_status_t astra_imageframe_get_frameindex(astra_imageframe_t imageFrame,
                                                            astra_frame_index_t* index);

ASTRA_API_EX astra_status_t astra_imageframe_get_data_byte_length(astra_imageframe_t imageFrame,
                                                                  size_t* byteLength);

ASTRA_API_EX astra_status_t astra_imageframe_get_data_ptr(astra_imageframe_t imageFrame,
                                                          void** data,
                                                          size_t* byteLength);

ASTRA_API_EX astra_status_t astra_imageframe_copy_data(astra_imageframe_t imageFrame,
                                                       void* data);

ASTRA_API_EX astra_status_t astra_imageframe_get_metadata(astra_imageframe_t imageFrame,
                                                          astra_image_metadata_t* metadata);
ASTRA_END_DECLS

inline void astra_pixelformat_get_bytes_per_pixel(astra_pixel_format_t format,
                                                  uint8_t* bpp)
{
    switch(format)
    {
    case ASTRA_PIXEL_FORMAT_RGB888:
        *bpp = 3;
        break;
    case ASTRA_PIXEL_FORMAT_YUV422:
        *bpp = 2;
        break;
    case ASTRA_PIXEL_FORMAT_GRAY8:
        *bpp = 1;
        break;
    case ASTRA_PIXEL_FORMAT_GRAY16:
        *bpp = 2;
        break;
    case ASTRA_PIXEL_FORMAT_DEPTH_MM:
        *bpp = 2;
        break;
    case ASTRA_PIXEL_FORMAT_UNKNOWN:
        *bpp = 1;
        break;
    case ASTRA_PIXEL_FORMAT_YUYV:
        *bpp = 2;
        break;
    case ASTRA_PIXEL_FORMAT_POINT:
        *bpp = 12;
        break;
    default:
        *bpp = 1;
        break;
    }
}

#endif // IMAGE_CAPI_H
