#pragma once

#include <entt/entt.hpp>
#include <Timer.hpp>

namespace sage
{
    class GameData;
    class AbilityState
    {
      protected:
        entt::registry* registry;
        entt::entity caster;
        entt::entity abilityEntity;
        GameData* gameData;
        Timer& cooldownTimer;
        Timer& executionDelayTimer;

      public:
        virtual void Update();
        virtual void Draw3D();
        virtual void OnEnter();
        virtual void OnExit();
        virtual ~AbilityState();
        AbilityState(
            entt::registry* _registry,
            entt::entity _caster,
            entt::entity _abilityEntity,
            GameData* _gameData,
            Timer& cooldownTimer,
            Timer& executionDelayTimer);
    };
} // namespace sage