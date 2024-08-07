#include "AbilityFunctions.hpp"

#include "components/Collideable.hpp"
#include "components/sgTransform.hpp"

#include "raymath.h"

namespace sage
{
    void Hit360AroundPoint(
        entt::registry* registry,
        entt::entity self,
        AbilityData abilityData,
        Vector3 point,
        float radius)
    {
        auto view = registry->view<CombatableActor>();
        for (auto& entity : view)
        {
            if (entity == self) continue;

            const auto& targetTransform = registry->get<sgTransform>(entity);
            const auto& targetCol = registry->get<Collideable>(entity);

            if (CheckCollisionBoxSphere(targetCol.worldBoundingBox, point, radius))
            {
                const auto& combatable = registry->get<CombatableActor>(entity);
                AttackData attackData{
                    .attacker = self,
                    .hit = entity,
                    .damage = abilityData.baseDamage,
                    .element = abilityData.element};
                combatable.onHit.publish(attackData);
            }
        }
    }

    void HitSingleTarget(
        entt::registry* registry,
        entt::entity self,
        AbilityData abilityData,
        entt::entity target)
    {
        assert(target != entt::null);

        auto& t = registry->get<sgTransform>(self);
        auto& enemyPos = registry->get<sgTransform>(target).position();
        Vector3 direction = Vector3Subtract(enemyPos, t.position());
        float angle = atan2f(direction.x, direction.z) * RAD2DEG;
        t.SetRotation({0, angle, 0}, self);

        if (registry->any_of<CombatableActor>(target))
        {
            auto& enemyCombatable = registry->get<CombatableActor>(target);
            AttackData attack{
                .attacker = self,
                .hit = target,
                .damage = abilityData.baseDamage,
                .element = abilityData.element};
            enemyCombatable.onHit.publish(attack);
        }
    }
} // namespace sage
