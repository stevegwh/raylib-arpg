//
// Created by steve on 02/10/2024.
//

#include "GameUiEngine.hpp"

namespace sage
{

    int GameUIEngine::CreateWindow(Vector2 pos, float w, float h)
    {
        Window window;
        window.id = nextId++;
        window.nPatchInfo = {Rectangle{0.0f, 0.0f, 64.0f, 64.0f}, 12, 40, 12, 12, NPATCH_NINE_PATCH};
        window.tex = nPatchTexture;
        window.pos = pos;
        window.rec = {pos.x, pos.y, w, h};
        windows.push_back(window);
        return window.id;
    }

    int GameUIEngine::CreatePanel(Vector2 pos, float w, float h, int parent)
    {
        Panel panel;
        panel.id = nextId++;
        panel.parent = GetWindowById(parent);
        panel.pos = pos;
        panel.rec = {panel.parent->pos.x + pos.x, panel.parent->pos.y + pos.y, w, h};
        panel.tex = LoadTexture("resources/textures/panel_background.png"); // Example texture
        GetWindowById(parent)->children.push_back(panel);
        return panel.id;
    }

    int GameUIEngine::CreateTextbox(const std::string& content, Vector2 pos, float w, float h, int parent)
    {
        TextBox textbox;
        textbox.id = nextId++;
        textbox.parent = parent;
        textbox.fontSize = 10; // Default font size
        textbox.pos = pos;
        textbox.rec = {GetWindowById(parent)->pos.x + pos.x, GetWindowById(parent)->pos.y + pos.y, w, h};
        textbox.content = content;
        textboxes.push_back(textbox);
        return textbox.id;
    }

    int GameUIEngine::CreateButton(Texture tex, Vector2 pos, float w, float h, int parent)
    {
        Button button;
        button.id = nextId++;
        button.parent = parent;
        button.tex = tex;
        button.pos = pos;
        button.w = w;
        button.h = h;
        button.rec = {GetWindowById(parent)->pos.x + pos.x, GetWindowById(parent)->pos.y + pos.y, w, h};
        buttons.push_back(button);
        return button.id;
    }

    Window* GameUIEngine::GetWindowById(int id)
    {
        for (auto& window : windows)
        {
            if (window.id == id)
            {
                return &window;
            }
        }
        return nullptr; // Return nullptr if window not found
    }

    void GameUIEngine::Draw2D()
    {
        for (auto& window : windows)
        {
            DrawTextureNPatch(window.tex, window.nPatchInfo, window.rec, window.pos, 0.0f, WHITE);

            for (auto& child : window.children)
            {
                if (child.id < 0) continue;                             // Skip invalid IDs
                DrawTextureRec(child.tex, child.rec, child.pos, WHITE); // Draw panel background
            }
        }

        // Draw textboxes on top of panels
        for (auto& textbox : textboxes)
        {
            if (textbox.id < 0) continue; // Skip invalid IDs
            DrawText(textbox.content.c_str(), textbox.rec.x, textbox.rec.y, textbox.fontSize, BLACK);
        }

        // Draw buttons on top
        for (auto& button : buttons)
        {
            if (button.id < 0) continue; // Skip invalid IDs
            DrawTextureRec(button.tex, {0, 0, button.w, button.h}, button.pos, WHITE);
        }
    }

    void GameUIEngine::Update()
    {
        // Handle input and update UI state here (e.g., button clicks, hover effects)
    }

    GameUIEngine::GameUIEngine(Settings* _settings, UserInput* _userInput, Cursor* _cursor)
        : nextId(1) // Initialize nextId to 1
    {
        nPatchTexture = LoadTexture("resources/textures/ninepatch_button.png");
    }

} // namespace sage