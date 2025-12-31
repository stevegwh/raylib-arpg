#pragma once

#include "AbilityStateMachine.hpp"
#include "GameModeStateMachine.hpp"
#include "PartyMemberStateMachine.hpp"
#include "PlayerStateMachine.hpp"
#include "WavemobStateMachine.hpp"

#include "entt/entt.hpp"

// #include <memory>

namespace lq
{
    class Systems; // forward declaration
    class Scene;   // forward declaration

    class StateMachines
    {
      public:
        // Systems
        std::unique_ptr<GameModeStateMachine> gameModeStateMachine;
        std::unique_ptr<WavemobStateMachine> wavemobStatemachine;
        std::unique_ptr<PlayerStateMachine> playerStateMachine;
        std::unique_ptr<PartyMemberStateMachine> partyMemberStateMachine;
        std::unique_ptr<AbilityStateMachine> abilityStateMachine;
        void Update() const;
        void Draw3D() const;
        StateMachines(entt::registry* _registry, Systems* _sys);
    };
} // namespace lq