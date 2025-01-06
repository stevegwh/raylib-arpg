#include "PlayerAbilitySystem.hpp"

#include "components/Ability.hpp"
#include "components/CombatableActor.hpp"
#include "GameData.hpp"
#include "systems/ControllableActorSystem.hpp"
#include "UserInput.hpp"

#include <cassert>

namespace sage
{

    void PlayerAbilitySystem::PressAbility(unsigned int slotNumber) const
    {
        const auto& selectedActor = gameData->controllableActorSystem->GetSelectedActor();
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

    void PlayerAbilitySystem::AbilityOnePressed()
    {
        const auto& selectedActor = gameData->controllableActorSystem->GetSelectedActor();
        const auto& abilitySlots = registry->get<CombatableActor>(selectedActor).abilities;
        std::cout << "Ability 1 pressed \n";
        if (abilitySlots[0] == entt::null)
        {
            std::cout << "Slot not bound to an ability \n";
            return;
        }
        auto& ab = registry->get<Ability>(abilitySlots[0]);

        if (!ab.CooldownReady())
        {
            std::cout << "Waiting for cooldown timer: " << ab.GetRemainingCooldownTime() << "\n";
            return;
        }
        ab.startCast.Publish(abilitySlots[0]);
    }

    void PlayerAbilitySystem::AbilityTwoPressed()
    {
        const auto& selectedActor = gameData->controllableActorSystem->GetSelectedActor();
        const auto& abilitySlots = registry->get<CombatableActor>(selectedActor).abilities;
        std::cout << "Ability 2 pressed \n";
        if (abilitySlots[1] == entt::null)
        {
            std::cout << "Slot not bound to an ability \n";
            return;
        }
        auto& ab = registry->get<Ability>(abilitySlots[1]);

        if (!ab.CooldownReady())
        {
            std::cout << "Waiting for cooldown timer: " << ab.GetRemainingCooldownTime() << "\n";
            return;
        }
        ab.startCast.Publish(abilitySlots[1]);
    }

    void PlayerAbilitySystem::AbilityThreePressed()
    {
        const auto& selectedActor = gameData->controllableActorSystem->GetSelectedActor();
        const auto& abilitySlots = registry->get<CombatableActor>(selectedActor).abilities;
        std::cout << "Ability 3 pressed \n";
        if (abilitySlots[2] == entt::null)
        {
            std::cout << "Slot not bound to an ability \n";
            return;
        }
        auto& ab = registry->get<Ability>(abilitySlots[2]);

        if (!ab.CooldownReady())
        {
            std::cout << "Waiting for cooldown timer: " << ab.GetRemainingCooldownTime() << "\n";
            return;
        }
        ab.startCast.Publish(abilitySlots[2]);
    }

    void PlayerAbilitySystem::AbilityFourPressed()
    {
        const auto& selectedActor = gameData->controllableActorSystem->GetSelectedActor();
        const auto& abilitySlots = registry->get<CombatableActor>(selectedActor).abilities;
        std::cout << "Ability 4 pressed \n";

        if (abilitySlots[3] == entt::null)
        {
            std::cout << "Slot not bound to an ability \n";
            return;
        }

        auto& ab = registry->get<Ability>(abilitySlots[3]);

        if (!ab.CooldownReady())
        {
            std::cout << "Waiting for cooldown timer: " << ab.GetRemainingCooldownTime() << "\n";
            return;
        }
        ab.startCast.Publish(abilitySlots[3]);
    }

    Ability* PlayerAbilitySystem::GetAbility(unsigned int slotNumber) const
    {
        assert(slotNumber < MAX_ABILITY_NUMBER);
        const auto& selectedActor = gameData->controllableActorSystem->GetSelectedActor();
        const auto& abilitySlots = registry->get<CombatableActor>(selectedActor).abilities;
        if (abilitySlots.at(slotNumber) == entt::null) return nullptr;
        return &registry->get<Ability>(abilitySlots.at(slotNumber));
    }

    void PlayerAbilitySystem::SwapAbility(unsigned int slot1, unsigned int slot2)
    {
        const auto& selectedActor = gameData->controllableActorSystem->GetSelectedActor();
        const auto& abilitySlots = registry->get<CombatableActor>(selectedActor).abilities;
        auto ability1 = abilitySlots.at(slot1);
        auto ability2 = abilitySlots.at(slot2);
        SetSlot(slot1, ability2);
        SetSlot(slot2, ability1);
    }

    void PlayerAbilitySystem::SetSlot(unsigned int slot, entt::entity abilityEntity) const
    {
        assert(slot < MAX_ABILITY_NUMBER);
        const auto& selectedActor = gameData->controllableActorSystem->GetSelectedActor();
        auto& abilitySlots = registry->get<CombatableActor>(selectedActor).abilities;
        abilitySlots[slot] = abilityEntity;
    }

    void PlayerAbilitySystem::Update()
    {
    }

    void PlayerAbilitySystem::Draw2D()
    {
    }

    void PlayerAbilitySystem::Draw3D()
    {
    }

    PlayerAbilitySystem::PlayerAbilitySystem(entt::registry* _registry, GameData* _gameData)
        : registry(_registry), gameData(_gameData)
    {
        gameData->userInput->keyOnePressed.Subscribe([this]() { AbilityOnePressed(); });
        gameData->userInput->keyTwoPressed.Subscribe([this]() { AbilityTwoPressed(); });
        gameData->userInput->keyThreePressed.Subscribe([this]() { AbilityThreePressed(); });
        gameData->userInput->keyFourPressed.Subscribe([this]() { AbilityFourPressed(); });
    }
} // namespace sage