//
// Created by Steve Wheeler on 22/07/2024.
//

#include "TimerManager.hpp"

#include "raylib.h"

namespace sage
{
	void TimerManager::Update()
	{
		float dt = GetFrameTime();
		for (auto& timer : timers)
		{
			timer->Update(dt);
		}
	}

	int TimerManager::AddTimer(float duration, entt::delegate<void()> callback)
	{
		timers.push_back(std::make_unique<Timer>(callback, duration));
		return timers.size() - 1;
	}
	
	// Probably not the best way of doing this (map would be better)
	void TimerManager::RemoveTimer(int id)
	{
		timers.erase(timers.begin() + id);
	}
} // sage