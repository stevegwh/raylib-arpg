#pragma once

#include <entt/entt.hpp>
#include <unordered_map>

namespace sage
{
    class GameData;

    enum class AbilityEnum // Names of premade abilities.
    {
        PLAYER_AUTOATTACK,
        ENEMY_AUTOATTACK,
        WHIRLWIND,
        RAINFOFIRE,
        FIREBALL,
        LIGHTNINGBALL
    };

    class AbilityFactory
    {
        entt::registry* registry;
        GameData* gameData;
        std::unordered_map<entt::entity, std::unordered_map<AbilityEnum, entt::entity>> abilityMap;

      public:
        entt::entity GetAbility(entt::entity entity, AbilityEnum abilityEnum);
        entt::entity RegisterAbility(entt::entity entity, AbilityEnum abilityEnum);

        AbilityFactory(entt::registry* _registry, GameData* _gameData);
    };

} // namespace sage