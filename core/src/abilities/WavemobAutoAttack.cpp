#include "WavemobAutoAttack.hpp"

#include "components/Animation.hpp"
#include "components/CombatableActor.hpp"
#include "components/sgTransform.hpp"

#include "systems/CollisionSystem.hpp"

#include <raymath.h>

static constexpr float COOLDOWN = 1.0f;

namespace sage
{

    void WavemobAutoAttack::Execute(entt::entity self)
    {
        auto& c = registry->get<CombatableActor>(self);
        if (c.target == entt::null) return;

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

    void WavemobAutoAttack::Init(entt::entity self)
    {
        if (active)
        {
            std::cout << "Trying to init but ability already active" << std::endl;
            return;
        }
        active = true;
        cooldownTimer.Start();
    }

    void WavemobAutoAttack::Cancel()
    {
        active = false;
        cooldownTimer.Stop();
    }

    void WavemobAutoAttack::Update(entt::entity self)
    {
        if (!active) return;
        cooldownTimer.Update(GetFrameTime());
        if (cooldownTimer.HasFinished())
        {
            Execute(self);
            cooldownTimer.Restart();
        }
    }

    WavemobAutoAttack::WavemobAutoAttack(
        entt::registry* _registry, CollisionSystem* _collisionSystem)
        : Ability(_registry, COOLDOWN, _collisionSystem)
    {
        attackData.element = AttackElement::PHYSICAL;
        attackData.damage = 10;
    }
} // namespace sage