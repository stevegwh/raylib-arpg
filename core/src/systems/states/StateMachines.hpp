#pragma once

#include "AbilityStateMachine.hpp"
#include "GameModeStateMachine.hpp"
#include "PartyMemberStateMachine.hpp"
#include "PlayerStateMachine.hpp"
#include "WavemobStateMachine.hpp"

#include "entt/entt.hpp"

// #include <memory>

namespace sage
{
    class Systems; // forward declaration
    class Scene;   // forward declaration

    class StateMachines
    {
      public:
        // Systems
        std::unique_ptr<GameModeStateController> gameModeStateMachine;
        std::unique_ptr<WavemobStateController> wavemobStatemachine;
        std::unique_ptr<PlayerStateController> playerStateMachine;
        std::unique_ptr<PartyMemberStateController> partyMemberStateMachine;
        std::unique_ptr<AbilityStateController> abilityStateMachine;
        void Update() const;
        void Draw3D() const;
        StateMachines(entt::registry* _registry, Systems* _sys);
    };
} // namespace sage