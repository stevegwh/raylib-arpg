#pragma once

#include "entt/entt.hpp"
#include <unordered_map>

namespace sage
{
    class Systems;

    enum class AbilityEnum // Names of premade abilities.
    {
        PLAYER_AUTOATTACK,
        ENEMY_AUTOATTACK,
        WHIRLWIND,
        RAINFOFIRE,
        FIREBALL,
        LIGHTNINGBALL
    };

    void createProjectile(
        entt::registry* registry, entt::entity caster, entt::entity abilityEntity, Systems* data);

    class AbilityFactory
    {
        entt::registry* registry;
        Systems* sys;
        std::unordered_map<entt::entity, std::unordered_map<AbilityEnum, entt::entity>>
            abilityMap; // Caster -> Ability Enum - Ability Entity Id

      public:
        entt::entity GetAbility(entt::entity caster, AbilityEnum abilityEnum);
        entt::entity RegisterAbility(entt::entity caster, AbilityEnum abilityEnum); // "Equip" ability

        AbilityFactory(entt::registry* _registry, Systems* _sys);
    };

} // namespace sage