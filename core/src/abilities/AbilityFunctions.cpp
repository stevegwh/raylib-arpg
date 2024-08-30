#include "AbilityFunctions.hpp"

#include "AbilityData.hpp"

#include "Camera.hpp"
#include "Cursor.hpp"
#include "GameData.hpp"
#include "GameObjectFactory.hpp"

#include "components/Collideable.hpp"
#include "components/sgTransform.hpp"
#include "systems/ActorMovementSystem.hpp"

#include "vfx/RainOfFireVFX.hpp"

#include "raymath.h"
#include <memory>

namespace sage
{

    void SingleTargetHit::Execute()
    {
        auto target = registry->get<CombatableActor>(caster).target;
        HitSingleTarget(registry, caster, abilityEntity, target);
    }

    void HitAllInRadius::Execute()
    {
        auto& ad = registry->get<AbilityData>(abilityEntity);
        if (ad.base.spawnBehaviour == AbilitySpawnBehaviour::AT_CURSOR)
        {
            auto& actorTransform = registry->get<sgTransform>(caster);
            Hit360AroundPoint(registry, caster, abilityEntity, ad.cursor->collision().point, 15);
        }
        else if (ad.base.spawnBehaviour == AbilitySpawnBehaviour::AT_CASTER)
        {
            auto& actorTransform = registry->get<sgTransform>(caster);
            Hit360AroundPoint(registry, caster, abilityEntity, actorTransform.position(), 15);
        }
    }

    void Hit360AroundPoint(
        entt::registry* registry, entt::entity caster, entt::entity abilityEntity, Vector3 point, float radius)
    {
        auto& abilityData = registry->get<AbilityData>(abilityEntity);
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
        entt::registry* registry, entt::entity caster, entt::entity abilityEntity, entt::entity target)
    {
        assert(target != entt::null);

        auto& abilityData = registry->get<AbilityData>(abilityEntity);

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
