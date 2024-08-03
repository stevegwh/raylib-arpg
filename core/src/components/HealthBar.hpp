//
// Created by steve on 20/05/2024.
//

#pragma once

#include <entt/entt.hpp>

#include "raylib.h"

namespace sage
{
	struct HealthBar
	{
		int* hp;
		float damageTaken = 0;
		RenderTexture2D healthBarTexture{};
		const int healthBarWidth = 200;
		const int healthBarHeight = 20;
		const Color healthBarColor = RED;
		const Color healthBarBgColor = BLACK;
		const Color healthBarBorderColor = MAROON;
		void Decrement(int value);
		void Increment(int value);
		void Update();
		HealthBar(int* _hp);
		~HealthBar();
	};
}
