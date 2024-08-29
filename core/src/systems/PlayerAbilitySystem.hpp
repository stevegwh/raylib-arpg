#pragma once

#include <entt/entt.hpp>

#include <array>
#include <memory>
#include <utility>
#include <vector>

namespace sage
{
    class GameData;
    class AbilityStateMachine;
    enum class AbilityEnum;
    class PlayerAbilitySystem
    {
        entt::registry* registry;
        entt::entity controlledActor;
        GameData* gameData;

        std::array<std::pair<AbilityEnum, AbilityStateMachine*>, 4> abilitySlots{};
        void abilityOnePressed();
        void abilityTwoPressed();
        void abilityThreePressed();
        void abilityFourPressed();
        void onActorChanged();

      public:
        void SetSlot(int slot, AbilityEnum abilityEnum);
        void Update();
        void Draw2D();
        void Draw3D();
        PlayerAbilitySystem(entt::registry* _registry, GameData* _gameData);
    };
} // namespace sage