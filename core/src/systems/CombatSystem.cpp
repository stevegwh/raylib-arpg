//
// Created by steve on 03/08/2024.
//

#include "CombatSystem.hpp"
#include "components/HealthBar.hpp"

namespace sage
{

    void CombatSystem::onComponentAdded(entt::entity entity)
    {
        auto& c = registry->get<CombatableActor>(entity);
        {
            entt::sink sink{c.onHit};
            sink.connect<&CombatSystem::RegisterAttack>(*this);
        }
    }

    void CombatSystem::onComponentRemoved(entt::entity entity)
    {
    }

    void CombatSystem::RegisterAttack(AttackData attackData)
    {
        // DoTs could be handled in attackData with a duration and a damage value. Use a
        // timer and callback? Can store timerId (and hit entity) in a map which the
        // callback function can call (I assume I can't pass these on as parameters
        // directly without some generic programming trickery)
        auto& targetCombat = registry->get<CombatableActor>(attackData.hit);
        targetCombat.hp -= attackData.damage;
        if (targetCombat.hp <= 0)
        {
            targetCombat.dying = true;
            targetCombat.hp = 0;
            targetCombat.onDeath.publish(attackData.hit);
        }
        if (registry->any_of<HealthBar>(attackData.hit))
        {
            auto& healthbar = registry->get<HealthBar>(attackData.hit);
            healthbar.Decrement(attackData.damage);
        }
    }

    CombatSystem::CombatSystem(entt::registry* _registry) : registry(_registry)
    {
        registry->on_construct<CombatableActor>()
            .connect<&CombatSystem::onComponentAdded>(this);
        registry->on_destroy<CombatableActor>()
            .connect<&CombatSystem::onComponentRemoved>(this);
    }
} // namespace sage