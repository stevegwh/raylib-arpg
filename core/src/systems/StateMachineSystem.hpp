//
// Created by Steve Wheeler on 07/06/2024.
//

#pragma once

#include "BaseSystem.hpp"
#include "components/states/StateMachineComponent.hpp"

#include <entt/entt.hpp>

namespace sage
{
	class StateMachineSystem : BaseSystem<StateMachineComponent>
	{
		template <typename Tuple, size_t... Indices>
		void RemoveStateComponents(entt::entity entity, std::index_sequence<Indices...>)
		{
			(RemoveStateComponent<std::tuple_element_t<Indices, Tuple>>(entity), ...);
		}

		template <typename Tuple>
		void RemoveStateComponents(entt::entity entity)
		{
			RemoveStateComponents<Tuple>(entity, std::make_index_sequence<std::tuple_size_v<Tuple>>{});
		}

		template <typename StateComponent>
		void RemoveStateComponent(entt::entity entity)
		{
			if (registry->any_of<StateComponent>(entity))
			{
				registry->get<StateComponent>(entity).Disable(entity);
				registry->remove<StateComponent>(entity);
			}
		}

	public:
		template <typename NewStateComponent, typename StateComponentsTuple>
		void ChangeState(entt::entity entity)
		{
			// Check if the entity already has the NewStateComponent
			if (registry->any_of<NewStateComponent>(entity)) return;


			// Remove any existing state components
			RemoveStateComponents<StateComponentsTuple>(entity);

			// Emplace the new component
			auto& newComponent = registry->emplace<NewStateComponent>(entity);
			newComponent.Enable(entity);
		}

		explicit StateMachineSystem(entt::registry* _registry);
	};
} // sage
