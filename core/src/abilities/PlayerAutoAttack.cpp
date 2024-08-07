#include "PlayerAutoAttack.hpp"

#include "components/Animation.hpp"
#include "components/CombatableActor.hpp"

#include "AbilityFunctions.hpp"

static constexpr float COOLDOWN = 1.0f;
static constexpr int DAMAGE = 10;

namespace sage
{
    static constexpr AbilityData _abilityData{
        .element = AttackElement::PHYSICAL,
        .cooldownDuration = COOLDOWN,
        .baseDamage = DAMAGE,
        .range = 5};

    void PlayerAutoAttack::Execute(entt::entity self)
    {
        auto& animation = registry->get<Animation>(self);
        animation.ChangeAnimationByEnum(AnimationEnum::AUTOATTACK, 4);

        auto target = registry->get<CombatableActor>(self).target;
        HitSingleTarget(registry, self, abilityData, target);
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
        if (cooldownTimer.HasFinished())
        {
            Execute(self);
            cooldownTimer.Restart();
        }
    }

    PlayerAutoAttack::PlayerAutoAttack(entt::registry* _registry)
        : Ability(_registry, _abilityData)
    {
    }
} // namespace sage