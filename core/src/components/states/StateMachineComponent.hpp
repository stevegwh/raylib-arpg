//
// Created by Steve Wheeler on 07/06/2024.
//

#include <entt/entt.hpp>

#pragma once

namespace sage
{
	struct StateMachineComponent
	{
		entt::sigh<void(entt::entity)> onEnable;
		entt::sigh<void(entt::entity)> onDisable;

		virtual void Enable(entt::entity entity)
		{
			onEnable.publish(entity);
		}

		virtual void Disable(entt::entity entity)
		{
			onDisable.publish(entity);
		}

		virtual ~StateMachineComponent() = default;
	};
}
