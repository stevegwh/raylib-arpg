#include "PlayerAbilitySystem.hpp"

#include "AbilitySystem.hpp"

#include "abilities/Abilities.hpp"
#include "abilities/Ability.hpp"
#include "GameData.hpp"
#include "systems/ControllableActorSystem.hpp"
#include "UserInput.hpp"

#include <cassert>

namespace sage
{
    void PlayerAbilitySystem::abilityOnePressed()
    {
        // if (currentAbilities.empty())
        // {
        //     currentAbilities =
        //         gameData->abilitySystem->GetAbilities(gameData->controllableActorSystem->GetControlledActor());
        // }

        std::cout << "Ability 1 pressed \n";
        if (abilitySlots[0] == -1 || !currentAbilities[abilitySlots[0]]->CooldownReady())
        {
            std::cout << "Waiting for cooldown timer: "
                      << currentAbilities[abilitySlots[0]]->GetRemainingCooldownTime() << "\n";
            return;
        }
        currentAbilities[abilitySlots[0]]->Init(gameData->controllableActorSystem->GetControlledActor());
        // TODO: Above does not work with "controlledactor" member, only with
        // GetControlledActor()
    }

    void PlayerAbilitySystem::abilityTwoPressed()
    {
        std::cout << "Ability 2 pressed \n";
        if (abilitySlots[1] == -1 || !currentAbilities[abilitySlots[1]]->CooldownReady())
        {
            std::cout << "Waiting for cooldown timer: "
                      << currentAbilities[abilitySlots[1]]->GetRemainingCooldownTime() << "\n";
            return;
        }
        currentAbilities[abilitySlots[1]]->Init(gameData->controllableActorSystem->GetControlledActor());
    }

    void PlayerAbilitySystem::abilityThreePressed()
    {
        std::cout << "Ability 3 pressed \n";
        if (abilitySlots[2] == -1 || !currentAbilities[abilitySlots[2]]->CooldownReady())
        {
            std::cout << "Waiting for cooldown timer: "
                      << currentAbilities[abilitySlots[2]]->GetRemainingCooldownTime() << "\n";
            return;
        }
        currentAbilities[abilitySlots[2]]->Init(controlledActor);
    }

    void PlayerAbilitySystem::abilityFourPressed()
    {
        std::cout << "Ability 4 pressed \n";
        if (abilitySlots[3] == -1 || !currentAbilities[abilitySlots[3]]->CooldownReady())
        {
            std::cout << "Waiting for cooldown timer: "
                      << currentAbilities[abilitySlots[3]]->GetRemainingCooldownTime() << "\n";
            return;
        }
        currentAbilities[abilitySlots[3]]->Init(controlledActor);
    }

    void PlayerAbilitySystem::RefreshAbilities()
    {
        currentAbilities =
            gameData->abilitySystem->GetAbilities(gameData->controllableActorSystem->GetControlledActor());
    }

    void PlayerAbilitySystem::onActorChanged()
    {
        controlledActor = gameData->controllableActorSystem->GetControlledActor();
        RefreshAbilities();
        // TODO: Change abilities based on the new actor
    }

    void PlayerAbilitySystem::ChangeAbility(int abilitySlot, int newAbilityIndex)
    {
        // More than likely should subscribe to an "ability end" event which then triggers
        // the change
        abilitySlots[abilitySlot] = newAbilityIndex;
    }

    void PlayerAbilitySystem::Update()
    {
        for (auto& ability : currentAbilities)
        {
            ability->Update(gameData->controllableActorSystem->GetControlledActor());
        }
    }

    void PlayerAbilitySystem::Draw2D()
    {
        // Draw GUI here (cooldowns etc)
    }

    void PlayerAbilitySystem::Draw3D()
    {
        for (auto& ability : currentAbilities)
        {
            ability->Draw3D(gameData->controllableActorSystem->GetControlledActor());
        }
    }

    PlayerAbilitySystem::PlayerAbilitySystem(entt::registry* _registry, GameData* _gameData)
        : registry(_registry), gameData(_gameData)
    {
        onActorChanged();
        {
            entt::sink sink{gameData->userInput->keyOnePressed};
            sink.connect<&PlayerAbilitySystem::abilityOnePressed>(this);
        }
        {
            entt::sink sink{gameData->userInput->keyTwoPressed};
            sink.connect<&PlayerAbilitySystem::abilityTwoPressed>(this);
        }
        {
            entt::sink sink{gameData->userInput->keyThreePressed};
            sink.connect<&PlayerAbilitySystem::abilityThreePressed>(this);
        }
        {
            entt::sink sink{gameData->userInput->keyFourPressed};
            sink.connect<&PlayerAbilitySystem::abilityFourPressed>(this);
        }

        {
            entt::sink sink{gameData->controllableActorSystem->onControlledActorChange};
            sink.connect<&PlayerAbilitySystem::onActorChanged>(this);
        }

        abilitySlots.fill(-1);

        // TODO: This should not be the main ability system.
        // Owner of abilities should be a different class. This should be just for player control of abilities
        // The AbilityResourceManager could initialise the abilities and manage them (but its static)

        // abilityMap.push_back(std::make_unique<WhirlwindAbility>(registry, gameData));
        // abilityMap.push_back(std::make_unique<FloorFire>(registry, gameData));
        // abilityMap.push_back(std::make_unique<RainOfFire>(registry, gameData));

        // TODO: Can get all abilities easily, but how to filter the auto attack? Do I just do it by name? Seems
        // fragile and "hard coded"

        assert(_gameData->controllableActorSystem->GetControlledActor() != entt::null);

        // TODO: Abilities is empty here
        // assert(!currentAbilities.empty());

        // TODO: These should be set by the player or another system
        ChangeAbility(0, 0);
        ChangeAbility(1, 1);
        ChangeAbility(2, 2);
    }
} // namespace sage