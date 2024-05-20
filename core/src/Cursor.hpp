//
// Created by Steve Wheeler on 04/05/2024.
//

#pragma once

#include "systems/CollisionSystem.hpp"
#include "systems/NavigationGridSystem.hpp"
#include "Camera.hpp"

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

    bool lockContext = false;

    entt::entity controlledActor;
    
    sage::CollisionSystem* collisionSystem;
    sage::NavigationGridSystem* navigationGridSystem;
    sage::Camera* sCamera;
    sage::UserInput* userInput;
    
    void getMouseRayCollision();
    void onMouseClick();
    void changeCursors(CollisionLayer collisionLayer);
public:
    std::string hitObjectName{};
    RayCollision collision {};
    CollisionInfo rayCollisionResultInfo;
    entt::sigh<void(entt::entity)> onCollisionHit{}; // Returns the hit entity (all layers)
    entt::sigh<void(entt::entity)> onNPCClick{};
    entt::sigh<void(entt::entity)> onFloorClick{};
    entt::sigh<void(entt::entity)> onAnyClick{};

    Cursor(entt::registry* registry,
           sage::CollisionSystem* _collisionSystem,
           sage::NavigationGridSystem* _navigationGridSystem,
           sage::Camera* _sCamera,
           sage::UserInput* _userInput);
    
    void Update();
    void Draw3D();
    void Draw2D();
    void OnControlledActorChange(entt::entity entity);
    void LockCursor();
    void UnlockCursor();
    bool isValidMove();
};
}

