//
// Created by Steve Wheeler on 01/08/2024.
//

#pragma once

#include "StateMachineComponent.hpp"
#include <tuple>

namespace sage
{
	class StateGameDefault : public StateMachineComponent
	{
	public:
		~StateGameDefault() override = default;
	};

	class StateGameWaveattack : public StateMachineComponent
	{
	public:
		~StateGameWaveattack() override = default;
	};
	using GameStates = std::tuple<StateGameDefault, StateGameWaveattack>;
} // sage