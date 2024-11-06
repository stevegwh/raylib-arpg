#pragma once

#include "components/Ability.hpp"

#include <entt/entt.hpp>

#include <array>

namespace sage
{
    class GameData;

    class PlayerAbilitySystem
    {
        entt::registry* registry;
        entt::entity controlledActor;
        GameData* gameData;

      public:
        void onActorChanged();
        void PressAbility(unsigned int slotNumber) const;
        void AbilityOnePressed();
        void AbilityTwoPressed();
        void AbilityThreePressed();
        void AbilityFourPressed();
        [[nodiscard]] Ability* GetAbility(unsigned int slotNumber) const;
        void SwapAbility(unsigned int slot1, unsigned int slot2);
        void SetSlot(unsigned int slot, entt::entity abilityEntity) const;
        void Update();
        void Draw2D();
        void Draw3D();
        PlayerAbilitySystem(entt::registry* _registry, GameData* _gameData);
    };
} // namespace sage