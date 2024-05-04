//
// Created by Steve Wheeler on 04/05/2024.
//

#pragma once

#include "Camera.hpp"
#include "systems/CollisionSystem.hpp"
#include <entt/entt.hpp>

namespace sage
{
class Cursor
{
    entt::registry* registry;
    Ray ray {0};
    std::string hitObjectName{};
    Color defaultColor = WHITE;
    Color hoverColor = LIME;
    sage::CollisionSystem* collisionSystem;
    sage::Camera* sCamera;
public:
    RayCollision collision {0};
    CollisionInfo rayCollisionResultInfo;

    bool GetMouseRayCollision();
    void Draw();
    void DrawDebugText() const;
    Cursor(entt::registry* registry,
           sage::CollisionSystem* _collisionSystem, 
           sage::Camera* _sCamera);
};
}

