// Created by Steve Wheeler on 30/06/2024.
#pragma once
#include "components/States.hpp"
#include "systems/states/StateMachine.hpp"

#include <entt/entt.hpp>
#include <vector>

namespace sage
{
    class GameData;
    struct AttackData;

    class TowerStateController : public StateMachineController<TowerStateController, TowerState, TowerStateEnum>
    {

        class DefaultState;
        class CombatState;

        DefaultState* defaultState;
        CombatState* combatState;

      protected:
        StateMachine* GetSystem(TowerStateEnum state) override;

      public:
        TowerStateController(entt::registry* _registry, GameData* gameData);
        void Update();
        void Draw3D();

        friend class StateMachineController; // Required for CRTP
    };
} // namespace sage