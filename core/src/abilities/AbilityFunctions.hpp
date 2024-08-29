#pragma once

#include "raylib.h"

#include <entt/entt.hpp>

namespace sage
{
    class Camera;
    class GameData;

    enum class AbilityFunctionEnum
    {
        SingleTargetHit,
        MultihitRadiusFromCursor,
        MultihitRadiusFromCaster
    };

    class AbilityFunction
    {
      protected:
        entt::registry* registry;
        entt::entity self;
        entt::entity abilityData;
        GameData* gameData;

      public:
        virtual void Execute(entt::entity abilityDataEntity) = 0;
        virtual ~AbilityFunction() = default;
        AbilityFunction(entt::registry* _registry, entt::entity _self, GameData* _gameData)
            : registry(_registry), self(_self), gameData(_gameData)
        {
        }
    };

    class SingleTargetHitFunc : public AbilityFunction
    {
      public:
        void Execute(entt::entity abilityDataEntity) override;
        SingleTargetHitFunc(entt::registry* _registry, entt::entity _self, GameData* _gameData)
            : AbilityFunction(_registry, _self, _gameData) {};
    };

    class MultihitRadiusFromCursor : public AbilityFunction
    {
      public:
        void Execute(entt::entity abilityDataEntity) override;
        MultihitRadiusFromCursor(entt::registry* _registry, entt::entity _self, GameData* _gameData)
            : AbilityFunction(_registry, _self, _gameData) {};
    };

    class MultihitRadiusFromCaster : public AbilityFunction
    {
      public:
        void Execute(entt::entity abilityDataEntity) override;
        MultihitRadiusFromCaster(entt::registry* _registry, entt::entity _self, GameData* _gameData)
            : AbilityFunction(_registry, _self, _gameData) {};
    };

    void Hit360AroundPoint(
        entt::registry* registry,
        entt::entity caster,
        entt::entity abilityDataEntity,
        Vector3 point,
        float radius);

    void HitSingleTarget(
        entt::registry* registry, entt::entity caster, entt::entity abilityDataEntity, entt::entity target);

    void ProjectileExplosion(
        entt::registry* registry, entt::entity caster, entt::entity abilityDataEntity, Vector3 point);
} // namespace sage