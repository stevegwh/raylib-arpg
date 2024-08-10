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

    PlayerAutoAttack::PlayerAutoAttack(entt::registry* _registry)
        : AutoAttackAbility(_registry, _abilityData)
    {
    }
} // namespace sage