//
// Created by steve on 02/10/2024.
//

#pragma once

#include "raylib.h"

#include <entt/entt.hpp>

namespace sage
{
    struct Settings;
    class UserInput;
    class Cursor;

    struct Window
    {
        int id;
        Vector2 pos;
        Texture tex;

        entt::sigh<void(int)> onWindowStartHover;
        entt::sigh<void(int)> onWindowEndHover;
    };

    struct Panel
    {
        int id;
        Vector2 pos;
        Texture tex;
    };

    struct Button
    {
        int id;
        Texture tex;
        Vector2 pos;
        float w;
        float h;
        entt::sigh<void(int)> onButtonStartHover;
        entt::sigh<void(int)> onButtonEndHover;
        entt::sigh<void(int)> onButtonPress;
    };

    class GameUIEngine
    {
      public:
        void CreateWindow(Vector2 pos);
        void CreatePanel(Vector2 pos, Window* parent);
        void CreateButton(Vector2 pos, Window* parent);

        GameUIEngine(Settings _settings, UserInput* _userInput, Cursor* _cursor);
    };
} // namespace sage
