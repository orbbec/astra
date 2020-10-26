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
#include <SFML/Graphics.hpp>
#include <astra_core/astra_core.hpp>
#include <astra/astra.hpp>
#include "LitDepthVisualizer.hpp"
#include <chrono>
#include <iostream>
#include <iomanip>
#include <key_handler.h>

enum ColorMode
{
    MODE_COLOR,
    MODE_IR_16,
    MODE_IR_RGB,
};

class MultiFrameListener : public astra::FrameListener
{
public:
    using BufferPtr = std::unique_ptr<uint8_t[]>;

    struct StreamView
    {
        sf::Sprite sprite_;
        sf::Texture texture_;
        BufferPtr buffer_;
        int width_{0};
        int height_{0};
    };

    MultiFrameListener()
    {
        prev_ = ClockType::now();
    }

    void init_texture(int width, int height, StreamView& view)
    {
        if (view.buffer_ == nullptr || width != view.width_ || height != view.height_)
        {
            view.width_ = width;
            view.height_ = height;

            // texture is RGBA
            const int byteLength = width * height * 4;

            view.buffer_ = BufferPtr(new uint8_t[byteLength]);

            clear_view(view);

            view.texture_.create(width, height);
            view.sprite_.setTexture(view.texture_);
            view.sprite_.setPosition(0, 0);
        }
    }

    void clear_view(StreamView& view)
    {
        const int byteLength = view.width_ * view.height_ * 4;
        std::fill(&view.buffer_[0], &view.buffer_[0] + byteLength, 0);
    }

    void check_fps()
    {
        const float frameWeight = .2f;

        const ClockType::time_point now = ClockType::now();
        const float elapsedMillis = std::chrono::duration_cast<DurationType>(now - prev_).count();

        elapsedMillis_ = elapsedMillis * frameWeight + elapsedMillis_ * (1.f - frameWeight);
        prev_ = now;

        const float fps = 1000.f / elapsedMillis;

        const auto precision = std::cout.precision();

        std::cout << std::fixed
                  << std::setprecision(1)
                  << fps << " fps ("
                  << std::setprecision(1)
                  << elapsedMillis_ << " ms)"
                  << std::setprecision(precision)
                  << std::endl;
    }

    void update_depth(astra::Frame& frame)
    {
        const astra::PointFrame pointFrame = frame.get<astra::PointFrame>();

        if (!pointFrame.is_valid())
        {
            clear_view(depthView_);
            depthView_.texture_.update(&depthView_.buffer_[0]);
            return;
        }

        const int depthWidth = pointFrame.width();
        const int depthHeight = pointFrame.height();

        init_texture(depthWidth, depthHeight, depthView_);

        if (isPaused_) { return; }

        visualizer_.update(pointFrame);

        astra::RgbPixel* vizBuffer = visualizer_.get_output();
        uint8_t* buffer = &depthView_.buffer_[0];
        for (int i = 0; i < depthWidth * depthHeight; i++)
        {
            const int rgbaOffset = i * 4;
            buffer[rgbaOffset] = vizBuffer[i].r;
            buffer[rgbaOffset + 1] = vizBuffer[i].g;
            buffer[rgbaOffset + 2] = vizBuffer[i].b;
            buffer[rgbaOffset + 3] = 255;
        }

        depthView_.texture_.update(&depthView_.buffer_[0]);
    }

    void update_color(astra::Frame& frame)
    {
        const astra::ColorFrame colorFrame = frame.get<astra::ColorFrame>();

        if (!colorFrame.is_valid())
        {
            clear_view(colorView_);
            colorView_.texture_.update(&colorView_.buffer_[0]);
            return;
        }

        const int colorWidth = colorFrame.width();
        const int colorHeight = colorFrame.height();

        init_texture(colorWidth, colorHeight, colorView_);

        if (isPaused_) { return; }

        const astra::RgbPixel* color = colorFrame.data();
        uint8_t* buffer = &colorView_.buffer_[0];

        for(int i = 0; i < colorWidth * colorHeight; i++)
        {
            const int rgbaOffset = i * 4;
            buffer[rgbaOffset] = color[i].r;
            buffer[rgbaOffset + 1] = color[i].g;
            buffer[rgbaOffset + 2] = color[i].b;
            buffer[rgbaOffset + 3] = 255;
        }

        colorView_.texture_.update(&colorView_.buffer_[0]);
    }

