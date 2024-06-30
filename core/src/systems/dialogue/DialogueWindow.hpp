//
// Created by Steve Wheeler on 12/05/2024.
//

#pragma once

#include "Settings.hpp"

#include "raylib.h"

namespace sage
{
	class DialogueWindow
	{
		Settings* settings;
		Vector2 windowPosition{};
		Vector2 contentSize;

	public:
		void Update();
		void Draw();
		DialogueWindow(Settings* _settings);
	};
} // sage
