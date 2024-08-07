#pragma once

#include <entt/entt.hpp>

#include "abilities/Ability.hpp"

#include <array>
#include <memory>
#include <vector>

namespace sage
{
    class GameData;
    class AbilitySystem // PlayerAbilitySystem (ControllableActorAbilitySystem?)
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
        AbilitySystem(entt::registry* _registry, GameData* _gameData);
    };
} // namespace sage