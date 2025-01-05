//
// Created by Steve Wheeler on 06/05/2024.
//

#include "Window.hpp"

#include <utility>

namespace sage::editor
{
    Window::Window(Vector2 _position, Vector2 _size, Vector2 _content_size, std::string _title)
        : title(std::move(_title)),
          content_size(_content_size),
          position(_position),
          size(_size),
          onWindowHover(std::make_unique<Event<>>()),
          onWindowHoverStop(std::make_unique<Event<>>())
    {
    }
} // namespace sage::editor
// editor
