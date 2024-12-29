//
// Created by Steve Wheeler on 29/12/2024.
//

#pragma once

#include <entt/entt.hpp>

namespace sage
{
    class GameData;
    class Cursor;
    class sgTransform;

    class DoorBehaviorComponent
    {
        entt::entity self;
        entt::connection connection;
        sgTransform* transform{};
        float openYRotation = 5;
        bool open = false;
        bool locked = false;

      public:
        void ExecuteBehavior(entt::entity clicked);
        ~DoorBehaviorComponent();
        DoorBehaviorComponent(entt::entity _self, sgTransform* _transform);

        friend class InteractableSystem;
    };

} // namespace sage
