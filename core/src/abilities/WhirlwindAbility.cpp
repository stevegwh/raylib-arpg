//
// Created by Steve Wheeler on 21/07/2024.
//

#include "WhirlwindAbility.hpp"

#include "components/Animation.hpp"
#include "components/CombatableActor.hpp"
#include "components/sgTransform.hpp"

#include "raylib.h"

static constexpr float COOLDOWN = 3.0f;
static constexpr float WINDUP = 0.65f;

namespace sage
{
    void WhirlwindAbility::Init(entt::entity self)
    {
        if (GetRemainingCooldownTime() > 0)
        {
            std::cout << "Waiting for cooldown \n";
            return;
        }

        std::cout << "Whirlwind ability used \n";
        active = true;
        auto& animation = registry->get<Animation>(self);
        animation.ChangeAnimationByEnum(AnimationEnum::SPIN, 3, true);

        cooldownTimer.Start();
        windupTimer.Start();
    }

    void WhirlwindAbility::Execute(entt::entity self)
    {
        auto& actorTransform = registry->get<sgTransform>(self);
        const auto& actorCol = registry->get<Collideable>(self);

        auto view = registry->view<CombatableActor>();

        for (auto& entity : view)
        {
            if (entity == self) continue;
            // if (std::find(hitUnits.begin(), hitUnits.end(), entity) != hitUnits.end())
            // continue;

            const auto& targetTransform = registry->get<sgTransform>(entity);
            const auto& targetCol = registry->get<Collideable>(entity);

            if (CheckCollisionBoxSphere(
                    targetCol.worldBoundingBox,
                    actorTransform.position(),
                    whirlwindRadius))
            {
                // hitUnits.push_back(entity);
                const auto& combatable = registry->get<CombatableActor>(entity);
                AttackData _attackData = attackData;
                _attackData.hit = entity;
                _attackData.attacker = self;
                combatable.onHit.publish(_attackData);
                std::cout << "Hit unit \n";
            }
        }
        active = false;
    }

    void WhirlwindAbility::Update(entt::entity self)
    {
        cooldownTimer.Update(GetFrameTime());
        if (!active) return;
        windupTimer.Update(GetFrameTime());
        if (windupTimer.HasFinished())
        {
            Execute(self);
            windupTimer.Reset();
        }
    }

    WhirlwindAbility::WhirlwindAbility(
        entt::registry* _registry, CollisionSystem* _collisionSystem)
        : Ability(_registry, COOLDOWN, _collisionSystem)
    {
        windupTimer.SetMaxTime(WINDUP);

        attackData.damage = 25.0f;
    }
} // namespace sage
