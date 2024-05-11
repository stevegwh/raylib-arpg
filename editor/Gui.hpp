//
// Created by Steve Wheeler on 06/05/2024.
//

#pragma once

#include "windows/Window.hpp"
#include "windows/FloatingWindow.hpp"
#include "Settings.hpp"
#include "UserInput.hpp"
#include "Cursor.hpp"

#include <entt/entt.hpp>

#include <vector>
#include <memory>
#include <string>


namespace sage::editor
{
class GUI
{
    Camera* camera;
    std::unique_ptr<FloatingWindow> objectprops;
    std::unique_ptr<FloatingWindow> toolprops;
    std::unique_ptr<FloatingWindow> toolbox;
    std::vector<Window*> windows;
    Vector2 screenSize;
    Settings* settings;
    void onWindowResize(Vector2 newScreenSize);
    static void drawDebugCollisionText(Cursor* cursor);
public:
    bool focused = false;
    entt::sigh<void()> saveButtonPressed;
    entt::sigh<void()> loadButtonPressed;
    void Update();
    void Draw(const std::string& mode, Cursor* cursor);
    GUI(Settings* _settings, UserInput* _userInput, Camera* camera);
    void MarkGUIActive();
    void MarkGUIInactive();
};

} // sage
// editor
