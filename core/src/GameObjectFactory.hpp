//
// Created by Steve Wheeler on 21/03/2024.
//

#pragma once

#include "entt/entt.hpp"
#include "raylib.h"

namespace sage
{
    class Scene;
    class GameData;

    struct GameObjectFactory
    {
        static entt::entity createEnemy(
            entt::registry* registry, GameData* data, Vector3 position, const char* name);
        static entt::entity createKnight(
            entt::registry* registry, GameData* data, Vector3 position, const char* name);
        static entt::entity createPlayer(
            entt::registry* registry, GameData* data, Vector3 position, const char* name);
        static void createBuilding(
            entt::registry* registry,
            GameData* data,
            Vector3 position,
            const char* name,
            const char* modelPath,
            const char* texturePath);
        static void loadMap(entt::registry* registry, Scene* scene, float& slices, const std::string& _mapPath);
        static void createFloor(entt::registry* registry, Scene* scene, BoundingBox bb);
    };
} // namespace sage
