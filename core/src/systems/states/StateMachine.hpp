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
                std::remove(lockedEntities.begin(), lockedEntities.end(), entity), lockedEntities.end());
        }
        virtual ~StateMachine() = default;

        explicit StateMachine(entt::registry* _registry) : registry(_registry)
        {
        }

      public:
        bool StateLocked(entt::entity entity)
        {
            if (std::find(lockedEntities.begin(), lockedEntities.end(), entity) != lockedEntities.end())
            {
                return true;
            }
            return false;
        }

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

        virtual StateMachine* GetSystem(StateEnum state) = 0;

        void OnComponentRemoved(entt::entity entity)
        {
            auto* derived = static_cast<Derived*>(this);
            auto& state = registry->get<StateName>(entity);
            entt::sink sink{state.onStateChanged};
            sink.template disconnect<&Derived::ChangeState>(derived);

            // TODO: Below seems like a bad idea
            derived->GetSystem(state.GetCurrentState())
                ->OnStateExit(entity); // Might not be a good idea if destroyed
        }

        void OnComponentAdded(entt::entity entity)
        {
            auto* derived = static_cast<Derived*>(this);
            auto& state = registry->get<StateName>(entity);
            entt::sink sink{state.onStateChanged};
            sink.template connect<&Derived::ChangeState>(derived);

            // TODO: Unsure if we really want to call OnStateEnter immediately. (Would init IDLESTATE)
            derived->GetSystem(state.GetCurrentState())->OnStateEnter(entity);
        }

        void ChangeState(entt::entity entity, StateEnum newState)
        {
            StateName oldState = registry->get<StateName>(entity);
            StateEnum oldStateEnum = registry->get<StateName>(entity).GetCurrentState();
            auto* derived = static_cast<Derived*>(this);
            if (derived->GetSystem(oldStateEnum)->StateLocked(entity))
            {
                return;
            }
            derived->GetSystem(oldStateEnum)->OnStateExit(entity);
            oldState.SetState(newState);
            derived->GetSystem(newState)->OnStateEnter(entity);
        }
        virtual ~StateMachineController() = default;

      public:
        explicit StateMachineController(entt::registry* _registry) : registry(_registry)
        {
            registry->template on_construct<StateName>()
                .template connect<&StateMachineController::OnComponentAdded>(this);
            registry->template on_destroy<StateName>()
                .template connect<&StateMachineController::OnComponentRemoved>(this);
        }
    };
} // namespace sage
