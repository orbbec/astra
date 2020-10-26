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
#include <astra/astra.hpp>
#include "LitDepthVisualizer.hpp"
#include <chrono>
#include <iostream>
#include <iomanip>
#include <key_handler.h>
#include <sstream>

class DepthFrameListener : public astra::FrameListener
{
public:
    DepthFrameListener(const astra::CoordinateMapper& coordinateMapper)
        : coordinateMapper_(coordinateMapper)
    {
        prev_ = ClockType::now();
        font_.loadFromFile("Inconsolata.otf");
    }

    void init_texture(int width, int height)
    {
        if (!displayBuffer_ ||
            width != displayWidth_ ||
            height != displayHeight_)
        {
            displayWidth_ = width;
            displayHeight_ = height;

            // texture is RGBA
            const int byteLength = displayWidth_ * displayHeight_ * 4;

            displayBuffer_ = BufferPtr(new uint8_t[byteLength]);
            std::fill(&displayBuffer_[0], &displayBuffer_[0] + byteLength, 0);

            texture_.create(displayWidth_, displayHeight_);
            sprite_.setTexture(texture_);
            sprite_.setPosition(0, 0);
        }
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

    void on_frame_ready(astra::StreamReader& reader,
                        astra::Frame& frame) override
    {
        const astra::PointFrame pointFrame = frame.get<astra::PointFrame>();

        const int width = pointFrame.width();
        const int height = pointFrame.height();

        init_texture(width, height);

        check_fps();

        if (isPaused_) { return; }

        copy_depth_data(frame);

        visualizer_.update(pointFrame);

        const astra::RgbPixel* vizBuffer = visualizer_.get_output();
        for (int i = 0; i < width * height; i++)
        {
            const int rgbaOffset = i * 4;
            displayBuffer_[rgbaOffset] = vizBuffer[i].r;
            displayBuffer_[rgbaOffset + 1] = vizBuffer[i].b;
            displayBuffer_[rgbaOffset + 2] = vizBuffer[i].g;
            displayBuffer_[rgbaOffset + 3] = 255;
        }

        texture_.update(displayBuffer_.get());
    }

    void copy_depth_data(astra::Frame& frame)
    {
        const astra::DepthFrame depthFrame = frame.get<astra::DepthFrame>();

        if (depthFrame.is_valid())
        {
            const int width = depthFrame.width();
            const int height = depthFrame.height();
            if (!depthData_ || width != depthWidth_ || height != depthHeight_)
            {
                depthWidth_ = width;
                depthHeight_ = height;

                // texture is RGBA
                const int byteLength = depthWidth_ * depthHeight_ * sizeof(uint16_t);

                depthData_ = DepthPtr(new int16_t[byteLength]);
            }

            depthFrame.copy_to(&depthData_[0]);
        }
    }

    void update_mouse_position(sf::RenderWindow& window)
    {
        const sf::Vector2i position = sf::Mouse::getPosition(window);
        const sf::Vector2u windowSize = window.getSize();

        mouseNormX_ = position.x / float(windowSize.x);
        mouseNormY_ = position.y / float(windowSize.y);
    }

    void draw_text(sf::RenderWindow& window,
                   sf::Text& text,
                   sf::Color color,
                   const int x,
                   const int y) const
    {
        text.setColor(sf::Color::Black);
        text.setPosition(x + 5, y + 5);
        window.draw(text);

        text.setColor(color);
        text.setPosition(x, y);
        window.draw(text);
    }

    void draw_mouse_overlay(sf::RenderWindow& window,
                            const float depthWScale,
                            const float depthHScale) const
    {
        if (!isMouseOverlayEnabled_ || !depthData_) { return; }

        const int mouseX = depthWidth_ * mouseNormX_;
        const int mouseY = depthHeight_ * mouseNormY_;

        if (mouseX >= depthWidth_ ||
            mouseY >= depthHeight_ ||
            mouseX < 0 ||
            mouseY < 0) { return; }

        const size_t index = (depthWidth_ * mouseY + mouseX);
        const short z = depthData_[index];

        float worldX, worldY, worldZ;
        coordinateMapper_.convert_depth_to_world(float(mouseX),
                                                 float(mouseY),
                                                 float(z),
                                                 &worldX,
                                                 &worldY,
                                                 &worldZ);

        std::stringstream str;
        str << std::fixed
            << std::setprecision(0)
            << "(" << mouseX << ", " << mouseY << ") "
            << "X: " << worldX << " Y: " << worldY << " Z: " << worldZ;

        const int characterSize = 40;
        sf::Text text(str.str(), font_);
        text.setCharacterSize(characterSize);
        text.setStyle(sf::Text::Bold);

        const float displayX = 10.f;
        const float margin = 10.f;
        const float displayY = window.getView().getSize().y - (margin + characterSize);

        draw_text(window, text, sf::Color::White, displayX, displayY);
    }

    void draw_to(sf::RenderWindow& window)
    {
        if (displayBuffer_ != nullptr)
        {
            const float depthWScale = window.getView().getSize().x / displayWidth_;
            const float depthHScale = window.getView().getSize().y / displayHeight_;

            sprite_.setScale(depthWScale, depthHScale);
            window.draw(sprite_);

            draw_mouse_overlay(window, depthWScale, depthHScale);
        }
    }

    void toggle_paused()
    {
        isPaused_ = !isPaused_;
    }

    bool is_paused() const
    {
        return isPaused_;
    }

    void toggle_overlay()
    {
        isMouseOverlayEnabled_ = !isMouseOverlayEnabled_;
    }

    bool overlay_enabled() const
    {
        return isMouseOverlayEnabled_;
    }

private:
    samples::common::LitDepthVisualizer visualizer_;

    using DurationType = std::chrono::milliseconds;
    using ClockType = std::chrono::high_resolution_clock;

    ClockType::time_point prev_;
    float elapsedMillis_{.0f};

    sf::Texture texture_;
    sf::Sprite sprite_;
    sf::Font font_;

    const astra::CoordinateMapper& coordinateMapper_;

    int displayWidth_{0};
    int displayHeight_{0};

    using BufferPtr = std::unique_ptr<uint8_t[]>;
    BufferPtr displayBuffer_{nullptr};

    int depthWidth_{0};
    int depthHeight_{0};

    using DepthPtr = std::unique_ptr<int16_t[]>;
    DepthPtr depthData_{nullptr};

    float mouseNormX_{0};
    float mouseNormY_{0};
    bool isPaused_{false};
    bool isMouseOverlayEnabled_{true};
};

int main(int argc, char** argv)
{
    astra::initialize();

    set_key_handler();

    sf::RenderWindow window(sf::VideoMode(1280, 960), "Depth Viewer");

    auto fullscreenStyle = sf::Style::Fullscreen;

    const sf::VideoMode fullScreenMode = sf::VideoMode::getFullscreenModes()[0];
    const sf::VideoMode windowedMode(1280, 1024);
    bool isFullScreen = false;

    astra::StreamSet streamSet;
    astra::StreamReader reader = streamSet.create_reader();
    reader.stream<astra::PointStream>().start();

    auto depthStream = reader.stream<astra::DepthStream>();
    depthStream.start();

    auto coordinateMapper = depthStream.coordinateMapper();
    DepthFrameListener listener(coordinateMapper);

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
                if (event.key.code == sf::Keyboard::C && event.key.control)
                {
                    window.close();
                }
                switch (event.key.code)
                {
                case sf::Keyboard::Escape:
                    window.close();
                    break;
                case sf::Keyboard::F:
                    if (isFullScreen)
                    {
                        window.create(windowedMode, "Depth Viewer", sf::Style::Default);
                    }
                    else
                    {
                        window.create(fullScreenMode, "Depth Viewer", fullscreenStyle);
                    }
                    isFullScreen = !isFullScreen;
                    break;
                case sf::Keyboard::R:
                    depthStream.enable_registration(!depthStream.registration_enabled());
                    break;
                case sf::Keyboard::M:
                    depthStream.enable_mirroring(!depthStream.mirroring_enabled());
                    break;
                case sf::Keyboard::P:
                    listener.toggle_paused();
                    break;
                case sf::Keyboard::Space:
                    listener.toggle_overlay();
                    break;
                default:
                    break;
                }
                break;
            }
            case sf::Event::MouseMoved:
            {
                listener.update_mouse_position(window);
                break;
            }
            default:
                break;
            }
        }

        // clear the window with black color
        window.clear(sf::Color::Black);

        listener.draw_to(window);
        window.display();

        if (!shouldContinue)
        {
            window.close();
        }
    }

    astra::terminate();

    return 0;
}
