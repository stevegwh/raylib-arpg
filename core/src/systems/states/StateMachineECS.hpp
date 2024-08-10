//
// Created by Steve Wheeler on 07/06/2024.
//

#pragma once

#include "../BaseSystem.hpp"

#include <entt/entt.hpp>

#include <algorithm>
#include <vector>

namespace sage
{
    template <typename Derived, typename DerivedStateComponent>
    class StateMachineECS : public BaseSystem
    {
        std::vector<entt::entity> lockedEntities;

        template <typename Tuple, size_t... Indices>
        void RemoveStateComponents(entt::entity entity, std::index_sequence<Indices...>)
        {
            (RemoveStateComponent<std::tuple_element_t<Indices, Tuple>>(entity), ...);
        }

        template <typename Tuple>
        void RemoveStateComponents(entt::entity entity)
        {
            RemoveStateComponents<Tuple>(
                entity, std::make_index_sequence<std::tuple_size_v<Tuple>>{});
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

      protected:
        void LockState(entt::entity entity)
        {
            lockedEntities.push_back(entity);
        }
        void UnlockState(entt::entity entity)
        {
            lockedEntities.erase(
                std::remove(lockedEntities.begin(), lockedEntities.end(), entity),
                lockedEntities.end());
        }
        template <typename NewStateComponent, typename StateComponentsTuple>
        void ChangeState(entt::entity entity)
        {
            if (std::find(lockedEntities.begin(), lockedEntities.end(), entity) !=
                lockedEntities.end())
            {
                return;
            }
            // Check if the entity already has the NewStateComponent
            if (registry->any_of<NewStateComponent>(entity)) return;

            // Remove any existing state components
            RemoveStateComponents<StateComponentsTuple>(entity);

            // Emplace the new component
            auto& newComponent = registry->emplace<NewStateComponent>(entity);
            newComponent.Enable(entity);
        }

      public:
        virtual void OnStateEnter(entt::entity entity) {};
        virtual void OnStateExit(entt::entity entity) {};
        void Update() override {};
        void Draw3D() override {};

        explicit StateMachineECS(entt::registry* _registry) : BaseSystem(_registry)
        {
            registry->template on_construct<DerivedStateComponent>()
                .template connect<&Derived::OnStateEnter>(static_cast<Derived*>(this));
            registry->template on_destroy<DerivedStateComponent>()
                .template connect<&Derived::OnStateExit>(static_cast<Derived*>(this));
        }
    };
} // namespace sage
