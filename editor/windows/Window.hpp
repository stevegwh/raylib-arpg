//
// Created by Steve Wheeler on 06/05/2024.
//

#pragma once

#include "raylib.h"

#include <string>


namespace sage::editor
{

class Window
{
protected:
    virtual void DrawContent() = 0;
public:
    std::string title;
    Vector2 content_size{};
    Vector2 position = { 10, 10 };
    Vector2 size = { 200, 400 };
    bool minimized = false;
    bool moving = false;
    bool resizing = false;
    Vector2 scroll{};
    
    virtual void Update() = 0;
    
    Window(Vector2 _position, Vector2 _size, Vector2 _content_size, std::string _title);

};

} // sage
// editor
