#include "AbilityFunctions.hpp"

#include "components/Collideable.hpp"
#include "components/sgTransform.hpp"

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
} // namespace sage
