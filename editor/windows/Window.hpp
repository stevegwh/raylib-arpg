//
// Created by Steve Wheeler on 06/05/2024.
//

#pragma once

#include "raylib.h"

#include "entt/entt.hpp"
#include "Event.hpp"

#include <string>

namespace sage::editor
{
    class Window
    {
      protected:
        virtual void DrawContent() = 0;

      public:
        std::string title;
        Vector2 content_size{};
        Vector2 position = {10, 10};
        Vector2 size = {200, 400};
        bool minimized = false;
        bool moving = false;
        bool resizing = false;
        bool hovering = false;
        Vector2 scroll{};

        Event<> onWindowHover{};
        Event<> onWindowHoverStop{};

        virtual void Update() = 0;

        virtual ~Window() = default;
        Window(Vector2 _position, Vector2 _size, Vector2 _content_size, std::string _title);
    };
} // namespace sage::editor
// editor
