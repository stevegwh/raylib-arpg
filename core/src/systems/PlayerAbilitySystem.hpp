#pragma once

#include "entt/entt.hpp"
#include "Event.hpp"

namespace sage
{
    class Systems;
    class Ability;

    class PlayerAbilitySystem
    {
        entt::registry* registry;
        Systems* sys;
        std::vector<Connection> abilityPressedConnections;

      public:
        void SubscribeToUserInput();
        void UnsubscribeFromUserInput();
        void PressAbility(unsigned int slotNumber) const;
        [[nodiscard]] Ability* GetAbility(unsigned int slotNumber) const;
        void SwapAbility(unsigned int slot1, unsigned int slot2);
        void SetSlot(unsigned int slot, entt::entity abilityEntity) const;
        PlayerAbilitySystem(entt::registry* _registry, Systems* _sys);
    };
} // namespace sage