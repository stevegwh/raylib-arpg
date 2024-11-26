//
// Created by Steve Wheeler on 11/08/2024.
//

#pragma once

#include <entt/entt.hpp>

#include <vector>

namespace sage
{
    class GameData;

    class StateMachine
    {
        std::vector<entt::entity> lockedEntities;

      protected:
        entt::registry* registry;
        GameData* gameData;

        void LockState(entt::entity entity)
        {
            lockedEntities.push_back(entity);
        }

        void UnlockState(entt::entity entity)
        {
            lockedEntities.erase(
                std::remove(lockedEntities.begin(), lockedEntities.end(), entity), lockedEntities.end());
        }

        explicit StateMachine(entt::registry* _registry, GameData* _gameData)
            : registry(_registry), gameData(_gameData)
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
        virtual ~StateMachine() = default;
        virtual void OnStateEnter(entt::entity entity){};
        virtual void OnStateExit(entt::entity entity){};
        virtual void Update(entt::entity entity){};
        virtual void Draw3D(entt::entity entity){};
    };

    template <typename StateName, typename StateEnum>
    class StateMachineController
    {
      protected:
        entt::registry* registry;

        std::unordered_map<StateEnum, std::unique_ptr<StateMachine>> states;

        StateMachine* GetSystem(StateEnum state)
        {
            return states[state].get();
        }

        void OnComponentRemoved(entt::entity entity)
        {
            auto& state = registry->get<StateName>(entity);
            entt::sink sink{state.onStateChanged};
            sink.template disconnect<&StateMachineController::ChangeState>(this);

            // TODO: Below seems like a bad idea
            // GetSystem(state.GetCurrentState())->OnStateExit(entity); // Might not be a good idea if destroyed
        }

        void OnComponentAdded(entt::entity entity)
        {
            auto& state = registry->get<StateName>(entity);
            entt::sink sink{state.onStateChanged};
            sink.template connect<&StateMachineController::ChangeState>(this);

            GetSystem(state.GetCurrentState())->OnStateEnter(entity);
        }

        void ChangeState(entt::entity entity, StateEnum newState)
        {
            auto& oldState = registry->get<StateName>(entity);
            StateEnum oldStateEnum = registry->get<StateName>(entity).GetCurrentState();
            if (GetSystem(oldStateEnum)->StateLocked(entity) || oldStateEnum == newState)
            {
                return;
            }
            GetSystem(oldStateEnum)->OnStateExit(entity);
            oldState.SetState(newState);
            GetSystem(newState)->OnStateEnter(entity);
        }

      public:
        virtual ~StateMachineController() = default;
        explicit StateMachineController(entt::registry* _registry) : registry(_registry)
        {
            registry->template on_construct<StateName>()
                .template connect<&StateMachineController::OnComponentAdded>(this);
            registry->template on_destroy<StateName>()
                .template connect<&StateMachineController::OnComponentRemoved>(this);
        }
    };
} // namespace sage
