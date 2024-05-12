//
// Created by Steve Wheeler on 12/05/2024.
//

#include "DialogueWindow.hpp"
#include "raygui.h"

namespace sage
{
void DialogueWindow::Update()
{

}
void DialogueWindow::Draw()
{
    GuiWindowBox({ static_cast<float>(settings->SCREEN_WIDTH) - contentSize.x,
                   static_cast<float>(settings->SCREEN_HEIGHT) - contentSize.y, 
                   contentSize.x, contentSize.y }, "#198# PORTABLE WINDOW");
}

DialogueWindow::DialogueWindow(Settings* _settings) :
settings(_settings), contentSize({ 300, 200 })
{}
} // sage