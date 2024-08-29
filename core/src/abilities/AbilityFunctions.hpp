#pragma once

#include "raylib.h"

#include <entt/entt.hpp>

namespace sage
{
    class Camera;

    enum class AbilityFunctionEnum
    {
        SingleTargetHit,
        MultihitRadiusFromCursor,
        MultihitRadiusFromCaster
    };

    class AbilityFunction
    {
      protected:
      public:
        virtual ~AbilityFunction() = default;
        virtual void Execute(entt::registry* registry, entt::entity caster, entt::entity abilityDataEntity) = 0;
    };

    class SingleTargetHitFunc : public AbilityFunction
    {
      public:
        void Execute(entt::registry* registry, entt::entity caster, entt::entity abilityDataEntity) override;
    };

    class MultihitRadiusFromCursor : public AbilityFunction
    {
      public:
        void Execute(entt::registry* registry, entt::entity caster, entt::entity abilityDataEntity) override;
    };

    class MultihitRadiusFromCaster : public AbilityFunction
    {
      public:
        void Execute(entt::registry* registry, entt::entity caster, entt::entity abilityDataEntity) override;
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