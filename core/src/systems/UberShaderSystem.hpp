//
// Created by steve on 21/11/2024.
//

#pragma once

#include "entt/entt.hpp"
#include "raylib.h"

namespace sage
{

    class GameData;

    class UberShaderSystem
    {
        entt::registry* registry;
        GameData* gameData;
        Shader shader{};
        int litLoc;
        int skinnedLoc;
        void onComponentAdded(entt::entity entity);
        void onComponentRemoved(entt::entity entity);

      public:
        UberShaderSystem(entt::registry* _registry, GameData* _gameData);
    };

} // namespace sage