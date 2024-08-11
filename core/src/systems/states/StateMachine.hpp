//
// Created by Steve Wheeler on 11/08/2024.
//

#pragma once

#include <entt/entt.hpp>

#include <vector>

namespace sage
{

    class StateMachine
    {
        std::vector<entt::entity> lockedEntities;

      protected:
        entt::registry* registry;

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
        virtual ~StateMachine() = default;

        explicit StateMachine(entt::registry* _registry) : registry(_registry)
        {
        }

      public:
        virtual void OnStateEnter(entt::entity entity) {};
        virtual void OnStateExit(entt::entity entity) {};
        virtual void Update(entt::entity entity) {};
        virtual void Draw3D(entt::entity entity) {};
    };

    template <typename Derived, typename StateName, typename StateEnum>
    class StateMachineController
    {
      protected:
        entt::registry* registry;
        std::vector<StateMachine*> systems;

        virtual StateMachine* GetSystem(StateEnum state) = 0;
        virtual void OnComponentRemoved(entt::entity entity) = 0;
        virtual void OnComponentAdded(entt::entity entity) = 0;

        void ChangeState(entt::entity entity, StateEnum oldState, StateEnum newState)
        {
            auto* derived = static_cast<Derived*>(this);
            derived->GetSystem(oldState)->OnStateExit(entity);
            derived->GetSystem(newState)->OnStateEnter(entity);
        }
        virtual ~StateMachineController() = default;

      public:
        explicit StateMachineController(entt::registry* _registry) : registry(_registry)
        {
            registry->template on_construct<StateName>()
                .template connect<&Derived::OnComponentAdded>(
                    static_cast<Derived*>(this));
            registry->template on_destroy<StateName>()
                .template connect<&Derived::OnComponentRemoved>(
                    static_cast<Derived*>(this));
        }
    };
} // namespace sage
