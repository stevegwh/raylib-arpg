#include "AbilityFunctions.hpp"

#include "AbilityData.hpp"

#include "components/Collideable.hpp"
#include "components/sgTransform.hpp"

#include "raymath.h"

namespace sage
{

    AbilityFunction::AbilityFunction(entt::registry* _registry) : registry(_registry)
    {
    }

    void PlayerAutoAttackFunc::Execute(entt::entity self, const AbilityData& abilityData)
    {
        auto target = registry->get<CombatableActor>(self).target;
        HitSingleTarget(registry, self, abilityData, target);
    }

    PlayerAutoAttackFunc::PlayerAutoAttackFunc(entt::registry* _registry)
        : AbilityFunction(_registry)
    {
    }

    void RainOfFireFunc::Execute(entt::entity self, const AbilityData& abilityData)
    {
        auto& actorTransform = registry->get<sgTransform>(self);
        Hit360AroundPoint(registry, self, abilityData, actorTransform.position(), 5);
    }

    RainOfFireFunc::RainOfFireFunc(entt::registry* _registry) : AbilityFunction(_registry)
    {
    }

    void WavemobAutoAttackFunc::Execute(entt::entity self, const AbilityData& abilityData)
    {
        auto target = registry->get<CombatableActor>(self).target;
        HitSingleTarget(registry, self, abilityData, target);
    }

    WavemobAutoAttackFunc::WavemobAutoAttackFunc(entt::registry* _registry)
        : AbilityFunction(_registry)
    {
    }

    void WhirlwindFunc::Execute(entt::entity self, const AbilityData& abilityData)
    {
        auto& actorTransform = registry->get<sgTransform>(self);
        Hit360AroundPoint(registry, self, abilityData, actorTransform.position(), 15);
    }

    WhirlwindFunc::WhirlwindFunc(entt::registry* _registry) : AbilityFunction(_registry)
    {
    }

    AbilityLibrary::AbilityLibrary(entt::registry* reg) : registry(reg)
    {
    }

    void AbilityLibrary::InitializeAbilities()
    {
        abilityFunctions.emplace(
            "PlayerAutoAttack", std::make_unique<PlayerAutoAttackFunc>(registry));
        abilityFunctions.emplace(
            "RainOfFire", std::make_unique<RainOfFireFunc>(registry));
        abilityFunctions.emplace(
            "WavemobAutoAttack", std::make_unique<WavemobAutoAttackFunc>(registry));
        abilityFunctions.emplace("Whirlwind", std::make_unique<WhirlwindFunc>(registry));
    }

    AbilityLibrary& AbilityLibrary::GetInstance(entt::registry* reg)
    {
        static AbilityLibrary instance(reg);
        return instance;
    }

    AbilityFunction* AbilityLibrary::GetAbility(const std::string& name)
    {
        if (abilityFunctions.empty())
        {
            InitializeAbilities();
        }
        auto it = abilityFunctions.find(name);
        return (it != abilityFunctions.end()) ? it->second.get() : nullptr;
    }

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
                    .damage = abilityData.baseData.baseDamage,
                    .element = abilityData.baseData.element};
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
                .damage = abilityData.baseData.baseDamage,
                .element = abilityData.baseData.element};
            enemyCombatable.onHit.publish(attack);
        }
    }
} // namespace sage
