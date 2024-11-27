//
// Created by Steve Wheeler on 11/08/2024.
//

#pragma once

#include <entt/entt.hpp>

#include <format>
#include <iostream>
#include <magic_enum.hpp>
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
        }

        void OnComponentAdded(entt::entity entity)
        {
            auto& state = registry->get<StateName>(entity);
            GetSystem(state.GetCurrentState())->OnStateEnter(entity);
        }

        // NB: Passes all arguments by value
        template <typename NewStateClass, typename... StateEnterArgs>
        void ChangeState(entt::entity entity, StateEnum newState, StateEnterArgs... args)
        // template <typename... StateEnterArgs>
        // void ChangeState(entt::entity entity, StateEnum newState, const StateEnterArgs&... args)
        {
            auto& oldState = registry->get<StateName>(entity);
            StateEnum oldStateEnum = registry->get<StateName>(entity).GetCurrentState();
            if (GetSystem(oldStateEnum)->StateLocked(entity) || oldStateEnum == newState)
            {
                return;
            }
            std::cout << std::format(
                "Entity {}, Exiting: {} \n", static_cast<int>(entity), magic_enum::enum_name(oldStateEnum));
            std::cout << std::format(
                "Entity {}, Entering: {} \n", static_cast<int>(entity), magic_enum::enum_name(newState));
            oldState.RemoveAllConnections();
            GetSystem(oldStateEnum)->OnStateExit(entity);
            oldState.SetState(newState);
            static_cast<NewStateClass*>(GetSystem(newState))
                ->OnStateEnter(entity, args...); // Allows for calls to overloaded but non-virtual functions
        }

        void ChangeState(entt::entity entity, StateEnum newState)
        {
            auto& oldState = registry->get<StateName>(entity);
            StateEnum oldStateEnum = registry->get<StateName>(entity).GetCurrentState();
            if (GetSystem(oldStateEnum)->StateLocked(entity) || oldStateEnum == newState)
            {
                return;
            }
            std::cout << std::format(
                "Entity {}, Exiting: {} \n", static_cast<int>(entity), magic_enum::enum_name(oldStateEnum));
            std::cout << std::format(
                "Entity {}, Entering: {} \n", static_cast<int>(entity), magic_enum::enum_name(newState));
            oldState.RemoveAllConnections();
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
