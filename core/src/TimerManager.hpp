//
// Created by Steve Wheeler on 22/07/2024.
//

#pragma once

#include "Timer.hpp"

#include <entt/entt.hpp>

#include <vector>
#include <memory>

namespace sage
{

	class TimerManager
	{
		std::vector<std::unique_ptr<Timer>> timers;
	public:
		void Update();
		int AddTimer(float duration, entt::delegate<void()> callback);
		void RemoveTimer(int id);
	};

} // sage
