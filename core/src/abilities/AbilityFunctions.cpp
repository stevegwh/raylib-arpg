#include "AbilityFunctions.hpp"

#include "AbilityData.hpp"

#include "Camera.hpp"
#include "Cursor.hpp"

#include "components/Collideable.hpp"
#include "components/sgTransform.hpp"

#include "particle/RainOfFireVFX.hpp"

#include "raymath.h"
#include <memory>

namespace sage
{

    void SingleTargetHitFunc::Execute(
        entt::registry* registry, entt::entity self, const AbilityData& ad)
    {
        auto target = registry->get<CombatableActor>(self).target;
        HitSingleTarget(registry, self, ad, target);
    }

    void MultihitRadiusFromCursor::Execute(
        entt::registry* registry, entt::entity self, const AbilityData& ad)
    {
        auto& actorTransform = registry->get<sgTransform>(self);
        Hit360AroundPoint(registry, self, ad, ad.cursor->collision().point, 15);
    }

    void MultihitRadiusFromCaster::Execute(
        entt::registry* registry, entt::entity self, const AbilityData& ad)
    {
        auto& actorTransform = registry->get<sgTransform>(self);
        Hit360AroundPoint(registry, self, ad, actorTransform.position(), 15);
    }

    AbilityResourceManager::AbilityResourceManager(entt::registry* reg) : registry(reg)
    {
    }

    void AbilityResourceManager::InitializeAbilities()
    {
        abilityFunctions.emplace(
            AbilityFunctionEnum::SingleTargetHit,
            std::make_unique<SingleTargetHitFunc>());
        abilityFunctions.emplace(
            AbilityFunctionEnum::MultihitRadiusFromCursor,
            std::make_unique<MultihitRadiusFromCursor>());
        abilityFunctions.emplace(
            AbilityFunctionEnum::MultihitRadiusFromCaster,
            std::make_unique<MultihitRadiusFromCaster>());
    }

    AbilityResourceManager& AbilityResourceManager::GetInstance(entt::registry* reg)
    {
        static AbilityResourceManager instance(reg);
        return instance;
    }

    std::unique_ptr<VisualFX> AbilityResourceManager::GetVisualFX(
        AbilityData::VisualFXData data, Camera* _camera)
    {
        if (data.name == "RainOfFire")
        {
            auto obj = std::make_unique<RainOfFireVFX>(_camera->getRaylibCam());
            data.ptr = obj.get();
            return std::move(obj);
        }

        return nullptr;
    }

    AbilityFunction* AbilityResourceManager::GetExecuteFunc(AbilityFunctionEnum name)
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
