//
// Created by Steve Wheeler on 21/03/2024.
//

#pragma once

#include "raylib.h"
#include <entt/entt.hpp>

namespace sage
{
typedef struct Scene Scene;
struct GameObjectFactory
{
    static entt::entity createPlayer(entt::registry* registry, Vector3 position, const char* name);
    static void createTower(entt::registry* registry, Vector3 position, const char* name);
    static void loadBlenderLevel(entt::registry* registry, Scene* scene);
};

} // sage
