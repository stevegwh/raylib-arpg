//
// Created by steve on 30/12/2024.
//

#pragma once

#include "entt/entt.hpp"

namespace sage
{
    class BaseSystems;
    class DoorSystem
    {
        entt::registry* registry;
        BaseSystems* sys;

      public:
        void UnlockDoor(entt::entity entity) const;
        void UnlockAndOpenDoor(entt::entity entity);
        void OpenClickedDoor(entt::entity entity) const;

        DoorSystem(entt::registry* _registry, BaseSystems* _sys);
    };

} // namespace sage
