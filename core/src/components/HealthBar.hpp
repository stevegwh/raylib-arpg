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
		int hp = 100;
		RenderTexture2D healthBarTexture;
		const int healthBarWidth = 200;
		const int healthBarHeight = 20;
		const Color healthBarColor = RED;
		const Color healthBarBgColor = BLACK;
		const Color healthBarBorderColor = MAROON;
		void Decrement(entt::entity entity, int value);
		void Increment(entt::entity entity, int value);
		void UpdateTexture() const;
		entt::sigh<void(entt::entity)> onHealthUpdate{};
		entt::sigh<void(entt::entity)> onHealthIsZero{};
		HealthBar();
		~HealthBar();
	};
}
