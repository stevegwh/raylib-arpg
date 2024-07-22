//
// Created by Steve Wheeler on 22/07/2024.
//

#pragma once

#include <entt/entt.hpp>

namespace sage
{

	class Timer
	{
		entt::delegate<void()> callback;
		float duration = 0;
		float timer = 0;
	public:
		void Update(float dt);
		Timer(entt::delegate<void()> _callback, float _duration);

	};

} // sage
