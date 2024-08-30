#pragma once

#include <entt/entt.hpp>
#include <memory>
#include <unordered_map>
#include <vector>

namespace sage
{
    class AbilityStateMachine;
    class GameData;

    // In charge of initialising, updating, drawing, returning and changing entity's abilities

    enum class AbilityEnum // Names of premade abilities.
    {
        PLAYER_AUTOATTACK,
        ENEMY_AUTOATTACK,
        WHIRLWIND,
        RAINFOFIRE,
        FIREBALL,
        LIGHTNINGBALL
    };

    class AbilitySystem // TODO: Rename to "AbilityRegistry"
    {
        entt::registry* registry;
        GameData* gameData;
        std::unordered_map<entt::entity, std::unordered_map<AbilityEnum, entt::entity>> abilityMap;

      public:
        entt::entity GetAbility(entt::entity entity, AbilityEnum abilityEnum);
        entt::entity RegisterAbility(entt::entity entity, AbilityEnum abilityEnum);

        AbilitySystem(entt::registry* _registry, GameData* _gameData);
    };
} // namespace sage