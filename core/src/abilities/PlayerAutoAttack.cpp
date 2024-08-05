#include "PlayerAutoAttack.hpp"

#include "components/Animation.hpp"
#include "components/CombatableActor.hpp"
#include "components/sgTransform.hpp"

#include <raymath.h>

namespace sage
{
    void PlayerAutoAttack::Execute(entt::entity self)
    {
        auto& c = registry->get<CombatableActor>(self);
        if (c.target == entt::null)
            return;

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
        if (active)
        {
            std::cout << "Trying to init but ability already active" << std::endl;
            return;
        }
        active = true;
        cooldownTimerId = timerManager->AddTimer(m_cooldownLimit, &PlayerAutoAttack::Execute, this, self);
    }

    void PlayerAutoAttack::Cancel()
    {
        active = false;
        timerManager->RemoveTimer(cooldownTimerId);
        cooldownTimerId = -1;
    }

    void PlayerAutoAttack::Update(entt::entity self)
    {
    }

    PlayerAutoAttack::PlayerAutoAttack(entt::registry* _registry, CollisionSystem* _collisionSystem,
                                       TimerManager* _timerManager)
        : Ability(_registry, _collisionSystem, _timerManager)
    {
        attackData.element = AttackElement::PHYSICAL;
        attackData.damage = 10;
        m_cooldownLimit = 1.0f;
    }
} // namespace sage