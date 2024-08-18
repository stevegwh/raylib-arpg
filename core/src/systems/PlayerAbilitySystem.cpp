#include "PlayerAbilitySystem.hpp"

#include "GameData.hpp"

#include "abilities/Abilities.hpp"

namespace sage
{
    void PlayerAbilitySystem::abilityOnePressed()
    {
        std::cout << "Ability 1 pressed \n";
        if (currentAbilities[0] == -1 || !abilityMap[currentAbilities[0]]->CooldownReady())
        {
            std::cout << "Waiting for cooldown timer: "
                      << abilityMap[currentAbilities[0]]->GetRemainingCooldownTime() << "\n";
            return;
        }
        abilityMap[currentAbilities[0]]->Init(gameData->controllableActorSystem->GetControlledActor());
        // TODO: Above does not work with "controlledactor" member, only with
        // GetControlledActor()
    }

    void PlayerAbilitySystem::abilityTwoPressed()
    {
        std::cout << "Ability 2 pressed \n";
        if (currentAbilities[1] == -1 || !abilityMap[currentAbilities[1]]->CooldownReady())
        {
            std::cout << "Waiting for cooldown timer: "
                      << abilityMap[currentAbilities[1]]->GetRemainingCooldownTime() << "\n";
            return;
        }
        abilityMap[currentAbilities[1]]->Init(gameData->controllableActorSystem->GetControlledActor());
    }

    void PlayerAbilitySystem::abilityThreePressed()
    {
        std::cout << "Ability 3 pressed \n";
        if (currentAbilities[2] == -1 || !abilityMap[currentAbilities[2]]->CooldownReady())
        {
            std::cout << "Waiting for cooldown timer: "
                      << abilityMap[currentAbilities[2]]->GetRemainingCooldownTime() << "\n";
            return;
        }
        abilityMap[currentAbilities[2]]->Init(controlledActor);
    }

    void PlayerAbilitySystem::abilityFourPressed()
    {
        std::cout << "Ability 4 pressed \n";
        if (currentAbilities[3] == -1 || !abilityMap[currentAbilities[3]]->CooldownReady())
        {
            std::cout << "Waiting for cooldown timer: "
                      << abilityMap[currentAbilities[3]]->GetRemainingCooldownTime() << "\n";
            return;
        }
        abilityMap[currentAbilities[3]]->Init(controlledActor);
    }

    void PlayerAbilitySystem::onActorChanged()
    {
        controlledActor = gameData->controllableActorSystem->GetControlledActor();
        // TODO: Change abilities based on the new actor
    }

    void PlayerAbilitySystem::ChangeAbility(int abilitySlot, int newAbilityIndex)
    {
        // More than likely should subscribe to an "ability end" event which then triggers
        // the change
        currentAbilities[abilitySlot] = newAbilityIndex;
    }

    void PlayerAbilitySystem::Update()
    {
        for (auto& ability : abilityMap)
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
        for (auto& ability : abilityMap)
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

        currentAbilities.fill(-1);
        abilityMap.push_back(std::make_unique<WhirlwindAbility>(registry, gameData));
        abilityMap.push_back(std::make_unique<FloorFire>(registry, gameData));
        abilityMap.push_back(std::make_unique<RainOfFire>(registry, gameData));
        // TODO: These should be set by the player or another system
        ChangeAbility(0, 0);
        ChangeAbility(1, 1);
        ChangeAbility(2, 2);
    }
} // namespace sage