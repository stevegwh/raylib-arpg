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
        std::cout << "Ability 1 pressed \n";

        if (abilitySlots[0].second == nullptr)
        {
            abilitySlots[0].second = gameData->abilitySystem->GetAbility(
                gameData->controllableActorSystem->GetControlledActor(), abilitySlots[0].first);
        }

        if (!abilitySlots[0].second->CooldownReady())
        {
            std::cout << "Waiting for cooldown timer: " << abilitySlots[0].second->GetRemainingCooldownTime()
                      << "\n";
            return;
        }
        abilitySlots[0].second->Init();
        // TODO: Above does not work with "controlledactor" member, only with
        // GetControlledActor()
    }
    void PlayerAbilitySystem::abilityTwoPressed()
    {
        std::cout << "Ability 2 pressed \n";

        if (abilitySlots[1].second == nullptr)
        {
            abilitySlots[1].second = gameData->abilitySystem->GetAbility(
                gameData->controllableActorSystem->GetControlledActor(), abilitySlots[1].first);
        }

        if (!abilitySlots[1].second->CooldownReady())
        {
            std::cout << "Waiting for cooldown timer: " << abilitySlots[1].second->GetRemainingCooldownTime()
                      << "\n";
            return;
        }
        abilitySlots[1].second->Init();
    }
    void PlayerAbilitySystem::abilityThreePressed()
    {
        std::cout << "Ability 3 pressed \n";

        if (abilitySlots[2].second == nullptr)
        {
            abilitySlots[2].second = gameData->abilitySystem->GetAbility(
                gameData->controllableActorSystem->GetControlledActor(), abilitySlots[2].first);
        }

        if (!abilitySlots[2].second->CooldownReady())
        {
            std::cout << "Waiting for cooldown timer: " << abilitySlots[2].second->GetRemainingCooldownTime()
                      << "\n";
            return;
        }
        abilitySlots[2].second->Init();
    }

    void PlayerAbilitySystem::abilityFourPressed()
    {
        std::cout << "Ability 4 pressed \n";

        if (abilitySlots[3].second == nullptr)
        {
            abilitySlots[3].second = gameData->abilitySystem->GetAbility(
                gameData->controllableActorSystem->GetControlledActor(), abilitySlots[3].first);
        }

        if (!abilitySlots[3].second->CooldownReady())
        {
            std::cout << "Waiting for cooldown timer: " << abilitySlots[3].second->GetRemainingCooldownTime()
                      << "\n";
            return;
        }
        abilitySlots[3].second->Init();
    }

    void PlayerAbilitySystem::onActorChanged()
    {
        controlledActor = gameData->controllableActorSystem->GetControlledActor();
        // TODO: Change abilities based on the new actor
    }

    void PlayerAbilitySystem::SetSlot(int slot, AbilityEnum abilityEnum)
    {
        assert(slot < 4);
        abilitySlots[slot] = std::make_pair(
            abilityEnum,
            gameData->abilitySystem->GetAbility(
                gameData->controllableActorSystem->GetControlledActor(), abilityEnum));
    }

    void PlayerAbilitySystem::Update()
    {
    }

    void PlayerAbilitySystem::Draw2D()
    {
        // Draw GUI here (cooldowns etc)
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
    }
} // namespace sage