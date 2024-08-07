#include "WavemobAutoAttack.hpp"

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

    void WavemobAutoAttack::Execute(entt::entity self)
    {
        auto target = registry->get<CombatableActor>(self).target;
        HitSingleTarget(registry, self, abilityData, target);

        auto& animation = registry->get<Animation>(self);
        animation.ChangeAnimationByEnum(AnimationEnum::AUTOATTACK, 4);
    }

    void WavemobAutoAttack::Init(entt::entity self)
    {
        if (active)
        {
            // std::cout << "Trying to init but ability already active" << std::endl;
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

    WavemobAutoAttack::WavemobAutoAttack(entt::registry* _registry)
        : Ability(_registry, _abilityData)
    {
    }
} // namespace sage