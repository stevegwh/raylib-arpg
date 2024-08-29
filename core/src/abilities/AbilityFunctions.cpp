#include "AbilityFunctions.hpp"

#include "AbilityData.hpp"

#include "Camera.hpp"
#include "Cursor.hpp"
#include "GameData.hpp"

#include "components/Collideable.hpp"
#include "components/sgTransform.hpp"

#include "vfx/RainOfFireVFX.hpp"

#include "raymath.h"
#include <memory>

namespace sage
{

    void SingleTargetHitFunc::Execute()
    {
        auto target = registry->get<CombatableActor>(self).target;
        HitSingleTarget(registry, self, abilityDataEntity, target);
    }

    void MultihitRadiusFromCursor::Execute()
    {
        auto& ad = registry->get<AbilityData>(abilityDataEntity);
        auto& actorTransform = registry->get<sgTransform>(self);
        Hit360AroundPoint(registry, self, abilityDataEntity, ad.cursor->collision().point, 15);
    }

    void MultihitRadiusFromCaster::Execute()
    {
        auto& actorTransform = registry->get<sgTransform>(self);
        Hit360AroundPoint(registry, self, abilityDataEntity, actorTransform.position(), 15);
    }

    void Hit360AroundPoint(
        entt::registry* registry, entt::entity caster, entt::entity abilityDataEntity, Vector3 point, float radius)
    {
        auto& abilityData = registry->get<AbilityData>(abilityDataEntity);
        auto view = registry->view<CombatableActor>();
        for (auto& entity : view)
        {
            if (entity == caster) continue;

            const auto& targetTransform = registry->get<sgTransform>(entity);
            const auto& targetCol = registry->get<Collideable>(entity);

            if (CheckCollisionBoxSphere(targetCol.worldBoundingBox, point, radius))
            {
                const auto& combatable = registry->get<CombatableActor>(entity);
                AttackData attackData{
                    .attacker = caster,
                    .hit = entity,
                    .damage = abilityData.base.baseDamage,
                    .element = abilityData.base.element};
                combatable.onHit.publish(attackData);
            }
        }
    }

    void HitSingleTarget(
        entt::registry* registry, entt::entity caster, entt::entity abilityDataEntity, entt::entity target)
    {
        assert(target != entt::null);

        auto& abilityData = registry->get<AbilityData>(abilityDataEntity);

        auto& t = registry->get<sgTransform>(caster);
        auto& enemyPos = registry->get<sgTransform>(target).position();
        Vector3 direction = Vector3Subtract(enemyPos, t.position());
        float angle = atan2f(direction.x, direction.z) * RAD2DEG;
        t.SetRotation({0, angle, 0}, caster);

        if (registry->any_of<CombatableActor>(target))
        {
            auto& enemyCombatable = registry->get<CombatableActor>(target);
            AttackData attack{
                .attacker = caster,
                .hit = target,
                .damage = abilityData.base.baseDamage,
                .element = abilityData.base.element};
            enemyCombatable.onHit.publish(attack);
        }
    }

    void ProjectileExplosion(
        entt::registry* registry, entt::entity caster, entt::entity abilityDataEntity, Vector3 point)
    {

        // Spawn projectile bb at caster position
        // Fire projectile
        // Wait
        // Check for hit periodically
        auto& checker = registry->emplace<CollisionChecker>(caster);
        entt::sink sink{checker.onHit};

        // On hit, call 360 around point

        // How to sync vfx? Through ability data pointer?
    }
} // namespace sage
