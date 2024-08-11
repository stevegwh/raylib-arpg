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
} // namespace sage
