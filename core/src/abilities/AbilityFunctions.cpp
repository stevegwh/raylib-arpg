#include "AbilityFunctions.hpp"

#include "AbilityData.hpp"

#include "Camera.hpp"
#include "Cursor.hpp"
#include "GameData.hpp"

#include "components/Collideable.hpp"
#include "components/sgTransform.hpp"
#include "systems/ActorMovementSystem.hpp"

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

    void ProjectileExplosionFromCaster::onHit(entt::entity caster, CollisionInfo collisionInfo)
    {
        Vector3 point = registry->get<sgTransform>(collisionInfo.collidedEntityId).position();
        Hit360AroundPoint(registry, self, abilityDataEntity, point, 15);

        auto& checker = registry->get<CollisionChecker>(abilityDataEntity);
        entt::sink sink{checker.onHit};
        sink.disconnect<&ProjectileExplosionFromCaster::onHit>(this);
        registry->remove<CollisionChecker>(abilityDataEntity);
    }

    void ProjectileExplosionFromCaster::Execute()
    {
        auto& projectileTrans = registry->emplace<sgTransform>(abilityDataEntity);
        auto& projectileCol = registry->emplace<Collideable>(abilityDataEntity);
        projectileCol.collisionLayer = CollisionLayer::PLAYER;
        auto& casterPos = registry->emplace<sgTransform>(self).position();
        projectileTrans.SetPosition(casterPos, self);

        auto target = registry->get<CombatableActor>(self).target;
        Vector3 point = registry->get<sgTransform>(target).position();

        gameData->actorMovementSystem->MoveToLocation(abilityDataEntity, point);

        auto& checker = registry->emplace<CollisionChecker>(abilityDataEntity);
        entt::sink sink{checker.onHit};
        sink.connect<&ProjectileExplosionFromCaster::onHit>(this);

        // On hit, call 360 around point

        // How to sync vfx? Through ability data pointer?
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

} // namespace sage
