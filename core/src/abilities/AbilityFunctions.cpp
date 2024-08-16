#include "AbilityFunctions.hpp"

#include "AbilityData.hpp"

#include "Camera.hpp"
#include "Cursor.hpp"

#include "components/Collideable.hpp"
#include "components/sgTransform.hpp"

#include "vfx/RainOfFireVFX.hpp"

#include "raymath.h"
#include <memory>

namespace sage
{

    void SingleTargetHitFunc::Execute(entt::registry* registry, entt::entity self, const AbilityData& ad)
    {
        auto target = registry->get<CombatableActor>(self).target;
        HitSingleTarget(registry, self, ad, target);
    }

    void MultihitRadiusFromCursor::Execute(entt::registry* registry, entt::entity self, const AbilityData& ad)
    {
        auto& actorTransform = registry->get<sgTransform>(self);
        Hit360AroundPoint(registry, self, ad, ad.cursor->collision().point, 15);
    }

    void MultihitRadiusFromCaster::Execute(entt::registry* registry, entt::entity self, const AbilityData& ad)
    {
        auto& actorTransform = registry->get<sgTransform>(self);
        Hit360AroundPoint(registry, self, ad, actorTransform.position(), 15);
    }

    void Hit360AroundPoint(
        entt::registry* registry, entt::entity self, AbilityData abilityData, Vector3 point, float radius)
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
                    .damage = abilityData.base.baseDamage,
                    .element = abilityData.base.element};
                combatable.onHit.publish(attackData);
            }
        }
    }

    void HitSingleTarget(entt::registry* registry, entt::entity self, AbilityData abilityData, entt::entity target)
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
                .damage = abilityData.base.baseDamage,
                .element = abilityData.base.element};
            enemyCombatable.onHit.publish(attack);
        }
    }
} // namespace sage
