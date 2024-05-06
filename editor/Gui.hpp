//
// Created by Steve Wheeler on 06/05/2024.
//

#pragma once

#include <vector>

#include "windows/Window.hpp"

namespace sage::editor
{

class GUI
{
    std::vector<Window> windows;
    
public:
    void Update();
    void Draw();
};

} // sage
// editor
