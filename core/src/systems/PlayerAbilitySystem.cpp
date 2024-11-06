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
        const auto& abilitySlots = registry->get<CombatableActor>(controlledActor).abilities;
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
        ab.startCast.publish(abilitySlots[slotNumber]);
    }

    void PlayerAbilitySystem::AbilityOnePressed()
    {
        const auto& abilitySlots = registry->get<CombatableActor>(controlledActor).abilities;
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
        ab.startCast.publish(abilitySlots[0]);
    }

    void PlayerAbilitySystem::AbilityTwoPressed()
    {
        const auto& abilitySlots = registry->get<CombatableActor>(controlledActor).abilities;
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
        ab.startCast.publish(abilitySlots[1]);
    }

    void PlayerAbilitySystem::AbilityThreePressed()
    {
        const auto& abilitySlots = registry->get<CombatableActor>(controlledActor).abilities;
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
        ab.startCast.publish(abilitySlots[2]);
    }

    void PlayerAbilitySystem::AbilityFourPressed()
    {
        const auto& abilitySlots = registry->get<CombatableActor>(controlledActor).abilities;
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
        ab.startCast.publish(abilitySlots[3]);
    }

    void PlayerAbilitySystem::onActorChanged()
    {
        controlledActor = gameData->controllableActorSystem->GetSelectedActor();
    }

    Ability* PlayerAbilitySystem::GetAbility(unsigned int slotNumber) const
    {
        assert(slotNumber < MAX_ABILITY_NUMBER);
        const auto& abilitySlots = registry->get<CombatableActor>(controlledActor).abilities;
        if (abilitySlots.at(slotNumber) == entt::null) return nullptr;
        return &registry->get<Ability>(abilitySlots.at(slotNumber));
    }

    void PlayerAbilitySystem::SwapAbility(unsigned int slot1, unsigned int slot2)
    {
        const auto& abilitySlots = registry->get<CombatableActor>(controlledActor).abilities;
        auto ability1 = abilitySlots.at(slot1);
        auto ability2 = abilitySlots.at(slot2);
        SetSlot(slot1, ability2);
        SetSlot(slot2, ability1);
    }

    void PlayerAbilitySystem::SetSlot(unsigned int slot, entt::entity abilityEntity) const
    {
        assert(slot < MAX_ABILITY_NUMBER);
        auto& abilitySlots = registry->get<CombatableActor>(controlledActor).abilities;
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
        onActorChanged();
        {
            entt::sink sink{gameData->userInput->keyOnePressed};
            sink.connect<&PlayerAbilitySystem::AbilityOnePressed>(this);
        }
        {
            entt::sink sink{gameData->userInput->keyTwoPressed};
            sink.connect<&PlayerAbilitySystem::AbilityTwoPressed>(this);
        }
        {
            entt::sink sink{gameData->userInput->keyThreePressed};
            sink.connect<&PlayerAbilitySystem::AbilityThreePressed>(this);
        }
        {
            entt::sink sink{gameData->userInput->keyFourPressed};
            sink.connect<&PlayerAbilitySystem::AbilityFourPressed>(this);
        }

        {
            entt::sink sink{gameData->controllableActorSystem->onControlledActorChange};
            sink.connect<&PlayerAbilitySystem::onActorChanged>(this);
        }
        onActorChanged();
    }
} // namespace sage