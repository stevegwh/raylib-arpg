//
// Created by Steve Wheeler on 21/03/2024.
//

#pragma once

#include "raylib.h"
#include "entt/entt.hpp"
#include "GameData.hpp"

namespace sage
{
typedef struct Scene Scene;
struct GameObjectFactory
{
    static entt::entity createPlayer(entt::registry* registry, GameData* data, Vector3 position, const char* name);
    static void createBuilding(entt::registry* registry, GameData* data, Vector3 position, const char* name,
                               const char* modelPath, const char* texturePath);
    static void loadBlenderLevel(entt::registry* registry, Scene* scene);
    static void createFloor(entt::registry* registry, Scene* scene, BoundingBox bb);
};

} // sage
