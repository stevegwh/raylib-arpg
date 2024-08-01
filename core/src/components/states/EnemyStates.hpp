//
// Created by Steve Wheeler on 01/08/2024.
//

#pragma once
#include "StateMachineComponent.hpp"

namespace sage
{
	class StateEnemyDefault : public StateMachineComponent
	{
	public:
		~StateEnemyDefault() override = default;
	};

	class StateEnemyCombat : public StateMachineComponent
	{
	public:
		~StateEnemyCombat() override = default;
	};
} // sage