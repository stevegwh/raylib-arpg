#pragma once

#include "raylib.h"

#include <entt/entt.hpp>

namespace sage
{
    class Camera;
    struct CollisionInfo;
    class GameData;

    enum class AbilityFunctionEnum
    {
        SingleTargetHit,
        HitAllInRadius
    };

    class AbilityFunction
    {
      protected:
        entt::registry* registry;
        entt::entity caster;
        entt::entity abilityEntity;
        GameData* gameData;

      public:
        virtual void Execute() = 0;
        virtual ~AbilityFunction() = default;
        AbilityFunction(
            entt::registry* _registry, entt::entity _self, entt::entity _abilityDataEntity, GameData* _gameData)
            : registry(_registry), caster(_self), abilityEntity(_abilityDataEntity), gameData(_gameData)
        {
        }
    };

    class SingleTargetHit : public AbilityFunction
    {
      public:
        void Execute() override;
        SingleTargetHit(
            entt::registry* _registry, entt::entity _self, entt::entity _abilityDataEntity, GameData* _gameData)
            : AbilityFunction(_registry, _self, _abilityDataEntity, _gameData) {};
    };

    class HitAllInRadius : public AbilityFunction
    {
      public:
        void Execute() override;
        HitAllInRadius(
            entt::registry* _registry, entt::entity _self, entt::entity _abilityDataEntity, GameData* _gameData)
            : AbilityFunction(_registry, _self, _abilityDataEntity, _gameData) {};
    };

    void Hit360AroundPoint(
        entt::registry* registry, entt::entity caster, entt::entity abilityEntity, Vector3 point, float radius);

    void HitSingleTarget(
        entt::registry* registry, entt::entity caster, entt::entity abilityEntity, entt::entity target);

} // namespace sage