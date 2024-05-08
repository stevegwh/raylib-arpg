//
// Created by Steve Wheeler on 06/05/2024.
//

#include "Gui.hpp"
#include "raygui.h"
#include "windows/FloatingWindow.hpp"

namespace sage::editor
{

void GUI::Draw()
{
    float modifier = 100;
    GuiGroupBox((Rectangle){ 0 + modifier, 8, 184, 40 }, NULL);
    if(GuiButton((Rectangle){ 8 + modifier, 8, 24, 24 }, "#002#"))
    {
        if (saveButtonPressed) saveButtonPressed();
    }
    if (GuiButton((Rectangle){ 40 + modifier, 8, 24, 24 }, "#001#"))
    {
        if (loadButtonPressed) loadButtonPressed();
    }

    for (auto& window : windows)
    {
        window->Update();
    }
}

GUI::GUI(Settings* _settings) :
    settings(_settings)
{
    windows.push_back(std::make_unique<FloatingWindow>(FloatingWindow({ static_cast<float>(settings->SCREEN_WIDTH - 10), 10 }, { 200, 400 },
                                                                      { 140, 320 }, "Window 1")));
    windows.push_back(std::make_unique<FloatingWindow>(FloatingWindow({ static_cast<float>(settings->SCREEN_WIDTH - 250), 10 }, { 200, 400 },
                                                                      { 140, 320 }, "Window 1")));
}
} // sage