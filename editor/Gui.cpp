//
// Created by Steve Wheeler on 06/05/2024.
//

#include "Gui.hpp"
#include "windows/FloatingWindow.hpp"
#include "UserInput.hpp"

#include "raygui.h"


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

void GUI::onWindowResize(Vector2 newScreenSize)
{
    toolbox->position = { 10, 135 };
    toolbox->size = { 200, 400 };
    toolbox->content_size = { 140, 320 };
    
    objectprops->position = { newScreenSize.x - 200 - 10, 100 };
    objectprops->size = { 200,  newScreenSize.y/2 - 100 };
    objectprops->content_size = { 140, 320 };

    toolprops->position = { newScreenSize.x - 200 - 10, 100 +  newScreenSize.y/2 - 100 };
    toolprops->size = { 200,  newScreenSize.y/2 - 100 };
    toolprops->content_size = { 140, 320 };
}

GUI::GUI(Settings* _settings, UserInput* _userInput) :
    settings(_settings)
{
    _userInput->dOnWindowUpdate.connect<&GUI::onWindowResize>(this);
    toolbox = std::make_unique<FloatingWindow>(FloatingWindow({ 10, 135 },
                                                              { 200, 400 },
                                                              { 140, 320 },
                                                              "Toolbox"));
    objectprops = std::make_unique<FloatingWindow>(FloatingWindow({ static_cast<float>(settings->SCREEN_WIDTH - 200 - 10), 100 },
                                                                  { 200,  static_cast<float>(settings->SCREEN_HEIGHT)/2 - 100 },
                                                                  { 140, 320 },
                                                                  "Object Properties"));
    toolprops = std::make_unique<FloatingWindow>(FloatingWindow({ static_cast<float>(settings->SCREEN_WIDTH - 200 - 10), 100 +  static_cast<float>(settings->SCREEN_HEIGHT)/2 - 100},
                                                              { 200,  static_cast<float>(settings->SCREEN_HEIGHT)/2 - 100 },
                                                              { 140, 320 },
                                                              "Tool Properties"));
    windows.push_back(toolbox.get());
    windows.push_back(objectprops.get());
    windows.push_back(toolprops.get());
}
} // sage