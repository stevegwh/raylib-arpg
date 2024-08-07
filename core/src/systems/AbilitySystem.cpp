#include "AbilitySystem.hpp"

#include "GameData.hpp"

#include "abilities/ConeOfCold.hpp"
#include "abilities/RainOfFireAbility.hpp"
#include "abilities/WhirlwindAbility.hpp"

namespace sage
{
    void AbilitySystem::abilityOnePressed()
    {
        std::cout << "Ability 1 pressed \n";
        if (currentAbilities[0] == -1 ||
            !abilityMap[currentAbilities[0]]->CooldownReady())
        {
            std::cout << "Waiting for cooldown timer: "
                      << abilityMap[currentAbilities[0]]->GetRemainingCooldownTime()
                      << "\n";
            return;
        }
        abilityMap[currentAbilities[0]]->Init(
            gameData->controllableActorSystem->GetControlledActor());
        // TODO: Above does not work with "controlledactor" member, only with
        // GetControlledActor()
    }

    void AbilitySystem::abilityTwoPressed()
    {
        std::cout << "Ability 2 pressed \n";
        if (currentAbilities[1] == -1 ||
            !abilityMap[currentAbilities[1]]->CooldownReady())
        {
            return;
        }
        abilityMap[currentAbilities[1]]->Init(controlledActor);
    }

    void AbilitySystem::abilityThreePressed()
    {
        std::cout << "Ability 3 pressed \n";
        if (currentAbilities[2] == -1 ||
            !abilityMap[currentAbilities[2]]->CooldownReady())
        {
            return;
        }
        abilityMap[currentAbilities[2]]->Init(controlledActor);
    }

    void AbilitySystem::abilityFourPressed()
    {
        std::cout << "Ability 4 pressed \n";
        if (currentAbilities[3] == -1 ||
            !abilityMap[currentAbilities[3]]->CooldownReady())
        {
            return;
        }
        abilityMap[currentAbilities[3]]->Init(controlledActor);
    }

    void AbilitySystem::onActorChanged()
    {
        controlledActor = gameData->controllableActorSystem->GetControlledActor();
        // TODO: Change abilities based on the new actor
    }

    void AbilitySystem::ChangeAbility(int abilitySlot, int newAbilityIndex)
    {
        // More than likely should subscribe to an "ability end" event which then triggers
        // the change
        currentAbilities[abilitySlot] = newAbilityIndex;
    }

    void AbilitySystem::Update()
    {
        for (auto& ability : abilityMap)
        {
            ability->Update(gameData->controllableActorSystem->GetControlledActor());
        }
    }

    void AbilitySystem::Draw2D()
    {
        // Draw GUI here (cooldowns etc)
    }

    void AbilitySystem::Draw3D()
    {
        for (auto& ability : abilityMap)
        {
            ability->Draw3D(gameData->controllableActorSystem->GetControlledActor());
        }
    }

    AbilitySystem::AbilitySystem(entt::registry* _registry, GameData* _gameData)
        : registry(_registry), gameData(_gameData)
    {
        onActorChanged();
        {
            entt::sink sink{gameData->userInput->keyOnePressed};
            sink.connect<&AbilitySystem::abilityOnePressed>(this);
        }
        {
            entt::sink sink{gameData->userInput->keyTwoPressed};
            sink.connect<&AbilitySystem::abilityTwoPressed>(this);
        }
        {
            entt::sink sink{gameData->userInput->keyThreePressed};
            sink.connect<&AbilitySystem::abilityThreePressed>(this);
        }
        {
            entt::sink sink{gameData->userInput->keyFourPressed};
            sink.connect<&AbilitySystem::abilityFourPressed>(this);
        }

        {
            entt::sink sink{gameData->controllableActorSystem->onControlledActorChange};
            sink.connect<&AbilitySystem::onActorChanged>(this);
        }

        currentAbilities.fill(-1);
        abilityMap.push_back(std::make_unique<WhirlwindAbility>(registry));
        abilityMap.push_back(std::make_unique<ConeOfCold>(registry));
        abilityMap.push_back(std::make_unique<RainOfFireAbility>(
            registry,
            gameData->camera.get(),
            gameData->cursor.get(),
            gameData->navigationGridSystem.get()));
        // TODO: These should be set by the player or another system
        ChangeAbility(0, 0);
        ChangeAbility(1, 1);
        ChangeAbility(2, 2);
    }
} // namespace sage