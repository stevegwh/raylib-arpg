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

    class AbilitySystem
    {
        entt::registry* registry;
        GameData* gameData;
        std::unordered_map<entt::entity, std::unordered_map<AbilityEnum, std::unique_ptr<AbilityStateMachine>>>
            abilityMap;

      public:
        AbilityStateMachine* GetAbility(entt::entity entity, AbilityEnum abilityEnum);
        std::vector<AbilityStateMachine*> GetAbilities(entt::entity entity);
        AbilityStateMachine* RegisterAbility(entt::entity entity, AbilityEnum abilityEnum);
        // Create custom ability?
        void Update();
        void Draw3D();

        AbilitySystem(entt::registry* _registry, GameData* _gameData);
    };
} // namespace sage