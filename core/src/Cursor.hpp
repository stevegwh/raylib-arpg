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
enum class CursorState {
    DEFAULT,
    NPC_HOVER,
    BUILDING_HOVER
};
class Cursor
{
    Texture2D* currentTex;
    Texture2D regulartex;
    Texture2D talktex;
    Texture2D movetex;
    Texture2D invalidmovetex;
    
    Vector2 position;
    entt::registry* registry;
    Ray ray {0};
    Color defaultColor = WHITE;
    Color hoverColor = LIME;
    Color invalidColor = RED;
    Color currentColor = WHITE;

    entt::entity controlledActor;
    
    sage::CollisionSystem* collisionSystem;
    sage::NavigationGridSystem* navigationGridSystem;
    sage::Camera* sCamera;
    
    void getMouseRayCollision();
public:
    std::string hitObjectName{};
    RayCollision collision {};
    CollisionInfo rayCollisionResultInfo;
    entt::sigh<void(entt::entity)> onCollisionHit{}; // Returns the hit entity (all layers)
    entt::sigh<void(entt::entity)> onNPCClick{};


    Cursor(entt::registry* registry,
           sage::CollisionSystem* _collisionSystem,
           sage::NavigationGridSystem* _navigationGridSystem,
           sage::Camera* _sCamera);
    
    void Update();
    void Draw3D();
    void Draw2D();
    void OnControlledActorChange(entt::entity entity);
};
}

