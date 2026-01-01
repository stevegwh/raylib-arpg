//
// Created by steve on 30/12/2024.
//

#pragma once

#include "entt/entt.hpp"

namespace sage
{
    class EngineSystems;
    class DoorSystem
    {
        entt::registry* registry;
        EngineSystems* sys;

      public:
        void UnlockDoor(entt::entity entity) const;
        void UnlockAndOpenDoor(entt::entity entity);
        void OpenClickedDoor(entt::entity entity) const;

        DoorSystem(entt::registry* _registry, EngineSystems* _sys);
    };

} // namespace sage