    void update_ir_16(astra::Frame& frame)
    {
        const astra::InfraredFrame16 irFrame = frame.get<astra::InfraredFrame16>();

        if (!irFrame.is_valid())
        {
            clear_view(colorView_);
            colorView_.texture_.update(&colorView_.buffer_[0]);
            return;
        }

        const int irWidth = irFrame.width();
        const int irHeight = irFrame.height();

        init_texture(irWidth, irHeight, colorView_);

        if (isPaused_) { return; }

        const uint16_t* ir_values = irFrame.data();
        uint8_t* buffer = &colorView_.buffer_[0];
        for (int i = 0; i < irWidth * irHeight; i++)
        {
            const int rgbaOffset = i * 4;
            const uint16_t value = ir_values[i];
            const uint8_t red = static_cast<uint8_t>(value >> 2);
            const uint8_t blue = 0x66 - red / 2;
            buffer[rgbaOffset] = red;
            buffer[rgbaOffset + 1] = 0;
            buffer[rgbaOffset + 2] = blue;
            buffer[rgbaOffset + 3] = 255;
        }

        colorView_.texture_.update(&colorView_.buffer_[0]);
    }

    void update_ir_rgb(astra::Frame& frame)
    {
        const astra::InfraredFrameRgb irFrame = frame.get<astra::InfraredFrameRgb>();

        if (!irFrame.is_valid())
        {
            clear_view(colorView_);
            colorView_.texture_.update(&colorView_.buffer_[0]);
            return;
        }

        int irWidth = irFrame.width();
        int irHeight = irFrame.height();

        init_texture(irWidth, irHeight, colorView_);

        if (isPaused_) { return; }

        const astra::RgbPixel* irRGB = irFrame.data();
        uint8_t* buffer = &colorView_.buffer_[0];
        for (int i = 0; i < irWidth * irHeight; i++)
        {
            const int rgbaOffset = i * 4;
            buffer[rgbaOffset] = irRGB[i].r;
            buffer[rgbaOffset + 1] = irRGB[i].g;
            buffer[rgbaOffset + 2] = irRGB[i].b;
            buffer[rgbaOffset + 3] = 255;
        }

        colorView_.texture_.update(&colorView_.buffer_[0]);
    }

    virtual void on_frame_ready(astra::StreamReader& reader,
                                astra::Frame& frame) override
    {
        update_depth(frame);

        switch (colorMode_)
        {
        case MODE_COLOR:
            update_color(frame);
            break;
        case MODE_IR_16:
            update_ir_16(frame);
            break;
        case MODE_IR_RGB:
            update_ir_rgb(frame);
            break;
        }

        check_fps();
    }

    void draw_to(sf::RenderWindow& window, sf::Vector2f origin, sf::Vector2f size)
    {
        const int viewSize = (int)(size.x / 2.0f);
        const sf::Vector2f windowSize = window.getView().getSize();

        if (depthView_.buffer_ != nullptr)
        {
            const float depthScale = viewSize / (float)depthView_.width_;
            const int horzCenter = origin.y * windowSize.y;

            depthView_.sprite_.setScale(depthScale, depthScale);
            depthView_.sprite_.setPosition(origin.x * windowSize.x, horzCenter);
            window.draw(depthView_.sprite_);
        }

        if (colorView_.buffer_ != nullptr)
        {
            const float colorScale = viewSize / (float)colorView_.width_;
            const int horzCenter = origin.y * windowSize.y;

            colorView_.sprite_.setScale(colorScale, colorScale);

            if (overlayDepth_)
            {
                colorView_.sprite_.setPosition(origin.x * windowSize.x, horzCenter);
                colorView_.sprite_.setColor(sf::Color(255, 255, 255, 128));
            }
            else
            {
                colorView_.sprite_.setPosition(origin.x * windowSize.x + viewSize, horzCenter);
                colorView_.sprite_.setColor(sf::Color(255, 255, 255, 255));
            }

            window.draw(colorView_.sprite_);
        }
    }

    void toggle_depth_overlay()
    {
        overlayDepth_ = !overlayDepth_;
    }

    bool get_overlay_depth() const
    {
        return overlayDepth_;
    }

    void toggle_paused()
    {
        isPaused_ = !isPaused_;
    }

    bool is_paused() const
    {
        return isPaused_;
    }

    ColorMode get_mode() const { return colorMode_; }
    void set_mode(ColorMode mode) { colorMode_ = mode; }

private:
    samples::common::LitDepthVisualizer visualizer_;

    using DurationType = std::chrono::milliseconds;
    using ClockType = std::chrono::high_resolution_clock;

    ClockType::time_point prev_;
    float elapsedMillis_{.0f};

    StreamView depthView_;
    StreamView colorView_;
    ColorMode colorMode_;
    bool overlayDepth_{ false };
    bool isPaused_{ false };
};

