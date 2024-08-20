#pragma once

#include <entt/entt.hpp>

#include <array>
#include <memory>
#include <vector>

namespace sage
{
    class GameData;
    class Ability;
    class PlayerAbilitySystem
    {
        entt::registry* registry;
        entt::entity controlledActor;
        GameData* gameData;

        std::vector<std::unique_ptr<Ability>> abilityMap;
        std::array<int, 4> currentAbilities{};
        void abilityOnePressed();
        void abilityTwoPressed();
        void abilityThreePressed();
        void abilityFourPressed();
        void onActorChanged();

      public:
        void ChangeAbility(int abilitySlot, int newAbilityIndex);
        void Update();
        void Draw2D();
        void Draw3D();
        PlayerAbilitySystem(entt::registry* _registry, GameData* _gameData);
    };
} // namespace sage