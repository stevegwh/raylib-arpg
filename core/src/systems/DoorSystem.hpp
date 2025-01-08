//
// Created by steve on 30/12/2024.
//

#pragma once

#include "entt/entt.hpp"

namespace sage
{
    class Systems;
    class DoorSystem
    {
        entt::registry* registry;
        Systems* sys;

      public:
        void UnlockDoor(entt::entity entity);
        void UnlockAndOpenDoor(entt::entity entity);
        void OpenClickedDoor(entt::entity entity);

        DoorSystem(entt::registry* _registry, Systems* _sys);
    };

} // namespace sage
