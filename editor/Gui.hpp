//
// Created by Steve Wheeler on 06/05/2024.
//

#pragma once

#include "windows/Window.hpp"
#include "Settings.hpp"

#include <entt/entt.hpp>

#include <vector>
#include <memory>



namespace sage::editor
{

class GUI
{
    std::vector<std::unique_ptr<Window>> windows;
    Settings* settings;
    
public:
    entt::delegate<void()> saveButtonPressed{};
    entt::delegate<void()> loadButtonPressed{};
    void Update();
    void Draw();
    GUI(Settings* _settings);
};

} // sage
// editor
