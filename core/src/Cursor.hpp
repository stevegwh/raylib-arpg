//
// Created by Steve Wheeler on 04/05/2024.
//

#pragma once

#include "Camera.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/NavigationGridSystem.hpp"

#include <entt/entt.hpp>

namespace sage
{
class Cursor
{
    entt::registry* registry;
    Ray ray {0};
    Color defaultColor = WHITE;
    Color hoverColor = LIME;
    Color invalidColor = RED;
    Color currentColor = WHITE;
    
    sage::CollisionSystem* collisionSystem;
    sage::NavigationGridSystem* navigationGridSystem;
    sage::Camera* sCamera;
    
    void getMouseRayCollision();
public:
    std::string hitObjectName{};
    RayCollision collision {0};
    CollisionInfo rayCollisionResultInfo;
    entt::delegate<void()> dOnCollisionHitEvent{};
    
    void Update();
    void Draw();
    Cursor(entt::registry* registry,
           sage::CollisionSystem* _collisionSystem,
           sage::NavigationGridSystem* _navigationGridSystem,
           sage::Camera* _sCamera);
};
}

