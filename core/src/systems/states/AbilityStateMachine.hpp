#pragma once

#include "abilities/AbilityData.hpp"
#include "components/Ability.hpp"
#include "components/States.hpp"
#include "StateMachine.hpp"
#include "Timer.hpp"

#include <entt/entt.hpp>
#include <memory>
#include <unordered_map>

namespace sage
{
    class GameData;
    class VisualFX;

    class AbilityStateController : StateMachineController<AbilityStateController, AbilityState, AbilityStateEnum>
    {
        class IdleState;
        class AwaitingExecutionState;
        class CursorSelectState;

        GameData* gameData;

        void executeAbility(entt::entity abilityEntity);
        bool checkRange(entt::entity abilityEntity);
        void spawnAbility(entt::entity abilityEntity);
        void cancelCast(entt::entity abilityEntity);
        void startCast(entt::entity abilityEntity);

        void onComponentAdded(entt::entity addedEntity);
        void onComponentRemoved(entt::entity addedEntity);

      public:
        void Update();
        void Draw3D();

        ~AbilityStateController();
        AbilityStateController(const AbilityStateController&) = delete;
        AbilityStateController& operator=(const AbilityStateController&) = delete;
        AbilityStateController(entt::registry* _registry, GameData* _gameData);

        friend class StateMachineController; // Required for CRTP
    };

} // namespace sage