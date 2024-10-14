#pragma once

#include "components/Ability.hpp"

#include <entt/entt.hpp>

#include <array>

namespace sage
{
    class GameData;
    static constexpr int MAX_ABILITY_NUMBER = 4;

    class PlayerAbilitySystem
    {
        entt::registry* registry;
        entt::entity controlledActor;
        GameData* gameData;
        std::array<entt::entity, MAX_ABILITY_NUMBER> abilitySlots{};
        void onActorChanged();

      public:
        void PressAbility(unsigned int slotNumber) const;
        void AbilityOnePressed();
        void AbilityTwoPressed();
        void AbilityThreePressed();
        void AbilityFourPressed();
        [[nodiscard]] Ability* GetAbility(int slotNumber) const;
        void SwapAbility(int slot1, int slot2);
        void SetSlot(int slot, entt::entity abilityEntity);
        void Update();
        void Draw2D();
        void Draw3D();
        PlayerAbilitySystem(entt::registry* _registry, GameData* _gameData);
    };
} // namespace sage