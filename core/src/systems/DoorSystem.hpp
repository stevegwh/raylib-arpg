//
// Created by steve on 30/12/2024.
//

#pragma once

#include <entt/entt.hpp>

namespace sage
{
    class GameData;
    class DoorSystem
    {
        entt::registry* registry;
        GameData* gameData;

      public:
        void UnlockDoor(entt::entity entity);
        void UnlockAndOpenDoor(entt::entity entity);
        void OpenClickedDoor(entt::entity entity);

        DoorSystem(entt::registry* _registry, GameData* _gameData);
    };

} // namespace sage
