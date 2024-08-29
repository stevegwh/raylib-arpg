#pragma once

#include "AbilityData.hpp"

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
        virtual void Execute(entt::registry* registry, entt::entity caster, const AbilityData& abilityData) = 0;
    };

    class SingleTargetHitFunc : public AbilityFunction
    {
      public:
        void Execute(entt::registry* registry, entt::entity caster, const AbilityData& ad) override;
    };

    class MultihitRadiusFromCursor : public AbilityFunction
    {
      public:
        void Execute(entt::registry* registry, entt::entity caster, const AbilityData& ad) override;
    };

    class MultihitRadiusFromCaster : public AbilityFunction
    {
      public:
        void Execute(entt::registry* registry, entt::entity caster, const AbilityData& ad) override;
    };

    void Hit360AroundPoint(
        entt::registry* registry, entt::entity caster, AbilityData abilityData, Vector3 point, float radius);

    void HitSingleTarget(
        entt::registry* registry, entt::entity caster, AbilityData abilityData, entt::entity target);

    void ProjectileExplosion(
        entt::registry* registry, entt::entity caster, AbilityData abilityData, Vector3 point);
} // namespace sage