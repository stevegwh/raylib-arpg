//
// Created by Steve Wheeler on 29/12/2024.
//

#pragma once

#include <entt/entt.hpp>

namespace sage
{
    class Cursor;
    class GameData;

    class DialogTriggerOnClickComponent
    {
        entt::entity self;
        entt::connection connection;
        entt::registry* registry{}; // Initialised by InteractableSystem
        GameData* gameData{};       // Initialised by InteractableSystem

      public:
        void ExecuteBehavior(entt::entity clicked) const;
        ~DialogTriggerOnClickComponent();
        DialogTriggerOnClickComponent(entt::entity _self, Cursor* _cursor);

        friend class InteractableSystem;
    };

} // namespace sage
