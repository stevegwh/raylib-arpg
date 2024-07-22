//
// Created by Steve Wheeler on 22/07/2024.
//

#include "Timer.hpp"

namespace sage
{

	void Timer::Update(float dt)
	{
		timer += dt;
		if (timer >= duration)
		{
			timer = 0;
			callback();
		}
	}

	Timer::Timer(entt::delegate<void()> _callback, float _duration)
			: callback(_callback), duration(_duration)
	{
	}
} // sage