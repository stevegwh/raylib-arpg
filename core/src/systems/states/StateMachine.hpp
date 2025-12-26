//
// Created by Steve Wheeler on 11/08/2024.
//

#pragma once

#include "entt/entt.hpp"

// #include <iostream>
#include <vector>

namespace sage
{
    class Systems;

    class State
    {
        std::vector<entt::entity> lockedEntities;

      protected:
        entt::registry* registry;
        Systems* sys;

        void LockState(entt::entity entity)
        {
            lockedEntities.push_back(entity);
        }

        void UnlockState(entt::entity entity)
        {
            lockedEntities.erase(
                std::remove(lockedEntities.begin(), lockedEntities.end(), entity), lockedEntities.end());
        }

        explicit State(entt::registry* _registry, Systems* _sys) : registry(_registry), sys(_sys)
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
        virtual ~State() = default;
        virtual void OnEnter(entt::entity entity) {};
        virtual void OnExit(entt::entity entity) {};
        virtual void Update(entt::entity entity) {};
        virtual void Draw3D(entt::entity entity) {};
    };

    template <typename StateComponentType, typename StateEnum>
    class StateMachine
    {
      protected:
        entt::registry* registry;

        std::unordered_map<StateEnum, std::unique_ptr<State>> states;

        template <typename StateDowncast>
        StateDowncast* GetState(StateEnum stateEnum)
        {
            return dynamic_cast<StateDowncast*>(states[stateEnum].get());
        }

        State* GetState(StateEnum state)
        {
            return states[state].get();
        }

        void OnComponentRemoved(entt::entity entity)
        {
        }

        void OnComponentAdded(entt::entity entity)
        {
            auto& stateComponent = registry->get<StateComponentType>(entity);
            GetState(stateComponent.GetCurrentState())->OnEnter(entity);
        }

      public:
        // NB: Passes all arguments by value
        // Allows for calls to overloaded functions that take more arguments than the base 'OnEnter' function.
        template <typename NewStateType, typename... StateEnterArgs>
        void ChangeState(entt::entity entity, StateEnum newState, StateEnterArgs... args)
        // template <typename... StateEnterArgs>
        // void ChangeState(entt::entity entity, StateEnum newState, const StateEnterArgs&... args)
        {
            auto& oldState = registry->get<StateComponentType>(entity);
            StateEnum oldStateEnum = registry->get<StateComponentType>(entity).GetCurrentState();
            if (GetState(oldStateEnum)->StateLocked(entity) || oldStateEnum == newState)
            {
                return;
            }
            //            std::cout << "------------- \n";
            //            std::cout << std::format(
            //                "Entity {}, Exiting: {} \n", static_cast<int>(entity),
            //                magic_enum::enum_name(oldStateEnum));
            //            std::cout << std::format(
            //                "Entity {}, Entering: {} \n", static_cast<int>(entity),
            //                magic_enum::enum_name(newState));
            oldState.RemoveAllSubscriptions();
            GetState(oldStateEnum)->OnExit(entity);
            oldState.SetState(newState);
            static_cast<NewStateType*>(GetState(newState))->OnEnter(entity, args...);
        }

        void ChangeState(entt::entity entity, StateEnum newState)
        {
            auto& oldState = registry->get<StateComponentType>(entity);
            StateEnum oldStateEnum = registry->get<StateComponentType>(entity).GetCurrentState();
            if (GetState(oldStateEnum)->StateLocked(entity) || oldStateEnum == newState)
            {
                return;
            }
            //            std::cout << "------------- \n";
            //            std::cout << std::format(
            //                "Entity {}, Exiting: {} \n", static_cast<int>(entity),
            //                magic_enum::enum_name(oldStateEnum));
            //            std::cout << std::format(
            //                "Entity {}, Entering: {} \n", static_cast<int>(entity),
            //                magic_enum::enum_name(newState));
            oldState.RemoveAllSubscriptions();
            GetState(oldStateEnum)->OnExit(entity);
            oldState.SetState(newState);
            GetState(newState)->OnEnter(entity);
        }

        virtual ~StateMachine() = default;
        explicit StateMachine(entt::registry* _registry) : registry(_registry)
        {
            registry->template on_construct<StateComponentType>()
                .template connect<&StateMachine::OnComponentAdded>(this);
            registry->template on_destroy<StateComponentType>()
                .template connect<&StateMachine::OnComponentRemoved>(this);
        }
    };
} // namespace sage
