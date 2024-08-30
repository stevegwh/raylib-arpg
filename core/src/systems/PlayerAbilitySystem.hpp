#pragma once

#include <entt/entt.hpp>

#include <array>
#include <memory>
#include <utility>
#include <vector>

namespace sage
{
    class GameData;

    class PlayerAbilitySystem
    {
        entt::registry* registry;
        entt::entity controlledActor;
        GameData* gameData;

        std::array<entt::entity, 4> abilitySlots{};
        void abilityOnePressed();
        void abilityTwoPressed();
        void abilityThreePressed();
        void abilityFourPressed();
        void onActorChanged();

      public:
        void SetSlot(int slot, entt::entity abilityEntity);
        void Update();
        void Draw2D();
        void Draw3D();
        PlayerAbilitySystem(entt::registry* _registry, GameData* _gameData);
    };
} // namespace sage