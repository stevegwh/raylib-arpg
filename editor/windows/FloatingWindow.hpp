//
// Created by Steve Wheeler on 06/05/2024.
//

#pragma once

#include "Window.hpp"

#include <string>

namespace sage::editor
{
	struct FloatingWindow : Window
	{
		void Update() override;
		void DrawContent() override;
		FloatingWindow(Vector2 _position, Vector2 _size, Vector2 _content_size, const std::string& _title);
	};
} // editor
