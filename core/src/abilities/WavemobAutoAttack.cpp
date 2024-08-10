#include "WavemobAutoAttack.hpp"

#include "components/Animation.hpp"
#include "components/CombatableActor.hpp"

#include "AbilityFunctions.hpp"

static constexpr float COOLDOWN = 1.0f;
static constexpr int DAMAGE = 10;

namespace sage
{
    static constexpr AbilityData _abilityData{
        .cooldownDuration = COOLDOWN,
        .range = 5,
        .baseDamage = DAMAGE,
        .element = AttackElement::PHYSICAL};

    WavemobAutoAttack::WavemobAutoAttack(entt::registry* _registry)
        : AutoAttackAbility(_registry, _abilityData)
    {
    }
} // namespace sage