astra::DepthStream configure_depth(astra::StreamReader& reader)
{
    auto depthStream = reader.stream<astra::DepthStream>();

    //We don't have to set the mode to start the stream, but if you want to here is how:
    astra::ImageStreamMode depthMode;

    depthMode.set_width(640);
    depthMode.set_height(480);
    depthMode.set_pixel_format(astra_pixel_formats::ASTRA_PIXEL_FORMAT_DEPTH_MM);
    depthMode.set_fps(30);

    depthStream.set_mode(depthMode);

    return depthStream;
}

astra::InfraredStream configure_ir(astra::StreamReader& reader, bool useRGB)
{
    auto irStream = reader.stream<astra::InfraredStream>();

    astra::ImageStreamMode irMode;
    irMode.set_width(640);
    irMode.set_height(480);

    if (useRGB)
    {
        irMode.set_pixel_format(astra_pixel_formats::ASTRA_PIXEL_FORMAT_RGB888);
    }
    else
    {
        irMode.set_pixel_format(astra_pixel_formats::ASTRA_PIXEL_FORMAT_GRAY16);
    }

    irMode.set_fps(30);
    irStream.set_mode(irMode);

    return irStream;
}

astra::ColorStream configure_color(astra::StreamReader& reader)
{
    auto colorStream = reader.stream<astra::ColorStream>();

    astra::ImageStreamMode colorMode;
    colorMode.set_width(640);
    colorMode.set_height(480);
    colorMode.set_pixel_format(astra_pixel_formats::ASTRA_PIXEL_FORMAT_RGB888);
    colorMode.set_fps(30);

    colorStream.set_mode(colorMode);

    return colorStream;
}

int main(int argc, char** argv)
{
    astra::initialize();

    set_key_handler();

    auto fullscreenStyle = sf::Style::Fullscreen;

    const sf::VideoMode fullScreenMode = sf::VideoMode::getFullscreenModes()[0];
    const sf::VideoMode windowedMode(1800, 675);

    bool isFullScreen = false;
    sf::RenderWindow window(windowedMode, "Stream Viewer");

    astra::StreamSet streamSet;
    astra::StreamReader reader = streamSet.create_reader();

    reader.stream<astra::PointStream>().start();

    auto depthStream = configure_depth(reader);
    depthStream.start();

    auto colorStream = configure_color(reader);
    colorStream.start();

    auto irStream = configure_ir(reader, false);

    MultiFrameListener listener;
    listener.set_mode(MODE_COLOR);

    reader.add_listener(listener);

    while (window.isOpen())
    {
        astra_temp_update();

        sf::Event event;
        while (window.pollEvent(event))
        {
            switch (event.type)
            {
            case sf::Event::Closed:
                window.close();
                break;
            case sf::Event::KeyPressed:
                {
                    switch (event.key.code)
                    {
                    case sf::Keyboard::Escape:
                        window.close();
                        break;
                    case sf::Keyboard::F:
                        if (isFullScreen)
                        {
                            isFullScreen = false;
                            window.create(windowedMode, "Stream Viewer", sf::Style::Default);
                        }
                        else
                        {
                            isFullScreen = true;
                            window.create(fullScreenMode, "Stream Viewer", fullscreenStyle);
                        }
                        break;
                    case sf::Keyboard::R:
                        depthStream.enable_registration(!depthStream.registration_enabled());
                        break;
                    case sf::Keyboard::M:
                        {
                            const bool newMirroring = !depthStream.mirroring_enabled();
                            depthStream.enable_mirroring(newMirroring);
                            colorStream.enable_mirroring(newMirroring);
                            irStream.enable_mirroring(newMirroring);
                        }
                        break;
                    case sf::Keyboard::G:
                        colorStream.stop();
                        configure_ir(reader, false);
                        listener.set_mode(MODE_IR_16);
                        irStream.start();
                        break;
                    case sf::Keyboard::I:
                        colorStream.stop();
                        configure_ir(reader, true);
                        listener.set_mode(MODE_IR_RGB);
                        irStream.start();
                        break;
                    case sf::Keyboard::O:
                        listener.toggle_depth_overlay();
                        if (listener.get_overlay_depth())
                        {
                            depthStream.enable_registration(true);
                        }
                        break;
                    case sf::Keyboard::P:
                        listener.toggle_paused();
                        break;
                    case sf::Keyboard::C:
                        if (event.key.control)
                        {
                            window.close();
                        }
                        else
                        {
                            irStream.stop();
                            listener.set_mode(MODE_COLOR);
                            colorStream.start();
                        }
                        break;
                    default:
                        break;
                    }
                    break;
                }
            default:
                break;
            }
        }

        // clear the window with black color
        window.clear(sf::Color::Black);
        listener.draw_to(window, sf::Vector2f(0.f, 0.f), sf::Vector2f(window.getSize().x, window.getSize().y));
        window.display();

        if (!shouldContinue)
        {
            window.close();
        }
    }

    astra::terminate();
    return 0;
}
