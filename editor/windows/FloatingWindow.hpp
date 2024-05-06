//
// Created by Steve Wheeler on 06/05/2024.
//

#pragma once

#include "Window.hpp"

#include <string>

namespace sage::editor
{

struct FloatingWindow : public Window
{
    void Update();
    void DrawContent() override;
    FloatingWindow(Vector2 _position, Vector2 _size, Vector2 _content_size, const std::string& _title);
    
    // GuiWindowFloating(&window_position, &window_size, &minimized, &moving, &resizing, &DrawContent, (Vector2) { 140, 320 }, &scroll, "Movable & Scalable Window");
    //    GuiWindowFloating(&window2_position, &window2_size, &minimized2, &moving2, &resizing2, &DrawContent, (Vector2) { 140, 320 }, &scroll2, "Another window");
};

} // editor
