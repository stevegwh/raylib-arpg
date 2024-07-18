//
// Created by Steve Wheeler on 07/06/2024.
//

#pragma once
#include "StateMachineComponent.hpp"

namespace sage
{
	class StatePlayerCombat : public StateMachineComponent
	{
	public:
		~StatePlayerCombat() override = default;
		entt::sigh<void(entt::entity)> onAttackCancel{}; // entity id of this component
	};
} // sage
