#include "PlayerAbilitySystem.hpp"

#include "components/Ability.hpp"
#include "components/CombatableActor.hpp"
#include "Systems.hpp"
#include "systems/ControllableActorSystem.hpp"
#include "UserInput.hpp"

#include <cassert>

namespace sage
{

    void PlayerAbilitySystem::PressAbility(unsigned int slotNumber) const
    {
        const auto& selectedActor = sys->controllableActorSystem->GetSelectedActor();
        const auto& abilitySlots = registry->get<CombatableActor>(selectedActor).abilities;
        std::cout << "Ability " << slotNumber << " pressed \n";
        if (abilitySlots[slotNumber] == entt::null)
        {
            std::cout << "Slot not bound to an ability \n";
            return;
        }
        auto& ab = registry->get<Ability>(abilitySlots[slotNumber]);

        if (!ab.CooldownReady())
        {
            std::cout << "Waiting for cooldown timer: " << ab.GetRemainingCooldownTime() << "\n";
            return;
        }
        ab.startCast.Publish(abilitySlots[slotNumber]);
    }

    Ability* PlayerAbilitySystem::GetAbility(unsigned int slotNumber) const
    {
        assert(slotNumber < MAX_ABILITY_NUMBER);
        const auto& selectedActor = sys->controllableActorSystem->GetSelectedActor();
        const auto& abilitySlots = registry->get<CombatableActor>(selectedActor).abilities;
        if (abilitySlots.at(slotNumber) == entt::null) return nullptr;
        return &registry->get<Ability>(abilitySlots.at(slotNumber));
    }

    void PlayerAbilitySystem::SwapAbility(unsigned int slot1, unsigned int slot2)
    {
        const auto& selectedActor = sys->controllableActorSystem->GetSelectedActor();
        const auto& abilitySlots = registry->get<CombatableActor>(selectedActor).abilities;
        auto ability1 = abilitySlots.at(slot1);
        auto ability2 = abilitySlots.at(slot2);
        SetSlot(slot1, ability2);
        SetSlot(slot2, ability1);
    }

    void PlayerAbilitySystem::SetSlot(unsigned int slot, entt::entity abilityEntity) const
    {
        assert(slot < MAX_ABILITY_NUMBER);
        const auto& selectedActor = sys->controllableActorSystem->GetSelectedActor();
        auto& abilitySlots = registry->get<CombatableActor>(selectedActor).abilities;
        abilitySlots[slot] = abilityEntity;
    }

    void PlayerAbilitySystem::SubscribeToUserInput()
    {
        if (!abilityPressedConnections.empty()) return;
        abilityPressedConnections.push_back(
            sys->userInput->keyOnePressed.Subscribe([this]() { PressAbility(0); }));
        abilityPressedConnections.push_back(
            sys->userInput->keyTwoPressed.Subscribe([this]() { PressAbility(1); }));
        abilityPressedConnections.push_back(
            sys->userInput->keyThreePressed.Subscribe([this]() { PressAbility(2); }));
        abilityPressedConnections.push_back(
            sys->userInput->keyFourPressed.Subscribe([this]() { PressAbility(3); }));
    }

    void PlayerAbilitySystem::UnsubscribeFromUserInput()
    {
        for (const auto& cnx : abilityPressedConnections)
        {
            cnx->UnSubscribe();
        }
        abilityPressedConnections.clear();
    }

    PlayerAbilitySystem::PlayerAbilitySystem(entt::registry* _registry, Systems* _sys)
        : registry(_registry), sys(_sys)
    {
        SubscribeToUserInput();
    }
} // namespace sage