#pragma once

#include "entt/entt.hpp"

namespace sage
{
    class Systems;
    class Ability;

    class PlayerAbilitySystem
    {
        entt::registry* registry;
        Systems* sys;

      public:
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
        PlayerAbilitySystem(entt::registry* _registry, Systems* _sys);
    };
} // namespace sage