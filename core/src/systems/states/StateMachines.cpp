#include "StateMachines.hpp"

#include "scenes/Scene.hpp"
#include "Systems.hpp"

namespace sage
{
    void StateMachines::Update() const
    {
        gameModeStateMachine->Update();
        wavemobStatemachine->Update();
        playerStateMachine->Update();
        partyMemberStateMachine->Update();
        abilityStateMachine->Update();
    }

    void StateMachines::Draw3D() const
    {
        wavemobStatemachine->Draw3D();
        playerStateMachine->Draw3D();
        partyMemberStateMachine->Draw3D();
        // gameStateMachine->Draw3D();
        abilityStateMachine->Draw3D();
    }

    StateMachines::StateMachines(entt::registry* _registry, Systems* _sys)
        : gameModeStateMachine(std::make_unique<GameModeStateController>(_registry, _sys)),
          wavemobStatemachine(std::make_unique<WavemobStateController>(_registry, _sys)),
          playerStateMachine(std::make_unique<PlayerStateController>(_registry, _sys)),
          partyMemberStateMachine(std::make_unique<PartyMemberStateController>(_registry, _sys)),
          abilityStateMachine(std::make_unique<AbilityStateController>(_registry, _sys))
    {
    }
} // namespace sage