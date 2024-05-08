//
// Created by Steve Wheeler on 06/05/2024.
//

#pragma once

#include "windows/Window.hpp"
#include "windows/FloatingWindow.hpp"
#include "Settings.hpp"
#include "UserInput.hpp"

#include <entt/entt.hpp>

#include <vector>
#include <memory>


namespace sage::editor
{
class GUI
{
    std::unique_ptr<FloatingWindow> objectprops;
    std::unique_ptr<FloatingWindow> toolprops;
    std::unique_ptr<FloatingWindow> toolbox;
    std::vector<Window*> windows;
    Settings* settings;
    void onWindowResize(Vector2 newScreenSize);
public:
    entt::delegate<void()> saveButtonPressed{};
    entt::delegate<void()> loadButtonPressed{};
    void Update();
    void Draw();
    GUI(Settings* _settings, UserInput* _userInput);
};

} // sage
// editor
