#include "PlayerAutoAttack.hpp"

#include "components/Animation.hpp"
#include "components/CombatableActor.hpp"
#include "components/sgTransform.hpp"

#include <iostream>
#include <raymath.h>

static constexpr float COOLDOWN = 1.0f;

namespace sage
{
    void PlayerAutoAttack::Execute(entt::entity self)
    {
        auto& c = registry->get<CombatableActor>(self);
        assert(c.target != entt::null);
        if (c.target == entt::null)
        {
            std::cout << "Target lost" << std::endl;
            return;
        }

        auto& t = registry->get<sgTransform>(self);
        // TODO: Check if target is present
        auto& enemyPos = registry->get<sgTransform>(c.target).position();
        Vector3 direction = Vector3Subtract(enemyPos, t.position());
        float angle = atan2f(direction.x, direction.z) * RAD2DEG;
        t.SetRotation({0, angle, 0}, self);

        auto& animation = registry->get<Animation>(self);
        animation.ChangeAnimationByEnum(AnimationEnum::AUTOATTACK, 4);
        if (registry->any_of<CombatableActor>(c.target))
        {
            auto& enemyCombatable = registry->get<CombatableActor>(c.target);
            AttackData attack = attackData;
            attack.attacker = self;
            attack.hit = c.target;
            enemyCombatable.onHit.publish(attack);
        }
    }

    void PlayerAutoAttack::Init(entt::entity self)
    {
        cooldownTimer.Start();
    }

    void PlayerAutoAttack::Cancel()
    {
        cooldownTimer.Stop();
    }

    void PlayerAutoAttack::Update(entt::entity self)
    {
        cooldownTimer.Update(GetFrameTime());
        // std::cout << "Autoattack timer: " << cooldownTimer.GetRemainingTime()
        //           << std::endl;
        if (cooldownTimer.HasFinished())
        {
            Execute(self);
            cooldownTimer.Restart();
        }
    }

    PlayerAutoAttack::PlayerAutoAttack(
        entt::registry* _registry, CollisionSystem* _collisionSystem)
        : Ability(_registry, COOLDOWN, _collisionSystem)
    {
        attackData.element = AttackElement::PHYSICAL;
        attackData.damage = 10;
    }
} // namespace sage