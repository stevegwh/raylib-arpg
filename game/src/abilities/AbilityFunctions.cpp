#include "AbilityFunctions.hpp"

#include "../../../engine/Cursor.hpp"
#include "components/Ability.hpp"
#include "components/CombatableActor.hpp"
#include "GameObjectFactory.hpp"
#include "Systems.hpp"

#include "engine/Camera.hpp"
#include "engine/components/Collideable.hpp"
#include "engine/components/sgTransform.hpp"
#include "engine/systems/ActorMovementSystem.hpp"

#include "vfx/RainOfFireVFX.hpp"

#include "raymath.h"
#include <iostream>
#include <memory>

namespace lq
{
    void AOEAtPoint(
        entt::registry* registry, entt::entity caster, entt::entity abilityEntity, Vector3 point, float radius)
    {
        auto& abilityData = registry->get<AbilityData>(abilityEntity);
        auto view = registry->view<CombatableActor>();
        for (auto& entity : view)
        {
            if (entity == caster) continue;

            const auto& targetTransform = registry->get<sage::sgTransform>(entity);
            const auto& targetCol = registry->get<sage::Collideable>(entity);

            if (CheckCollisionBoxSphere(targetCol.worldBoundingBox, point, radius))
            {
                const auto& combatable = registry->get<CombatableActor>(entity);
                AttackData attackData{
                    .attacker = caster,
                    .hit = entity,
                    .damage = abilityData.base.baseDamage,
                    .elements = abilityData.base.elements};
                combatable.onHit.Publish(attackData);
            }
        }
    }

    void HitSingleTarget(
        entt::registry* registry, entt::entity caster, entt::entity abilityEntity, entt::entity target)
    {
        assert(target != entt::null);

        auto& abilityData = registry->get<AbilityData>(abilityEntity);

        auto& t = registry->get<sage::sgTransform>(caster);
        auto& enemyPos = registry->get<sage::sgTransform>(target).GetWorldPos();
        Vector3 direction = Vector3Subtract(enemyPos, t.GetWorldPos());
        float angle = atan2f(direction.x, direction.z) * RAD2DEG;
        t.SetRotation({0, angle, 0});

        if (registry->any_of<CombatableActor>(target))
        {
            auto& enemyCombatable = registry->get<CombatableActor>(target);
            AttackData attack{
                .attacker = caster,
                .hit = target,
                .damage = abilityData.base.baseDamage,
                .elements = abilityData.base.elements};
            enemyCombatable.onHit.Publish(attack);
        }
    }

} // namespace lq
