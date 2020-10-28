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
#include "oni_depthstream.hpp"
#include <astra/capi/streams/depth_parameters.h>

namespace orbbec { namespace ni {

    depthstream::depthstream(astra::PluginServiceProxy& pluginService,
                             astra_streamset_t streamSet,
                             openni::Device& oniDevice,
                             stream_listener& listener)
        : devicestream(pluginService,
                       streamSet,
                       astra::StreamDescription(
                           ASTRA_STREAM_DEPTH,
                           DEFAULT_SUBTYPE),
                       oniDevice,
                       openni::SENSOR_DEPTH,
                       listener)
    {
        PROFILE_FUNC();
    }

    astra_status_t depthstream::on_open()
    {
        auto rc = devicestream::on_open();

        if (rc != astra_status_t::ASTRA_STATUS_SUCCESS)
        {
            return rc;
        }

        PROFILE_FUNC();
        refresh_conversion_cache(oniStream_.getHorizontalFieldOfView(),
                                 oniStream_.getVerticalFieldOfView(),
                                 mode_.width(),
                                 mode_.height());

        return rc;
    }

    void depthstream::refresh_conversion_cache(float horizontalFov,
                                               float verticalFov,
                                               int resolutionX,
                                               int resolutionY)
    {
        PROFILE_FUNC();
        conversionCache_.xzFactor = std::tan(horizontalFov / 2) * 2;
        conversionCache_.yzFactor = std::tan(verticalFov / 2) * 2;
        conversionCache_.resolutionX = resolutionX;
        conversionCache_.resolutionY = resolutionY;
        conversionCache_.halfResX = conversionCache_.resolutionX / 2;
        conversionCache_.halfResY = conversionCache_.resolutionY / 2;
        conversionCache_.coeffX = conversionCache_.resolutionX / conversionCache_.xzFactor;
        conversionCache_.coeffY = conversionCache_.resolutionY / conversionCache_.yzFactor;
    }


    void depthstream::on_connection_removed(astra_bin_t bin,
                                            astra_streamconnection_t connection)
    {
        PROFILE_FUNC();

        devicestream::on_connection_removed(bin, connection);

#ifdef __ANDROID__
        PROFILE_UPDATE();
        PROFILE_OUTPUT("/sdcard/profile_openni_sensor.txt");
#endif
    }

    void depthstream::on_get_parameter(astra_streamconnection_t connection,
                                       astra_parameter_id id,
                                       astra_parameter_bin_t& parameterBin)
    {
      std::cout << "funck" << std::endl;
        PROFILE_FUNC();
        switch (id)
        {
        case ASTRA_PARAMETER_DEPTH_CONVERSION_CACHE:
        {
            std::size_t resultByteLength = sizeof(conversion_cache_t);

            astra_parameter_data_t parameterData;
            astra_status_t rc = pluginService().get_parameter_bin(resultByteLength,
                                                                  &parameterBin,
                                                                  &parameterData);
            if (rc == ASTRA_STATUS_SUCCESS)
            {
                std::memcpy(parameterData, &conversionCache_, resultByteLength);
            }
            break;
        }
        case ASTRA_PARAMETER_DEPTH_REGISTRATION:
        {
            std::size_t resultByteLength = sizeof(bool);

            astra_parameter_data_t parameterData;
            astra_status_t rc = pluginService().get_parameter_bin(resultByteLength,
                                                                  &parameterBin,
                                                                  &parameterData);

            if (rc == ASTRA_STATUS_SUCCESS)
            {
                bool* enable = static_cast<bool*>(parameterData);

                openni::ImageRegistrationMode mode = oniDevice_.getImageRegistrationMode();

                switch (mode)
                {
                case openni::ImageRegistrationMode::IMAGE_REGISTRATION_DEPTH_TO_COLOR:
                    *enable = true;
                    break;
                case openni::ImageRegistrationMode::IMAGE_REGISTRATION_OFF:
                default:
                    *enable = false;
                    break;
                }
            }
        }
        default:
            devicestream::on_get_parameter(connection, id, parameterBin);
            break;
        }
    }

    void depthstream::on_set_parameter(astra_streamconnection_t connection,
                                       astra_parameter_id id,
                                       size_t inByteLength,
                                       astra_parameter_data_t inData)
    {
        switch (id)
        {
        case ASTRA_PARAMETER_DEPTH_REGISTRATION:
        {
            bool enable = *static_cast<bool*>(inData);
            openni::ImageRegistrationMode mode = enable
                ? openni::ImageRegistrationMode::IMAGE_REGISTRATION_DEPTH_TO_COLOR
                : openni::ImageRegistrationMode::IMAGE_REGISTRATION_OFF;

            if (oniDevice_.isImageRegistrationModeSupported(mode))
            {
                openni::Status rc =
                    oniDevice_.setImageRegistrationMode(mode);

                LOG_INFO("orbbec.ni.depthstream",
                         "depth registration: %s, successful: %s",
                         enable ? "enabled" : "disabled",
                         rc == openni::Status::STATUS_OK ? "yes" : "no");
            }
            break;
        }
        default:
            devicestream::on_set_parameter(connection, id, inByteLength, inData);
            break;
        }
    }
}}
