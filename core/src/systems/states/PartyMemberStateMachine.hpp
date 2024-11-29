// Created by Steve Wheeler on 30/06/2024.
#pragma once

#include "components/States.hpp"
#include "StateMachine.hpp"
#include <entt/entt.hpp>

namespace sage
{
    class GameData;

    class PartyMemberStateController final : public StateMachineController<PartyMemberState, PartyMemberStateEnum>
    {
        GameData* gameData;

        class DefaultState;
        class FollowingLeaderState;
        class WaitingForLeaderState;
        class DestinationUnreachableState;

        void onComponentAdded(entt::entity entity);
        void onComponentRemoved(entt::entity entity);

      public:
        void Update();
        void Draw3D();

        ~PartyMemberStateController() override = default;
        PartyMemberStateController(entt::registry* _registry, GameData* gameData);

        friend class StateMachineController; // Required for CRTP
        friend class PlayerStateController;
    };
} // namespace sage