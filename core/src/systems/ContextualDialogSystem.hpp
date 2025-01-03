//
// Created by Steve Wheeler on 03/01/2025.
//

#pragma once

#include <entt/entt.hpp>

namespace sage
{
    class GameData;

    class ContextualDialogSystem
    {
        entt::registry* registry;
        GameData* gameData;

      public:
        void Update() const;
        void Draw2D() const;

        ContextualDialogSystem(entt::registry* _registry, GameData* _gameData);
    };

} // namespace sage
