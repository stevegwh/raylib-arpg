//
// Created by steve on 02/10/2024.
//

#pragma once

#include "raylib.h"

#include <entt/entt.hpp>
#include <vector>

namespace sage
{
    struct Settings;
    class UserInput;
    class Cursor;

    struct Panel;

    struct Window
    {
        int id;
        Vector2 pos;
        Rectangle rec;
        Texture tex;
        NPatchInfo nPatchInfo;
        entt::sigh<void(int)> onWindowStartHover;
        entt::sigh<void(int)> onWindowEndHover;
        std::vector<Panel> children;
    };

    struct Panel
    {
        int id;
        Window* parent;
        Vector2 pos;
        Rectangle rec;
        Texture tex;
    };

    struct TextBox
    {
        int id;
        int parent;
        float fontSize;
        Vector2 pos;
        Rectangle rec;
        // font?
        // color?
        std::string content;
        // Text wrap, scroll bar etc?
    };

    struct Button
    {
        int id;
        int parent;
        Texture tex;
        Vector2 pos;
        Rectangle rec;
        float w;
        float h;
        entt::sigh<void(int)> onButtonStartHover;
        entt::sigh<void(int)> onButtonEndHover;
        entt::sigh<void(int)> onButtonPress;
    };

    class GameUIEngine
    {
        Texture nPatchTexture;
        std::vector<Window> windows;
        std::vector<TextBox> textboxes;
        std::vector<Button> buttons;
        int nextId;

      public:
        int CreateWindow(Vector2 pos, float w, float h);
        int CreatePanel(Vector2 pos, float w, float h, int parent);
        int CreateTextbox(const std::string& content, Vector2 pos, float w, float h, int parent);
        int CreateButton(Texture tex, Vector2 pos, float w, float h, int parent);

        void Draw2D();
        void Update();

        GameUIEngine(Settings* _settings, UserInput* _userInput, Cursor* _cursor);

      private:
        Window* GetWindowById(int id); // Add this line
    };
} // namespace sage
