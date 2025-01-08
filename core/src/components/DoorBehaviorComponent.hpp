//
// Created by Steve Wheeler on 29/12/2024.
//

#pragma once

#include "entt/entt.hpp"

namespace sage
{
    class DoorBehaviorComponent
    {
        entt::connection connection;
        float openYRotation = 5;
        bool open = false;
        bool locked = true;

      public:
        ~DoorBehaviorComponent();
        DoorBehaviorComponent() = default;

        friend class DoorSystem;
    };

} // namespace sage
